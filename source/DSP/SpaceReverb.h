#pragma once

#include <juce_dsp/juce_dsp.h>

// Dedicated cinematic reverb layer controlled by the SPACE macro.
// Wet-only parallel insert — dry signal passes through unchanged.
// Send path: tap → HPF → LPF → pre-delay → reverb → wet return.
class SpaceReverb
{
public:
    static constexpr float tailLengthSeconds = 10.0f;
    static constexpr float bypassThreshold = 0.002f;
    static constexpr float maxOutputGain = 0.62f;
    static constexpr float maxPreDelayMs = 62.0f;
    static constexpr float minPreDelayMs = 8.0f;

    static constexpr float sendHighPassHz = 800.0f;
    static constexpr float sendLowPassHz = 5000.0f;
    static constexpr float sendFilterResonance = 0.62f;

    void prepare (double sampleRate, int samplesPerBlock)
    {
        sampleRateHz = sampleRate;
        maxBlockSize = samplesPerBlock;

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
        spec.numChannels = 2;

        reverb.prepare (spec);

        juce::dsp::ProcessSpec monoSpec;
        monoSpec.sampleRate = sampleRate;
        monoSpec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
        monoSpec.numChannels = 1;

        for (auto* filter : { &leftHighPass, &rightHighPass, &leftLowPass, &rightLowPass })
        {
            filter->prepare (monoSpec);
            filter->reset();
        }

        configureSendFilters();

        constexpr double spaceParameterSmoothingSeconds = 0.25;
        smoothedSpaceAmount.reset (sampleRate, spaceParameterSmoothingSeconds);
        smoothedSpaceAmount.setCurrentAndTargetValue (0.0f);

        const auto maxPreDelaySamples = static_cast<int> (std::ceil (sampleRate * maxPreDelayMs / 1000.0)) + 1;

        juce::dsp::ProcessSpec delaySpec;
        delaySpec.sampleRate = sampleRate;
        delaySpec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
        delaySpec.numChannels = 1;

        leftPreDelay.setMaximumDelayInSamples (maxPreDelaySamples);
        leftPreDelay.prepare (delaySpec);
        rightPreDelay.setMaximumDelayInSamples (maxPreDelaySamples);
        rightPreDelay.prepare (delaySpec);

        wetBuffer.setSize (2, samplesPerBlock);

        reset();
    }

    void reset()
    {
        reverb.reset();
        leftPreDelay.reset();
        rightPreDelay.reset();
        leftHighPass.reset();
        rightHighPass.reset();
        leftLowPass.reset();
        rightLowPass.reset();
        spaceAmountTarget = 0.0f;
        smoothedSpaceAmount.setCurrentAndTargetValue (0.0f);
    }

    void setSpaceAmount (float amount)
    {
        spaceAmountTarget = juce::jlimit (0.0f, 1.0f, amount);
        smoothedSpaceAmount.setTargetValue (spaceAmountTarget);
    }

    static float getTailLengthSeconds()
    {
        return tailLengthSeconds;
    }

    void processBlock (juce::dsp::AudioBlock<float> block)
    {
        if (block.getNumChannels() < 2)
            return;

        const auto numSamples = static_cast<int> (block.getNumSamples());
        jassert (numSamples <= maxBlockSize);

        smoothedSpaceAmount.skip (numSamples);
        const auto amount = smoothedSpaceAmount.getCurrentValue();

        if (amount < bypassThreshold)
            return;

        auto* leftIn = block.getChannelPointer (0);
        auto* rightIn = block.getChannelPointer (1);
        auto* leftWet = wetBuffer.getWritePointer (0);
        auto* rightWet = wetBuffer.getWritePointer (1);

        const auto preDelayMs = minPreDelayMs + amount * (maxPreDelayMs - minPreDelayMs);
        const auto preDelaySamples = static_cast<int> (preDelayMs * sampleRateHz / 1000.0);
        leftPreDelay.setDelay (static_cast<float> (preDelaySamples));
        rightPreDelay.setDelay (static_cast<float> (preDelaySamples));

        for (int i = 0; i < numSamples; ++i)
        {
            const auto filteredLeft = leftLowPass.processSample (
                0,
                leftHighPass.processSample (0, leftIn[i]));

            const auto filteredRight = rightLowPass.processSample (
                0,
                rightHighPass.processSample (0, rightIn[i]));

            leftPreDelay.pushSample (0, filteredLeft);
            rightPreDelay.pushSample (0, filteredRight);
            leftWet[i] = leftPreDelay.popSample (0);
            rightWet[i] = rightPreDelay.popSample (0);
        }

        juce::dsp::Reverb::Parameters params;
        params.roomSize = 0.68f + amount * 0.31f;
        params.damping = 0.74f - amount * 0.46f;
        params.wetLevel = 1.0f;
        params.dryLevel = 0.0f;
        params.width = 0.72f + amount * 0.28f;
        params.freezeMode = 0.0f;
        reverb.setParameters (params);

        auto wetBlock = juce::dsp::AudioBlock<float> (wetBuffer).getSubBlock (0, block.getNumSamples());
        juce::dsp::ProcessContextReplacing<float> context (wetBlock);
        reverb.process (context);

        const auto mixGain = amount * maxOutputGain;

        for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
        {
            auto* destination = block.getChannelPointer (channel);
            const auto* source = wetBuffer.getReadPointer (static_cast<int> (channel));

            for (int i = 0; i < numSamples; ++i)
                destination[i] += source[i] * mixGain;
        }
    }

private:
    void configureSendFilters()
    {
        leftHighPass.setType (juce::dsp::StateVariableTPTFilterType::highpass);
        rightHighPass.setType (juce::dsp::StateVariableTPTFilterType::highpass);
        leftLowPass.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        rightLowPass.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

        for (auto* filter : { &leftHighPass, &rightHighPass })
        {
            filter->setCutoffFrequency (sendHighPassHz);
            filter->setResonance (sendFilterResonance);
        }

        for (auto* filter : { &leftLowPass, &rightLowPass })
        {
            filter->setCutoffFrequency (sendLowPassHz);
            filter->setResonance (sendFilterResonance);
        }
    }

    juce::dsp::Reverb reverb;
    juce::dsp::StateVariableTPTFilter<float> leftHighPass;
    juce::dsp::StateVariableTPTFilter<float> rightHighPass;
    juce::dsp::StateVariableTPTFilter<float> leftLowPass;
    juce::dsp::StateVariableTPTFilter<float> rightLowPass;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> leftPreDelay;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> rightPreDelay;
    juce::AudioBuffer<float> wetBuffer;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedSpaceAmount;

    double sampleRateHz = 44100.0;
    int maxBlockSize = 512;
    float spaceAmountTarget = 0.0f;
};
