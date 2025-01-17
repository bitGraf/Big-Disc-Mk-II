#pragma once

namespace rh {

    // number of simultaneous sound cues that can play
    const u8 NumSoundChannels = 32;

    struct SoundCueSpec {
        std::string soundFile;
        float volume = 1.0f;

        // Set by engine
        f32 length = -1.0f;
        u32 soundID = 0;
    };

    struct BackingTrackSpec {
        std::string soundFile;
        u32 soundID;
    };

    struct SoundChannelStatus {
        bool active;
        float length, current;
        std::string cue;
        float volume;

        u32 soundID;
    };

    struct SoundEngineStatus {
        SoundChannelStatus channels[NumSoundChannels];
        int queueSize;
    };

    class SoundEngine {
    public:

        static void Init();

        static void CreateSoundCue(const std::string& cue, SoundCueSpec spec);
        static void CueSound(const std::string& cue);
        static void CueSound(const std::string& cue, laml::Vec3 position);

        static void CreateBackingTrack(const std::string& track, BackingTrackSpec spec);
        static void PlayTrack(const std::string& track);

        static void StartStream();
        static void StopStream();
        static void PauseStream();
        static void ResumeStream();

        static void SetListenerPosition(laml::Vec3 position);
        static void SetListenerVelocity(laml::Vec3 velocity);
        static void SetListenerOrientation(laml::Vec3 at, laml::Vec3 up);

        static SoundEngineStatus GetStatus();

        static void Update(double dt);

        static void Shutdown();
    };
}