// Ported from mod-playerbots (AzerothCore, GPL v2+), src/Ai/Raid/ZA/ZAMultipliers.h, and adapted for this core.
#pragma once
#include "playerbot/strategy/Multiplier.h"
#include "ZaHelpers.h"
#include <string>

namespace ai
{
    // Base: 1.0 outside of the Zul'Aman boss encounters
    class ZaEncounterMultiplier : public Multiplier
    {
    public:
        ZaEncounterMultiplier(PlayerbotAI* ai, std::string name) : Multiplier(ai, name) {}

        float GetValue(Action* action) final;

    protected:
        virtual float GetValueInEncounter(Action* action) = 0;
    };

#define ZA_MULTIPLIER(clazz, name) \
    class clazz : public ZaEncounterMultiplier \
    { \
    public: \
        clazz(PlayerbotAI* ai) : ZaEncounterMultiplier(ai, name) {} \
    protected: \
        float GetValueInEncounter(Action* action) override; \
    }

    // General
    ZA_MULTIPLIER(ZaDelayDpsCooldownsMultiplier, "zul'aman delay dps cooldowns");
    ZA_MULTIPLIER(ZaDisableTankActionsMultiplier, "zul'aman disable tank actions");
    ZA_MULTIPLIER(ZaControlMisdirectionMultiplier, "zul'aman control misdirection");
    ZA_MULTIPLIER(ZaDisableCombatFormationMoveMultiplier, "zul'aman disable combat formation move");
    ZA_MULTIPLIER(ZaAvoidWhirlwindMultiplier, "zul'aman avoid whirlwind");
    // Not in mod-playerbots: keeps the table driven "tbc raid" rules for these bosses from
    // contradicting this strategy (see the porting report)
    ZA_MULTIPLIER(ZaTbcRaidOverlapMultiplier, "zul'aman tbc raid overlap");

    // Akil'zon <Eagle Avatar>
    ZA_MULTIPLIER(ZaAkilzonStayInEyeOfTheStormMultiplier, "akil'zon stay in eye of the storm");

    // Jan'alai <Dragonhawk Avatar>
    ZA_MULTIPLIER(ZaJanalaiStayAwayFromFireBombsMultiplier, "jan'alai stay away from fire bombs");
    ZA_MULTIPLIER(ZaJanalaiDoNotCrowdControlHatchersMultiplier, "jan'alai do not crowd control hatchers");

    // Halazzi <Lynx Avatar>
    ZA_MULTIPLIER(ZaHalazziDisableAutoDpsTargetingMultiplier, "halazzi disable auto dps targeting");

    // Hex Lord Malacrass
    ZA_MULTIPLIER(ZaHexLordMalacrassUnstableAfflictionMultiplier, "hex lord malacrass unstable affliction");
    ZA_MULTIPLIER(ZaHexLordMalacrassSpellReflectionMultiplier, "hex lord malacrass spell reflection");
    ZA_MULTIPLIER(ZaHexLordMalacrassStayAwayFromFreezingTrapMultiplier, "hex lord malacrass stay away from freezing trap");

    // Zul'jin
    ZA_MULTIPLIER(ZaZuljinStopAttackingDuringPhaseChangeMultiplier, "zul'jin stop attacking during phase change");
    ZA_MULTIPLIER(ZaZuljinEagleDisableAvoidAoeMultiplier, "zul'jin eagle disable avoid aoe");

#undef ZA_MULTIPLIER
}
