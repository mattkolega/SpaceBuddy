#include "window.h"

#include "common/log.h"

static constexpr int WINDOW_WIDTH  {800};
static constexpr int WINDOW_HEIGHT {600};

std::optional<Window> Window::create() {
    static constexpr auto WINDOW_FLAGS = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* handle = SDL_CreateWindow(
        "SpaceBuddy",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        WINDOW_FLAGS
    );

    if (!handle) {
        log::fatal("Failed to create handle. SDL_Error: {}", SDL_GetError());
        return std::nullopt;
    }

    return Window{handle};
}

Window::~Window() {
    if (!handle) return;
    SDL_DestroyWindow(handle);
    handle = nullptr;
}

Window::Window(Window&& other) noexcept
    : handle(other.handle) {
    other.handle = nullptr;
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        if (handle) SDL_DestroyWindow(handle);

        handle = other.handle;
        other.handle = nullptr;
    }
    return *this;
}
