#pragma once

#include "StereoSample.h"

#include <juce_dsp/juce_dsp.h>

#include <cmath>

// Hidden harmonic foundation: mono sine summed pre-filter at very low level.
// Not modulated by macros; follows note frequency and pitch wheel only.
class SineSubLayer
{
public:
    static constexpr float mixGain = 0.085f;
    static constexpr float maxDriftCents = 3.0f;
    static constexpr float driftRateHz = 0.04f;

    void prepare (double sampleRate, int samplesPerBlock)
    {
        sampleRateHz = sampleRate;

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
        spec.numChannels = 1;

        oscillator.prepare (spec);
        oscillator.initialise ([] (float phase) { return std::sin (phase); });
        oscillator.reset();

        driftPhaseIncrement = juce::MathConstants<float>::twoPi * driftRateHz
                            / static_cast<float> (sampleRateHz);

        updateOscillatorFrequency();
    }

    void reset()
    {
        oscillator.reset();
        driftPhase = 0.17f;
        updateOscillatorFrequency();
    }

    void setFrequency (float frequencyHz)
    {
        baseFrequencyHz = juce::jmax (20.0f, frequencyHz);
        updateOscillatorFrequency();
    }

    StereoSample processSample()
    {
        const auto driftCents = maxDriftCents * std::sin (driftPhase);
        driftPhase += driftPhaseIncrement;

        const auto driftRatio = std::pow (2.0f, driftCents / 1200.0f);
        oscillator.setFrequency (baseFrequencyHz * driftRatio);

        const auto sample = oscillator.processSample (0.0f) * mixGain;
        return { sample, sample };
    }

private:
    void updateOscillatorFrequency()
    {
        oscillator.setFrequency (baseFrequencyHz);
    }

    juce::dsp::Oscillator<float> oscillator;
    double sampleRateHz = 44100.0;
    float baseFrequencyHz = 440.0f;
    float driftPhase = 0.17f;
    float driftPhaseIncrement = 0.0f;
};
