#pragma once

#include "Control/MacroManager.h"
#include "Control/ModulationMatrix.h"
#include "Control/Parameters.h"
#include "Control/PresetManager.h"
#include "DSP/SynthEngine.h"

#include <juce_audio_processors/juce_audio_processors.h>

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void setMacro (int index, float value);
    float getMacro (int index) const;

    MacroManager& getMacroManager();
    const MacroManager& getMacroManager() const;

    ModulationMatrix& getModulationMatrix();
    const ModulationMatrix& getModulationMatrix() const;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    const juce::AudioProcessorValueTreeState& getAPVTS() const { return apvts; }

    const PresetManager& getPresetManager() const { return presetManager; }

private:
    void updateMacrosFromParameters();
    bool applyPreset (int index);

    SynthEngine synthEngine;
    PresetManager presetManager;
    juce::AudioProcessorValueTreeState apvts;
    int currentProgramIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
