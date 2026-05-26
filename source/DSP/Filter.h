#pragma once

#include "StereoSample.h"

#include <juce_dsp/juce_dsp.h>

// Resonant stereo low-pass per voice. Cutoff/resonance targets are smoothed to avoid clicks.
class Filter
{
public:
    static constexpr float defaultCutoffHz = 1050.0f;
    static constexpr float defaultResonance = 0.85f;
    static constexpr float minCutoffHz = 400.0f;
    static constexpr float maxCutoffHz = 2000.0f;
    static constexpr float minResonance = 0.5f;
    static constexpr float maxResonance = 1.15f;

    void prepare (double sampleRate, int samplesPerBlock)
    {
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
        spec.numChannels = 1;

        leftFilter.prepare (spec);
        rightFilter.prepare (spec);

        leftFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        rightFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

        constexpr double smoothingTimeSeconds = 0.04;
        smoothedCutoff.reset (sampleRate, smoothingTimeSeconds);
        smoothedResonance.reset (sampleRate, smoothingTimeSeconds);
        smoothedCutoffModulation.reset (sampleRate, smoothingTimeSeconds);

        cutoff = defaultCutoffHz;
        resonance = defaultResonance;
        cutoffModulation = 0.0f;
        resonanceModulation = 0.0f;

        updateSmoothedTargets (true);
    }

    void reset()
    {
        leftFilter.reset();
        rightFilter.reset();
    }

    void setCutoffHz (float newCutoffHz)
    {
        cutoff = juce::jlimit (minCutoffHz, maxCutoffHz, newCutoffHz);
        updateSmoothedTargets();
    }

    void setResonance (float newResonance)
    {
        resonance = juce::jlimit (minResonance, maxResonance, newResonance);
        updateSmoothedTargets();
    }

    float getCutoffHz() const { return cutoff; }

    float getResonance() const { return resonance; }

    // Static offsets from envelopes or other slow control sources.
    void setCutoffModulation (float offsetHz)
    {
        cutoffModulation = offsetHz;
    }

    void setResonanceModulation (float offset)
    {
        resonanceModulation = offset;
        updateSmoothedTargets();
    }

    StereoSample processSample (const StereoSample& input, float realtimeCutoffModHz = 0.0f)
    {
        smoothedCutoffModulation.setTargetValue (cutoffModulation + realtimeCutoffModHz);
        const auto totalCutoffMod = smoothedCutoffModulation.getNextValue();

        const auto targetCutoff = juce::jlimit (minCutoffHz, maxCutoffHz, cutoff + totalCutoffMod);
        smoothedCutoff.setTargetValue (targetCutoff);

        const auto cutoffHz = smoothedCutoff.getNextValue();
        const auto res = smoothedResonance.getNextValue();

        leftFilter.setCutoffFrequency (cutoffHz);
        rightFilter.setCutoffFrequency (cutoffHz);
        leftFilter.setResonance (res);
        rightFilter.setResonance (res);

        return {
            leftFilter.processSample (0, input.left),
            rightFilter.processSample (0, input.right),
        };
    }

private:
    void updateSmoothedTargets (bool forceImmediate = false)
    {
        const auto targetCutoff = juce::jlimit (minCutoffHz, maxCutoffHz, cutoff + cutoffModulation);
        const auto targetResonance = juce::jlimit (minResonance, maxResonance, resonance + resonanceModulation);

        if (forceImmediate)
        {
            smoothedCutoffModulation.setCurrentAndTargetValue (cutoffModulation);
            smoothedCutoff.setCurrentAndTargetValue (targetCutoff);
            smoothedResonance.setCurrentAndTargetValue (targetResonance);
            applyParametersToFilters (targetCutoff, targetResonance);
        }
        else
        {
            smoothedCutoff.setTargetValue (targetCutoff);
            smoothedResonance.setTargetValue (targetResonance);
        }
    }

    void applyParametersToFilters (float cutoffHz, float res)
    {
        leftFilter.setCutoffFrequency (cutoffHz);
        rightFilter.setCutoffFrequency (cutoffHz);
        leftFilter.setResonance (res);
        rightFilter.setResonance (res);
    }

    juce::dsp::StateVariableTPTFilter<float> leftFilter;
    juce::dsp::StateVariableTPTFilter<float> rightFilter;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedCutoff;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedResonance;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedCutoffModulation;

    float cutoff = defaultCutoffHz;
    float resonance = defaultResonance;
    float cutoffModulation = 0.0f;
    float resonanceModulation = 0.0f;
};
