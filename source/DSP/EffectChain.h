#pragma once

#include "ChorusSection.h"
#include "Reverb.h"
#include "SpaceReverb.h"

#include <juce_dsp/juce_dsp.h>

// Post-voice effects: chorus widening, main reverb, then SPACE cinematic reverb.
class EffectChain
{
public:
    void prepare (double sampleRate, int samplesPerBlock)
    {
        chorusSection.prepare (sampleRate, samplesPerBlock);
        reverb.prepare (sampleRate, samplesPerBlock);
        spaceReverb.prepare (sampleRate, samplesPerBlock);
    }

    void reset()
    {
        chorusSection.reset();
        reverb.reset();
        spaceReverb.reset();
    }

    void setSpaceAmount (float amount)
    {
        spaceReverb.setSpaceAmount (amount);
    }

    void processBlock (juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() < 2)
            return;

        auto block = juce::dsp::AudioBlock<float> (buffer).getSubBlock (0, static_cast<size_t> (buffer.getNumSamples()));
        chorusSection.processBlock (block);
        reverb.processBlock (block);
        spaceReverb.processBlock (block);
    }

    ChorusSection& getChorusSection() { return chorusSection; }

    Reverb& getReverb() { return reverb; }

    SpaceReverb& getSpaceReverb() { return spaceReverb; }

private:
    ChorusSection chorusSection;
    Reverb reverb;
    SpaceReverb spaceReverb;
};
