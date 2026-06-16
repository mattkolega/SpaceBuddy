#pragma once

#include <optional>
#include <utility>

#include "audio.h"
#include "core/spaceinvaders.h"
#include "window.h"
#include "renderer.h"

struct Sound {
    const u8 port{}, bit{};
    const AudioSample sample;
    const PlaybackType playbackType;
    bool previousBitState{};
};

class App {
public:
    static std::optional<App> create();

    bool run(std::string_view romPath);

private:
    SpaceInvaders m_spaceInvaders;
    Window        m_window;
    Renderer      m_renderer;
    AudioManager  m_audioManager;

    u16 m_loadedHighscore{};

    explicit App(Window window, Renderer renderer, AudioManager audioManager)
        : m_window(std::move(window))
        , m_renderer(std::move(renderer))
        , m_audioManager(std::move(audioManager)) {}

    bool m_isRunning{true};

    void saveHighScore();
    void loadHighScore();

    void handleEvents();
    void handleAudio();
};
