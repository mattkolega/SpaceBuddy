#include "app.h"
#include "audio.h"

#include <chrono>
#include <string_view>
#include <thread>

#include <SDL3/SDL.h>

static constexpr int FPS{60};

static std::array<Sound, AUDIOSAMPLE_COUNT> sounds {{
    {3, 0, AudioSample::UfoRepeat,      PlaybackType::Loop,    false},
    {3, 1, AudioSample::Shoot,          PlaybackType::Oneshot, false},
    {3, 2, AudioSample::PlayerDeath,    PlaybackType::Oneshot, false},
    {3, 3, AudioSample::InvaderKilled,  PlaybackType::Oneshot, false},
    {5, 0, AudioSample::FleetMovement1, PlaybackType::Oneshot, false},
    {5, 1, AudioSample::FleetMovement2, PlaybackType::Oneshot, false},
    {5, 2, AudioSample::FleetMovement3, PlaybackType::Oneshot, false},
    {5, 3, AudioSample::FleetMovement4, PlaybackType::Oneshot, false},
    {5, 4, AudioSample::UfoHit,         PlaybackType::Oneshot, false}
}};

std::optional<App> App::create() {
    auto platform = Platform::create();
    if (!platform) return std::nullopt;

    auto renderer = Renderer::create((*platform).getWindow(), SpaceInvaders::FRAMEBUFFER_WIDTH, SpaceInvaders::FRAMEBUFFER_HEIGHT);
    if (!renderer) return std::nullopt;

    auto audioManager = AudioManager::create();
    if (!audioManager) return std::nullopt;

    audioManager->loadSamples();

    return App{std::move(*platform), std::move(*renderer), std::move(*audioManager)};
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

        handleAudio();

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

void App::handleAudio() {
    for (auto& sound : sounds) {
        bool currentBitState{m_spaceInvaders.getOutputBit(sound.port, sound.bit) != 0};

        if (currentBitState == sound.previousBitState) continue;

        if (sound.playbackType == PlaybackType::Loop) {
            if (currentBitState == 1) {
                m_audioManager.playLoop(sound.sample);
            } else {
                m_audioManager.stopLoop(sound.sample);
            }
        } else {
            if (currentBitState == 1) m_audioManager.playOneshot(sound.sample);
        }

        sound.previousBitState = currentBitState;
    }
}
