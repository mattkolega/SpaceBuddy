#pragma once

#include <SDL3/SDL.h>

#include <core/bus.h>

class Application {
public:
    ~Application();

    void init();
    void run();
private:
    Bus bus;

    SDL_Window* m_window   { nullptr };
    SDL_Renderer* m_renderer { nullptr };
    SDL_Texture* m_texture   { nullptr };

    SDL_Event m_event;

    bool m_quit { false };

    void handleEvents();
    void updateDisplay();
    void close();
};