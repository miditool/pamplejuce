#pragma once

#include <juce_dsp/juce_dsp.h>

// Subtle stereo widening for atmospheric pads. Sits after voice filtering, before reverb.
class ChorusSection
{
public:
    static constexpr float defaultRateHz = 0.1f;
    static constexpr float defaultDepth = 0.22f;
    static constexpr float defaultCentreDelayMs = 11.0f;
    static constexpr float defaultFeedback = 0.08f;
    static constexpr float defaultMix = 0.28f;

    void prepare (double sampleRate, int samplesPerBlock)
    {
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
        spec.numChannels = 2;

        chorus.prepare (spec);
        configureDefaultPadSettings();

        constexpr double smoothingTimeSeconds = 0.05;
        smoothedRate.reset (sampleRate, smoothingTimeSeconds);
        smoothedDepth.reset (sampleRate, smoothingTimeSeconds);
        smoothedCentreDelay.reset (sampleRate, smoothingTimeSeconds);
        smoothedFeedback.reset (sampleRate, smoothingTimeSeconds);
        smoothedMix.reset (sampleRate, smoothingTimeSeconds);

        applySmoothedParameters (true);
    }

    void reset()
    {
        chorus.reset();
    }

    void setRateHz (float rateHz)
    {
        rate = juce::jlimit (0.01f, 5.0f, rateHz);
        smoothedRate.setTargetValue (rate);
    }

    void setDepth (float newDepth)
    {
        depth = juce::jlimit (0.0f, 1.0f, newDepth);
        smoothedDepth.setTargetValue (depth);
    }

    void setCentreDelayMs (float delayMs)
    {
        centreDelayMs = juce::jlimit (1.0f, 100.0f, delayMs);
        smoothedCentreDelay.setTargetValue (centreDelayMs);
    }

    void setFeedback (float newFeedback)
    {
        feedback = juce::jlimit (0.0f, 0.2f, newFeedback);
        smoothedFeedback.setTargetValue (feedback);
    }

    void setMix (float newMix)
    {
        mix = juce::jlimit (0.0f, 1.0f, newMix);
        smoothedMix.setTargetValue (mix);
    }

    void setMixModulation (float offset)
    {
        mixModulation = offset;
    }

    void setDepthModulation (float offset)
    {
        depthModulation = offset;
    }

    void processBlock (juce::dsp::AudioBlock<float> block)
    {
        applySmoothedParameters (false, static_cast<int> (block.getNumSamples()));

        juce::dsp::ProcessContextReplacing<float> context (block);
        chorus.process (context);
    }

private:
    void configureDefaultPadSettings()
    {
        rate = defaultRateHz;
        depth = defaultDepth;
        centreDelayMs = defaultCentreDelayMs;
        feedback = defaultFeedback;
        mix = defaultMix;
    }

    void applySmoothedParameters (bool forceImmediate, int numSamplesToSkip = 0)
    {
        const auto effectiveMix = juce::jlimit (0.0f, 1.0f, defaultMix + mixModulation);
        const auto effectiveDepth = juce::jlimit (0.0f, 1.0f, defaultDepth + depthModulation);

        if (forceImmediate)
        {
            smoothedRate.setCurrentAndTargetValue (rate);
            smoothedDepth.setCurrentAndTargetValue (effectiveDepth);
            smoothedCentreDelay.setCurrentAndTargetValue (centreDelayMs);
            smoothedFeedback.setCurrentAndTargetValue (feedback);
            smoothedMix.setCurrentAndTargetValue (effectiveMix);
        }
        else
        {
            smoothedDepth.setTargetValue (effectiveDepth);
            smoothedMix.setTargetValue (effectiveMix);
        }

        if (numSamplesToSkip > 0)
        {
            smoothedRate.skip (numSamplesToSkip);
            smoothedDepth.skip (numSamplesToSkip);
            smoothedCentreDelay.skip (numSamplesToSkip);
            smoothedFeedback.skip (numSamplesToSkip);
            smoothedMix.skip (numSamplesToSkip);
        }

        chorus.setRate (smoothedRate.getCurrentValue());
        chorus.setDepth (smoothedDepth.getCurrentValue());
        chorus.setCentreDelay (smoothedCentreDelay.getCurrentValue());
        chorus.setFeedback (smoothedFeedback.getCurrentValue());
        chorus.setMix (smoothedMix.getCurrentValue());
    }

    juce::dsp::Chorus<float> chorus;
    juce::dsp::ProcessSpec spec {};

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedRate;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDepth;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedCentreDelay;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedFeedback;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedMix;

    float rate = defaultRateHz;
    float depth = defaultDepth;
    float centreDelayMs = defaultCentreDelayMs;
    float feedback = defaultFeedback;
    float mix = defaultMix;
    float mixModulation = 0.0f;
    float depthModulation = 0.0f;
};
