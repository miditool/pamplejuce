#pragma once

#include "ModulationIds.h"

// Serializable modulation routing entry for future preset / APVTS storage.
struct ModConnection
{
    ModSourceID source = ModSourceID::None;
    ModDestinationID destination = ModDestinationID::None;
    float amount = 0.0f;
    bool active = false;

    bool operator== (const ModConnection& other) const
    {
        return source == other.source
            && destination == other.destination
            && amount == other.amount
            && active == other.active;
    }
};
