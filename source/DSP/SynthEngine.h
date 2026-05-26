#pragma once

#include "Control/MacroManager.h"
#include "Control/MacroMapping.h"
#include "Control/ModulationMatrix.h"
#include "EffectChain.h"
#include "Voice.h"

#include <juce_audio_basics/juce_audio_basics.h>

// Owns the JUCE Synthesiser and all voices. The processor delegates audio/MIDI here.
class SynthEngine
{
public:
    static constexpr int maxVoices = 16;

    void prepare (double sampleRate, int samplesPerBlock)
    {
        synthesiser.setCurrentPlaybackSampleRate (sampleRate);

        if (synthesiser.getNumSounds() == 0)
        {
            synthesiser.addSound (new SynthSound());

            for (int i = 0; i < maxVoices; ++i)
                synthesiser.addVoice (new SynthVoice());
        }

        for (int i = 0; i < synthesiser.getNumVoices(); ++i)
            if (auto* voice = dynamic_cast<SynthVoice*> (synthesiser.getVoice (i)))
            {
                voice->prepare (sampleRate, samplesPerBlock);
                voice->setModulationMatrix (&modulationMatrix);
            }

        effectChain.prepare (sampleRate, samplesPerBlock);
        macroManager.prepare (sampleRate);
        modulationMatrix.prepare (sampleRate);
        MacroMapping::configureDefaultRouting (modulationMatrix);
        applyModulation (0);
    }

    void reset()
    {
        synthesiser.allNotesOff (0, true);
        effectChain.reset();
    }

    void setMacro (int index, float value)
    {
        macroManager.setMacro (index, value);
    }

    float getMacro (int index) const
    {
        return macroManager.getMacro (index);
    }

    MacroManager& getMacroManager() { return macroManager; }

    const MacroManager& getMacroManager() const { return macroManager; }

    ModulationMatrix& getModulationMatrix() { return modulationMatrix; }

    const ModulationMatrix& getModulationMatrix() const { return modulationMatrix; }

    void renderNextBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        applyModulation (buffer.getNumSamples());

        buffer.clear();

        synthesiser.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
        effectChain.processBlock (buffer);
    }

    float getTailLengthSeconds() const
    {
        return juce::jmax (SynthVoice::getTailLengthSeconds(), Reverb::getTailLengthSeconds());
    }

private:
    void applyModulation (int numSamples)
    {
        if (numSamples > 0)
            macroManager.skip (numSamples);

        modulationMatrix.evaluateBlock (macroManager);
        modulationMatrix.applyBlockModulations (synthesiser, effectChain);
    }

    juce::Synthesiser synthesiser;
    EffectChain effectChain;
    MacroManager macroManager;
    ModulationMatrix modulationMatrix;
};
