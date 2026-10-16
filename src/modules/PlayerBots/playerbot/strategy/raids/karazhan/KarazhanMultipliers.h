// Ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara/KaraMultipliers) and adapted for this core.
// The actions are classified by their names (this core has other action classes than mod-playerbots).
#pragma once
#include "playerbot/strategy/Multiplier.h"

namespace ai
{
#define KARAZHAN_MULTIPLIER(ClassName, multiplierName) \
    class ClassName : public Multiplier \
    { \
    public: \
        ClassName(PlayerbotAI* ai) : Multiplier(ai, multiplierName) {} \
        float GetValue(Action* action) override; \
    };

    KARAZHAN_MULTIPLIER(KarazhanSetTremorTotemMultiplier, "karazhan set tremor totem")
    KARAZHAN_MULTIPLIER(KarazhanAttumenDisableAutomaticTargetingMultiplier, "attumen the huntsman disable automatic targeting")
    KARAZHAN_MULTIPLIER(KarazhanAttumenStayStackedMultiplier, "attumen the huntsman stay stacked")
    KARAZHAN_MULTIPLIER(KarazhanAttumenWaitForDpsMultiplier, "attumen the huntsman wait for dps")
    KARAZHAN_MULTIPLIER(KarazhanMaidenDisableCombatFormationMoveMultiplier, "maiden of virtue disable combat formation move")
    KARAZHAN_MULTIPLIER(KarazhanMaidenSetGroundingTotemMultiplier, "maiden of virtue set grounding totem")
    KARAZHAN_MULTIPLIER(KarazhanCuratorDisableTankAssistMultiplier, "the curator disable tank assist")
    KARAZHAN_MULTIPLIER(KarazhanCuratorDisableCombatFormationMoveMultiplier, "the curator disable combat formation move")
    KARAZHAN_MULTIPLIER(KarazhanCuratorDelayBloodlustMultiplier, "the curator delay bloodlust and heroism")
    KARAZHAN_MULTIPLIER(KarazhanIllhoofDontDotFiendishImpsMultiplier, "terestian illhoof don't dot fiendish imps")
    KARAZHAN_MULTIPLIER(KarazhanAranArcaneExplosionRunAwayMultiplier, "shade of aran arcane explosion run away")
    KARAZHAN_MULTIPLIER(KarazhanAranFlameWreathDisableMovementMultiplier, "shade of aran flame wreath disable movement")
    KARAZHAN_MULTIPLIER(KarazhanNetherspiteKeepBlockingBeamMultiplier, "netherspite keep blocking beam")
    KARAZHAN_MULTIPLIER(KarazhanNetherspiteWaitForDpsMultiplier, "netherspite wait for dps")
    KARAZHAN_MULTIPLIER(KarazhanMalchezaarEnfeebleMultiplier, "prince malchezaar enfeeble")
    KARAZHAN_MULTIPLIER(KarazhanMalchezaarDelayBloodlustMultiplier, "prince malchezaar delay bloodlust and heroism")
    KARAZHAN_MULTIPLIER(KarazhanNightbaneDisablePetsMultiplier, "nightbane disable pets")
    KARAZHAN_MULTIPLIER(KarazhanNightbaneWaitForDpsMultiplier, "nightbane wait for dps")
    KARAZHAN_MULTIPLIER(KarazhanNightbaneDisableAvoidAoeMultiplier, "nightbane disable avoid aoe")
    KARAZHAN_MULTIPLIER(KarazhanNightbaneDisableMovementMultiplier, "nightbane disable movement")

#undef KARAZHAN_MULTIPLIER
}
