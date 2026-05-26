#include "MainEditor.h"

MainEditor::MainEditor (PluginProcessor& processor)
    : AudioProcessorEditor (&processor),
      processorRef (processor)
{
    titleLabel.setText (PRODUCT_NAME_WITHOUT_VERSION, juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    titleLabel.setFont (juce::FontOptions { 18.0f, juce::Font::bold });
    titleLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (titleLabel);

    macrosSectionLabel.setText ("Macros", juce::dontSendNotification);
    macrosSectionLabel.setJustificationType (juce::Justification::centredLeft);
    macrosSectionLabel.setFont (juce::FontOptions { 14.0f, juce::Font::bold });
    macrosSectionLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (macrosSectionLabel);

    engineSectionLabel.setText ("Synth Engine", juce::dontSendNotification);
    engineSectionLabel.setJustificationType (juce::Justification::centredLeft);
    engineSectionLabel.setFont (juce::FontOptions { 14.0f, juce::Font::bold });
    engineSectionLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (engineSectionLabel);

    configureMacroKnob (airKnob, MacroManager::Air);
    configureMacroKnob (motionKnob, MacroManager::Motion);
    configureMacroKnob (widthKnob, MacroManager::Width);
    configureMacroKnob (warmthKnob, MacroManager::Warmth);

    for (auto* knob : macroKnobs)
        addAndMakeVisible (knob);

    setSize (520, 360);
}

void MainEditor::configureMacroKnob (MacroKnob& knob, int macroIndex)
{
    knob.setValue (processorRef.getMacro (macroIndex), juce::dontSendNotification);
    knob.setOnValueChanged ([this, macroIndex] (float value)
    {
        processorRef.setMacro (macroIndex, value);
    });
}

void MainEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour { 0xff1a1a1e });

    g.setColour (juce::Colour { 0xff2a2a32 });
    g.fillRoundedRectangle (macrosSectionBounds.toFloat(), 6.0f);
    g.fillRoundedRectangle (engineSectionBounds.toFloat(), 6.0f);

    g.setColour (juce::Colour { 0xff3a3a44 });
    g.drawRoundedRectangle (macrosSectionBounds.toFloat(), 6.0f, 1.0f);
    g.drawRoundedRectangle (engineSectionBounds.toFloat(), 6.0f, 1.0f);
}

void MainEditor::resized()
{
    auto bounds = getLocalBounds().reduced (16);

    titleLabel.setBounds (bounds.removeFromTop (28));
    bounds.removeFromTop (8);

    macrosSectionBounds = bounds.removeFromTop (160);
    bounds.removeFromTop (12);
    engineSectionBounds = bounds;

    auto macrosContent = macrosSectionBounds.reduced (12);
    macrosSectionLabel.setBounds (macrosContent.removeFromTop (22));
    macrosContent.removeFromTop (4);

    const auto knobWidth = macrosContent.getWidth() / static_cast<int> (macroKnobs.size());

    for (auto* knob : macroKnobs)
    {
        auto knobArea = macrosContent.removeFromLeft (knobWidth).reduced (6, 0);
        knob->setBounds (knobArea);
    }

    auto engineContent = engineSectionBounds.reduced (12);
    engineSectionLabel.setBounds (engineContent.removeFromTop (22));
}
