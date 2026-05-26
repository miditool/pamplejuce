#include "MacroMapping.h"

#include "ModulationMatrix.h"

#include "../DSP/LFO.h"

#include <juce_core/juce_core.h>
#include <cmath>
#include <algorithm>

bool MacroMapping::debugEnabled = false;
std::array<float, MacroManager::numMacros> MacroMapping::lastRawMacros {};
std::array<float, MacroManager::numMacros> MacroMapping::lastShapedMacros {};

void MacroMapping::configureDefaultRouting (ModulationMatrix& matrix)
{
    matrix.clearConnections();

    // AIR: cutoff-dominant brightness; reverb kept minimal to avoid SPACE overlap.
    matrix.addConnection (ModSourceID::MacroAir, ModDestinationID::FilterCutoff, airCutoffDepthHz);
    matrix.addConnection (ModSourceID::MacroAir, ModDestinationID::ReverbWet, airReverbWetDepth);
    matrix.addConnection (ModSourceID::MacroAir, ModDestinationID::FilterResonance, airResonanceClarityDepth);
    matrix.addConnection (ModSourceID::MacroAir, ModDestinationID::ReverbDamping, airReverbDampingDepth);

    // MOTION: LFO depth primary; chorus depth adds secondary animation.
    matrix.addConnection (ModSourceID::MacroMotion, ModDestinationID::LFODepth, motionLfoDepth);
    matrix.addConnection (ModSourceID::MacroMotion, ModDestinationID::ChorusDepth, motionChorusDepth);
    matrix.addConnection (ModSourceID::LFO1, ModDestinationID::FilterCutoff, LFO::maxCutoffModDepthHz);
    matrix.addConnection (ModSourceID::Envelope, ModDestinationID::FilterCutoff, envelopeCutoffDepthHz);

    // WIDTH: chorus mix + depth for mono ↔ wide stereo image only.
    matrix.addConnection (ModSourceID::MacroWidth, ModDestinationID::ChorusMix, widthChorusMixDepth);
    matrix.addConnection (ModSourceID::MacroWidth, ModDestinationID::ChorusDepth, widthChorusDepth);

    // WARMTH: cutoff darkening + resonance softening; reverb damping secondary.
    matrix.addConnection (ModSourceID::MacroWarmth, ModDestinationID::FilterResonance, warmthResonanceDepth);
    matrix.addConnection (ModSourceID::MacroWarmth, ModDestinationID::FilterCutoff, warmthCutoffDepthHz);
    matrix.addConnection (ModSourceID::MacroWarmth, ModDestinationID::ReverbDamping, warmthReverbDampingDepth);
}

void MacroMapping::accumulateGlueModulations (const MacroManager& macroManager, ModulationMatrix& matrix)
{
    const auto airGlue = getGlueBipolar (ModSourceID::MacroAir, macroManager);
    const auto motionGlue = getGlueBipolar (ModSourceID::MacroMotion, macroManager);
    const auto widthGlue = getGlueBipolar (ModSourceID::MacroWidth, macroManager);
    const auto warmthGlue = getGlueBipolar (ModSourceID::MacroWarmth, macroManager);
    const auto spaceGlue = getGlueUnidirectionalSpace (macroManager);

    auto& engineValues = matrix.getEngineModulationState().blockValues;

    engineValues[static_cast<size_t> (ModDestinationID::ReverbWet)]
        += airGlue * glueAirToReverbWet;

    if (airGlue > 0.0f)
    {
        engineValues[static_cast<size_t> (ModDestinationID::ChorusMix)]
            += airGlue * glueAirToChorusMix;
    }

    for (auto& voiceState : matrix.getVoiceModulationStates())
    {
        voiceState.blockValues[static_cast<size_t> (ModDestinationID::LFODepth)]
            += widthGlue * glueWidthToLfoDepth;
    }

    engineValues[static_cast<size_t> (ModDestinationID::ReverbDamping)]
        += motionGlue * glueMotionToReverbDamping;

    for (auto& voiceState : matrix.getVoiceModulationStates())
    {
        voiceState.blockValues[static_cast<size_t> (ModDestinationID::FilterResonance)]
            += warmthGlue * glueWarmthToResonance;
    }

    if (warmthGlue > 0.0f)
    {
        engineValues[static_cast<size_t> (ModDestinationID::ReverbDamping)]
            += warmthGlue * glueWarmthToReverbDamping;
    }

    engineValues[static_cast<size_t> (ModDestinationID::ReverbWet)]
        += spaceGlue * glueSpaceToReverbWet;

    engineValues[static_cast<size_t> (ModDestinationID::ChorusDepth)]
        += spaceGlue * glueSpaceToChorusDepth;
}

float MacroMapping::getGlueBipolar (ModSourceID source, const MacroManager& macroManager)
{
    const auto activation = [&macroManager] (int index, float gentlePower, float aggressivePower)
    {
        const auto raw = macroManager.getSmoothedMacro (index);
        const auto glueActivation = shapeGlueActivation (raw);

        if (glueActivation <= 0.0f)
            return 0.0f;

        return shapeMacroMusical (raw, gentlePower, aggressivePower) * glueActivation;
    };

    switch (source)
    {
        case ModSourceID::MacroAir:
            return activation (MacroManager::Air, airGentlePower, airAggressivePower);

        case ModSourceID::MacroMotion:
            return activation (MacroManager::Motion, motionGentlePower, motionAggressivePower);

        case ModSourceID::MacroWidth:
            return activation (MacroManager::Width, widthGentlePower, widthAggressivePower);

        case ModSourceID::MacroWarmth:
            return activation (MacroManager::Warmth, warmthGentlePower, warmthAggressivePower);

        default:
            return 0.0f;
    }
}

float MacroMapping::computeEffectiveSpaceAmount (const MacroManager& macroManager)
{
    auto amount = shapeSpaceMacro (macroManager.getSmoothedMacro (MacroManager::Space));

    const auto airGlue = getGlueBipolar (ModSourceID::MacroAir, macroManager);

    if (airGlue > 0.0f)
        amount += airGlue * glueAirToSpaceAmount;

    return juce::jlimit (0.0f, 1.0f, amount);
}

float MacroMapping::shapeGlueActivation (float macroValue)
{
    if (macroValue == macroCenter)
        return 0.0f;

    const auto distance = std::abs (macroValue - macroCenter);

    if (distance <= glueInnerEdge)
        return 0.0f;

    const auto t = (distance - glueInnerEdge) / (0.5f - glueInnerEdge);
    return std::pow (std::clamp (t, 0.0f, 1.0f), glueActivationPower);
}

float MacroMapping::shapeGlueActivationUnidirectional (float macroValue)
{
    if (macroValue <= 0.0f)
        return 0.0f;

    macroValue = std::clamp (macroValue, 0.0f, 1.0f);

    if (macroValue <= glueSpaceOnset)
        return 0.0f;

    const auto t = (macroValue - glueSpaceOnset) / (1.0f - glueSpaceOnset);
    return std::pow (std::clamp (t, 0.0f, 1.0f), glueActivationPower);
}

float MacroMapping::getGlueUnidirectionalSpace (const MacroManager& macroManager)
{
    const auto raw = macroManager.getSmoothedMacro (MacroManager::Space);
    const auto activation = shapeGlueActivationUnidirectional (raw);

    if (activation <= 0.0f)
        return 0.0f;

    return shapeSpaceMacro (raw) * activation;
}

float MacroMapping::getSourceBipolar (ModSourceID source, const MacroManager& macroManager)
{
    switch (source)
    {
        case ModSourceID::MacroAir:
            return shapeMacroMusical (macroManager.getSmoothedMacro (MacroManager::Air),
                                      airGentlePower,
                                      airAggressivePower);

        case ModSourceID::MacroMotion:
            return shapeMacroMusical (macroManager.getSmoothedMacro (MacroManager::Motion),
                                      motionGentlePower,
                                      motionAggressivePower);

        case ModSourceID::MacroWidth:
            return shapeMacroMusical (macroManager.getSmoothedMacro (MacroManager::Width),
                                      widthGentlePower,
                                      widthAggressivePower);

        case ModSourceID::MacroWarmth:
            return shapeMacroMusical (macroManager.getSmoothedMacro (MacroManager::Warmth),
                                      warmthGentlePower,
                                      warmthAggressivePower);

        default:
            return 0.0f;
    }
}

float MacroMapping::shapeMacroBipolar (float macroValue)
{
    return shapeMacroMusical (macroValue, motionGentlePower, motionAggressivePower);
}

float MacroMapping::shapeSpaceMacro (float macroValue)
{
    if (macroValue <= 0.0f)
        return 0.0f;

    macroValue = std::clamp (macroValue, 0.0f, 1.0f);
    return 1.0f - std::pow (1.0f - macroValue, spaceEasePower);
}

float MacroMapping::shapeMacroMusical (float macroValue, float gentlePower, float aggressivePower)
{
    if (macroValue == macroCenter)
        return 0.0f;

    if (macroValue > macroCenter)
    {
        if (macroValue <= macroUpperKnee)
        {
            const float t = (macroValue - macroCenter) / (macroUpperKnee - macroCenter);
            return std::pow (t, gentlePower) * macroKneeOutput;
        }

        const float t = (macroValue - macroUpperKnee) / (1.0f - macroUpperKnee);
        return macroKneeOutput + (1.0f - macroKneeOutput)
             * (1.0f - std::pow (1.0f - t, aggressivePower));
    }

    if (macroValue >= macroLowerKnee)
    {
        const float t = (macroCenter - macroValue) / (macroCenter - macroLowerKnee);
        return -std::pow (t, gentlePower) * macroKneeOutput;
    }

    const float t = (macroLowerKnee - macroValue) / macroLowerKnee;
    return -(macroKneeOutput + (1.0f - macroKneeOutput)
            * (1.0f - std::pow (1.0f - t, aggressivePower)));
}

void MacroMapping::setDebugEnabled (bool enabled)
{
    debugEnabled = enabled;
}

bool MacroMapping::isDebugEnabled()
{
    return debugEnabled;
}

float MacroMapping::getLastRawMacro (int index)
{
    if (index >= 0 && index < MacroManager::numMacros)
        return lastRawMacros[static_cast<size_t> (index)];

    return MacroManager::defaultMacroValue;
}

float MacroMapping::getLastShapedMacro (int index)
{
    if (index >= 0 && index < MacroManager::numMacros)
        return lastShapedMacros[static_cast<size_t> (index)];

    return 0.0f;
}

void MacroMapping::updateDebugState (const MacroManager& macroManager)
{
    bool changed = false;

    for (int i = 0; i < MacroManager::numMacros; ++i)
    {
        const auto raw = macroManager.getSmoothedMacro (i);
        const auto shaped = (i == MacroManager::Space)
            ? shapeSpaceMacro (raw)
            : getSourceBipolar (static_cast<ModSourceID> (static_cast<int> (ModSourceID::MacroAir) + i),
                                macroManager);

        if (raw != lastRawMacros[static_cast<size_t> (i)]
            || shaped != lastShapedMacros[static_cast<size_t> (i)])
            changed = true;

        lastRawMacros[static_cast<size_t> (i)] = raw;
        lastShapedMacros[static_cast<size_t> (i)] = shaped;
    }

    if (debugEnabled && changed)
    {
        DBG ("Macro raw     AIR=" << lastRawMacros[0] << " MOTION=" << lastRawMacros[1]
             << " WIDTH=" << lastRawMacros[2] << " WARMTH=" << lastRawMacros[3]
             << " SPACE=" << lastRawMacros[4]);
        DBG ("Macro shaped  AIR=" << lastShapedMacros[0] << " MOTION=" << lastShapedMacros[1]
             << " WIDTH=" << lastShapedMacros[2] << " WARMTH=" << lastShapedMacros[3]
             << " SPACE=" << lastShapedMacros[4]);
    }
}
