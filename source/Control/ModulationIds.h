#pragma once

#include <cstdint>

enum class ModSourceID : std::uint8_t
{
    None = 0,
    LFO1,
    Envelope,
    Velocity,
    MacroAir,
    MacroMotion,
    MacroWidth,
    MacroWarmth,
    Count
};

enum class ModDestinationID : std::uint8_t
{
    None = 0,
    FilterCutoff,
    FilterResonance,
    OscillatorPitch,
    ChorusMix,
    ChorusDepth,
    ReverbWet,
    ReverbDamping,
    LFODepth,
    Count
};

inline constexpr bool isEngineDestination (ModDestinationID destination)
{
    switch (destination)
    {
        case ModDestinationID::ChorusMix:
        case ModDestinationID::ChorusDepth:
        case ModDestinationID::ReverbWet:
        case ModDestinationID::ReverbDamping:
            return true;

        default:
            return false;
    }
}

inline constexpr bool isVoiceDestination (ModDestinationID destination)
{
    return destination != ModDestinationID::None && ! isEngineDestination (destination);
}

inline constexpr bool isAudioRateDestination (ModDestinationID destination)
{
    return destination == ModDestinationID::FilterCutoff;
}

inline constexpr const char* getModSourceName (ModSourceID source)
{
    switch (source)
    {
        case ModSourceID::LFO1:         return "LFO1";
        case ModSourceID::Envelope:     return "Envelope";
        case ModSourceID::Velocity:     return "Velocity";
        case ModSourceID::MacroAir:     return "Macro Air";
        case ModSourceID::MacroMotion:  return "Macro Motion";
        case ModSourceID::MacroWidth:   return "Macro Width";
        case ModSourceID::MacroWarmth:  return "Macro Warmth";
        default:                        return "None";
    }
}

inline constexpr const char* getModDestinationName (ModDestinationID destination)
{
    switch (destination)
    {
        case ModDestinationID::FilterCutoff:     return "Filter Cutoff";
        case ModDestinationID::FilterResonance:  return "Filter Resonance";
        case ModDestinationID::OscillatorPitch:  return "Oscillator Pitch";
        case ModDestinationID::ChorusMix:        return "Chorus Mix";
        case ModDestinationID::ChorusDepth:      return "Chorus Depth";
        case ModDestinationID::ReverbWet:        return "Reverb Wet";
        case ModDestinationID::ReverbDamping:    return "Reverb Damping";
        case ModDestinationID::LFODepth:         return "LFO Depth";
        default:                                   return "None";
    }
}
