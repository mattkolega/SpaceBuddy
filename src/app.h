#pragma once

#include <optional>
#include <utility>

#include "audio.h"
#include "core/spaceinvaders.h"
#include "platform.h"
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
    Platform      m_platform;
    Renderer      m_renderer;
    AudioManager  m_audioManager;

    explicit App(Platform platform, Renderer renderer, AudioManager audioManager)
        : m_platform(std::move(platform))
        , m_renderer(std::move(renderer))
        , m_audioManager(std::move(audioManager)) {}

    bool m_isRunning{true};

    void handleEvents();
    void handleAudio();
};
