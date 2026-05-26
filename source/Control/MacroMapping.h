#pragma once

#include "MacroManager.h"

#include <array>

class ModulationMatrix;

// Nonlinear macro shaping and default modulation depths for the matrix.
class MacroMapping
{
public:
    static constexpr float macroCurvePower = 3.0f;

    // Default connection depths (increased for clearer macro response).
    static constexpr float airCutoffDepthHz = 4200.0f;
    static constexpr float airReverbWetDepth = 0.20f;
    static constexpr float motionLfoDepth = 0.50f;
    static constexpr float widthChorusMixDepth = 0.22f;
    static constexpr float widthChorusDepth = 0.20f;
    static constexpr float warmthResonanceDepth = 0.20f;
    static constexpr float warmthReverbDampingDepth = 0.25f;

    static void configureDefaultRouting (ModulationMatrix& matrix);

    // Maps 0.0–1.0 macro to bipolar -1..1 with zero at 0.5.
    // Ease-out curve: more movement in the 0.6–1.0 (and 0.0–0.4) ranges.
    static float shapeMacroBipolar (float macroValue);

    static void setDebugEnabled (bool enabled);
    static bool isDebugEnabled();

    static float getLastRawMacro (int index);
    static float getLastShapedMacro (int index);

    static void updateDebugState (const MacroManager& macroManager);

private:
    static bool debugEnabled;
    static std::array<float, MacroManager::numMacros> lastRawMacros;
    static std::array<float, MacroManager::numMacros> lastShapedMacros;
};
