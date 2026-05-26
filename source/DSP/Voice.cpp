#include "Voice.h"

#include "../Control/ModulationIds.h"
#include "../Control/ModulationMatrix.h"

void SynthVoice::prepare (double sampleRate, int samplesPerBlock)
{
    unisonSection.prepare (sampleRate, samplesPerBlock);
    filter.prepare (sampleRate, samplesPerBlock);
    modulationRouter.prepare (sampleRate, samplesPerBlock);

    adsr.setSampleRate (sampleRate);
    adsr.setParameters ({
        defaultAttack,
        defaultDecay,
        defaultSustain,
        defaultRelease,
    });
}

void SynthVoice::setModulationMatrix (ModulationMatrix* matrix)
{
    modulationMatrix = matrix;
}

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*> (sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity,
                            juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
    currentFrequency = static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
    currentLevel = velocity;

    unisonSection.setFrequency (currentFrequency);
    unisonSection.reset();
    filter.reset();
    modulationRouter.reset();
    adsr.noteOn();
}

void SynthVoice::stopNote (float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
        adsr.noteOff();
    else
    {
        adsr.reset();
        clearCurrentNote();
    }
}

void SynthVoice::pitchWheelMoved (int newPitchWheelValue)
{
    const auto pitchBendSemitones = (newPitchWheelValue - 8192) / 8192.0f * 2.0f;
    unisonSection.setFrequency (currentFrequency * std::pow (2.0f, pitchBendSemitones / 12.0f));
}

void SynthVoice::controllerMoved (int, int) {}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! adsr.isActive())
        return;

    const auto leftChannel = 0;
    const auto rightChannel = juce::jmax (0, outputBuffer.getNumChannels() - 1);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        if (! adsr.isActive())
        {
            clearCurrentNote();
            break;
        }

        currentEnvelopeValue = adsr.getNextSample();
        const auto gain = currentEnvelopeValue * currentLevel;
        const auto oscOutput = unisonSection.processSample();

        const auto cutoffModHz = modulationMatrix != nullptr
            ? modulationMatrix->evaluateVoiceDestination (*this, ModDestinationID::FilterCutoff)
            : 0.0f;

        const auto filtered = filter.processSample (oscOutput, cutoffModHz);

        outputBuffer.addSample (leftChannel, startSample + sample, filtered.left * gain);

        if (rightChannel != leftChannel)
            outputBuffer.addSample (rightChannel, startSample + sample, filtered.right * gain);
    }
}

float SynthVoice::getTailLengthSeconds()
{
    return defaultRelease;
}

Filter& SynthVoice::getFilter()
{
    return filter;
}

ModulationRouter& SynthVoice::getModulationRouter()
{
    return modulationRouter;
}

float SynthVoice::getEnvelopeValue() const
{
    return currentEnvelopeValue;
}

float SynthVoice::getVelocity() const
{
    return currentLevel;
}

void SynthVoice::setPitchModulationSemitones (float semitones)
{
    unisonSection.setPitchModulationSemitones (semitones);
}
