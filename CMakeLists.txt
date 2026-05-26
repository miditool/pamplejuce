cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# This tells cmake we have goodies in the /cmake folder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include (PamplejuceVersion)

# Modern concise way to add dependencies to your project
include (CPM)

# Configures universal binaries and decides which version of macOS to support
include(PamplejuceMacOS)

# Couple tweaks that IMO should be JUCE defaults
include(JUCEDefaults)

# The internal name of the project and the name of JUCE's shared code target
# Note: This cannot have spaces!
set(PROJECT_NAME "LiquidLush")

# What DAWs will display (Spaces allowed)
set(PRODUCT_NAME "Liquid Lush")

# Used for the MacOS bundle name and Installers
set(COMPANY_NAME "MIDITool")

# Used for the MacOS bundle identifier (and signing)
set(BUNDLE_ID "com.miditool.liquidlush")

# Set the plugin formats you want built
set(FORMATS Standalone AU VST3 AUv3)

# Force JUCE to work with sccache
# instruct MSVC to embed debug info (/Z7) instead of emitting a .pdb
set(CMAKE_POLICY_DEFAULT_CMP0141        NEW      CACHE STRING "" FORCE)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded CACHE STRING "" FORCE)

# For simplicity, the name of the CMake project is also the name of the target
project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})

# Log build environment info 
include(PamplejuceLog)

# JUCE submodule
add_subdirectory(JUCE)

# CLAP format (DISABLED)
# add_subdirectory(modules/clap-juce-extensions EXCLUDE_FROM_ALL)

# This adds the melatonin inspector module
add_subdirectory (modules/melatonin_inspector)

# See `docs/CMake API.md` in the JUCE repo for all config options
juce_add_plugin("${PROJECT_NAME}"
    # Icons for the standalone app
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icon.png"

    COMPANY_NAME "${COMPANY_NAME}"
    BUNDLE_ID "${BUNDLE_ID}"

    COPY_PLUGIN_AFTER_BUILD TRUE

    # A four-character plugin id (First character MUST be uppercase for AU formats)
    PLUGIN_MANUFACTURER_CODE Trau

    # A unique four-character plugin id (Must have at least one upper-case character)
    PLUGIN_CODE Atmo
    FORMATS "${FORMATS}"

    PRODUCT_NAME "${PRODUCT_NAME}"

    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE)

# This lets us use our code in both the JUCE targets and our Test target
add_library(SharedCode INTERFACE)

# CLAP features (DISABLED)
# clap_juce_extensions_plugin(TARGET "${PROJECT_NAME}"
#     CLAP_ID "${BUNDLE_ID}"
#     CLAP_FEATURES audio-effect)

# Enable fast math, C++20 and a few other target defaults
include(SharedCodeDefaults)

# Manually list all .h and .cpp files for the plugin
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h")
target_sources(SharedCode INTERFACE ${SourceFiles})
target_include_directories(SharedCode INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/source")

# Adds a BinaryData target for embedding assets into the binary
include(Assets)

# MacOS only: Cleans up folder and target organization on Xcode.
include(XcodePrettify)

# Preprocessor definitions for JUCE and your plugin
target_compile_definitions(SharedCode
    INTERFACE
    JUCE_WEB_BROWSER=0  
    JUCE_USE_CURL=0     
    JUCE_VST3_CAN_REPLACE_VST2=0
    CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    VERSION="${CURRENT_VERSION}"
    PRODUCT_NAME_WITHOUT_VERSION="Liquid Lush"
)

# Link to any other modules you added
target_link_libraries(SharedCode
    INTERFACE
    Assets
    melatonin_inspector
    juce_audio_utils
    juce_audio_processors
    juce_dsp
    juce_gui_basics
    juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

# Link the JUCE plugin targets our SharedCode target
target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode)

# IPP support (DISABLED)
# include(PamplejuceIPP)

# Everything related to the tests target
include(Tests)

# A separate target for Benchmarks 
include(Benchmarks)

# Output some config for CI 
include(GitHubENV)