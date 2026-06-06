#include "platform.h"

#include "common/log.h"

static constexpr int WINDOW_WIDTH  {800};
static constexpr int WINDOW_HEIGHT {600};

std::optional<Platform> Platform::create() {
    if (!SDL_SetAppMetadata("SpaceBuddy", "0.1.0", "com.mattkolega.spacebuddy")) {
        log::fatal("Failed to set metadata. SDL_Error: {}", SDL_GetError());
        return std::nullopt;
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        log::fatal("Failed to initialise SDL. SDL_Error: {}", SDL_GetError());
        return std::nullopt;
    }

    const auto windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow(
        "SpaceBuddy",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        windowFlags
    );

    if (!window) {
        SDL_Quit();
        log::fatal("Failed to create window. SDL_Error: {}", SDL_GetError());
        return std::nullopt;
    }

    return Platform{window};
}

Platform::~Platform() {
    if (!m_window) return;

    SDL_DestroyWindow(m_window);
    m_window = nullptr;
    SDL_Quit();
}

Platform::Platform(Platform&& other) noexcept
    : m_window(other.m_window) {
    other.m_window = nullptr;
}

Platform& Platform::operator=(Platform&& other) noexcept {
    if (this != &other) {
        if (m_window) SDL_DestroyWindow(m_window);

        m_window = other.m_window;
        other.m_window = nullptr;
    }
    return *this;
}
