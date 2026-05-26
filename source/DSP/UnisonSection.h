#pragma once

#include "Oscillator.h"
#include "OscillatorSection.h"
#include "StereoSample.h"
#include "UnisonConfig.h"

#include <algorithm>
#include <array>
#include <cmath>

// Wraps the oscillator stack with optional internal unison summing.
// unisonCount == 1 bypasses unison and uses the dual-oscillator pad section.
class UnisonSection
{
public:
    static constexpr float unisonVoiceLevel = 0.78f;
    static constexpr float maxDriftCents = 3.5f;
    static constexpr float baseDriftRateHz = 0.055f;

    void prepare (double sampleRate, int samplesPerBlock)
    {
        sampleRateHz = sampleRate;
        oscillatorSection.prepare (sampleRate, samplesPerBlock);

        for (auto& voice : runtimeVoices)
            voice.oscillator.prepare (sampleRate, samplesPerBlock);

        configureRuntimeVoices();
        applyUnisonVoiceLayout();
    }

    void setUnisonConfig (const UnisonConfig& config)
    {
        unisonConfig = config;
        unisonConfig.unisonCount = std::clamp (unisonConfig.unisonCount, 1, UnisonConfig::maxUnisonVoices);
        applyUnisonVoiceLayout();
    }

    const UnisonConfig& getUnisonConfig() const { return unisonConfig; }

    std::array<UnisonVoiceState, UnisonConfig::maxUnisonVoices>& getUnisonVoiceStates() { return unisonVoiceStates; }

    const std::array<UnisonVoiceState, UnisonConfig::maxUnisonVoices>& getUnisonVoiceStates() const { return unisonVoiceStates; }

    void setFrequency (float frequencyHz)
    {
        baseFrequencyHz = frequencyHz;
        oscillatorSection.setFrequency (frequencyHz);
    }

    void setPitchModulationSemitones (float semitones)
    {
        pitchModulationSemitones = semitones;
        oscillatorSection.setPitchModulationSemitones (semitones);
    }

    void reset()
    {
        oscillatorSection.reset();

        for (size_t i = 0; i < runtimeVoices.size(); ++i)
        {
            auto& voice = runtimeVoices[i];
            voice.driftPhase = static_cast<float> (i) * 1.37f;
            voice.oscillator.setPhaseOffset (unisonVoiceStates[i].phaseOffset);
            voice.oscillator.reset();
        }
    }

    StereoSample processSample()
    {
        if (unisonConfig.unisonCount <= 1)
            return oscillatorSection.processSample();

        StereoSample output;
        const auto voiceGain = unisonVoiceLevel / static_cast<float> (unisonConfig.unisonCount);
        const auto pitchRatio = std::pow (2.0f, pitchModulationSemitones / 12.0f);
        const auto effectiveBaseFrequency = baseFrequencyHz * pitchRatio;

        for (int i = 0; i < unisonConfig.unisonCount; ++i)
        {
            const auto index = static_cast<size_t> (i);
            const auto& voiceState = unisonVoiceStates[index];
            auto& runtimeVoice = runtimeVoices[index];

            const auto driftCents = maxDriftCents * std::sin (runtimeVoice.driftPhase);
            const auto totalDetuneCents = voiceState.detuneCents + driftCents;
            const auto detuneRatio = std::pow (2.0f, totalDetuneCents / 1200.0f);

            runtimeVoice.oscillator.setFrequency (effectiveBaseFrequency * detuneRatio);
            const auto sample = runtimeVoice.oscillator.processSample();
            const auto panned = applyPan (sample, voiceState.panOffset, voiceGain);

            output.left += panned.left;
            output.right += panned.right;

            runtimeVoice.driftPhase += runtimeVoice.driftPhaseIncrement;
        }

        return output;
    }

private:
    struct RuntimeVoice
    {
        Oscillator oscillator;
        float driftPhase = 0.0f;
        float driftPhaseIncrement = 0.0f;
    };

    void configureRuntimeVoices()
    {
        for (size_t i = 0; i < runtimeVoices.size(); ++i)
        {
            auto& voice = runtimeVoices[i];
            voice.oscillator.setWaveform (Waveform::Saw);

            const auto driftRateHz = baseDriftRateHz * (1.0f + static_cast<float> (i) * 0.09f);
            voice.driftPhaseIncrement = juce::MathConstants<float>::twoPi * driftRateHz
                                      / static_cast<float> (sampleRateHz);
            voice.driftPhase = static_cast<float> (i) * 1.37f;
        }
    }

    void applyUnisonVoiceLayout()
    {
        unisonVoiceStates.fill ({});

        if (unisonConfig.unisonCount <= 1)
            return;

        const auto voiceSpan = static_cast<float> (unisonConfig.unisonCount - 1);

        for (int i = 0; i < unisonConfig.unisonCount; ++i)
        {
            const auto normalized = voiceSpan > 0.0f ? (static_cast<float> (i) / voiceSpan) * 2.0f - 1.0f : 0.0f;
            const auto index = static_cast<size_t> (i);

            auto& voiceState = unisonVoiceStates[index];
            voiceState.detuneCents = normalized * unisonConfig.detuneSpread * 50.0f;
            voiceState.panOffset = normalized * unisonConfig.stereoSpread;
            voiceState.phaseOffset = normalized * juce::MathConstants<float>::pi * 0.42f;

            runtimeVoices[index].oscillator.setPhaseOffset (voiceState.phaseOffset);
        }
    }

    static StereoSample applyPan (float sample, float pan, float level)
    {
        const auto panNormalized = std::clamp ((pan + 1.0f) * 0.5f, 0.0f, 1.0f);
        const auto leftGain = level * std::cos (panNormalized * juce::MathConstants<float>::halfPi);
        const auto rightGain = level * std::sin (panNormalized * juce::MathConstants<float>::halfPi);

        return { sample * leftGain, sample * rightGain };
    }

    OscillatorSection oscillatorSection;
    UnisonConfig unisonConfig {};
    std::array<UnisonVoiceState, UnisonConfig::maxUnisonVoices> unisonVoiceStates {};
    std::array<RuntimeVoice, UnisonConfig::maxUnisonVoices> runtimeVoices {};
    double sampleRateHz = 44100.0;
    float baseFrequencyHz = 440.0f;
    float pitchModulationSemitones = 0.0f;
};
