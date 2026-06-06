#pragma once

#include <array>
#include <memory>
#include <optional>
#include <utility>

#include "miniaudio.h"

#include "common/types.h"

enum class PlaybackType {
    Oneshot,
    Loop,
};

enum class AudioSample : u8 {
    UfoRepeat,
    Shoot,
    PlayerDeath,
    InvaderKilled,
    FleetMovement1,
    FleetMovement2,
    FleetMovement3,
    FleetMovement4,
    UfoHit,
    _Count
};

inline constexpr usize AUDIOSAMPLE_COUNT{std::to_underlying(AudioSample::_Count)};

class AudioManager {
public:
    static std::optional<AudioManager> create();

    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    AudioManager(AudioManager&& other) = default;
    AudioManager& operator=(AudioManager&& other) = default;

    void loadSamples();
    void playOneshot(AudioSample sample);
    void playLoop(AudioSample sample);
    void stopLoop(AudioSample sample);

private:
    explicit AudioManager(
        std::unique_ptr<ma_engine> engine,
        std::array<std::unique_ptr<ma_sound>, AUDIOSAMPLE_COUNT> sounds
    )
        : m_engine(std::move(engine)), m_sounds(std::move(sounds)) {}

    std::unique_ptr<ma_engine> m_engine;
    std::array<std::unique_ptr<ma_sound>, AUDIOSAMPLE_COUNT> m_sounds;
};