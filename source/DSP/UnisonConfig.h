#pragma once

// Per-unison-voice offsets applied before summing (Serum-style unison layer).
struct UnisonVoiceState
{
    float detuneCents = 0.0f;
    float panOffset = 0.0f;
    float phaseOffset = 0.0f;
};

// Global unison settings for one polyphonic voice.
struct UnisonConfig
{
    static constexpr int maxUnisonVoices = 8;

    int unisonCount = 1;
    float detuneSpread = 0.0f;
    float stereoSpread = 0.0f;
};
