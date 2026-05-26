#pragma once

#include <BinaryData.h>

#include <juce_audio_processors/juce_audio_processors.h>

#include <algorithm>
#include <utility>
#include <vector>

class PresetManager
{
public:
    struct Preset
    {
        juce::String name;
        juce::String xmlContent;
    };

    PresetManager()
    {
        loadEmbeddedPresets();
    }

    int getNumPresets() const
    {
        return static_cast<int> (presets.size());
    }

    juce::String getPresetName (int index) const
    {
        if (index >= 0 && index < getNumPresets())
            return presets[static_cast<size_t> (index)].name;

        return {};
    }

    std::unique_ptr<juce::XmlElement> getPresetXml (int index) const
    {
        if (index >= 0 && index < getNumPresets())
            return juce::parseXML (presets[static_cast<size_t> (index)].xmlContent);

        return nullptr;
    }

private:
    static juce::String displayNameFromPath (const juce::String& path)
    {
        auto filename = path.fromLastOccurrenceOf ("/", false, false);

        if (filename.isEmpty())
            filename = path.fromLastOccurrenceOf ("\\", false, false);

        auto baseName = filename.upToLastOccurrenceOf (".", false, false);

        if (const auto dashIndex = baseName.indexOfChar ('-'); dashIndex >= 0)
            baseName = baseName.substring (dashIndex + 1);

        juce::StringArray tokens;
        tokens.addTokens (baseName, "-", juce::StringRef());

        for (auto& token : tokens)
        {
            token = token.trim();

            if (token.isNotEmpty())
                token = token.substring (0, 1).toUpperCase() + token.substring (1).toLowerCase();
        }

        return tokens.joinIntoString (" ");
    }

    void loadEmbeddedPresets()
    {
        std::vector<std::pair<juce::String, juce::String>> discovered;
        discovered.reserve (static_cast<size_t> (BinaryData::namedResourceListSize));

        for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
        {
            const auto* originalPath = BinaryData::originalFilenames[i];

            if (originalPath == nullptr)
                continue;

            const juce::String path (originalPath);

            if (! path.endsWithIgnoreCase (".xml"))
                continue;

            int dataSize = 0;
            const auto* data = BinaryData::getNamedResource (BinaryData::namedResourceList[i], dataSize);

            if (data == nullptr || dataSize <= 0)
                continue;

            discovered.emplace_back (path, juce::String (data, dataSize));
        }

        std::sort (discovered.begin(), discovered.end(), [] (const auto& a, const auto& b)
        {
            return a.first.compareNatural (b.first) < 0;
        });

        presets.reserve (discovered.size());

        for (auto& [path, content] : discovered)
        {
            Preset preset;
            preset.name = displayNameFromPath (path);
            preset.xmlContent = std::move (content);
            presets.push_back (std::move (preset));
        }
    }

    std::vector<Preset> presets;
};
