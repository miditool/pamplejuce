#include "ModulationMatrix.h"

#include "../DSP/EffectChain.h"
#include "../DSP/LFO.h"
#include "../DSP/Voice.h"
#include "MacroMapping.h"

#include <juce_audio_basics/juce_audio_basics.h>

void ModulationMatrix::prepare (double /*sampleRate*/)
{
    loadDefaultRouting();
}

void ModulationMatrix::clearConnections()
{
    for (auto& connection : connections)
        connection = {};
}

void ModulationMatrix::loadDefaultRouting()
{
    MacroMapping::configureDefaultRouting (*this);
}

int ModulationMatrix::addConnection (ModSourceID source, ModDestinationID destination, float amount)
{
    for (int i = 0; i < maxConnections; ++i)
    {
        if (! connections[static_cast<size_t> (i)].active)
        {
            connections[static_cast<size_t> (i)] = { source, destination, amount, true };
            return i;
        }
    }

    return -1;
}

bool ModulationMatrix::removeConnection (int connectionIndex)
{
    if (connectionIndex < 0 || connectionIndex >= maxConnections)
        return false;

    connections[static_cast<size_t> (connectionIndex)].active = false;
    return true;
}

const std::array<ModConnection, ModulationMatrix::maxConnections>& ModulationMatrix::getConnections() const
{
    return connections;
}

void ModulationMatrix::setConnections (const std::array<ModConnection, ModulationMatrix::maxConnections>& newConnections)
{
    connections = newConnections;
}

void ModulationMatrix::evaluateBlock (const MacroManager& macroManager)
{
    MacroMapping::updateDebugState (macroManager);
    clearAccumulators();

    for (const auto& connection : connections)
    {
        if (! connection.active || isAudioRateDestination (connection.destination))
            continue;

        const auto sourceValue = MacroMapping::getSourceBipolar (connection.source, macroManager);
        const auto contribution = sourceValue * connection.amount;

        if (isEngineDestination (connection.destination))
        {
            engineState.blockValues[static_cast<size_t> (connection.destination)] += contribution;
        }
        else if (isVoiceDestination (connection.destination))
        {
            for (auto& voiceState : voiceStates)
                voiceState.blockValues[static_cast<size_t> (connection.destination)] += contribution;
        }
    }

    MacroMapping::accumulateGlueModulations (macroManager, *this);
}

void ModulationMatrix::applyBlockModulations (juce::Synthesiser& synthesiser, EffectChain& effectChain)
{
    for (int voiceIndex = 0; voiceIndex < synthesiser.getNumVoices(); ++voiceIndex)
    {
        if (auto* voice = dynamic_cast<SynthVoice*> (synthesiser.getVoice (voiceIndex)))
        {
            const auto& voiceState = voiceStates[static_cast<size_t> (voiceIndex)];

            voice->getFilter().setCutoffModulation (
                voiceState.blockValues[static_cast<size_t> (ModDestinationID::FilterCutoff)]);

            voice->getFilter().setResonanceModulation (
                voiceState.blockValues[static_cast<size_t> (ModDestinationID::FilterResonance)]);

            voice->getModulationRouter().getFilterCutoffLfo().setDepthModulation (
                voiceState.blockValues[static_cast<size_t> (ModDestinationID::LFODepth)]);

            voice->setPitchModulationSemitones (
                voiceState.blockValues[static_cast<size_t> (ModDestinationID::OscillatorPitch)]);
        }
    }

    auto& chorus = effectChain.getChorusSection();
    chorus.setMixModulation (engineState.blockValues[static_cast<size_t> (ModDestinationID::ChorusMix)]);
    chorus.setDepthModulation (engineState.blockValues[static_cast<size_t> (ModDestinationID::ChorusDepth)]);

    auto& reverb = effectChain.getReverb();
    reverb.setWetModulation (engineState.blockValues[static_cast<size_t> (ModDestinationID::ReverbWet)]);
    reverb.setDampingModulation (engineState.blockValues[static_cast<size_t> (ModDestinationID::ReverbDamping)]);
}

float ModulationMatrix::evaluateVoiceDestination (SynthVoice& voice, ModDestinationID destination)
{
    if (destination != ModDestinationID::FilterCutoff)
        return 0.0f;

    float total = 0.0f;
    auto& lfo = voice.getModulationRouter().getFilterCutoffLfo();
    const auto bipolar = lfo.processSample();
    const auto depth = lfo.getNextSmoothedDepth();

    for (const auto& connection : connections)
    {
        if (! connection.active)
            continue;

        if (connection.source == ModSourceID::LFO1 && connection.destination == destination)
            total += bipolar * depth * connection.amount;
        else if (connection.source == ModSourceID::Envelope && connection.destination == destination)
            total += voice.getEnvelopeValue() * connection.amount;
        else if (connection.source == ModSourceID::Velocity && connection.destination == destination)
            total += voice.getVelocity() * connection.amount;
    }

    return total;
}

void ModulationMatrix::clearAccumulators()
{
    for (auto& voiceState : voiceStates)
        voiceState.blockValues.fill (0.0f);

    engineState.blockValues.fill (0.0f);
}
