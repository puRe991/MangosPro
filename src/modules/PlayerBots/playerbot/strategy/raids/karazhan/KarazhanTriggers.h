// Ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara/KaraTriggers) and adapted for this core.
#pragma once
#include "playerbot/strategy/Trigger.h"

namespace ai
{
#define KARAZHAN_TRIGGER(ClassName, triggerName) \
    class ClassName : public Trigger \
    { \
    public: \
        ClassName(PlayerbotAI* ai) : Trigger(ai, triggerName) {} \
        bool IsActive() override; \
    };

    // General
    KARAZHAN_TRIGGER(KarazhanBotIsNotInCombatTrigger, "karazhan bot is not in combat")
    KARAZHAN_TRIGGER(KarazhanEnemiesCastFearTrigger, "karazhan enemies cast fear")

    // Trash
    KARAZHAN_TRIGGER(KarazhanManaWarpIsAboutToExplodeTrigger, "mana warp is about to explode")

    // Attumen the Huntsman
    KARAZHAN_TRIGGER(KarazhanAttumenPhaseOneActiveTrigger, "attumen the huntsman phase one active")
    KARAZHAN_TRIGGER(KarazhanAttumenPhaseTwoActiveTrigger, "attumen the huntsman phase two active")
    KARAZHAN_TRIGGER(KarazhanAttumenPhaseTransitionTrigger, "attumen the huntsman phase transition")

    // Moroes
    KARAZHAN_TRIGGER(KarazhanMoroesShouldPrioritizeAddsTrigger, "moroes should prioritize adds")

    // Maiden of Virtue
    KARAZHAN_TRIGGER(KarazhanMaidenBossEngagedByTanksTrigger, "maiden of virtue boss engaged by tanks")
    KARAZHAN_TRIGGER(KarazhanMaidenHolyWrathDealsChainDamageTrigger, "maiden of virtue holy wrath deals chain damage")
    KARAZHAN_TRIGGER(KarazhanMaidenGroundingTotemConsumesHolyFireTrigger, "maiden of virtue grounding totem consumes holy fire")

    // The Big Bad Wolf
    KARAZHAN_TRIGGER(KarazhanBigBadWolfBossEngagedByTankTrigger, "big bad wolf boss engaged by tank")
    KARAZHAN_TRIGGER(KarazhanBigBadWolfChasingLittleRedRidingHoodTrigger, "big bad wolf boss is chasing little red riding hood")

    // Romulo and Julianne
    KARAZHAN_TRIGGER(KarazhanRomuloAndJulianneBothBossesRevivedTrigger, "romulo and julianne both bosses revived")

    // The Wizard of Oz
    KARAZHAN_TRIGGER(KarazhanWizardOfOzNeedTargetPriorityTrigger, "wizard of oz need target priority")
    KARAZHAN_TRIGGER(KarazhanWizardOfOzStrawmanIsVulnerableToFireTrigger, "wizard of oz strawman is vulnerable to fire")

    // The Curator
    KARAZHAN_TRIGGER(KarazhanCuratorAstralFlareSpawnedTrigger, "the curator astral flare spawned")
    KARAZHAN_TRIGGER(KarazhanCuratorBossEngagedByTanksTrigger, "the curator boss engaged by tanks")
    KARAZHAN_TRIGGER(KarazhanCuratorBossEngagedByRangedTrigger, "the curator boss engaged by ranged")

    // Terestian Illhoof
    KARAZHAN_TRIGGER(KarazhanIllhoofShouldPrioritizeChainsTrigger, "terestian illhoof should prioritize chains")

    // Shade of Aran
    KARAZHAN_TRIGGER(KarazhanAranArcaneExplosionIsCastingTrigger, "shade of aran arcane explosion is casting")
    KARAZHAN_TRIGGER(KarazhanAranFlameWreathIsActiveTrigger, "shade of aran flame wreath is active")
    KARAZHAN_TRIGGER(KarazhanAranConjuredElementalsSummonedTrigger, "shade of aran conjured elementals summoned")
    KARAZHAN_TRIGGER(KarazhanAranBossCastsCounterspellNearbyTrigger, "shade of aran boss casts counterspell nearby")

    // Netherspite
    KARAZHAN_TRIGGER(KarazhanNetherspiteRedBeamIsActiveTrigger, "netherspite red beam is active")
    KARAZHAN_TRIGGER(KarazhanNetherspiteBlueBeamIsActiveTrigger, "netherspite blue beam is active")
    KARAZHAN_TRIGGER(KarazhanNetherspiteGreenBeamIsActiveTrigger, "netherspite green beam is active")
    KARAZHAN_TRIGGER(KarazhanNetherspiteBotIsNotBeamBlockerTrigger, "netherspite bot is not beam blocker")
    KARAZHAN_TRIGGER(KarazhanNetherspiteBossIsBanishedTrigger, "netherspite boss is banished")
    KARAZHAN_TRIGGER(KarazhanNetherspiteManageTimersAndTrackersTrigger, "netherspite should manage timers and trackers")

    // Prince Malchezaar
    KARAZHAN_TRIGGER(KarazhanMalchezaarBotIsEnfeebledTrigger, "prince malchezaar bot is enfeebled")
    KARAZHAN_TRIGGER(KarazhanMalchezaarEngagedByNonTanksTrigger, "prince malchezaar engaged by non-tanks")
    KARAZHAN_TRIGGER(KarazhanMalchezaarBossEngagedByTanksTrigger, "prince malchezaar boss engaged by tanks")

    // Nightbane
    KARAZHAN_TRIGGER(KarazhanNightbaneBossEngagedByTanksTrigger, "nightbane boss engaged by tanks")
    KARAZHAN_TRIGGER(KarazhanNightbaneGroundPhaseEngagedByRangedTrigger, "nightbane ground phase engaged by ranged")
    KARAZHAN_TRIGGER(KarazhanNightbanePetsIgnoreCollisionTrigger, "nightbane pets ignore collision to chase flying boss")
    KARAZHAN_TRIGGER(KarazhanNightbaneBossIsFlyingTrigger, "nightbane boss is flying")
    KARAZHAN_TRIGGER(KarazhanNightbaneBotWentOutOfBoundsTrigger, "nightbane bot went out of bounds")
    KARAZHAN_TRIGGER(KarazhanNightbaneManageTimersAndTrackersTrigger, "nightbane should manage timers and trackers")

#undef KARAZHAN_TRIGGER
}
