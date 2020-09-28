#include "Engine.h"
#include <iostream>

int main(int argc, char** argv) {
    std::vector<char*> arguments;
    arguments.assign(argv, argv + argc);
    for (char* arg : arguments) {
        std::cout << arg << "\n";
    }

    Engine engine(720, 480);
    while(engine.running) {
        engine.startFrame();
        engine.events();
        engine.update();
        engine.render();
    }
}

