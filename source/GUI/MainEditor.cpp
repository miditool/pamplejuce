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

    presetsSectionLabel.setText ("Presets", juce::dontSendNotification);
    presetsSectionLabel.setJustificationType (juce::Justification::centredLeft);
    presetsSectionLabel.setFont (juce::FontOptions { 14.0f, juce::Font::bold });
    presetsSectionLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (presetsSectionLabel);

    attachMacroKnob (airKnob, ParamIDs::macroAir, 0);
    attachMacroKnob (motionKnob, ParamIDs::macroMotion, 1);
    attachMacroKnob (widthKnob, ParamIDs::macroWidth, 2);
    attachMacroKnob (warmthKnob, ParamIDs::macroWarmth, 3);
    attachMacroKnob (spaceKnob, ParamIDs::macroSpace, 4);

    for (auto* knob : macroKnobs)
        addAndMakeVisible (knob);

    populatePresetSelector();

    presetBox.onChange = [this]
    {
        processorRef.setCurrentProgram (presetBox.getSelectedItemIndex());
    };

    previousPresetButton.onClick = [this] { changePresetByOffset (-1); };
    nextPresetButton.onClick = [this] { changePresetByOffset (1); };

    addAndMakeVisible (presetBox);
    addAndMakeVisible (previousPresetButton);
    addAndMakeVisible (nextPresetButton);

    syncPresetSelector();
    processorRef.addListener (this);

    setSize (620, 360);
}

MainEditor::~MainEditor()
{
    processorRef.removeListener (this);
}

void MainEditor::attachMacroKnob (MacroKnob& knob, const char* paramID, size_t attachmentIndex)
{
    macroAttachments[attachmentIndex] = std::make_unique<juce::SliderParameterAttachment> (
        *processorRef.getAPVTS().getParameter (paramID),
        knob.getSlider());
}

void MainEditor::populatePresetSelector()
{
    presetBox.clear (juce::dontSendNotification);

    const auto& presetManager = processorRef.getPresetManager();

    for (int i = 0; i < presetManager.getNumPresets(); ++i)
        presetBox.addItem (presetManager.getPresetName (i), i + 1);
}

void MainEditor::syncPresetSelector()
{
    presetBox.setSelectedItemIndex (processorRef.getCurrentProgram(), juce::dontSendNotification);
}

void MainEditor::changePresetByOffset (int offset)
{
    const auto numPresets = processorRef.getNumPrograms();

    if (numPresets <= 0)
        return;

    const auto currentIndex = processorRef.getCurrentProgram();
    const auto nextIndex = (currentIndex + offset + numPresets) % numPresets;
    processorRef.setCurrentProgram (nextIndex);
    syncPresetSelector();
}

void MainEditor::audioProcessorChanged (juce::AudioProcessor*, const ChangeDetails& details)
{
    if (details.programChanged)
        syncPresetSelector();
}

void MainEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour { 0xff1a1a1e });

    g.setColour (juce::Colour { 0xff2a2a32 });
    g.fillRoundedRectangle (macrosSectionBounds.toFloat(), 6.0f);
    g.fillRoundedRectangle (presetsSectionBounds.toFloat(), 6.0f);

    g.setColour (juce::Colour { 0xff3a3a44 });
    g.drawRoundedRectangle (macrosSectionBounds.toFloat(), 6.0f, 1.0f);
    g.drawRoundedRectangle (presetsSectionBounds.toFloat(), 6.0f, 1.0f);
}

void MainEditor::resized()
{
    auto bounds = getLocalBounds().reduced (16);

    titleLabel.setBounds (bounds.removeFromTop (28));
    bounds.removeFromTop (8);

    macrosSectionBounds = bounds.removeFromTop (160);
    bounds.removeFromTop (12);
    presetsSectionBounds = bounds;

    auto macrosContent = macrosSectionBounds.reduced (12);
    macrosSectionLabel.setBounds (macrosContent.removeFromTop (22));
    macrosContent.removeFromTop (4);

    const auto knobWidth = macrosContent.getWidth() / static_cast<int> (macroKnobs.size());

    for (auto* knob : macroKnobs)
    {
        auto knobArea = macrosContent.removeFromLeft (knobWidth).reduced (6, 0);
        knob->setBounds (knobArea);
    }

    auto presetsContent = presetsSectionBounds.reduced (12);
    presetsSectionLabel.setBounds (presetsContent.removeFromTop (22));
    presetsContent.removeFromTop (8);

    auto presetControls = presetsContent.removeFromTop (28);
    previousPresetButton.setBounds (presetControls.removeFromLeft (32));
    presetControls.removeFromLeft (8);
    nextPresetButton.setBounds (presetControls.removeFromLeft (32));
    presetControls.removeFromLeft (8);
    presetBox.setBounds (presetControls);
}
