#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "SscStrategy.h"
#include "SscTriggers.h"
#include "SscActions.h"

// Serpentshrine Cavern (map 548) - strategy "boss ssc"
// Ported from mod-playerbots (src/Ai/Raid/SSC, GPL v2+) and adapted for this core.
namespace ai
{
    class SscStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        SscStrategyContext()
        {
            creators["boss ssc"] = [](PlayerbotAI* ai) { return new SscStrategy(ai); };
        }
    };

    class SscActionContext : public NamedObjectContext<Action>
    {
    public:
        SscActionContext()
        {
            creators["serpent shrine cavern erase timers and trackers"] = [](PlayerbotAI* ai) { return new SscEraseTimersAndTrackersAction(ai); };
            creators["underbog colossus escape toxic pool"] = [](PlayerbotAI* ai) { return new SscUnderbogColossusEscapeToxicPoolAction(ai); };
            creators["greyheart tidecaller mark water elemental totem"] = [](PlayerbotAI* ai) { return new SscGreyheartTidecallerMarkWaterElementalTotemAction(ai); };
            creators["hydross the unstable position frost tank"] = [](PlayerbotAI* ai) { return new SscHydrossPositionFrostTankAction(ai); };
            creators["hydross the unstable position nature tank"] = [](PlayerbotAI* ai) { return new SscHydrossPositionNatureTankAction(ai); };
            creators["hydross the unstable prioritize elemental adds"] = [](PlayerbotAI* ai) { return new SscHydrossPrioritizeElementalAddsAction(ai); };
            creators["hydross the unstable misdirect boss to tank"] = [](PlayerbotAI* ai) { return new SscHydrossMisdirectBossToTankAction(ai); };
            creators["hydross the unstable stop dps upon phase change"] = [](PlayerbotAI* ai) { return new SscHydrossStopDpsUponPhaseChangeAction(ai); };
            creators["hydross the unstable manage timers"] = [](PlayerbotAI* ai) { return new SscHydrossManageTimersAction(ai); };
            creators["the lurker below run around behind boss"] = [](PlayerbotAI* ai) { return new SscLurkerRunAroundBehindBossAction(ai); };
            creators["the lurker below position main tank"] = [](PlayerbotAI* ai) { return new SscLurkerPositionMainTankAction(ai); };
            creators["the lurker below spread ranged in arc"] = [](PlayerbotAI* ai) { return new SscLurkerSpreadRangedInArcAction(ai); };
            creators["the lurker below tanks pick up adds"] = [](PlayerbotAI* ai) { return new SscLurkerTanksPickUpAddsAction(ai); };
            creators["the lurker below manage spout timer"] = [](PlayerbotAI* ai) { return new SscLurkerManageSpoutTimerAction(ai); };
            creators["leotheras the blind target spellbinders"] = [](PlayerbotAI* ai) { return new SscLeotherasTargetSpellbindersAction(ai); };
            creators["leotheras the blind demon form tank attack boss"] = [](PlayerbotAI* ai) { return new SscLeotherasDemonFormTankAttackBossAction(ai); };
            creators["leotheras the blind melee tanks don't attack demon form"] = [](PlayerbotAI* ai) { return new SscLeotherasMeleeTanksDontAttackDemonFormAction(ai); };
            creators["leotheras the blind run away from whirlwind"] = [](PlayerbotAI* ai) { return new SscLeotherasRunAwayFromWhirlwindAction(ai); };
            creators["leotheras the blind melee dps run away from boss"] = [](PlayerbotAI* ai) { return new SscLeotherasMeleeDpsRunAwayFromBossAction(ai); };
            creators["leotheras the blind final phase assign dps priority"] = [](PlayerbotAI* ai) { return new SscLeotherasFinalPhaseAssignDpsPriorityAction(ai); };
            creators["leotheras the blind misdirect boss to demon form tank"] = [](PlayerbotAI* ai) { return new SscLeotherasMisdirectBossToDemonFormTankAction(ai); };
            creators["leotheras the blind manage dps wait timers"] = [](PlayerbotAI* ai) { return new SscLeotherasManageDpsWaitTimersAction(ai); };
            creators["leotheras the blind heal demon form tank"] = [](PlayerbotAI* ai) { return new SscLeotherasHealDemonFormTankAction(ai); };
            creators["leotheras the blind attack mind controlled player"] = [](PlayerbotAI* ai) { return new SscLeotherasAttackMindControlledPlayerAction(ai); };
            creators["fathom-lord karathress main tank position boss"] = [](PlayerbotAI* ai) { return new SscKarathressMainTankPositionBossAction(ai); };
            creators["fathom-lord karathress first assist tank position caribdis"] = [](PlayerbotAI* ai) { return new SscKarathressFirstAssistTankPositionCaribdisAction(ai); };
            creators["fathom-lord karathress second assist tank position sharkkis"] = [](PlayerbotAI* ai) { return new SscKarathressSecondAssistTankPositionSharkkisAction(ai); };
            creators["fathom-lord karathress third assist tank position tidalvess"] = [](PlayerbotAI* ai) { return new SscKarathressThirdAssistTankPositionTidalvessAction(ai); };
            creators["fathom-lord karathress position caribdis tank healer"] = [](PlayerbotAI* ai) { return new SscKarathressPositionCaribdisTankHealerAction(ai); };
            creators["fathom-lord karathress misdirect bosses to tanks"] = [](PlayerbotAI* ai) { return new SscKarathressMisdirectBossesToTanksAction(ai); };
            creators["fathom-lord karathress assign dps priority"] = [](PlayerbotAI* ai) { return new SscKarathressAssignDpsPriorityAction(ai); };
            creators["fathom-lord karathress manage dps timer"] = [](PlayerbotAI* ai) { return new SscKarathressManageDpsTimerAction(ai); };
            creators["morogrim tidewalker misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new SscMorogrimMisdirectBossToMainTankAction(ai); };
            creators["morogrim tidewalker move boss to tank position"] = [](PlayerbotAI* ai) { return new SscMorogrimMoveBossToTankPositionAction(ai); };
            creators["morogrim tidewalker phase 2 reposition ranged"] = [](PlayerbotAI* ai) { return new SscMorogrimPhase2RepositionRangedAction(ai); };
            creators["lady vashj main tank position boss"] = [](PlayerbotAI* ai) { return new SscVashjMainTankPositionBossAction(ai); };
            creators["lady vashj phase 1 spread ranged in arc"] = [](PlayerbotAI* ai) { return new SscVashjPhase1SpreadRangedInArcAction(ai); };
            creators["lady vashj set grounding totem in main tank group"] = [](PlayerbotAI* ai) { return new SscVashjSetGroundingTotemInMainTankGroupAction(ai); };
            creators["lady vashj misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new SscVashjMisdirectBossToMainTankAction(ai); };
            creators["lady vashj static charge move away from group"] = [](PlayerbotAI* ai) { return new SscVashjStaticChargeMoveAwayFromGroupAction(ai); };
            creators["lady vashj loot tainted core"] = [](PlayerbotAI* ai) { return new SscVashjLootTaintedCoreAction(ai); };
            creators["lady vashj destroy tainted core"] = [](PlayerbotAI* ai) { return new SscVashjDestroyTaintedCoreAction(ai); };
            creators["lady vashj assign phase 2 and phase 3 dps priority"] = [](PlayerbotAI* ai) { return new SscVashjAssignPhase2AndPhase3DpsPriorityAction(ai); };
            creators["lady vashj tank attack and move away strider"] = [](PlayerbotAI* ai) { return new SscVashjTankAttackAndMoveAwayStriderAction(ai); };
            creators["lady vashj use free action abilities"] = [](PlayerbotAI* ai) { return new SscVashjUseFreeActionAbilitiesAction(ai); };
            creators["hydross the unstable frost phase spread out"] = [](PlayerbotAI* ai) { return new SscHydrossFrostPhaseSpreadOutAction(ai); };
            creators["leotheras the blind position ranged"] = [](PlayerbotAI* ai) { return new SscLeotherasPositionRangedAction(ai); };
            creators["lady vashj avoid toxic spores"] = [](PlayerbotAI* ai) { return new SscVashjAvoidToxicSporesAction(ai); };
            creators["lady vashj pass the tainted core"] = [](PlayerbotAI* ai) { return new SscVashjPassTheTaintedCoreAction(ai); };
            creators["leotheras the blind destroy inner demon"] = [](PlayerbotAI* ai) { return new SscLeotherasDestroyInnerDemonAction(ai); };
        }
    };

    class SscTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        SscTriggerContext()
        {
            creators["serpent shrine cavern bot is not in combat"] = [](PlayerbotAI* ai) { return new SscBotIsNotInCombatTrigger(ai); };
            creators["underbog colossus spawned toxic pool after death"] = [](PlayerbotAI* ai) { return new SscUnderbogColossusSpawnedToxicPoolAfterDeathTrigger(ai); };
            creators["greyheart tidecaller water elemental totem spawned"] = [](PlayerbotAI* ai) { return new SscGreyheartTidecallerWaterElementalTotemSpawnedTrigger(ai); };
            creators["hydross the unstable bot is frost tank"] = [](PlayerbotAI* ai) { return new SscHydrossBotIsFrostTankTrigger(ai); };
            creators["hydross the unstable bot is nature tank"] = [](PlayerbotAI* ai) { return new SscHydrossBotIsNatureTankTrigger(ai); };
            creators["hydross the unstable elementals spawned"] = [](PlayerbotAI* ai) { return new SscHydrossElementalsSpawnedTrigger(ai); };
            creators["hydross the unstable danger from water tombs"] = [](PlayerbotAI* ai) { return new SscHydrossDangerFromWaterTombsTrigger(ai); };
            creators["hydross the unstable tank needs aggro upon phase change"] = [](PlayerbotAI* ai) { return new SscHydrossTankNeedsAggroUponPhaseChangeTrigger(ai); };
            creators["hydross the unstable aggro resets upon phase change"] = [](PlayerbotAI* ai) { return new SscHydrossAggroResetsUponPhaseChangeTrigger(ai); };
            creators["hydross the unstable need to manage timers"] = [](PlayerbotAI* ai) { return new SscHydrossNeedToManageTimersTrigger(ai); };
            creators["the lurker below spout is active"] = [](PlayerbotAI* ai) { return new SscLurkerSpoutIsActiveTrigger(ai); };
            creators["the lurker below boss is active for main tank"] = [](PlayerbotAI* ai) { return new SscLurkerBossIsActiveForMainTankTrigger(ai); };
            creators["the lurker below boss casts geyser"] = [](PlayerbotAI* ai) { return new SscLurkerBossCastsGeyserTrigger(ai); };
            creators["the lurker below boss is submerged"] = [](PlayerbotAI* ai) { return new SscLurkerBossIsSubmergedTrigger(ai); };
            creators["the lurker below need to prepare timer for spout"] = [](PlayerbotAI* ai) { return new SscLurkerNeedToPrepareTimerForSpoutTrigger(ai); };
            creators["leotheras the blind boss is inactive"] = [](PlayerbotAI* ai) { return new SscLeotherasBossIsInactiveTrigger(ai); };
            creators["leotheras the blind boss transformed into demon form"] = [](PlayerbotAI* ai) { return new SscLeotherasBossTransformedIntoDemonFormTrigger(ai); };
            creators["leotheras the blind only warlock should tank demon form"] = [](PlayerbotAI* ai) { return new SscLeotherasOnlyWarlockShouldTankDemonFormTrigger(ai); };
            creators["leotheras the blind boss engaged by ranged"] = [](PlayerbotAI* ai) { return new SscLeotherasBossEngagedByRangedTrigger(ai); };
            creators["leotheras the blind boss channeling whirlwind"] = [](PlayerbotAI* ai) { return new SscLeotherasBossChannelingWhirlwindTrigger(ai); };
            creators["leotheras the blind bot has too many chaos blast stacks"] = [](PlayerbotAI* ai) { return new SscLeotherasBotHasTooManyChaosBlastStacksTrigger(ai); };
            creators["leotheras the blind inner demon has awakened"] = [](PlayerbotAI* ai) { return new SscLeotherasInnerDemonHasAwakenedTrigger(ai); };
            creators["leotheras the blind entered final phase"] = [](PlayerbotAI* ai) { return new SscLeotherasEnteredFinalPhaseTrigger(ai); };
            creators["leotheras the blind demon form tank needs aggro"] = [](PlayerbotAI* ai) { return new SscLeotherasDemonFormTankNeedsAggroTrigger(ai); };
            creators["leotheras the blind boss wipes aggro upon phase change"] = [](PlayerbotAI* ai) { return new SscLeotherasBossWipesAggroUponPhaseChangeTrigger(ai); };
            creators["leotheras the blind demon form tank needs healing"] = [](PlayerbotAI* ai) { return new SscLeotherasDemonFormTankNeedsHealingTrigger(ai); };
            creators["leotheras the blind player is mind controlled"] = [](PlayerbotAI* ai) { return new SscLeotherasPlayerIsMindControlledTrigger(ai); };
            creators["fathom-lord karathress boss engaged by main tank"] = [](PlayerbotAI* ai) { return new SscKarathressBossEngagedByMainTankTrigger(ai); };
            creators["fathom-lord karathress caribdis engaged by first assist tank"] = [](PlayerbotAI* ai) { return new SscKarathressCaribdisEngagedByFirstAssistTankTrigger(ai); };
            creators["fathom-lord karathress sharkkis engaged by second assist tank"] = [](PlayerbotAI* ai) { return new SscKarathressSharkkisEngagedBySecondAssistTankTrigger(ai); };
            creators["fathom-lord karathress tidalvess engaged by third assist tank"] = [](PlayerbotAI* ai) { return new SscKarathressTidalvessEngagedByThirdAssistTankTrigger(ai); };
            creators["fathom-lord karathress caribdis tank needs dedicated healer"] = [](PlayerbotAI* ai) { return new SscKarathressCaribdisTankNeedsDedicatedHealerTrigger(ai); };
            creators["fathom-lord karathress pulling bosses"] = [](PlayerbotAI* ai) { return new SscKarathressPullingBossesTrigger(ai); };
            creators["fathom-lord karathress determining kill order"] = [](PlayerbotAI* ai) { return new SscKarathressDeterminingKillOrderTrigger(ai); };
            creators["fathom-lord karathress tanks need to establish aggro"] = [](PlayerbotAI* ai) { return new SscKarathressTanksNeedToEstablishAggroTrigger(ai); };
            creators["morogrim tidewalker pulling boss"] = [](PlayerbotAI* ai) { return new SscMorogrimPullingBossTrigger(ai); };
            creators["morogrim tidewalker boss engaged by main tank"] = [](PlayerbotAI* ai) { return new SscMorogrimBossEngagedByMainTankTrigger(ai); };
            creators["morogrim tidewalker water globules are incoming"] = [](PlayerbotAI* ai) { return new SscMorogrimWaterGlobulesAreIncomingTrigger(ai); };
            creators["lady vashj boss engaged by main tank"] = [](PlayerbotAI* ai) { return new SscVashjBossEngagedByMainTankTrigger(ai); };
            creators["lady vashj boss engaged by ranged in phase 1"] = [](PlayerbotAI* ai) { return new SscVashjBossEngagedByRangedInPhase1Trigger(ai); };
            creators["lady vashj casts shock blast on highest aggro"] = [](PlayerbotAI* ai) { return new SscVashjCastsShockBlastOnHighestAggroTrigger(ai); };
            creators["lady vashj bot has static charge"] = [](PlayerbotAI* ai) { return new SscVashjBotHasStaticChargeTrigger(ai); };
            creators["lady vashj pulling boss in phase 1 and phase 3"] = [](PlayerbotAI* ai) { return new SscVashjPullingBossInPhase1AndPhase3Trigger(ai); };
            creators["lady vashj tainted core can be looted"] = [](PlayerbotAI* ai) { return new SscVashjTaintedCoreCanBeLootedTrigger(ai); };
            creators["lady vashj tainted core was looted"] = [](PlayerbotAI* ai) { return new SscVashjTaintedCoreWasLootedTrigger(ai); };
            creators["lady vashj tainted core is unusable"] = [](PlayerbotAI* ai) { return new SscVashjTaintedCoreIsUnusableTrigger(ai); };
            creators["lady vashj adds spawn in phase 2 and phase 3"] = [](PlayerbotAI* ai) { return new SscVashjAddsSpawnInPhase2AndPhase3Trigger(ai); };
            creators["lady vashj coilfang strider is approaching"] = [](PlayerbotAI* ai) { return new SscVashjCoilfangStriderIsApproachingTrigger(ai); };
            creators["lady vashj toxic sporebats are spewing poison clouds"] = [](PlayerbotAI* ai) { return new SscVashjToxicSporebatsAreSpewingPoisonCloudsTrigger(ai); };
            creators["lady vashj bot is entangled in toxic spores or static charge"] = [](PlayerbotAI* ai) { return new SscVashjBotIsEntangledInToxicSporesOrStaticChargeTrigger(ai); };
        }
    };

    class SscValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        SscValueContext()
        {
        }
    };
}
