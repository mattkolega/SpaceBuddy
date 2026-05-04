#include "app.h"

#include <chrono>
#include <string_view>
#include <thread>

#include <SDL3/SDL.h>

static constexpr int FPS {60};

std::optional<App> App::create() {
    auto platform = Platform::create();
    if (!platform) return std::nullopt;

    auto renderer = Renderer::create((*platform).getWindow(), SpaceInvaders::FRAMEBUFFER_WIDTH, SpaceInvaders::FRAMEBUFFER_HEIGHT);
    if (!renderer) return std::nullopt;

    return App{std::move(*platform), std::move(*renderer)};
}

bool App::run(std::string_view romPath) {
    bool success = m_spaceInvaders.loadRom(romPath);
    if (!success) return false;

    while (m_isRunning) {
        auto frameStart = std::chrono::steady_clock::now();
        static constexpr auto nsPerSec = std::chrono::nanoseconds(std::chrono::seconds(1));
        auto frameDeadline = frameStart + (nsPerSec / FPS);

        handleEvents();

        m_spaceInvaders.runFrame();

        m_renderer.drawPixelBuffer(m_spaceInvaders.getFramebuffer());

        auto frameEnd = std::chrono::steady_clock::now();
        if (frameDeadline > frameEnd) std::this_thread::sleep_for(frameDeadline - frameEnd);
    }

    return true;
}

void App::handleEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
            switch (event.key.scancode) {
            case SDL_SCANCODE_Z:
                m_spaceInvaders.handleInput(InputType::P1Fire, true);
                break;
            case SDL_SCANCODE_LEFT:
                m_spaceInvaders.handleInput(InputType::P1Left, true);
                break;
            case SDL_SCANCODE_RIGHT:
                m_spaceInvaders.handleInput(InputType::P1Right, true);
                break;
            case SDL_SCANCODE_I:
                m_spaceInvaders.handleInput(InputType::Credit, true);
                break;
            case SDL_SCANCODE_RETURN:
                m_spaceInvaders.handleInput(InputType::P1Start, true);
                break;
            default:
                break;
            }
            break;
        case SDL_EVENT_KEY_UP:
            switch (event.key.scancode) {
            case SDL_SCANCODE_Z:
                m_spaceInvaders.handleInput(InputType::P1Fire, false);
                break;
            case SDL_SCANCODE_LEFT:
                m_spaceInvaders.handleInput(InputType::P1Left, false);
                break;
            case SDL_SCANCODE_RIGHT:
                m_spaceInvaders.handleInput(InputType::P1Right, false);
                break;
            case SDL_SCANCODE_I:
                m_spaceInvaders.handleInput(InputType::Credit, false);
                break;
            case SDL_SCANCODE_RETURN:
                m_spaceInvaders.handleInput(InputType::P1Start, false);
                break;
            default:
                break;
            }
            break;
        case SDL_EVENT_QUIT:
            m_isRunning = false;
            break;
        }
	}
}