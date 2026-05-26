#pragma once

#include "ChorusSection.h"
#include "Reverb.h"

#include <juce_dsp/juce_dsp.h>

// Post-voice effects: chorus widening, then reverb ambience.
class EffectChain
{
public:
    void prepare (double sampleRate, int samplesPerBlock)
    {
        chorusSection.prepare (sampleRate, samplesPerBlock);
        reverb.prepare (sampleRate, samplesPerBlock);
    }

    void reset()
    {
        chorusSection.reset();
        reverb.reset();
    }

    void processBlock (juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() < 2)
            return;

        auto block = juce::dsp::AudioBlock<float> (buffer).getSubBlock (0, static_cast<size_t> (buffer.getNumSamples()));
        chorusSection.processBlock (block);
        reverb.processBlock (block);
    }

    ChorusSection& getChorusSection() { return chorusSection; }

    Reverb& getReverb() { return reverb; }

private:
    ChorusSection chorusSection;
    Reverb reverb;
};
