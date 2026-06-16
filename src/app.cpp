#include "app.h"

#include <cerrno>
#include <chrono>
#include <limits>
#include <string_view>
#include <thread>

#include <SDL3/SDL.h>

#include "common/log.h"
#include "platform.h"

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
    auto window = Window::create();
    if (!window) return std::nullopt;

    auto renderer = Renderer::create(window->handle, SpaceInvaders::FRAMEBUFFER_WIDTH, SpaceInvaders::FRAMEBUFFER_HEIGHT);
    if (!renderer) return std::nullopt;

    auto audioManager = AudioManager::create();
    if (!audioManager) return std::nullopt;

    audioManager->loadSamples();

    return App{std::move(*window), std::move(*renderer), std::move(*audioManager)};
}

bool App::run(std::string_view romPath) {
    bool success = m_spaceInvaders.loadRom(romPath);
    if (!success) return false;

    loadHighScore();

    while (m_isRunning) {
        auto frameStart = std::chrono::steady_clock::now();
        static constexpr auto nsPerSec = std::chrono::nanoseconds(std::chrono::seconds(1));
        auto frameDeadline = frameStart + (nsPerSec / FPS);

        handleEvents();

        m_spaceInvaders.runFrame();

        // The Space Invaders ROM zeroes out highscore memory on init, so we need to override it with this check
        if (m_loadedHighscore > m_spaceInvaders.getHighScore()) {
            m_spaceInvaders.setHighScore(m_loadedHighscore);
        }

        m_renderer.drawPixelBuffer(m_spaceInvaders.getFramebuffer());

        handleAudio();

        auto frameEnd = std::chrono::steady_clock::now();
        if (frameDeadline > frameEnd) std::this_thread::sleep_for(frameDeadline - frameEnd);
    }

    saveHighScore();

    return true;
}

void App::saveHighScore() {
    auto dataDir = platform::getDataPath();
    if (!dataDir) {
        log::err("Failed to get path to data dir.");
        return;
    }

    auto highScoreFilePath = *dataDir / "highscore.txt";

    u16 highscore = m_spaceInvaders.getHighScore();

    // Clear errno so we don't use stale value
    errno = 0;

    std::ofstream out{highScoreFilePath, std::ios::trunc};
    if (!out) {
        int err = errno;
        log::err("Failed to open file: `{}` Error: {}", highScoreFilePath, std::strerror(err));
        return;
    }

    out << highscore << '\n';
}

void App::loadHighScore() {
    auto dataDir = platform::getDataPath();
    if (!dataDir) {
        log::err("Failed to get path to data dir.");
        return;
    }

    auto highScoreFilePath = *dataDir / "highscore.txt";

    // Clear errno so we don't use stale value
    errno = 0;

    std::ifstream file{highScoreFilePath};
    if (!file) {
        int err = errno;
        if (err == ENOENT) {
            log::info("Skipping highscore loading. highscore.txt doesn't exist.");
        } else {
            log::err("Failed to open file: `{}` Error: {}", highScoreFilePath, std::strerror(err));
        }
        return;
    }

    i64 number{};
    if (!(file >> number)) {
        log::err("Failed to load highscore value from file.");
        return;
    }

    if (number < 0 || number > std::numeric_limits<u16>::max()) {
        log::err("Failed to load highscore value. Please ensure it's a positive number and no bigger than {}.", std::numeric_limits<u16>::max());
        return;
    }

    m_loadedHighscore = static_cast<u16>(number);
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
