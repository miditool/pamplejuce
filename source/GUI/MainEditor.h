#pragma once

#include "Control/ParameterIds.h"
#include "MacroKnob.h"
#include "PluginProcessor.h"

#include <array>
#include <memory>

class MainEditor : public juce::AudioProcessorEditor,
                   private juce::AudioProcessorListener
{
public:
    explicit MainEditor (PluginProcessor& processor);
    ~MainEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void attachMacroKnob (MacroKnob& knob, const char* paramID, size_t attachmentIndex);
    void populatePresetSelector();
    void syncPresetSelector();
    void changePresetByOffset (int offset);

    void audioProcessorParameterChanged (juce::AudioProcessor*, int, float) override {}
    void audioProcessorChanged (juce::AudioProcessor*, const ChangeDetails& details) override;

    PluginProcessor& processorRef;

    juce::Label titleLabel;
    juce::Label macrosSectionLabel;
    juce::Label presetsSectionLabel;

    MacroKnob airKnob { "AIR" };
    MacroKnob motionKnob { "MOTION" };
    MacroKnob widthKnob { "WIDTH" };
    MacroKnob warmthKnob { "WARMTH" };
    MacroKnob spaceKnob { "SPACE", MacroManager::defaultSpaceValue };

    std::array<MacroKnob*, 5> macroKnobs { &airKnob, &motionKnob, &widthKnob, &warmthKnob, &spaceKnob };
    std::array<std::unique_ptr<juce::SliderParameterAttachment>, 5> macroAttachments;

    juce::ComboBox presetBox;
    juce::TextButton previousPresetButton { "<" };
    juce::TextButton nextPresetButton { ">" };

    juce::Rectangle<int> macrosSectionBounds;
    juce::Rectangle<int> presetsSectionBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainEditor)
};
