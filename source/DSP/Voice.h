#pragma once

#include "Filter.h"
#include "ModulationRouter.h"
#include "SineSubLayer.h"
#include "UnisonSection.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class ModulationMatrix;

// Every note the synth can play uses this sound type.
class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int /*midiNoteNumber*/) override { return true; }

    bool appliesToChannel (int /*midiChannel*/) override { return true; }
};

// One polyphonic voice: oscillators -> filter -> envelope. Chorus/reverb run on the engine bus.
class SynthVoice : public juce::SynthesiserVoice
{
public:
    static constexpr float defaultAttack = 0.5f;
    static constexpr float defaultDecay = 0.3f;
    static constexpr float defaultSustain = 0.8f;
    static constexpr float defaultRelease = 1.5f;

    void prepare (double sampleRate, int samplesPerBlock);
    void setModulationMatrix (ModulationMatrix* matrix);

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    static float getTailLengthSeconds();

    Filter& getFilter();
    ModulationRouter& getModulationRouter();
    float getEnvelopeValue() const;
    float getVelocity() const;
    void setPitchModulationSemitones (float semitones);

    UnisonSection& getUnisonSection() { return unisonSection; }

    const UnisonConfig& getUnisonConfig() const { return unisonSection.getUnisonConfig(); }

private:
    UnisonSection unisonSection;
    SineSubLayer sineSubLayer;
    Filter filter;
    ModulationRouter modulationRouter;
    juce::ADSR adsr;
    ModulationMatrix* modulationMatrix = nullptr;
    float currentFrequency = 440.0f;
    float currentLevel = 0.0f;
    float currentEnvelopeValue = 0.0f;
};
