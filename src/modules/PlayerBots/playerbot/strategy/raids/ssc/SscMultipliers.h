// Ported from mod-playerbots (src/Ai/Raid/SSC/SSCMultipliers.h, GPL v2+) and adapted for this core.
#pragma once
#include "playerbot/strategy/Multiplier.h"

namespace ai
{
#define SSC_MULTIPLIER(className, multiplierName) \
    class className : public Multiplier \
    { \
    public: \
        className(PlayerbotAI* ai) : Multiplier(ai, multiplierName) {} \
        float GetValue(Action* action) override; \
    };

    // Trash
    SSC_MULTIPLIER(SscUnderbogColossusEscapeToxicPoolMultiplier, "ssc underbog colossus escape toxic pool")

    // Hydross the Unstable
    SSC_MULTIPLIER(SscHydrossDisableTankActionsMultiplier, "ssc hydross disable tank actions")
    SSC_MULTIPLIER(SscHydrossWaitForDpsMultiplier, "ssc hydross wait for dps")
    SSC_MULTIPLIER(SscHydrossControlMisdirectionMultiplier, "ssc hydross control misdirection")

    // The Lurker Below
    SSC_MULTIPLIER(SscLurkerStayAwayFromSpoutMultiplier, "ssc lurker stay away from spout")
    SSC_MULTIPLIER(SscLurkerMaintainRangedSpreadMultiplier, "ssc lurker maintain ranged spread")
    SSC_MULTIPLIER(SscLurkerDisableTankAssistMultiplier, "ssc lurker disable tank assist")

    // Leotheras the Blind
    SSC_MULTIPLIER(SscLeotherasAvoidWhirlwindMultiplier, "ssc leotheras avoid whirlwind")
    SSC_MULTIPLIER(SscLeotherasDisableTankActionsMultiplier, "ssc leotheras disable tank actions")
    SSC_MULTIPLIER(SscLeotherasMeleeDpsAvoidChaosBlastMultiplier, "ssc leotheras melee dps avoid chaos blast")
    SSC_MULTIPLIER(SscLeotherasFocusOnInnerDemonMultiplier, "ssc leotheras focus on inner demon")
    SSC_MULTIPLIER(SscLeotherasWaitForDpsMultiplier, "ssc leotheras wait for dps")
    SSC_MULTIPLIER(SscLeotherasSaveCooldownsForFinalPhaseMultiplier, "ssc leotheras save cooldowns for final phase")

    // Fathom-Lord Karathress
    SSC_MULTIPLIER(SscKarathressDisableTankActionsMultiplier, "ssc karathress disable tank actions")
    SSC_MULTIPLIER(SscKarathressDisableAoeMultiplier, "ssc karathress disable aoe")
    SSC_MULTIPLIER(SscKarathressControlMisdirectionMultiplier, "ssc karathress control misdirection")
    SSC_MULTIPLIER(SscKarathressWaitForDpsMultiplier, "ssc karathress wait for dps")
    SSC_MULTIPLIER(SscKarathressCaribdisTankHealerMaintainPositionMultiplier, "ssc karathress caribdis tank healer maintain position")

    // Morogrim Tidewalker
    SSC_MULTIPLIER(SscMorogrimDelayBloodlustAndHeroismMultiplier, "ssc morogrim delay bloodlust and heroism")
    SSC_MULTIPLIER(SscMorogrimDisableTankActionsMultiplier, "ssc morogrim disable tank actions")
    SSC_MULTIPLIER(SscMorogrimMaintainPhase2StackingMultiplier, "ssc morogrim maintain phase 2 stacking")

    // Lady Vashj
    SSC_MULTIPLIER(SscVashjDelayCooldownsMultiplier, "ssc vashj delay cooldowns")
    SSC_MULTIPLIER(SscVashjMainTankGroupShamanUseGroundingTotemMultiplier, "ssc vashj main tank group shaman use grounding totem")
    SSC_MULTIPLIER(SscVashjMaintainPhase1RangedSpreadMultiplier, "ssc vashj maintain phase 1 ranged spread")
    SSC_MULTIPLIER(SscVashjStaticChargeStayAwayFromGroupMultiplier, "ssc vashj static charge stay away from group")
    SSC_MULTIPLIER(SscVashjDoNotLootTheTaintedCoreMultiplier, "ssc vashj do not loot the tainted core")
    SSC_MULTIPLIER(SscVashjCorePassersPrioritizePositioningMultiplier, "ssc vashj core passers prioritize positioning")
    SSC_MULTIPLIER(SscVashjDisableAutomaticTargetingAndMovementMultiplier, "ssc vashj disable automatic targeting and movement")

#undef SSC_MULTIPLIER
}
