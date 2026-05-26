#pragma once

#include <array>

#include <juce_dsp/juce_dsp.h>

// Serum-style macro controls (0.0–1.0). 0.5 is neutral and preserves default DSP settings.
class MacroManager
{
public:
    static constexpr int numMacros = 5;
    static constexpr float defaultMacroValue = 0.5f;
    static constexpr float defaultSpaceValue = 0.0f;

    enum MacroIndex : int
    {
        Air = 0,
        Motion,
        Width,
        Warmth,
        Space,
    };

    void prepare (double sampleRate)
    {
        constexpr double defaultSmoothingTimeSeconds = 0.075;
        constexpr double spaceSmoothingTimeSeconds = 0.225;

        for (int i = 0; i < numMacros; ++i)
        {
            auto& macroValue = macroValues[static_cast<size_t> (i)];
            const auto smoothingTime = (i == Space) ? spaceSmoothingTimeSeconds : defaultSmoothingTimeSeconds;
            macroValue.reset (sampleRate, smoothingTime);

            const auto initialValue = (i == Space) ? defaultSpaceValue : defaultMacroValue;
            macroValue.setCurrentAndTargetValue (initialValue);
        }
    }

    void setMacro (int index, float value)
    {
        if (index >= 0 && index < numMacros)
            macroValues[static_cast<size_t> (index)].setTargetValue (juce::jlimit (0.0f, 1.0f, value));
    }

    float getMacro (int index) const
    {
        if (index >= 0 && index < numMacros)
            return macroValues[static_cast<size_t> (index)].getTargetValue();

        return defaultMacroValue;
    }

    float getSmoothedMacro (int index) const
    {
        if (index >= 0 && index < numMacros)
            return macroValues[static_cast<size_t> (index)].getCurrentValue();

        return defaultMacroValue;
    }

    void skip (int numSamples)
    {
        for (auto& macroValue : macroValues)
            macroValue.skip (numSamples);
    }

    // Maps a macro around a DSP default so that 0.5 returns defaultValue exactly.
    static float mapAroundDefault (float macroValue, float defaultValue, float span)
    {
        return defaultValue + (macroValue - defaultMacroValue) * 2.0f * span;
    }

    static constexpr const char* getMacroName (int index)
    {
        switch (index)
        {
            case Air:    return "AIR";
            case Motion: return "MOTION";
            case Width:  return "WIDTH";
            case Warmth: return "WARMTH";
            case Space:  return "SPACE";
            default:     return "MACRO";
        }
    }

private:
    std::array<juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>, numMacros> macroValues {};
};
