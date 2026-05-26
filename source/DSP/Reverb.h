#pragma once

#include <juce_dsp/juce_dsp.h>

// Cinematic stereo reverb for the engine-level effect chain (after chorus).
class Reverb
{
public:
    static constexpr float defaultRoomSize = 0.9f;
    static constexpr float defaultDamping = 0.5f;
    static constexpr float defaultWetLevel = 0.32f;
    static constexpr float defaultDryLevel = 0.78f;
    static constexpr float defaultWidth = 1.0f;
    static constexpr float tailLengthSeconds = 4.0f;

    void prepare (double sampleRate, int samplesPerBlock)
    {
        juce::ignoreUnused (samplesPerBlock);

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
        spec.numChannels = 2;

        reverb.prepare (spec);
        configureDefaultPadSettings();

        constexpr double smoothingTimeSeconds = 0.075;
        smoothedRoomSize.reset (sampleRate, smoothingTimeSeconds);
        smoothedDamping.reset (sampleRate, smoothingTimeSeconds);
        smoothedWetLevel.reset (sampleRate, smoothingTimeSeconds);
        smoothedDryLevel.reset (sampleRate, smoothingTimeSeconds);
        smoothedWidth.reset (sampleRate, smoothingTimeSeconds);

        applySmoothedParameters (true);
    }

    void reset()
    {
        reverb.reset();
    }

    void setRoomSize (float newRoomSize)
    {
        roomSize = juce::jlimit (0.0f, 1.0f, newRoomSize);
        smoothedRoomSize.setTargetValue (roomSize);
    }

    void setDamping (float newDamping)
    {
        damping = juce::jlimit (0.0f, 1.0f, newDamping);
        smoothedDamping.setTargetValue (damping);
    }

    void setWetLevel (float newWetLevel)
    {
        wetLevel = juce::jlimit (0.0f, 1.0f, newWetLevel);
        smoothedWetLevel.setTargetValue (wetLevel);
    }

    void setDryLevel (float newDryLevel)
    {
        dryLevel = juce::jlimit (0.0f, 1.0f, newDryLevel);
        smoothedDryLevel.setTargetValue (dryLevel);
    }

    void setWidth (float newWidth)
    {
        width = juce::jlimit (0.0f, 1.0f, newWidth);
        smoothedWidth.setTargetValue (width);
    }

    void setWetModulation (float offset)
    {
        wetModulation = offset;
    }

    void setDampingModulation (float offset)
    {
        dampingModulation = offset;
    }

    static float getTailLengthSeconds()
    {
        return tailLengthSeconds;
    }

    void processBlock (juce::dsp::AudioBlock<float> block)
    {
        if (block.getNumChannels() < 2)
            return;

        applySmoothedParameters (false, static_cast<int> (block.getNumSamples()));

        juce::dsp::ProcessContextReplacing<float> context (block);
        reverb.process (context);
    }

private:
    void configureDefaultPadSettings()
    {
        roomSize = defaultRoomSize;
        damping = defaultDamping;
        wetLevel = defaultWetLevel;
        dryLevel = defaultDryLevel;
        width = defaultWidth;
    }

    void applySmoothedParameters (bool forceImmediate, int numSamplesToSkip = 0)
    {
        const auto effectiveWet = juce::jlimit (0.0f, 1.0f, defaultWetLevel + wetModulation);
        const auto effectiveDamping = juce::jlimit (0.0f, 1.0f, defaultDamping + dampingModulation);

        if (forceImmediate)
        {
            smoothedRoomSize.setCurrentAndTargetValue (roomSize);
            smoothedDamping.setCurrentAndTargetValue (effectiveDamping);
            smoothedWetLevel.setCurrentAndTargetValue (effectiveWet);
            smoothedDryLevel.setCurrentAndTargetValue (dryLevel);
            smoothedWidth.setCurrentAndTargetValue (width);
        }
        else
        {
            smoothedDamping.setTargetValue (effectiveDamping);
            smoothedWetLevel.setTargetValue (effectiveWet);
        }

        if (numSamplesToSkip > 0)
        {
            smoothedRoomSize.skip (numSamplesToSkip);
            smoothedDamping.skip (numSamplesToSkip);
            smoothedWetLevel.skip (numSamplesToSkip);
            smoothedDryLevel.skip (numSamplesToSkip);
            smoothedWidth.skip (numSamplesToSkip);
        }

        juce::dsp::Reverb::Parameters params;
        params.roomSize = smoothedRoomSize.getCurrentValue();
        params.damping = smoothedDamping.getCurrentValue();
        params.wetLevel = smoothedWetLevel.getCurrentValue();
        params.dryLevel = smoothedDryLevel.getCurrentValue();
        params.width = smoothedWidth.getCurrentValue();
        params.freezeMode = 0.0f;

        reverb.setParameters (params);
    }

    juce::dsp::Reverb reverb;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedRoomSize;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDamping;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedWetLevel;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDryLevel;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedWidth;

    float roomSize = defaultRoomSize;
    float damping = defaultDamping;
    float wetLevel = defaultWetLevel;
    float dryLevel = defaultDryLevel;
    float width = defaultWidth;
    float wetModulation = 0.0f;
    float dampingModulation = 0.0f;
};
