// Karazhan (map 532) - strategy "boss karazhan"
// Boss tactics ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara) and adapted for this core.
#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "KarazhanStrategy.h"
#include "KarazhanActions.h"
#include "KarazhanTriggers.h"
#include "KarazhanValues.h"

namespace ai
{
    class KarazhanStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        KarazhanStrategyContext()
        {
            creators["boss karazhan"] = [](PlayerbotAI* ai) { return new KarazhanBossStrategy(ai); };
        }
    };

    class KarazhanActionContext : public NamedObjectContext<Action>
    {
    public:
        KarazhanActionContext()
        {
            creators["karazhan reset encounter states"] = [](PlayerbotAI* ai) { return new KarazhanResetEncounterStatesAction(ai); };
            creators["karazhan set tremor totem"] = [](PlayerbotAI* ai) { return new KarazhanSetTremorTotemAction(ai); };
            creators["mana warp stun creature before warp breach"] = [](PlayerbotAI* ai) { return new KarazhanManaWarpStunAction(ai); };
            creators["attumen the huntsman handle phase one"] = [](PlayerbotAI* ai) { return new KarazhanAttumenHandlePhaseOneAction(ai); };
            creators["attumen the huntsman handle phase two"] = [](PlayerbotAI* ai) { return new KarazhanAttumenHandlePhaseTwoAction(ai); };
            creators["attumen the huntsman set dps timer"] = [](PlayerbotAI* ai) { return new KarazhanAttumenSetDpsTimerAction(ai); };
            creators["moroes mark target"] = [](PlayerbotAI* ai) { return new KarazhanMoroesMarkTargetAction(ai); };
            creators["maiden of virtue tank position boss"] = [](PlayerbotAI* ai) { return new KarazhanMaidenTankPositionBossAction(ai); };
            creators["maiden of virtue position ranged between pillars"] = [](PlayerbotAI* ai) { return new KarazhanMaidenPositionRangedAction(ai); };
            creators["maiden of virtue set grounding totem"] = [](PlayerbotAI* ai) { return new KarazhanMaidenSetGroundingTotemAction(ai); };
            creators["big bad wolf position boss"] = [](PlayerbotAI* ai) { return new KarazhanBigBadWolfPositionBossAction(ai); };
            creators["big bad wolf little red riding hood run away"] = [](PlayerbotAI* ai) { return new KarazhanBigBadWolfRunAwayAction(ai); };
            creators["romulo and julianne mark target"] = [](PlayerbotAI* ai) { return new KarazhanRomuloAndJulianneMarkTargetAction(ai); };
            creators["wizard of oz mark target"] = [](PlayerbotAI* ai) { return new KarazhanWizardOfOzMarkTargetAction(ai); };
            creators["wizard of oz scorch strawman"] = [](PlayerbotAI* ai) { return new KarazhanWizardOfOzScorchStrawmanAction(ai); };
            creators["the curator mark astral flare"] = [](PlayerbotAI* ai) { return new KarazhanCuratorMarkAstralFlareAction(ai); };
            creators["the curator position boss"] = [](PlayerbotAI* ai) { return new KarazhanCuratorPositionBossAction(ai); };
            creators["the curator spread ranged"] = [](PlayerbotAI* ai) { return new KarazhanCuratorSpreadRangedAction(ai); };
            creators["terestian illhoof mark target"] = [](PlayerbotAI* ai) { return new KarazhanIllhoofMarkTargetAction(ai); };
            creators["shade of aran run away from arcane explosion"] = [](PlayerbotAI* ai) { return new KarazhanAranRunAwayFromArcaneExplosionAction(ai); };
            creators["shade of aran stop moving during flame wreath"] = [](PlayerbotAI* ai) { return new KarazhanAranStopMovingDuringFlameWreathAction(ai); };
            creators["shade of aran mark conjured elemental"] = [](PlayerbotAI* ai) { return new KarazhanAranMarkConjuredElementalAction(ai); };
            creators["shade of aran ranged maintain distance"] = [](PlayerbotAI* ai) { return new KarazhanAranRangedMaintainDistanceAction(ai); };
            creators["netherspite block red beam"] = [](PlayerbotAI* ai) { return new KarazhanNetherspiteBlockRedBeamAction(ai); };
            creators["netherspite block blue beam"] = [](PlayerbotAI* ai) { return new KarazhanNetherspiteBlockBlueBeamAction(ai); };
            creators["netherspite block green beam"] = [](PlayerbotAI* ai) { return new KarazhanNetherspiteBlockGreenBeamAction(ai); };
            creators["netherspite avoid beam and void zone"] = [](PlayerbotAI* ai) { return new KarazhanNetherspiteAvoidBeamAndVoidZoneAction(ai); };
            creators["netherspite banish phase avoid void zone"] = [](PlayerbotAI* ai) { return new KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction(ai); };
            creators["netherspite manage timers and trackers"] = [](PlayerbotAI* ai) { return new KarazhanNetherspiteManageTimersAndTrackersAction(ai); };
            creators["prince malchezaar enfeebled bot avoid hazard"] = [](PlayerbotAI* ai) { return new KarazhanMalchezaarEnfeebledAvoidHazardAction(ai); };
            creators["prince malchezaar non-tank avoid infernal"] = [](PlayerbotAI* ai) { return new KarazhanMalchezaarNonTankAvoidInfernalAction(ai); };
            creators["prince malchezaar tanks position boss"] = [](PlayerbotAI* ai) { return new KarazhanMalchezaarTanksPositionBossAction(ai); };
            creators["nightbane ground phase tanks position boss"] = [](PlayerbotAI* ai) { return new KarazhanNightbaneTanksPositionBossAction(ai); };
            creators["nightbane ground phase coordinate ranged movement"] = [](PlayerbotAI* ai) { return new KarazhanNightbaneCoordinateRangedAction(ai); };
            creators["nightbane control pet aggression"] = [](PlayerbotAI* ai) { return new KarazhanNightbaneControlPetAggressionAction(ai); };
            creators["nightbane flight phase stack and move"] = [](PlayerbotAI* ai) { return new KarazhanNightbaneFlightPhaseStackAction(ai); };
            creators["nightbane teleport back to terrace"] = [](PlayerbotAI* ai) { return new KarazhanNightbaneTeleportBackAction(ai); };
            creators["nightbane manage timers and trackers"] = [](PlayerbotAI* ai) { return new KarazhanNightbaneManageTimersAndTrackersAction(ai); };
        }
    };

    class KarazhanTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        KarazhanTriggerContext()
        {
            creators["karazhan bot is not in combat"] = [](PlayerbotAI* ai) { return new KarazhanBotIsNotInCombatTrigger(ai); };
            creators["karazhan enemies cast fear"] = [](PlayerbotAI* ai) { return new KarazhanEnemiesCastFearTrigger(ai); };
            creators["mana warp is about to explode"] = [](PlayerbotAI* ai) { return new KarazhanManaWarpIsAboutToExplodeTrigger(ai); };
            creators["attumen the huntsman phase one active"] = [](PlayerbotAI* ai) { return new KarazhanAttumenPhaseOneActiveTrigger(ai); };
            creators["attumen the huntsman phase two active"] = [](PlayerbotAI* ai) { return new KarazhanAttumenPhaseTwoActiveTrigger(ai); };
            creators["attumen the huntsman phase transition"] = [](PlayerbotAI* ai) { return new KarazhanAttumenPhaseTransitionTrigger(ai); };
            creators["moroes should prioritize adds"] = [](PlayerbotAI* ai) { return new KarazhanMoroesShouldPrioritizeAddsTrigger(ai); };
            creators["maiden of virtue boss engaged by tanks"] = [](PlayerbotAI* ai) { return new KarazhanMaidenBossEngagedByTanksTrigger(ai); };
            creators["maiden of virtue holy wrath deals chain damage"] = [](PlayerbotAI* ai) { return new KarazhanMaidenHolyWrathDealsChainDamageTrigger(ai); };
            creators["maiden of virtue grounding totem consumes holy fire"] = [](PlayerbotAI* ai) { return new KarazhanMaidenGroundingTotemConsumesHolyFireTrigger(ai); };
            creators["big bad wolf boss engaged by tank"] = [](PlayerbotAI* ai) { return new KarazhanBigBadWolfBossEngagedByTankTrigger(ai); };
            creators["big bad wolf boss is chasing little red riding hood"] = [](PlayerbotAI* ai) { return new KarazhanBigBadWolfChasingLittleRedRidingHoodTrigger(ai); };
            creators["romulo and julianne both bosses revived"] = [](PlayerbotAI* ai) { return new KarazhanRomuloAndJulianneBothBossesRevivedTrigger(ai); };
            creators["wizard of oz need target priority"] = [](PlayerbotAI* ai) { return new KarazhanWizardOfOzNeedTargetPriorityTrigger(ai); };
            creators["wizard of oz strawman is vulnerable to fire"] = [](PlayerbotAI* ai) { return new KarazhanWizardOfOzStrawmanIsVulnerableToFireTrigger(ai); };
            creators["the curator astral flare spawned"] = [](PlayerbotAI* ai) { return new KarazhanCuratorAstralFlareSpawnedTrigger(ai); };
            creators["the curator boss engaged by tanks"] = [](PlayerbotAI* ai) { return new KarazhanCuratorBossEngagedByTanksTrigger(ai); };
            creators["the curator boss engaged by ranged"] = [](PlayerbotAI* ai) { return new KarazhanCuratorBossEngagedByRangedTrigger(ai); };
            creators["terestian illhoof should prioritize chains"] = [](PlayerbotAI* ai) { return new KarazhanIllhoofShouldPrioritizeChainsTrigger(ai); };
            creators["shade of aran arcane explosion is casting"] = [](PlayerbotAI* ai) { return new KarazhanAranArcaneExplosionIsCastingTrigger(ai); };
            creators["shade of aran flame wreath is active"] = [](PlayerbotAI* ai) { return new KarazhanAranFlameWreathIsActiveTrigger(ai); };
            creators["shade of aran conjured elementals summoned"] = [](PlayerbotAI* ai) { return new KarazhanAranConjuredElementalsSummonedTrigger(ai); };
            creators["shade of aran boss casts counterspell nearby"] = [](PlayerbotAI* ai) { return new KarazhanAranBossCastsCounterspellNearbyTrigger(ai); };
            creators["netherspite red beam is active"] = [](PlayerbotAI* ai) { return new KarazhanNetherspiteRedBeamIsActiveTrigger(ai); };
            creators["netherspite blue beam is active"] = [](PlayerbotAI* ai) { return new KarazhanNetherspiteBlueBeamIsActiveTrigger(ai); };
            creators["netherspite green beam is active"] = [](PlayerbotAI* ai) { return new KarazhanNetherspiteGreenBeamIsActiveTrigger(ai); };
            creators["netherspite bot is not beam blocker"] = [](PlayerbotAI* ai) { return new KarazhanNetherspiteBotIsNotBeamBlockerTrigger(ai); };
            creators["netherspite boss is banished"] = [](PlayerbotAI* ai) { return new KarazhanNetherspiteBossIsBanishedTrigger(ai); };
            creators["netherspite should manage timers and trackers"] = [](PlayerbotAI* ai) { return new KarazhanNetherspiteManageTimersAndTrackersTrigger(ai); };
            creators["prince malchezaar bot is enfeebled"] = [](PlayerbotAI* ai) { return new KarazhanMalchezaarBotIsEnfeebledTrigger(ai); };
            creators["prince malchezaar engaged by non-tanks"] = [](PlayerbotAI* ai) { return new KarazhanMalchezaarEngagedByNonTanksTrigger(ai); };
            creators["prince malchezaar boss engaged by tanks"] = [](PlayerbotAI* ai) { return new KarazhanMalchezaarBossEngagedByTanksTrigger(ai); };
            creators["nightbane boss engaged by tanks"] = [](PlayerbotAI* ai) { return new KarazhanNightbaneBossEngagedByTanksTrigger(ai); };
            creators["nightbane ground phase engaged by ranged"] = [](PlayerbotAI* ai) { return new KarazhanNightbaneGroundPhaseEngagedByRangedTrigger(ai); };
            creators["nightbane pets ignore collision to chase flying boss"] = [](PlayerbotAI* ai) { return new KarazhanNightbanePetsIgnoreCollisionTrigger(ai); };
            creators["nightbane boss is flying"] = [](PlayerbotAI* ai) { return new KarazhanNightbaneBossIsFlyingTrigger(ai); };
            creators["nightbane bot went out of bounds"] = [](PlayerbotAI* ai) { return new KarazhanNightbaneBotWentOutOfBoundsTrigger(ai); };
            creators["nightbane should manage timers and trackers"] = [](PlayerbotAI* ai) { return new KarazhanNightbaneManageTimersAndTrackersTrigger(ai); };
        }
    };

    class KarazhanValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        KarazhanValueContext()
        {
            creators["karazhan nearby creatures"] = [](PlayerbotAI* ai) { return new KarazhanNearbyCreaturesValue(ai); };
        }
    };
}
