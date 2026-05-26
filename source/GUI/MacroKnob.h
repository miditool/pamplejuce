#pragma once

#include "Control/MacroManager.h"

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

// Rotary macro control bound to MacroManager (0.0–1.0).
class MacroKnob : public juce::Component
{
public:
    explicit MacroKnob (const juce::String& name)
    {
        label.setText (name, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (label);

        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 18);
        slider.setRange (0.0, 1.0, 0.001);
        slider.setDoubleClickReturnValue (true, MacroManager::defaultMacroValue);
        slider.setScrollWheelEnabled (false);
        slider.onValueChange = [this]
        {
            if (onValueChanged != nullptr)
                onValueChanged (static_cast<float> (slider.getValue()));
        };
        addAndMakeVisible (slider);
    }

    void setOnValueChanged (std::function<void (float)> callback)
    {
        onValueChanged = std::move (callback);
    }

    void setValue (float value, juce::NotificationType notification)
    {
        slider.setValue (static_cast<double> (value), notification);
    }

    float getValue() const
    {
        return static_cast<float> (slider.getValue());
    }

    void resized() override
    {
        auto area = getLocalBounds();
        label.setBounds (area.removeFromTop (20));
        slider.setBounds (area);
    }

private:
    juce::Label label;
    juce::Slider slider;
    std::function<void (float)> onValueChanged;
};
