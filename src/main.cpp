#include "Engine.h"

int main(int argc, char** argv) {
    //std::vector<char*> arguments;
    //arguments.assign(argv, argv + argc);
    //for (char* arg : arguments) {
    //    std::cout << arg << "\n";
    //}

    Engine engine(1200, 900);
    while(engine.running) {
        engine.startFrame();
        engine.events();
        engine.update();
        engine.render();
    }
}

