#pragma once

#include <util.h>
#include <Sector.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

const float FOV                 = 90 * 3.1415f / 180.0f;
const float PLAYER_SPEED        = 5.0f;
const float MOUSE_SENSITIVITY   = 0.001f;

using namespace linalg::aliases;

struct Player {
     float3 pos;
     float angle;
};

class Engine {
public:
    Engine(unsigned int width, unsigned int height);
    ~Engine();
    // Forbid copy and assignment
    Engine(const Engine&) = delete;
    Engine operator=(const Engine&) = delete;

    // Main loop
    void startFrame();
    void events();
    void update();
    void render();
    bool running;

private:

    // Game states/menus
    enum State {    // Once more states are implemented, use a std::map to create a finite state machine. (sounds cool)
        WORLD, MAP
    };

    // Window variables
    int window_width, window_height;
    Window main_window;

    // Time variables
    Uint64 time_init;
    Uint64 time_prev;
    Uint64 time_curr;
    double dt_seconds;
    double time_total_seconds;

    // Current game state
    Player player;
    State current_state;
    float map_zoom;

    // Map data
    std::vector<Wall> walls;
    std::vector<Sector> sectors;

    // Rendering functions
    void renderMap();       // Renders the map view
    void renderWorld();     // Renders the fps view
    // Renders each column with recursively to determine visible sectors and draw them
    void renderSector(const Sector& sector, int col, Ray camera_ray, float radians);
};
