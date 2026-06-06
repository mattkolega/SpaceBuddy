#pragma once

#include <optional>

#include <SDL3/SDL.h>

class Platform {
public:
    static std::optional<Platform> create();

    ~Platform();

    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;

    Platform(Platform&& other) noexcept;
    Platform& operator=(Platform&& other) noexcept;

    SDL_Window* getWindow() const { return m_window; }

private:
    explicit Platform(SDL_Window* window) : m_window(window) {}

    SDL_Window* m_window {nullptr};
};
