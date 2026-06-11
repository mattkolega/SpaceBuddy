#pragma once

#include <optional>

#include <SDL3/SDL.h>

class Window {
public:
    SDL_Window* handle{nullptr};

    static std::optional<Window> create();

    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

private:
    explicit Window(SDL_Window* handle) : handle(handle) {}
};
