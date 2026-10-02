#ifndef APP_H
#define APP_H

#include "Simulation.h"

#include <SDL2/SDL.h>

class App {
public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void run();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    Simulation simulation;
    bool running;
    bool paused;

    void processEvents();
    void render();
    void renderControlPanel();
    void configureStyle();
};

#endif
