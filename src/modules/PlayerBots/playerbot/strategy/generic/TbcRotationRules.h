#pragma once

#include <cstdint>

namespace ai
{
    namespace tbc
    {
        // A conservative finishing check, not an estimate of group DPS.
        inline bool UseBleedFinisher(uint32_t targetHealth, uint32_t botMaxHealth, float targetHealthPct)
        {
            return targetHealthPct > 20.0f && targetHealth > botMaxHealth;
        }

        inline bool SlamWindow(uint32_t swingRemaining, uint32_t swingInterval, uint32_t castTime)
        {
            // Cast immediately after a swing, never just before the next one.
            return swingInterval > 0 && swingRemaining > 0 &&
                swingRemaining <= swingInterval &&
                swingInterval - swingRemaining <= 600 && castTime <= 1000;
        }

        inline bool PreferWindfury(unsigned melee, unsigned casters)
        {
            return melee > 0 && melee >= casters;
        }
    }
}
