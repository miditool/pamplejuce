#include "helpers/test_helpers.h"
#include <PluginProcessor.h>
#include <Control/ParameterIds.h>
#include <Control/MacroManager.h>
#include <Control/MacroMapping.h>
#include <Control/ModulationIds.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/catch_approx.hpp>

TEST_CASE ("one is equal to one", "[dummy]")
{
    REQUIRE (1 == 1);
}

TEST_CASE ("Plugin instance", "[instance]")
{
    PluginProcessor testPlugin;

    SECTION ("name")
    {
        CHECK_THAT (testPlugin.getName().toStdString(),
            Catch::Matchers::Equals ("Atmospheric Liquid Pad"));
    }
}

TEST_CASE ("Macro parameters default to neutral", "[parameters]")
{
    PluginProcessor plugin;

    CHECK (plugin.getMacro (MacroManager::Air) == Catch::Approx (MacroManager::defaultMacroValue));
    CHECK (plugin.getMacro (MacroManager::Motion) == Catch::Approx (MacroManager::defaultMacroValue));
    CHECK (plugin.getMacro (MacroManager::Width) == Catch::Approx (MacroManager::defaultMacroValue));
    CHECK (plugin.getMacro (MacroManager::Warmth) == Catch::Approx (MacroManager::defaultMacroValue));
    CHECK (plugin.getMacro (MacroManager::Space) == Catch::Approx (MacroManager::defaultSpaceValue));
}

TEST_CASE ("SPACE macro shaping is silent at zero", "[parameters]")
{
    CHECK (MacroMapping::shapeSpaceMacro (0.0f) == Catch::Approx (0.0f));
    CHECK (MacroMapping::shapeSpaceMacro (1.0f) == Catch::Approx (1.0f));
    CHECK (MacroMapping::shapeSpaceMacro (0.10f) > 0.12f);
    CHECK (MacroMapping::shapeSpaceMacro (0.40f) > 0.45f);
    CHECK (MacroMapping::shapeSpaceMacro (0.70f) > 0.72f);
}

TEST_CASE ("Bipolar macros are neutral at center", "[parameters]")
{
    MacroManager macroManager;
    macroManager.prepare (44100.0);

    for (int i = 0; i < MacroManager::Space; ++i)
        macroManager.setMacro (i, MacroManager::defaultMacroValue);

    CHECK (MacroMapping::getSourceBipolar (ModSourceID::MacroAir, macroManager) == Catch::Approx (0.0f));
    CHECK (MacroMapping::getSourceBipolar (ModSourceID::MacroMotion, macroManager) == Catch::Approx (0.0f));
    CHECK (MacroMapping::getSourceBipolar (ModSourceID::MacroWidth, macroManager) == Catch::Approx (0.0f));
    CHECK (MacroMapping::getSourceBipolar (ModSourceID::MacroWarmth, macroManager) == Catch::Approx (0.0f));

    CHECK (MacroMapping::getGlueBipolar (ModSourceID::MacroAir, macroManager) == Catch::Approx (0.0f));
    CHECK (MacroMapping::getGlueBipolar (ModSourceID::MacroMotion, macroManager) == Catch::Approx (0.0f));
    CHECK (MacroMapping::getGlueBipolar (ModSourceID::MacroWidth, macroManager) == Catch::Approx (0.0f));
    CHECK (MacroMapping::getGlueBipolar (ModSourceID::MacroWarmth, macroManager) == Catch::Approx (0.0f));
    CHECK (MacroMapping::computeEffectiveSpaceAmount (macroManager) == Catch::Approx (0.0f));
}

TEST_CASE ("Macro glue stays inactive near center", "[parameters]")
{
    MacroManager macroManager;
    macroManager.prepare (44100.0);

    macroManager.setMacro (MacroManager::Air, 0.58f);
    macroManager.setMacro (MacroManager::Space, 0.12f);

    CHECK (std::abs (MacroMapping::getGlueBipolar (ModSourceID::MacroAir, macroManager)) < 0.02f);
    CHECK (MacroMapping::computeEffectiveSpaceAmount (macroManager) == Catch::Approx (0.0f));
}

TEST_CASE ("Macro state persists across save and load", "[state]")
{
    PluginProcessor savedPlugin;
    savedPlugin.setMacro (MacroManager::Air, 0.82f);
    savedPlugin.setMacro (MacroManager::Motion, 0.15f);
    savedPlugin.setMacro (MacroManager::Width, 0.67f);
    savedPlugin.setMacro (MacroManager::Warmth, 0.91f);
    savedPlugin.setMacro (MacroManager::Space, 0.73f);

    juce::MemoryBlock stateBlock;
    savedPlugin.getStateInformation (stateBlock);

    PluginProcessor loadedPlugin;
    loadedPlugin.setStateInformation (stateBlock.getData(), static_cast<int> (stateBlock.getSize()));

    CHECK (loadedPlugin.getMacro (MacroManager::Air) == Catch::Approx (0.82f));
    CHECK (loadedPlugin.getMacro (MacroManager::Motion) == Catch::Approx (0.15f));
    CHECK (loadedPlugin.getMacro (MacroManager::Width) == Catch::Approx (0.67f));
    CHECK (loadedPlugin.getMacro (MacroManager::Warmth) == Catch::Approx (0.91f));
    CHECK (loadedPlugin.getMacro (MacroManager::Space) == Catch::Approx (0.73f));
}

TEST_CASE ("Factory presets are available", "[presets]")
{
    PluginProcessor plugin;

    CHECK (plugin.getNumPrograms() > 1);
    CHECK (plugin.getProgramName (0).isNotEmpty());
    CHECK (plugin.getProgramName (0).equalsIgnoreCase ("Init"));
}

TEST_CASE ("Loading a factory preset changes macro values", "[presets]")
{
    PluginProcessor plugin;

    const auto cinematicCloudIndex = 3;
    REQUIRE (cinematicCloudIndex < plugin.getNumPrograms());

    plugin.setCurrentProgram (cinematicCloudIndex);

    CHECK (plugin.getCurrentProgram() == cinematicCloudIndex);
    CHECK (plugin.getMacro (MacroManager::Space) == Catch::Approx (0.92f));
    CHECK (plugin.getMacro (MacroManager::Air) == Catch::Approx (0.48f));
}

TEST_CASE ("Preset state persists across save and load", "[presets]")
{
    PluginProcessor savedPlugin;
    savedPlugin.setCurrentProgram (5);

    juce::MemoryBlock stateBlock;
    savedPlugin.getStateInformation (stateBlock);

    PluginProcessor loadedPlugin;
    loadedPlugin.setStateInformation (stateBlock.getData(), static_cast<int> (stateBlock.getSize()));

    CHECK (loadedPlugin.getMacro (MacroManager::Width) == Catch::Approx (0.82f));
    CHECK (loadedPlugin.getMacro (MacroManager::Space) == Catch::Approx (0.62f));
}


#ifdef PAMPLEJUCE_IPP
    #include <ipp.h>

TEST_CASE ("IPP version", "[ipp]")
{
    #if defined(__APPLE__)
        // macOS uses 2021.9.1 from pip wheel (only x86_64 version available)
        CHECK_THAT (ippsGetLibVersion()->Version, Catch::Matchers::Equals ("2021.9.1 (r0x7e208212)"));
    #else
        CHECK_THAT (ippsGetLibVersion()->Version, Catch::Matchers::Equals ("2026.0.0 (r0xa7ad6ebc)"));
    #endif
}
#endif
