#pragma once

#include "MacroManager.h"
#include "ParameterIds.h"

#include <juce_audio_processors/juce_audio_processors.h>

inline const char* getMacroParamID (int index)
{
    static constexpr const char* ids[] = {
        ParamIDs::macroAir,
        ParamIDs::macroMotion,
        ParamIDs::macroWidth,
        ParamIDs::macroWarmth,
        ParamIDs::macroSpace,
    };

    if (index >= 0 && index < MacroManager::numMacros)
        return ids[static_cast<size_t> (index)];

    return ParamIDs::macroAir;
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const juce::NormalisableRange<float> macroRange (0.0f, 1.0f, 0.001f);

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::macroAir, 1 },
        "AIR",
        macroRange,
        MacroManager::defaultMacroValue));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::macroMotion, 1 },
        "MOTION",
        macroRange,
        MacroManager::defaultMacroValue));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::macroWidth, 1 },
        "WIDTH",
        macroRange,
        MacroManager::defaultMacroValue));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::macroWarmth, 1 },
        "WARMTH",
        macroRange,
        MacroManager::defaultMacroValue));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::macroSpace, 1 },
        "SPACE",
        macroRange,
        MacroManager::defaultSpaceValue));

    return layout;
}
