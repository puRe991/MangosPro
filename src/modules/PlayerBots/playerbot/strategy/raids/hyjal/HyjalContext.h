#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "HyjalStrategy.h"
#include "HyjalActions.h"
#include "HyjalTriggers.h"
#include "HyjalValues.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

// Hyjal Summit (map 534) - strategy "boss hyjal"
// Boss tactics ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/Hyjal) and adapted for this core.
namespace ai
{
    class HyjalStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        HyjalStrategyContext()
        {
            creators["boss hyjal"] = [](PlayerbotAI* ai) { return new HyjalBossStrategy(ai); };
        }
    };

    class HyjalActionContext : public NamedObjectContext<Action>
    {
    public:
        HyjalActionContext()
        {
            using namespace hyjal;

            // General
            creators["hyjal reset encounter states"] = [](PlayerbotAI* ai) { return new HyjalResetEncounterStatesAction(ai); };
            creators["hyjal remove dangerous dot"] = [](PlayerbotAI* ai) { return new HyjalRemoveDangerousDotAction(ai); };

            // Rage Winterchill
            creators["rage winterchill misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new HyjalMisdirectBossToMainTankAction(ai, "rage winterchill misdirect boss to main tank", NPC_RAGE_WINTERCHILL); };
            creators["rage winterchill main tank position boss"] = [](PlayerbotAI* ai) { return new HyjalMainTankPositionBossAction(ai, "rage winterchill main tank position boss", NPC_RAGE_WINTERCHILL, WINTERCHILL_TANK_POSITION); };
            creators["rage winterchill spread ranged in circle"] = [](PlayerbotAI* ai) { return new RageWinterchillSpreadRangedInCircleAction(ai); };
            creators["rage winterchill ranged get out of death and decay"] = [](PlayerbotAI* ai) { return new RageWinterchillRangedGetOutOfDeathAndDecayAction(ai); };
            creators["rage winterchill melee maneuver through death and decay"] = [](PlayerbotAI* ai) { return new RageWinterchillMeleeManeuverThroughDeathAndDecayAction(ai); };

            // Anetheron
            creators["anetheron misdirect boss and infernals to tanks"] = [](PlayerbotAI* ai) { return new AnetheronMisdirectBossAndInfernalsToTanksAction(ai); };
            creators["anetheron main tank position boss"] = [](PlayerbotAI* ai) { return new HyjalMainTankPositionBossAction(ai, "anetheron main tank position boss", NPC_ANETHERON, ANETHERON_TANK_POSITION); };
            creators["anetheron spread ranged in circle"] = [](PlayerbotAI* ai) { return new AnetheronSpreadRangedInCircleAction(ai); };
            creators["anetheron move away from inferno target"] = [](PlayerbotAI* ai) { return new AnetheronMoveAwayFromInfernoTargetAction(ai); };
            creators["anetheron bring infernal to infernal tank"] = [](PlayerbotAI* ai) { return new AnetheronBringInfernalToInfernalTankAction(ai); };
            creators["anetheron infernal tank take position"] = [](PlayerbotAI* ai) { return new AnetheronInfernalTankTakePositionAction(ai); };
            creators["anetheron get out of immolation"] = [](PlayerbotAI* ai) { return new AnetheronGetOutOfImmolationAction(ai); };
            creators["anetheron assign dps priority"] = [](PlayerbotAI* ai) { return new AnetheronAssignDpsPriorityAction(ai); };

            // Kaz'rogal
            creators["kaz'rogal misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new HyjalMisdirectBossToMainTankAction(ai, "kaz'rogal misdirect boss to main tank", NPC_KAZROGAL); };
            creators["kaz'rogal main tank position boss"] = [](PlayerbotAI* ai) { return new HyjalMainTankPositionBossAction(ai, "kaz'rogal main tank position boss", NPC_KAZROGAL, KAZROGAL_TANK_POSITION); };
            creators["kaz'rogal assist tanks move in front"] = [](PlayerbotAI* ai) { return new KazrogalAssistTanksMoveInFrontAction(ai); };
            creators["kaz'rogal spread ranged in arc"] = [](PlayerbotAI* ai) { return new KazrogalSpreadRangedInArcAction(ai); };
            creators["kaz'rogal move away from group"] = [](PlayerbotAI* ai) { return new KazrogalMoveAwayFromGroupAction(ai); };
            creators["kaz'rogal activate aspect of the viper"] = [](PlayerbotAI* ai) { return new KazrogalActivateAspectOfTheViperAction(ai); };
            creators["kaz'rogal cancel immunity"] = [](PlayerbotAI* ai) { return new KazrogalCancelImmunityAction(ai); };
            creators["kaz'rogal warlock manage mana"] = [](PlayerbotAI* ai) { return new KazrogalWarlockManageManaAction(ai); };

            // Azgalor
            creators["azgalor misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new HyjalMisdirectBossToMainTankAction(ai, "azgalor misdirect boss to main tank", NPC_AZGALOR); };
            creators["azgalor main tank position boss"] = [](PlayerbotAI* ai) { return new HyjalMainTankPositionBossAction(ai, "azgalor main tank position boss", NPC_AZGALOR, AZGALOR_TANK_POSITION, 60.0f); };
            creators["azgalor disperse ranged"] = [](PlayerbotAI* ai) { return new AzgalorDisperseRangedAction(ai); };
            creators["azgalor melee maneuver through fire"] = [](PlayerbotAI* ai) { return new AzgalorMeleeManeuverThroughFireAction(ai); };
            creators["azgalor ranged get out of rain of fire"] = [](PlayerbotAI* ai) { return new AzgalorRangedGetOutOfRainOfFireAction(ai); };
            creators["azgalor move to doomguard tank"] = [](PlayerbotAI* ai) { return new AzgalorMoveToDoomguardTankAction(ai); };
            creators["azgalor tank position doomguard"] = [](PlayerbotAI* ai) { return new AzgalorTankPositionDoomguardAction(ai); };
            creators["azgalor determine dps priority"] = [](PlayerbotAI* ai) { return new AzgalorDetermineDpsPriorityAction(ai); };

            // Archimonde
            creators["archimonde misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new HyjalMisdirectBossToMainTankAction(ai, "archimonde misdirect boss to main tank", NPC_ARCHIMONDE); };
            creators["archimonde move boss to initial position"] = [](PlayerbotAI* ai) { return new HyjalMainTankPositionBossAction(ai, "archimonde move boss to initial position", NPC_ARCHIMONDE, ARCHIMONDE_INITIAL_POSITION, 60.0f); };
            creators["archimonde set tremor totem"] = [](PlayerbotAI* ai) { return new ArchimondeSetTremorTotemAction(ai); };
            creators["archimonde keep air burst away from tank"] = [](PlayerbotAI* ai) { return new ArchimondeKeepAirBurstAwayFromTankAction(ai); };
            creators["archimonde spread ranged"] = [](PlayerbotAI* ai) { return new ArchimondeSpreadRangedAction(ai); };
            creators["archimonde avoid doomfire"] = [](PlayerbotAI* ai) { return new ArchimondeAvoidDoomfireAction(ai); };
        }
    };

    class HyjalTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        HyjalTriggerContext()
        {
            using namespace hyjal;

            // General
            creators["hyjal no encounter in progress"] = [](PlayerbotAI* ai) { return new HyjalNoEncounterInProgressTrigger(ai); };

            // Rage Winterchill
            creators["rage winterchill pulling boss"] = [](PlayerbotAI* ai) { return new HyjalPullingBossTrigger(ai, "rage winterchill pulling boss", NPC_RAGE_WINTERCHILL); };
            creators["rage winterchill should be tanked"] = [](PlayerbotAI* ai) { return new HyjalBossShouldBeTankedTrigger(ai, "rage winterchill should be tanked", NPC_RAGE_WINTERCHILL, 0.0f, false); };
            creators["rage winterchill ranged should spread"] = [](PlayerbotAI* ai) { return new RageWinterchillRangedShouldSpreadTrigger(ai); };
            creators["rage winterchill melee near death and decay"] = [](PlayerbotAI* ai) { return new RageWinterchillMeleeNearDeathAndDecayTrigger(ai); };
            creators["rage winterchill ranged in death and decay"] = [](PlayerbotAI* ai) { return new RageWinterchillRangedInDeathAndDecayTrigger(ai); };

            // Anetheron
            creators["anetheron pulling boss or infernal"] = [](PlayerbotAI* ai) { return new AnetheronPullingBossOrInfernalTrigger(ai); };
            creators["anetheron should be tanked"] = [](PlayerbotAI* ai) { return new HyjalBossShouldBeTankedTrigger(ai, "anetheron should be tanked", NPC_ANETHERON); };
            creators["anetheron ranged should spread"] = [](PlayerbotAI* ai) { return new AnetheronRangedShouldSpreadTrigger(ai); };
            creators["anetheron near inferno target"] = [](PlayerbotAI* ai) { return new AnetheronNearInfernoTargetTrigger(ai); };
            creators["anetheron targeted by infernal"] = [](PlayerbotAI* ai) { return new AnetheronTargetedByInfernalTrigger(ai); };
            creators["anetheron infernals pulse immolation"] = [](PlayerbotAI* ai) { return new AnetheronInfernalsPulseImmolationTrigger(ai); };
            creators["anetheron infernals should be tanked away"] = [](PlayerbotAI* ai) { return new AnetheronInfernalsShouldBeTankedAwayTrigger(ai); };
            creators["anetheron should divide dps"] = [](PlayerbotAI* ai) { return new AnetheronShouldDivideDpsTrigger(ai); };

            // Kaz'rogal
            creators["kaz'rogal pulling boss"] = [](PlayerbotAI* ai) { return new HyjalPullingBossTrigger(ai, "kaz'rogal pulling boss", NPC_KAZROGAL); };
            creators["kaz'rogal should be tanked"] = [](PlayerbotAI* ai) { return new HyjalBossShouldBeTankedTrigger(ai, "kaz'rogal should be tanked", NPC_KAZROGAL); };
            creators["kaz'rogal can split malevolent cleave damage"] = [](PlayerbotAI* ai) { return new KazrogalCanSplitMalevolentCleaveDamageTrigger(ai); };
            creators["kaz'rogal ranged should avoid war stomp"] = [](PlayerbotAI* ai) { return new KazrogalRangedShouldAvoidWarStompTrigger(ai); };
            creators["kaz'rogal low on mana"] = [](PlayerbotAI* ai) { return new KazrogalLowOnManaTrigger(ai); };
            creators["kaz'rogal hunter should preserve mana"] = [](PlayerbotAI* ai) { return new KazrogalHunterShouldPreserveManaTrigger(ai); };
            creators["kaz'rogal mark on mage or paladin"] = [](PlayerbotAI* ai) { return new KazrogalMarkOnMageOrPaladinTrigger(ai); };
            creators["kaz'rogal immunity no longer needed"] = [](PlayerbotAI* ai) { return new KazrogalImmunityNoLongerNeededTrigger(ai); };
            creators["kaz'rogal warlock should manage mana"] = [](PlayerbotAI* ai) { return new KazrogalWarlockShouldManageManaTrigger(ai); };

            // Azgalor
            creators["azgalor pulling boss"] = [](PlayerbotAI* ai) { return new HyjalPullingBossTrigger(ai, "azgalor pulling boss", NPC_AZGALOR); };
            creators["azgalor should be tanked"] = [](PlayerbotAI* ai) { return new HyjalBossShouldBeTankedTrigger(ai, "azgalor should be tanked", NPC_AZGALOR); };
            creators["azgalor ranged should spread"] = [](PlayerbotAI* ai) { return new AzgalorRangedShouldSpreadTrigger(ai); };
            creators["azgalor melee near rain of fire"] = [](PlayerbotAI* ai) { return new AzgalorMeleeNearRainOfFireTrigger(ai); };
            creators["azgalor ranged in rain of fire"] = [](PlayerbotAI* ai) { return new AzgalorRangedInRainOfFireTrigger(ai); };
            creators["azgalor bot is doomed"] = [](PlayerbotAI* ai) { return new AzgalorBotIsDoomedTrigger(ai); };
            creators["azgalor should control doomguards"] = [](PlayerbotAI* ai) { return new AzgalorShouldControlDoomguardsTrigger(ai); };
            creators["azgalor should divide dps"] = [](PlayerbotAI* ai) { return new AzgalorShouldDivideDpsTrigger(ai); };

            // Archimonde
            creators["archimonde pulling boss"] = [](PlayerbotAI* ai) { return new HyjalPullingBossTrigger(ai, "archimonde pulling boss", NPC_ARCHIMONDE); };
            creators["archimonde should be tanked"] = [](PlayerbotAI* ai) { return new HyjalBossShouldBeTankedTrigger(ai, "archimonde should be tanked", NPC_ARCHIMONDE, raidboss::BOSS_ENGAGED_HEALTH_PCT, false); };
            creators["archimonde shaman should protect against fear"] = [](PlayerbotAI* ai) { return new ArchimondeShamanShouldProtectAgainstFearTrigger(ai); };
            creators["archimonde casting air burst"] = [](PlayerbotAI* ai) { return new ArchimondeCastingAirBurstTrigger(ai); };
            creators["archimonde ranged should spread"] = [](PlayerbotAI* ai) { return new ArchimondeRangedShouldSpreadTrigger(ai); };
            creators["archimonde near doomfire"] = [](PlayerbotAI* ai) { return new ArchimondeNearDoomfireTrigger(ai); };
            creators["archimonde stood in doomfire"] = [](PlayerbotAI* ai) { return new ArchimondeStoodInDoomfireTrigger(ai); };
        }
    };

    class HyjalValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        HyjalValueContext()
        {
            using namespace hyjal;

            creators["hyjal boss"] = [](PlayerbotAI* ai) { return new HyjalBossValue(ai); };
            creators["hyjal infernals"] = [](PlayerbotAI* ai) { return new HyjalInfernalsValue(ai); };
            creators["hyjal death and decay"] = [](PlayerbotAI* ai) { return new HyjalHazardPositionsValue(ai, "hyjal death and decay", SPELL_DEATH_AND_DECAY, DEATH_AND_DECAY_SEARCH_RADIUS); };
            creators["hyjal rain of fire"] = [](PlayerbotAI* ai) { return new HyjalHazardPositionsValue(ai, "hyjal rain of fire", SPELL_RAIN_OF_FIRE, RAIN_OF_FIRE_SEARCH_RADIUS); };
            creators["hyjal doomfire trail"] = [](PlayerbotAI* ai) { return new HyjalHazardPositionsValue(ai, "hyjal doomfire trail", SPELL_DOOMFIRE_TRAIL, DOOMFIRE_SEARCH_RADIUS); };
            creators["kaz'rogal below mana threshold"] = [](PlayerbotAI* ai) { return new KazrogalBelowManaThresholdValue(ai); };
            creators["rage winterchill ranged position reached"] = [](PlayerbotAI* ai) { return new HyjalRangedPositionReachedValue(ai, "rage winterchill ranged position reached"); };
            creators["anetheron ranged position reached"] = [](PlayerbotAI* ai) { return new HyjalRangedPositionReachedValue(ai, "anetheron ranged position reached"); };
        }
    };
}
