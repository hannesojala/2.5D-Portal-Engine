#pragma once

#include <util.h>
#include <Sector.h>
#include <vector>
#include <algorithm>

const float FOV = 90 * 3.1415f / 180.0f;
const float PLAYER_SPEED = 5.0f;
const float MOUSE_SENSITIVITY = 0.001f;

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

    void startFrame();

    void events();
    void update();
    void render();

    bool running;

private:

    // Once more states are implemented, use a map to create a finite state machine.
    enum State {
        WORLD, MAP
    };

    int window_width, window_height;

    Window main_window;

    Uint64 time_init;
    Uint64 time_prev;
    Uint64 time_curr;
    double dt_seconds;
    double time_total_seconds;

    int2 relative_mouse_motion;

    Player player;

    State current_state;
    float map_zoom;

    std::vector<Wall> walls;
    std::vector<Sector> sectors;

    void renderMap();
    void renderWorld();
    void renderSector(const Sector& sector, int col, Ray camera_ray, float radians);
    void renderSolid(const Sector& sector, int col, float dist);
    void renderPortal(int wall_id, const Sector& sector, int col, float dist, Ray camera_ray, float radians);
};
