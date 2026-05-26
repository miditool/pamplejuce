#include "MacroMapping.h"

#include "ModulationIds.h"
#include "ModulationMatrix.h"

#include "../DSP/LFO.h"

#include <cmath>

bool MacroMapping::debugEnabled = false;
std::array<float, MacroManager::numMacros> MacroMapping::lastRawMacros {};
std::array<float, MacroManager::numMacros> MacroMapping::lastShapedMacros {};

void MacroMapping::configureDefaultRouting (ModulationMatrix& matrix)
{
    matrix.clearConnections();

    matrix.addConnection (ModSourceID::MacroAir, ModDestinationID::FilterCutoff, airCutoffDepthHz);
    matrix.addConnection (ModSourceID::MacroWarmth, ModDestinationID::FilterResonance, warmthResonanceDepth);
    matrix.addConnection (ModSourceID::MacroMotion, ModDestinationID::LFODepth, motionLfoDepth);
    matrix.addConnection (ModSourceID::LFO1, ModDestinationID::FilterCutoff, LFO::maxCutoffModDepthHz);
    matrix.addConnection (ModSourceID::MacroAir, ModDestinationID::ReverbWet, airReverbWetDepth);
    matrix.addConnection (ModSourceID::MacroWidth, ModDestinationID::ChorusMix, widthChorusMixDepth);
    matrix.addConnection (ModSourceID::MacroWidth, ModDestinationID::ChorusDepth, widthChorusDepth);
    matrix.addConnection (ModSourceID::MacroWarmth, ModDestinationID::ReverbDamping, warmthReverbDampingDepth);
}

float MacroMapping::shapeMacroBipolar (float macroValue)
{
    const float bipolar = (macroValue - MacroManager::defaultMacroValue) * 2.0f;

    if (bipolar == 0.0f)
        return 0.0f;

    const auto sign = bipolar > 0.0f ? 1.0f : -1.0f;
    const auto absBipolar = std::abs (bipolar);

    return sign * (1.0f - std::pow (1.0f - absBipolar, macroCurvePower));
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
        const auto shaped = shapeMacroBipolar (raw);

        if (raw != lastRawMacros[static_cast<size_t> (i)]
            || shaped != lastShapedMacros[static_cast<size_t> (i)])
            changed = true;

        lastRawMacros[static_cast<size_t> (i)] = raw;
        lastShapedMacros[static_cast<size_t> (i)] = shaped;
    }

    if (debugEnabled && changed)
    {
        DBG ("Macro raw     AIR=" << lastRawMacros[0] << " MOTION=" << lastRawMacros[1]
             << " WIDTH=" << lastRawMacros[2] << " WARMTH=" << lastRawMacros[3]);
        DBG ("Macro shaped  AIR=" << lastShapedMacros[0] << " MOTION=" << lastShapedMacros[1]
             << " WIDTH=" << lastShapedMacros[2] << " WARMTH=" << lastShapedMacros[3]);
    }
}
