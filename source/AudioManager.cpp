

#include "AudioManager.h"

#include <algorithm>
#include <limits>

namespace
{
    constexpr std::array<
        const char*,
        static_cast<std::size_t>(GameSound::Count)
    > kFiles = { {
        "",
        "engine_idle_loop.wav",
        "engine_ride_loop.wav",
        "engine_boost_loop.wav",
        "engine_start.wav",
        "jump.wav",
        "landing_soft.wav",
        "landing_hard.wav",
        "boost_start.wav",
        "flip_whoosh.wav",
        "stunt_single.wav",
        "stunt_double.wav",
        "stunt_triple.wav",
        "countdown_tick.wav",
        "countdown_go.wav",
        "menu_move.wav",
        "menu_select.wav",
        "checkpoint.wav",
        "race_finish.wav",
        "unlock.wav",
        "haunted_ambience_loop.wav",
        "haunted_run_music_loop.wav"
    } };
}

std::size_t AudioManager::Index(GameSound sound)
{
    return static_cast<std::size_t>(sound);
}

float AudioManager::ClampVolume(float volume)
{
    return std::clamp(volume, 0.0f, 1.0f);
}

AudioManager::~AudioManager()
{
    Shutdown();
}

bool AudioManager::CreateStream(
    SDL_AudioStream*& result
)
{
    result = SDL_CreateAudioStream(
        &sourceSpec_,
        &deviceSpec_
    );

    if (!result)
    {
        SDL_Log(
            "Create audio stream failed: %s",
            SDL_GetError()
        );

        return false;
    }

    if (!SDL_BindAudioStream(device_, result))
    {
        SDL_Log(
            "Bind audio stream failed: %s",
            SDL_GetError()
        );

        SDL_DestroyAudioStream(result);
        result = nullptr;

        return false;
    }

    return true;
}

bool AudioManager::Init()
{
    Shutdown();

    const char* base = SDL_GetBasePath();

    const std::string audioFolder =
        std::string(base ? base : "") +
        "assets/audio/";

    // Load all WAV files into memory.
    for (std::size_t i = 1; i < kSoundCount; ++i)
    {
        SDL_AudioSpec spec{};

        Uint8* wavData = nullptr;
        Uint32 wavLength = 0;

        const std::string path =
            audioFolder + kFiles[i];

        if (!SDL_LoadWAV(
            path.c_str(),
            &spec,
            &wavData,
            &wavLength))
        {
            SDL_Log(
                "Failed to load audio %s: %s",
                path.c_str(),
                SDL_GetError()
            );

            Shutdown();
            return false;
        }

        if (i == 1)
        {
            sourceSpec_ = spec;
        }

        if (
            spec.freq != sourceSpec_.freq ||
            spec.channels != sourceSpec_.channels ||
            spec.format != sourceSpec_.format
            )
        {
            SDL_Log(
                "Audio format mismatch: %s",
                path.c_str()
            );

            SDL_free(wavData);
            Shutdown();

            return false;
        }

        Clip& clip = clips_[i];

        clip.spec = spec;

        clip.data.assign(
            wavData,
            wavData + wavLength
        );

        SDL_free(wavData);

        const Uint64 bytesPerSecond =
            static_cast<Uint64>(spec.freq) *
            static_cast<Uint64>(spec.channels) *
            2ULL;

        clip.durationMs =
            bytesPerSecond != 0
            ? static_cast<Uint64>(wavLength) *
            1000ULL / bytesPerSecond
            : 0ULL;
    }

    // Open the default playback device.
    device_ = SDL_OpenAudioDevice(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        nullptr
    );

    if (device_ == 0)
    {
        SDL_Log(
            "Open audio device failed: %s",
            SDL_GetError()
        );

        Shutdown();
        return false;
    }

    if (!SDL_GetAudioDeviceFormat(
        device_,
        &deviceSpec_,
        nullptr))
    {
        SDL_Log(
            "Get audio device format failed: %s",
            SDL_GetError()
        );

        Shutdown();
        return false;
    }

    // Create independent sound-effect voices.
    for (Voice& voice : voices_)
    {
        if (!CreateStream(voice.stream))
        {
            Shutdown();
            return false;
        }
    }

    // Create three dedicated looping channels.
    if (
        !CreateStream(engine_.stream) ||
        !CreateStream(music_.stream) ||
        !CreateStream(ambience_.stream)
        )
    {
        Shutdown();
        return false;
    }

    ready_ = true;

    if (!SDL_ResumeAudioDevice(device_))
    {
        SDL_Log(
            "Audio playback failed: %s",
            SDL_GetError()
        );

        Shutdown();
        return false;
    }

    SDL_Log(
        "TorqRift audio ready: 21 WAVs loaded"
    );

    return true;
}

void AudioManager::Play(
    GameSound sound,
    float volume
)
{
    if (
        !ready_ ||
        sound == GameSound::None ||
        Index(sound) >= kSoundCount
        )
    {
        return;
    }

    const Clip& clip = clips_[Index(sound)];

    if (clip.data.empty())
    {
        return;
    }

    const Uint64 now = SDL_GetTicks();

    Voice* availableVoice = nullptr;

    for (Voice& voice : voices_)
    {
        if (now >= voice.busyUntilMs)
        {
            availableVoice = &voice;
            break;
        }
    }

    // Avoid interrupting another active sound.
    if (!availableVoice)
    {
        return;
    }

    SDL_ClearAudioStream(
        availableVoice->stream
    );

    SDL_SetAudioStreamGain(
        availableVoice->stream,
        ClampVolume(volume)
    );

    if (SDL_PutAudioStreamData(
        availableVoice->stream,
        clip.data.data(),
        static_cast<int>(clip.data.size())))
    {
        availableVoice->busyUntilMs =
            now + clip.durationMs + 75ULL;
    }
    else
    {
        SDL_Log(
            "Cannot queue sound: %s",
            SDL_GetError()
        );
    }
}

void AudioManager::SetLoop(
    LoopChannel& channel,
    GameSound sound,
    float volume
)
{
    if (
        !ready_ ||
        !channel.stream ||
        sound == channel.sound
        )
    {
        return;
    }

    SDL_ClearAudioStream(channel.stream);

    channel.sound = sound;

    SDL_SetAudioStreamGain(
        channel.stream,
        ClampVolume(volume)
    );

    PumpLoop(channel);
}

void AudioManager::PumpLoop(
    LoopChannel& channel
)
{
    if (
        !ready_ ||
        !channel.stream ||
        channel.sound == GameSound::None
        )
    {
        return;
    }

    const Clip& clip =
        clips_[Index(channel.sound)];

    if (clip.data.empty())
    {
        return;
    }

    const int remaining =
        SDL_GetAudioStreamQueued(channel.stream);

    const int bytesPerSecond =
        sourceSpec_.freq *
        sourceSpec_.channels *
        2;

    const int threshold =
        bytesPerSecond / 5;

    // Refill the stream near the end of a loop.
    if (
        remaining >= 0 &&
        remaining <= threshold
        )
    {
        if (!SDL_PutAudioStreamData(
            channel.stream,
            clip.data.data(),
            static_cast<int>(clip.data.size())))
        {
            SDL_Log(
                "Cannot queue loop: %s",
                SDL_GetError()
            );
        }
    }
}

void AudioManager::SetEngine(GameSound sound)
{
    if (
        sound != GameSound::None &&
        sound != GameSound::EngineIdle &&
        sound != GameSound::EngineRide &&
        sound != GameSound::EngineBoost
        )
    {
        return;
    }

    const float volume =
        sound == GameSound::EngineBoost
        ? 0.45f
        : 0.48f;

    SetLoop(
        engine_,
        sound,
        volume
    );
}

void AudioManager::SetMusic(bool enabled)
{
    SetLoop(
        music_,
        enabled
        ? GameSound::HauntedMusic
        : GameSound::None,
        0.12f
    );
}

void AudioManager::SetAmbience(bool enabled)
{
    SetLoop(
        ambience_,
        enabled
        ? GameSound::HauntedAmbience
        : GameSound::None,
        0.14f
    );
}

void AudioManager::Update()
{
    if (!ready_)
    {
        return;
    }

    PumpLoop(engine_);
    PumpLoop(music_);
    PumpLoop(ambience_);
}

void AudioManager::StopAll()
{
    if (!ready_)
    {
        return;
    }

    SetEngine(GameSound::None);
    SetMusic(false);
    SetAmbience(false);

    for (Voice& voice : voices_)
    {
        if (voice.stream)
        {
            SDL_ClearAudioStream(voice.stream);
        }

        voice.busyUntilMs = 0;
    }
}

void AudioManager::Shutdown()
{
    for (Voice& voice : voices_)
    {
        if (voice.stream)
        {
            SDL_DestroyAudioStream(voice.stream);
        }

        voice.stream = nullptr;
        voice.busyUntilMs = 0;
    }

    for (LoopChannel* channel :
        { &engine_, &music_, &ambience_ })
    {
        if (channel->stream)
        {
            SDL_DestroyAudioStream(
                channel->stream
            );
        }

        channel->stream = nullptr;
        channel->sound = GameSound::None;
    }

    if (device_ != 0)
    {
        SDL_CloseAudioDevice(device_);
        device_ = 0;
    }

    for (Clip& clip : clips_)
    {
        clip.data.clear();
    }

    ready_ = false;
}

