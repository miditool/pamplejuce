#pragma once

#include "OscillatorSection.h"
#include "UnisonConfig.h"

#include <juce_dsp/juce_dsp.h>

#include <array>

// Wraps the oscillator stack and prepares for future unison summing.
// unisonCount == 1 bypasses the unison layer entirely (current sound).
class UnisonSection
{
public:
    void prepare (double sampleRate, int samplesPerBlock)
    {
        oscillatorSection.prepare (sampleRate, samplesPerBlock);
        applyUnisonVoiceLayout();
    }

    void setUnisonConfig (const UnisonConfig& config)
    {
        unisonConfig = config;
        unisonConfig.unisonCount = juce::jlimit (1, UnisonConfig::maxUnisonVoices, unisonConfig.unisonCount);
        applyUnisonVoiceLayout();
    }

    const UnisonConfig& getUnisonConfig() const { return unisonConfig; }

    std::array<UnisonVoiceState, UnisonConfig::maxUnisonVoices>& getUnisonVoiceStates() { return unisonVoiceStates; }

    const std::array<UnisonVoiceState, UnisonConfig::maxUnisonVoices>& getUnisonVoiceStates() const { return unisonVoiceStates; }

    void setFrequency (float frequencyHz)
    {
        oscillatorSection.setFrequency (frequencyHz);
    }

    void setPitchModulationSemitones (float semitones)
    {
        oscillatorSection.setPitchModulationSemitones (semitones);
    }

    void reset()
    {
        oscillatorSection.reset();
    }

    StereoSample processSample()
    {
        if (unisonConfig.unisonCount <= 1)
            return oscillatorSection.processSample();

        StereoSample output;

        for (int i = 0; i < unisonConfig.unisonCount; ++i)
        {
            const auto& voiceState = unisonVoiceStates[static_cast<size_t> (i)];

            // Future: apply voiceState detune/pan/phase and accumulate a dedicated osc copy.
            juce::ignoreUnused (voiceState);
        }

        return output;
    }

private:
    void applyUnisonVoiceLayout()
    {
        unisonVoiceStates.fill ({});

        if (unisonConfig.unisonCount <= 1)
            return;

        const auto voiceSpan = static_cast<float> (unisonConfig.unisonCount - 1);

        for (int i = 0; i < unisonConfig.unisonCount; ++i)
        {
            const auto normalized = voiceSpan > 0.0f ? (static_cast<float> (i) / voiceSpan) * 2.0f - 1.0f : 0.0f;

            auto& voiceState = unisonVoiceStates[static_cast<size_t> (i)];
            voiceState.detuneCents = normalized * unisonConfig.detuneSpread * 50.0f;
            voiceState.panOffset = normalized * unisonConfig.stereoSpread;
            voiceState.phaseOffset = 0.0f;
        }
    }

    OscillatorSection oscillatorSection;
    UnisonConfig unisonConfig {};
    std::array<UnisonVoiceState, UnisonConfig::maxUnisonVoices> unisonVoiceStates {};
};
