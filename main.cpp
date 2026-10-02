#define SDL_MAIN_HANDLED

#include "App.h"

#include <exception>
#include <iostream>

int runApplication() {
    try {
        App app;
        app.run();
        return 0;
    } catch (const std::exception& exception) {
#ifdef _WIN32
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Stigmergic Pixels",
            exception.what(),
            nullptr
        );
#else
        std::cerr << "Error: " << exception.what() << '\n';
#endif
        return 1;
    }
}

#ifdef _WIN32

#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return runApplication();
}

#else

int main() {
    return runApplication();
}

#endif
