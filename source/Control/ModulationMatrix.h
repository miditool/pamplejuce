#pragma once

#include "MacroManager.h"
#include "ModConnection.h"
#include "ModulationIds.h"

#include <array>

class SynthVoice;
class EffectChain;

namespace juce { class Synthesiser; }

// Serum-style modulation matrix. All routing flows through connections here.
class ModulationMatrix
{
public:
    static constexpr int maxConnections = 32;
    static constexpr int maxVoices = 16;

    struct VoiceModulationState
    {
        std::array<float, static_cast<size_t> (ModDestinationID::Count)> blockValues {};
    };

    struct EngineModulationState
    {
        std::array<float, static_cast<size_t> (ModDestinationID::Count)> blockValues {};
    };

    void prepare (double sampleRate);

    void clearConnections();

    void loadDefaultRouting();

    int addConnection (ModSourceID source, ModDestinationID destination, float amount);

    bool removeConnection (int connectionIndex);

    const std::array<ModConnection, maxConnections>& getConnections() const;

    void setConnections (const std::array<ModConnection, maxConnections>& newConnections);

    void evaluateBlock (const MacroManager& macroManager);

    void applyBlockModulations (juce::Synthesiser& synthesiser, EffectChain& effectChain);

    float evaluateVoiceDestination (SynthVoice& voice, ModDestinationID destination);

private:
    void clearAccumulators();

    static float getBlockSourceValue (ModSourceID source, const MacroManager& macroManager);

    static float getMacroBipolar (const MacroManager& macroManager, int macroIndex);

    std::array<ModConnection, maxConnections> connections {};
    std::array<VoiceModulationState, maxVoices> voiceStates {};
    EngineModulationState engineState {};
};
