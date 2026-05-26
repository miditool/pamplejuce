#pragma once

#include "MacroManager.h"
#include "ModulationIds.h"

#include <array>

class ModulationMatrix;

// Nonlinear macro shaping, perceptual glue, and default modulation depths for the matrix.
class MacroMapping
{
public:
    // Piecewise knees: stable 0.32–0.68 mid-band, stronger contrast toward 0.0 and 1.0.
    static constexpr float macroCenter = 0.5f;
    static constexpr float macroUpperKnee = 0.68f;
    static constexpr float macroLowerKnee = 0.32f;
    static constexpr float macroKneeOutput = 0.21f;

    static constexpr float airGentlePower = 2.15f;
    static constexpr float airAggressivePower = 6.2f;

    static constexpr float motionGentlePower = 1.92f;
    static constexpr float motionAggressivePower = 6.0f;

    static constexpr float widthGentlePower = 2.45f;
    static constexpr float widthAggressivePower = 6.5f;

    static constexpr float warmthGentlePower = 2.25f;
    static constexpr float warmthAggressivePower = 5.8f;

    // AIR — cutoff-dominant brightness; minimal reverb overlap with SPACE.
    // Depths tuned for 400–2000 Hz pad range (default 1050 Hz, neutral at 0.5).
    static constexpr float airCutoffDepthHz = 1120.0f;
    static constexpr float airReverbWetDepth = 0.04f;
    static constexpr float airResonanceClarityDepth = -0.05f;
    static constexpr float airReverbDampingDepth = -0.04f;

    // Envelope swell into filter for emotional pad attacks.
    static constexpr float envelopeCutoffDepthHz = 480.0f;

    // MOTION — LFO depth primary; chorus depth adds visible animation layer.
    static constexpr float motionLfoDepth = 0.92f;
    static constexpr float motionChorusDepth = 0.38f;

    // WIDTH — chorus mix/depth only; strong mono ↔ wide stereo contrast.
    static constexpr float widthChorusMixDepth = 0.52f;
    static constexpr float widthChorusDepth = 0.48f;

    // WARMTH — cutoff darkening + resonance softening primary; reverb damping secondary.
    static constexpr float warmthResonanceDepth = -0.38f;
    static constexpr float warmthCutoffDepthHz = -680.0f;
    static constexpr float warmthReverbDampingDepth = 0.24f;

    // SPACE — unidirectional ease-out (off at 0.0, cinematic expansion at 1.0).
    static constexpr float spaceEasePower = 1.92f;

    // Perceptual glue — soft cross-domain bleed (~5–12% of primary depths).
    static constexpr float glueInnerEdge = 0.10f;
    static constexpr float glueActivationPower = 2.8f;
    static constexpr float glueSpaceOnset = 0.20f;

    static constexpr float glueAirToReverbWet = 0.004f;
    static constexpr float glueAirToChorusMix = 0.035f;
    static constexpr float glueAirToSpaceAmount = 0.036f;

    static constexpr float glueSpaceToReverbWet = 0.012f;
    static constexpr float glueSpaceToChorusDepth = 0.030f;

    static constexpr float glueWidthToLfoDepth = 0.075f;

    static constexpr float glueMotionToReverbDamping = 0.028f;

    static constexpr float glueWarmthToResonance = -0.030f;
    static constexpr float glueWarmthToReverbDamping = 0.022f;

    static void configureDefaultRouting (ModulationMatrix& matrix);

    static void accumulateGlueModulations (const MacroManager& macroManager,
                                           ModulationMatrix& matrix);

    static float getSourceBipolar (ModSourceID source, const MacroManager& macroManager);

    static float getGlueBipolar (ModSourceID source, const MacroManager& macroManager);

    // Maps 0.0–1.0 macro to bipolar -1..1 with zero at 0.5.
    static float shapeMacroBipolar (float macroValue);

    // Unidirectional 0→1 shaping for the SPACE macro (silent at 0.0).
    static float shapeSpaceMacro (float macroValue);

    // SPACE amount with subtle AIR→SPACE glue (neutral when macros are centered).
    static float computeEffectiveSpaceAmount (const MacroManager& macroManager);

    static void setDebugEnabled (bool enabled);
    static bool isDebugEnabled();

    static float getLastRawMacro (int index);
    static float getLastShapedMacro (int index);

    static void updateDebugState (const MacroManager& macroManager);

private:
    static float shapeMacroMusical (float macroValue, float gentlePower, float aggressivePower);
    static float shapeGlueActivation (float macroValue);
    static float shapeGlueActivationUnidirectional (float macroValue);
    static float getGlueUnidirectionalSpace (const MacroManager& macroManager);

    static bool debugEnabled;
    static std::array<float, MacroManager::numMacros> lastRawMacros;
    static std::array<float, MacroManager::numMacros> lastShapedMacros;
};
