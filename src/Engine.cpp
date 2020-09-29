#include "Engine.h"

Engine::Engine(unsigned int width, unsigned int height) :
    running(true),
    window_width(width), window_height(height),
    main_window(Window("Engine", width, height)),
    time_init(SDL_GetPerformanceCounter()),
    time_prev(0), time_curr(time_init), dt_seconds(0.0), time_total_seconds(0.0),
    player({{0,0,0}, 0}),
    current_state(MAP),
    map_zoom(32)
{
    // Read map
    std::ifstream map_file("map");
    std::string line;
    std::getline(map_file, line);
    std::istringstream n_walls_line(line);
    int n_walls;
    n_walls_line >> n_walls;
    for (int i = 0; i < n_walls; i++) {
        std::getline(map_file, line);
        std::cout << line << std::endl;
        std::istringstream lines_stream(line);
        float2 p1, p2;
        int sec_id;
        lines_stream >> p1.x >> p1.y >> p2.x >> p2.y >> sec_id;
        walls.push_back({p1, p2, sec_id});
    }
    std::getline(map_file, line);
    std::istringstream n_secs_line(line);
    int n_secs;
    n_secs_line >> n_secs;
    for (int i = 0; i < n_secs; i++) {
        std::getline(map_file, line);
        std::cout << line << std::endl;
        std::istringstream lines_stream(line);
        float floor, ceil;
        int w_begin, w_end;
        lines_stream >> floor >> ceil >> w_begin >> w_end;
        sectors.push_back({floor, ceil, w_begin, w_end});
    }
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
    // Rendering states choose to clear or not
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
    main_window.clear({255,255,255,255});
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
        for (Sector sector : sectors) { // naiive
            if (sector.containsPoint(player.pos.xy(), walls)) {
                renderSector(sector, col, camera_ray, radians);
            }
        }
    }
}

void Engine::renderSector(const Sector& sector, int col, Ray camera_ray, float radians) {
    // Find nearest intersection in sectors walls
    float min_dist = INFINITY;
    int min_dist_wall = -1;
    for (auto it = walls.begin() + sector.walls_begin; it <= walls.begin() + sector.walls_end; it++) {
        float2 point;
        if ((*it).facingFront(camera_ray) && (*it).rayIntersect(camera_ray, &point)) {
            float eucDist = sqrt((player.pos.x-point.x)*(player.pos.x-point.x) + (player.pos.y-point.y)*(player.pos.y-point.y));
            float dist = eucDist * cos(radians - player.angle);
            if (dist < min_dist) {
                min_dist = dist;
                min_dist_wall = it - walls.begin();
            }
        }
    }

    // Find top and bottom of the wall or portal
    int wall_top = window_height/2 - (window_height/min_dist * (sector.ceil  - player.pos.z)) / (FOV);
    int wall_bot = window_height/2 + (window_height/min_dist * (sector.floor + player.pos.z)) / (FOV);

    // if the wall exists
    if (min_dist_wall >= 0) {
        // if the wall does not continue to another sector
        if (walls[min_dist_wall].next_sector == -1) {
            main_window.drawLineRGBA(col, wall_top, col, wall_bot, RGBA {0, 0, (unsigned char) (255/std::max(min_dist+1.0f, 1.0f)), 255} );
        }
        // if the wall does go to another sector
        else {
            // Render next sector through portal
            if (walls[min_dist_wall].next_sector >= 0) {
                renderSector(sectors[walls[min_dist_wall].next_sector], col, camera_ray, radians);

                // Render top and bottom
                int topTop = window_height/2 - (window_height/min_dist * (sector.ceil - player.pos.z)) / (FOV);
                int botTop = window_height/2 - (window_height/min_dist * (sectors[walls[min_dist_wall].next_sector].ceil - player.pos.z)) / (FOV);
                main_window.drawLineRGBA(col, topTop, col, botTop, RGBA {255, 0, 255, 255} );

                int botBot = window_height/2 + (window_height/min_dist * (sectors[walls[min_dist_wall].next_sector].floor + player.pos.z)) / (FOV);
                int topBot = window_height/2 + (window_height/min_dist * (sector.floor + player.pos.z)) / (FOV);
                main_window.drawLineRGBA(col, topBot, col, botBot, RGBA {255, 255, 0 , 255} );
            }
        }
    }
    // Draw the ceiling and floor of the sector
    if (wall_top > 0) main_window.drawLineRGBA(col, 0, col, wall_top, RGBA{0,255,0,255});
    if (wall_bot < window_height) main_window.drawLineRGBA(col, window_height, col, wall_bot, RGBA{255,0,0,255});
}
