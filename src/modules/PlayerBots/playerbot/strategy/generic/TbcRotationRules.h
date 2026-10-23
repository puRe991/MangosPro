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

        // Curse of Doom only pays off if the target lives its full minute. Rough
        // estimate: every group member contributes about 800 damage per second.
        inline bool CurseOfDoomFight(uint32_t targetHealth, unsigned groupSize)
        {
            return groupSize > 0 && targetHealth >= groupSize * 800u * 60u;
        }

        // Raid curse of a warlock by its rank among the group's warlocks
        // (Malediction holders first): one Elements, one Recklessness with
        // enough melee, everyone else Doom on long fights and Agony otherwise.
        inline const char* RaidCurse(unsigned rank, unsigned melee, bool doomFight)
        {
            if (rank == 0)
                return "curse of the elements";
            if (rank == 1 && melee >= 4)
                return "curse of recklessness";
            return doomFight ? "curse of doom" : "curse of agony";
        }
    }
}
