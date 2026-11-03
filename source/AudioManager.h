
#pragma once

#include <SDL3/SDL.h>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

enum class GameSound : std::uint8_t
{
    None = 0,

    EngineIdle,
    EngineRide,
    EngineBoost,
    EngineStart,

    Jump,
    LandingSoft,
    LandingHard,
    BoostStart,
    FlipWhoosh,

    StuntSingle,
    StuntDouble,
    StuntTriple,

    CountdownTick,
    CountdownGo,

    MenuMove,
    MenuSelect,

    Checkpoint,
    RaceFinish,
    Unlock,

    HauntedAmbience,
    HauntedMusic,

    Count
};

class AudioManager
{
public:
    AudioManager() = default;
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    bool Init();
    void Shutdown();

    // Play overlapping sound effects.
    void Play(
        GameSound sound,
        float volume = 0.70f
    );

    // Dedicated looping audio channels.
    void SetEngine(GameSound sound);
    void SetMusic(bool enabled);
    void SetAmbience(bool enabled);

    // Call once each frame.
    void Update();

    void StopAll();

private:
    struct Clip
    {
        std::vector<Uint8> data;
        SDL_AudioSpec spec{};
        Uint64 durationMs = 0;
    };

    struct Voice
    {
        SDL_AudioStream* stream = nullptr;
        Uint64 busyUntilMs = 0;
    };

    struct LoopChannel
    {
        SDL_AudioStream* stream = nullptr;
        GameSound sound = GameSound::None;
    };

    static constexpr std::size_t kVoiceCount = 16;

    static constexpr std::size_t kSoundCount =
        static_cast<std::size_t>(GameSound::Count);

    bool CreateStream(SDL_AudioStream*& result);

    void SetLoop(
        LoopChannel& channel,
        GameSound sound,
        float volume
    );

    void PumpLoop(LoopChannel& channel);

    static std::size_t Index(GameSound sound);
    static float ClampVolume(float volume);

    SDL_AudioDeviceID device_ = 0;

    SDL_AudioSpec sourceSpec_{};
    SDL_AudioSpec deviceSpec_{};

    std::array<Clip, kSoundCount> clips_{};
    std::array<Voice, kVoiceCount> voices_{};

    LoopChannel engine_{};
    LoopChannel music_{};
    LoopChannel ambience_{};

    bool ready_ = false;
};

