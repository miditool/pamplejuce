#pragma once

#include "LFO.h"

// Holds per-voice modulation sources. Routing is handled by ModulationMatrix.
class ModulationRouter
{
public:
    void prepare (double sampleRate, int samplesPerBlock)
    {
        filterCutoffLfo.prepare (sampleRate, samplesPerBlock);
    }

    void reset()
    {
        filterCutoffLfo.reset();
    }

    LFO& getFilterCutoffLfo() { return filterCutoffLfo; }

private:
    LFO filterCutoffLfo;
};
