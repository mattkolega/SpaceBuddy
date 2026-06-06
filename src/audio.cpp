#include "audio.h"

#include <memory>
#include <string_view>

#include "common/fs.h"
#include "common/log.h"

static constexpr auto SOUND_FLAGS{MA_SOUND_FLAG_NO_PITCH | MA_SOUND_FLAG_NO_SPATIALIZATION};

static constexpr std::array<std::string_view, static_cast<usize>(AudioSample::_Count)> audioFilenames{
    "ufo_lowpitch.wav",
    "shoot.wav",
    "explosion.wav",
    "invaderkilled.wav",
    "fastinvader1.wav",
    "fastinvader2.wav",
    "fastinvader3.wav",
    "fastinvader4.wav",
    "invaderkilled.wav",
};

std::optional<AudioManager> AudioManager::create() {
    ma_result result;
    auto engine = std::make_unique<ma_engine>();

    result = ma_engine_init(NULL, engine.get());
    if (result != MA_SUCCESS) {
        return std::nullopt;
    }

    std::array<std::unique_ptr<ma_sound>, static_cast<usize>(AudioSample::_Count)> sounds;
    for (auto& s : sounds) s = std::make_unique<ma_sound>();

    return AudioManager{std::move(engine), std::move(sounds)};
}

AudioManager::~AudioManager() {
    for (const auto& sound : m_sounds) { ma_sound_uninit(sound.get()); }
    ma_engine_uninit(m_engine.get());
}

void AudioManager::loadSamples() {
    ma_result result;

    for (usize i{0}; i < audioFilenames.size(); i++) {
        auto filepath = fs::getExeDir() / "assets" / "sounds" / audioFilenames[i];
        result = ma_sound_init_from_file(m_engine.get(), filepath.c_str(), SOUND_FLAGS, NULL, NULL, m_sounds[i].get());

        if (result != MA_SUCCESS) {
            log::warn("Failed to load audio file: {}", filepath);
        }
    }
}

void AudioManager::playOneshot(AudioSample sample) {
    const auto& sound = m_sounds[static_cast<usize>(sample)];
    ma_sound_seek_to_pcm_frame(sound.get(), 0);
    ma_sound_start(sound.get());
}

void AudioManager::playLoop(AudioSample sample) {
    const auto& sound = m_sounds[static_cast<usize>(sample)];
    ma_sound_set_looping(sound.get(), MA_TRUE);
    ma_sound_seek_to_pcm_frame(sound.get(), 0);
    ma_sound_start(sound.get());
}

void AudioManager::stopLoop(AudioSample sample) {
    const auto& sound = m_sounds[static_cast<usize>(sample)];
    ma_sound_stop(sound.get());
}
