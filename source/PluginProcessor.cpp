#include "PluginProcessor.h"
#include "GUI/MainEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts (*this, nullptr, juce::Identifier { "PARAMETERS" }, createParameterLayout())
{
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return synthEngine.getTailLengthSeconds();
}

int PluginProcessor::getNumPrograms()
{
    return presetManager.getNumPresets();
}

int PluginProcessor::getCurrentProgram()
{
    return currentProgramIndex;
}

void PluginProcessor::setCurrentProgram (int index)
{
    if (applyPreset (index))
        updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withProgramChanged (true));
}

const juce::String PluginProcessor::getProgramName (int index)
{
    return presetManager.getPresetName (index);
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synthEngine.prepare (sampleRate, samplesPerBlock);
    updateMacrosFromParameters();
}

void PluginProcessor::releaseResources()
{
    synthEngine.reset();
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    updateMacrosFromParameters();
    synthEngine.renderNextBlock (buffer, midiMessages);
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new MainEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    const auto state = apvts.copyState();
    const std::unique_ptr<juce::XmlElement> xml (state.createXml());

    if (xml != nullptr)
        copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    const std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));

    updateMacrosFromParameters();
}

void PluginProcessor::setMacro (int index, float value)
{
    if (auto* param = apvts.getParameter (getMacroParamID (index)))
        param->setValueNotifyingHost (param->convertTo0to1 (value));
}

float PluginProcessor::getMacro (int index) const
{
    if (auto* param = apvts.getRawParameterValue (getMacroParamID (index)))
        return param->load();

    return MacroManager::defaultMacroValue;
}

MacroManager& PluginProcessor::getMacroManager()
{
    return synthEngine.getMacroManager();
}

const MacroManager& PluginProcessor::getMacroManager() const
{
    return synthEngine.getMacroManager();
}

ModulationMatrix& PluginProcessor::getModulationMatrix()
{
    return synthEngine.getModulationMatrix();
}

const ModulationMatrix& PluginProcessor::getModulationMatrix() const
{
    return synthEngine.getModulationMatrix();
}

void PluginProcessor::updateMacrosFromParameters()
{
    for (int i = 0; i < MacroManager::numMacros; ++i)
    {
        if (auto* param = apvts.getRawParameterValue (getMacroParamID (i)))
            synthEngine.setMacro (i, param->load());
    }
}

bool PluginProcessor::applyPreset (int index)
{
    if (index < 0 || index >= presetManager.getNumPresets())
        return false;

    const auto presetXml = presetManager.getPresetXml (index);

    if (presetXml == nullptr || ! presetXml->hasTagName (apvts.state.getType()))
        return false;

    apvts.replaceState (juce::ValueTree::fromXml (*presetXml));
    updateMacrosFromParameters();
    currentProgramIndex = index;
    return true;
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
