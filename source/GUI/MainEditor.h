#pragma once

#include "MacroKnob.h"
#include "PluginProcessor.h"

class MainEditor : public juce::AudioProcessorEditor
{
public:
    explicit MainEditor (PluginProcessor& processor);
    ~MainEditor() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void configureMacroKnob (MacroKnob& knob, int macroIndex);

    PluginProcessor& processorRef;

    juce::Label titleLabel;
    juce::Label macrosSectionLabel;
    juce::Label engineSectionLabel;

    MacroKnob airKnob { "AIR" };
    MacroKnob motionKnob { "MOTION" };
    MacroKnob widthKnob { "WIDTH" };
    MacroKnob warmthKnob { "WARMTH" };

    std::array<MacroKnob*, 4> macroKnobs { &airKnob, &motionKnob, &widthKnob, &warmthKnob };

    juce::Rectangle<int> macrosSectionBounds;
    juce::Rectangle<int> engineSectionBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainEditor)
};
