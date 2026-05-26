#pragma once

#include <juce_dsp/juce_dsp.h>

enum class Waveform
{
    Sine,
    Saw,
};

// Single band-limited oscillator. Used by OscillatorSection and future unison voices.
class Oscillator
{
public:
    void prepare (double sampleRate, int samplesPerBlock)
    {
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
        spec.numChannels = 1;

        oscillator.prepare (spec);
        setWaveform (currentWaveform);
        oscillator.reset();
    }

    void setWaveform (Waveform waveform)
    {
        currentWaveform = waveform;

        switch (currentWaveform)
        {
            case Waveform::Sine:
                oscillator.initialise (sineWave);
                break;

            case Waveform::Saw:
                oscillator.initialise (sawWave);
                break;
        }
    }

    void setFrequency (float frequencyHz)
    {
        oscillator.setFrequency (frequencyHz);
    }

    void reset()
    {
        oscillator.reset();
    }

    float processSample()
    {
        return oscillator.processSample (0.0f);
    }

private:
    static float sineWave (float phase)
    {
        return std::sin (phase);
    }

    static float sawWave (float phase)
    {
        // juce::dsp::Oscillator passes phase in the range [-pi, pi].
        return phase / juce::MathConstants<float>::pi;
    }

    juce::dsp::Oscillator<float> oscillator;
    juce::dsp::ProcessSpec spec {};
    Waveform currentWaveform = Waveform::Saw;
};
