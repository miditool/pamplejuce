#pragma once

#include <juce_dsp/juce_dsp.h>

// Slow sine LFO for atmospheric pad movement. Output is bipolar (-1 to +1).
class LFO
{
public:
    static constexpr float defaultRateHz = 0.06f;
    static constexpr float minRateHz = 0.03f;
    static constexpr float maxRateHz = 0.12f;
    static constexpr float defaultDepth = 0.4f;
    static constexpr float maxCutoffModDepthHz = 900.0f;

    void prepare (double sampleRate, int samplesPerBlock)
    {
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
        spec.numChannels = 1;

        oscillator.prepare (spec);
        oscillator.initialise (sineWave);
        setRateHz (rateHz);

        constexpr double smoothingTimeSeconds = 0.05;
        smoothedDepth.reset (sampleRate, smoothingTimeSeconds);
        updateDepthTarget (true);

        reset();
    }

    void reset()
    {
        oscillator.reset();
    }

    void setRateHz (float newRateHz)
    {
        rateHz = juce::jlimit (minRateHz, maxRateHz, newRateHz);
        oscillator.setFrequency (rateHz);
    }

    void setDepthModulation (float depthOffset)
    {
        depthModulation = depthOffset;
        updateDepthTarget();
    }

    float getRateHz() const { return rateHz; }

    float getDepth() const { return depth; }

    float processSample()
    {
        return oscillator.processSample (0.0f);
    }

    float getNextSmoothedDepth()
    {
        return smoothedDepth.getNextValue();
    }

private:
    static float sineWave (float phase)
    {
        return std::sin (phase);
    }

    void updateDepthTarget (bool forceImmediate = false)
    {
        const auto targetDepth = juce::jlimit (0.0f, 1.0f, depth + depthModulation);

        if (forceImmediate)
            smoothedDepth.setCurrentAndTargetValue (targetDepth);
        else
            smoothedDepth.setTargetValue (targetDepth);
    }

    juce::dsp::Oscillator<float> oscillator;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDepth;

    float rateHz = defaultRateHz;
    float depth = defaultDepth;
    float depthModulation = 0.0f;
};
