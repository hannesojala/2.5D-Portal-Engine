#include "Engine.h"

#define SIDE_A 1
#define SIDE_B 0

Engine::Engine(unsigned int width, unsigned int height) :
    running(true),
    window_width(width), window_height(height),
    main_window(Window("PortalCaster", width, height)),
    time_init(SDL_GetPerformanceCounter()),
    time_prev(0), time_curr(time_init), dt_seconds(0.0), time_total_seconds(0.0),
    player({{0,0,0}, 0}),
    current_state(MAP),
    map_zoom(32)
{
    walls = {
        {{ 0, 0},{ 0,10}, -1},
        {{ 0,10},{10,10}, -1},
        {{10,10},{10, 0},  1},
        {{10, 0},{ 0, 0}, -1},

        {{10, 0},{10,10},  0},
        {{10,10},{20,10}, -1},
        {{20,10},{20, 0}, -1},
        {{20, 0},{10, 0}, -1}
    };

    sectors = {
        {2.0, 2.0, 0, 3},
        {1.0, 1.0, 4, 8}
    };
}

Engine::~Engine() {
    SDL_Quit();
}

void Engine::startFrame() {
    time_prev = time_curr;
    time_curr = SDL_GetPerformanceCounter();
    dt_seconds = (double) (time_curr - time_prev) / (double) SDL_GetPerformanceFrequency();
    time_total_seconds += dt_seconds;
}

void Engine::events() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
        case SDL_QUIT :
            running = false;
            break;
        case SDL_KEYDOWN :
            switch(event.key.keysym.sym) {
            case SDLK_TAB : // if in render go to map, if not go back to render
                current_state = current_state == WORLD ? MAP : WORLD;
                break;
            case SDLK_ESCAPE :
                SDL_SetRelativeMouseMode(SDL_bool(!SDL_GetRelativeMouseMode()));
                break;
            }
            break;
        case SDL_MOUSEWHEEL :
            map_zoom += event.wheel.y;
            if (map_zoom < 0) map_zoom = 0;
            break;
        case SDL_WINDOWEVENT :
            switch(event.window.type) {
            case SDL_WINDOWEVENT_CLOSE :
                running = false;
                break;
            }
            break;
        }
    }
    // Relative mouse movement
    int mouse_dx;
    SDL_GetRelativeMouseState(&mouse_dx, NULL);
    player.angle += MOUSE_SENSITIVITY * mouse_dx;

    // Access keys with keystate[SDL_SCANCODE(key)]
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    if(keystate[SDL_SCANCODE_W])
        player.pos += PLAYER_SPEED * float(dt_seconds) * float3{cos(player.angle), sin(player.angle), 0};
    if(keystate[SDL_SCANCODE_A])
        player.pos += PLAYER_SPEED * float(dt_seconds) * float3{cos(player.angle-3.1415f/2), sin(player.angle-3.1415f/2), 0};
    if(keystate[SDL_SCANCODE_S])
        player.pos -= PLAYER_SPEED * float(dt_seconds) * float3{cos(player.angle), sin(player.angle), 0};
    if(keystate[SDL_SCANCODE_D])
        player.pos -= PLAYER_SPEED * float(dt_seconds) * float3{cos(player.angle-3.1415f/2), sin(player.angle-3.1415f/2), 0};
    if(keystate[SDL_SCANCODE_LEFT])
        player.angle -= dt_seconds * PLAYER_SPEED;
    if(keystate[SDL_SCANCODE_RIGHT])
        player.angle += dt_seconds * PLAYER_SPEED;
    if(keystate[SDL_SCANCODE_LSHIFT])
        player.pos.z -= dt_seconds * PLAYER_SPEED;
    if(keystate[SDL_SCANCODE_SPACE])
        player.pos.z += dt_seconds * PLAYER_SPEED;

}

void Engine::update() {

}

void Engine::render() {
    main_window.clear({255,255,255,255});

    switch (current_state) {
    case WORLD :
        renderWorld();
        break;
    case MAP :
        renderMap();
        break;
    };

    main_window.render();
}

void Engine::renderMap() {
    main_window.setColor({0,0,0,255});
    for (Sector sector : sectors) {
        for (auto it = walls.begin() + sector.walls_begin; it <= walls.begin() + sector.walls_end; it++) {
            int2 p1 = int2(map_zoom * ((*it).p1 - player.pos.xy())) + int2{window_width/2, window_height/2};
            int2 p2 = int2(map_zoom * ((*it).p2 - player.pos.xy())) + int2{window_width/2, window_height/2};
            main_window.drawLine(p1.x , p1.y, p2.x, p2.y);
        }
    }
    int2 arrow{int(cos(player.angle)*map_zoom + window_width/2), int(sin(player.angle)*map_zoom + window_height/2)};
    main_window.drawLine(window_width/2, window_height/2, arrow.x, arrow.y);
}

void Engine::renderWorld() {
    for (int col = 0; col < window_width; col++) {
        float radians = player.angle + atan(window_width/window_height * FOV * float(col-window_width/2) / float(window_width/2));
        Ray camera_ray({player.pos.xy(), {cos(radians), sin(radians)} });
        for (Sector sector : sectors) { // replace with better way :)
            if (sector.containsPoint(player.pos.xy(), walls)) {
                renderSector(sector, col, camera_ray, radians);
            }
        }
    }
}

void Engine::renderSector(const Sector& sector, int col, Ray camera_ray, float radians) {
    float nearest_dist = INFINITY;
    int nearest_wall = -1;
    for (auto it = walls.begin() + sector.walls_begin; it <= walls.begin() + sector.walls_end; it++) {
        Wall wall = *it;
        float2 point;
        if (wall.facingFront(camera_ray) && wall.rayIntersect(camera_ray, &point)) {
            float eucDist = sqrt((player.pos.x-point.x)*(player.pos.x-point.x) + (player.pos.y-point.y)*(player.pos.y-point.y));
            float dist = eucDist * cos(radians - player.angle);
            if (dist < nearest_dist) {
                nearest_dist = dist;
                nearest_wall = it - walls.begin();
            }
        }
    }
    if (walls[nearest_wall].next_sector == -1) {
        renderSolid(sector, col, nearest_dist);
    }
    else {
        renderPortal(nearest_wall, sector, col, nearest_dist, camera_ray, radians);
    }
}

void Engine::renderSolid(const Sector& sector, int col, float dist) {
    int top = window_height/2 - (window_height/dist * (sector.ceil - player.pos.z)) / (FOV); // Divide by FOV for vertical adjustment
    int bot = window_height/2 + (window_height/dist * (sector.floor + player.pos.z)) / (FOV); // change to add units
    main_window.drawLineRGBA(col, top, col, bot,
                    RGBA {0, 0, (unsigned char) (255/std::max(dist+1.0f, 1.0f)), 255} );
    //ceiling above wall
    if (top > 0) main_window.drawLineRGBA(col, 0, col, top, RGBA{0,255,0,255});
    //floor below wall
    if (bot < window_height) main_window.drawLineRGBA(col, window_height, col, bot, RGBA{255,0,0,255});
}

void Engine::renderPortal(int wall_id, const Sector& sector, int col, float dist, Ray camera_ray, float radians) {
    int top = window_height/2 - (window_height/dist * (sector.ceil - player.pos.z)) / (FOV); // Divide by FOV for vertical adjustment
    int bot = window_height/2 + (window_height/dist * (sector.floor + player.pos.z)) / (FOV); // change to add units

    if (wall_id >= 0) {
        // Render next sector
        Wall portal = walls[wall_id];
        if (portal.next_sector >= 0) {
            Sector next = sectors[portal.next_sector];
            renderSector(next, col, camera_ray, radians);
            // Render top and bottom

            int topTop = window_height/2 - (window_height/dist * (sector.ceil - player.pos.z)) / (FOV); // Divide by FOV for vertical adjustment
            int botTop = window_height/2 - (window_height/dist * (next.ceil - player.pos.z)) / (FOV);
            main_window.drawLineRGBA(col, topTop, col, botTop,
                                 RGBA {255, 0, 255, 255} );
            int botBot = window_height/2 + (window_height/dist * (next.floor + player.pos.z)) / (FOV); // Divide by FOV for vertical adjustment
            int topBot = window_height/2 + (window_height/dist * (sector.floor + player.pos.z)) / (FOV);
            main_window.drawLineRGBA(col, topBot, col, botBot,
                                 RGBA {255, 255, 0 , 255} );
        }
    }

    //ceiling above wall
    if (top > 0) main_window.drawLineRGBA(col, 0, col, top, RGBA{0,255,0,255});
    //floor below wall
    if (bot < window_height) main_window.drawLineRGBA(col, window_height, col, bot, RGBA{255,0,0,255});
}
