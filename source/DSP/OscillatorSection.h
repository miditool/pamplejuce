#pragma once

#include "Oscillator.h"
#include "StereoSample.h"

#include <juce_dsp/juce_dsp.h>

#include <array>

// Per-oscillator-stack voice settings (dual saw pad layer).
struct OscillatorVoiceConfig
{
    float detuneCents = 0.0f;
    float pan = 0.0f;
    float level = 0.35f;
    Waveform waveform = Waveform::Saw;
};

// Dual-oscillator pad section with detune and stereo spread.
class OscillatorSection
{
public:
    static constexpr int maxOscillatorVoices = 8;

    void prepare (double sampleRate, int samplesPerBlock)
    {
        for (auto& voice : voices)
            voice.oscillator.prepare (sampleRate, samplesPerBlock);

        configureDefaultPadVoices();
    }

    void setFrequency (float frequencyHz)
    {
        baseFrequency = frequencyHz;
        updateVoiceFrequencies();
    }

    void setPitchModulationSemitones (float semitones)
    {
        pitchModulationSemitones = semitones;
        updateVoiceFrequencies();
    }

    void reset()
    {
        for (auto& voice : voices)
            voice.oscillator.reset();
    }

    StereoSample processSample()
    {
        return processOscillatorStackSample();
    }

private:
    struct Voice
    {
        Oscillator oscillator;
    };

    StereoSample processOscillatorStackSample()
    {
        StereoSample output;

        for (int i = 0; i < activeOscillatorVoiceCount; ++i)
        {
            const auto& config = oscillatorVoiceConfigs[static_cast<size_t> (i)];
            const auto sample = voices[static_cast<size_t> (i)].oscillator.processSample();
            const auto panned = applyPan (sample, config.pan, config.level);

            output.left += panned.left;
            output.right += panned.right;
        }

        return output;
    }

    void configureDefaultPadVoices()
    {
        activeOscillatorVoiceCount = 2;

        oscillatorVoiceConfigs = {{
            { 0.0f, -0.4f, 0.38f, Waveform::Saw },
            { 8.0f,  0.4f, 0.38f, Waveform::Saw },
        }};

        for (int i = 0; i < activeOscillatorVoiceCount; ++i)
        {
            const auto& config = oscillatorVoiceConfigs[static_cast<size_t> (i)];
            voices[static_cast<size_t> (i)].oscillator.setWaveform (config.waveform);
        }

        updateVoiceFrequencies();
    }

    void updateVoiceFrequencies()
    {
        const auto pitchRatio = std::pow (2.0f, pitchModulationSemitones / 12.0f);
        const auto effectiveBaseFrequency = baseFrequency * pitchRatio;

        for (int i = 0; i < activeOscillatorVoiceCount; ++i)
        {
            const auto& config = oscillatorVoiceConfigs[static_cast<size_t> (i)];
            const auto detuneRatio = std::pow (2.0f, config.detuneCents / 1200.0f);
            voices[static_cast<size_t> (i)].oscillator.setFrequency (effectiveBaseFrequency * detuneRatio);
        }
    }

    static StereoSample applyPan (float sample, float pan, float level)
    {
        const auto panNormalized = juce::jlimit (0.0f, 1.0f, (pan + 1.0f) * 0.5f);
        const auto leftGain = level * std::cos (panNormalized * juce::MathConstants<float>::halfPi);
        const auto rightGain = level * std::sin (panNormalized * juce::MathConstants<float>::halfPi);

        return { sample * leftGain, sample * rightGain };
    }

    std::array<Voice, maxOscillatorVoices> voices {};
    std::array<OscillatorVoiceConfig, maxOscillatorVoices> oscillatorVoiceConfigs {};
    int activeOscillatorVoiceCount = 0;
    float baseFrequency = 440.0f;
    float pitchModulationSemitones = 0.0f;
};
