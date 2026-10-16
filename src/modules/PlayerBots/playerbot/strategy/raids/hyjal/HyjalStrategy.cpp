// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/Hyjal/HyjalStrategy) and adapted for this core.
#include "playerbot/playerbot.h"
#include "HyjalStrategy.h"
#include "HyjalMultipliers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;

namespace
{
    void AddTrigger(std::list<TriggerNode*>& triggers, const char* trigger, const char* action, float relevance)
    {
        triggers.push_back(new TriggerNode(trigger, NextAction::array(0, new NextAction(action, relevance), NULL)));
    }

    const float RAID = raidboss::ACTION_RAID;
}

void HyjalBossStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // General
    AddTrigger(triggers, "hyjal no encounter in progress", "hyjal reset encounter states", ACTION_EMERGENCY + 9);

    // Rage Winterchill
    AddTrigger(triggers, "rage winterchill pulling boss", "rage winterchill misdirect boss to main tank", RAID + 1);
    AddTrigger(triggers, "rage winterchill should be tanked", "rage winterchill main tank position boss", RAID);
    AddTrigger(triggers, "rage winterchill ranged should spread", "rage winterchill spread ranged in circle", RAID);
    AddTrigger(triggers, "rage winterchill melee near death and decay", "rage winterchill melee maneuver through death and decay", ACTION_EMERGENCY + 1);
    AddTrigger(triggers, "rage winterchill ranged in death and decay", "rage winterchill ranged get out of death and decay", ACTION_EMERGENCY + 1);

    // Anetheron
    AddTrigger(triggers, "anetheron pulling boss or infernal", "anetheron misdirect boss and infernals to tanks", RAID + 2);
    AddTrigger(triggers, "anetheron should be tanked", "anetheron main tank position boss", RAID);
    AddTrigger(triggers, "anetheron ranged should spread", "anetheron spread ranged in circle", RAID + 1);
    AddTrigger(triggers, "anetheron targeted by infernal", "anetheron bring infernal to infernal tank", ACTION_EMERGENCY + 7);
    AddTrigger(triggers, "anetheron near inferno target", "anetheron move away from inferno target", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "anetheron infernals pulse immolation", "anetheron get out of immolation", ACTION_EMERGENCY + 1);
    AddTrigger(triggers, "anetheron infernals should be tanked away", "anetheron infernal tank take position", ACTION_EMERGENCY + 1);
    AddTrigger(triggers, "anetheron should divide dps", "anetheron assign dps priority", RAID);

    // Kaz'rogal
    AddTrigger(triggers, "kaz'rogal pulling boss", "kaz'rogal misdirect boss to main tank", RAID + 1);
    AddTrigger(triggers, "kaz'rogal should be tanked", "kaz'rogal main tank position boss", RAID);
    AddTrigger(triggers, "kaz'rogal can split malevolent cleave damage", "kaz'rogal assist tanks move in front", RAID);
    AddTrigger(triggers, "kaz'rogal ranged should avoid war stomp", "kaz'rogal spread ranged in arc", RAID);
    AddTrigger(triggers, "kaz'rogal low on mana", "kaz'rogal move away from group", ACTION_EMERGENCY + 2);
    AddTrigger(triggers, "kaz'rogal hunter should preserve mana", "kaz'rogal activate aspect of the viper", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "kaz'rogal mark on mage or paladin", "hyjal remove dangerous dot", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "kaz'rogal immunity no longer needed", "kaz'rogal cancel immunity", RAID + 1);
    AddTrigger(triggers, "kaz'rogal warlock should manage mana", "kaz'rogal warlock manage mana", ACTION_EMERGENCY + 6);

    // Azgalor
    AddTrigger(triggers, "azgalor pulling boss", "azgalor misdirect boss to main tank", RAID + 2);
    AddTrigger(triggers, "azgalor should be tanked", "azgalor main tank position boss", RAID);
    AddTrigger(triggers, "azgalor ranged should spread", "azgalor disperse ranged", RAID + 1);
    AddTrigger(triggers, "azgalor melee near rain of fire", "azgalor melee maneuver through fire", ACTION_EMERGENCY + 1);
    AddTrigger(triggers, "azgalor ranged in rain of fire", "azgalor ranged get out of rain of fire", ACTION_EMERGENCY + 1);
    AddTrigger(triggers, "azgalor bot is doomed", "azgalor move to doomguard tank", ACTION_EMERGENCY + 2);
    AddTrigger(triggers, "azgalor should control doomguards", "azgalor tank position doomguard", RAID);
    AddTrigger(triggers, "azgalor should divide dps", "azgalor determine dps priority", RAID);

    // Archimonde
    AddTrigger(triggers, "archimonde pulling boss", "archimonde misdirect boss to main tank", RAID + 1);
    AddTrigger(triggers, "archimonde should be tanked", "archimonde move boss to initial position", RAID);
    AddTrigger(triggers, "archimonde shaman should protect against fear", "archimonde set tremor totem", RAID + 1);
    AddTrigger(triggers, "archimonde casting air burst", "archimonde keep air burst away from tank", ACTION_EMERGENCY + 8);
    AddTrigger(triggers, "archimonde ranged should spread", "archimonde spread ranged", RAID);
    AddTrigger(triggers, "archimonde near doomfire", "archimonde avoid doomfire", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "archimonde stood in doomfire", "hyjal remove dangerous dot", ACTION_EMERGENCY + 7);
}

void HyjalBossStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AddTrigger(triggers, "hyjal no encounter in progress", "hyjal reset encounter states", ACTION_EMERGENCY + 9);
}

// Used when the strategy is also added to the reaction engine (checked while casting): replaces the
// spell interrupt hooks of mod-playerbots for Inferno, Air Burst and Doomfire.
void HyjalBossStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    AddTrigger(triggers, "anetheron near inferno target", "anetheron move away from inferno target", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "archimonde casting air burst", "archimonde keep air burst away from tank", ACTION_EMERGENCY + 8);
    AddTrigger(triggers, "archimonde near doomfire", "archimonde avoid doomfire", ACTION_EMERGENCY + 6);
}

void HyjalBossStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    // General
    multipliers.push_back(new HyjalDelayDpsCooldownsMultiplier(ai));
    multipliers.push_back(new HyjalDisableDisperseAndTankFaceMultiplier(ai));

    // Rage Winterchill
    multipliers.push_back(new RageWinterchillMeleeControlAvoidanceMultiplier(ai));
    multipliers.push_back(new RageWinterchillRangedControlAvoidanceMultiplier(ai));

    // Anetheron
    multipliers.push_back(new AnetheronDisableAssistTargetingMultiplier(ai));
    multipliers.push_back(new AnetheronAvoidAccidentalInfernalAggroMultiplier(ai));
    multipliers.push_back(new AnetheronInfernalTargetRunToPositionMultiplier(ai));
    multipliers.push_back(new AnetheronControlMovementMultiplier(ai));
    multipliers.push_back(new AnetheronControlMisdirectionMultiplier(ai));

    // Kaz'rogal
    multipliers.push_back(new KazrogalControlLowManaMovementMultiplier(ai));
    multipliers.push_back(new KazrogalKeepAspectOfTheViperActiveMultiplier(ai));

    // Azgalor
    multipliers.push_back(new AzgalorDisableAutoTargetingAndPositioningMultiplier(ai));
    multipliers.push_back(new AzgalorDoomedBotPrioritizePositioningMultiplier(ai));
    multipliers.push_back(new AzgalorMeleeDpsControlAvoidanceMultiplier(ai));
    multipliers.push_back(new AzgalorRangedControlAvoidanceMultiplier(ai));

    // Archimonde
    multipliers.push_back(new ArchimondeControlDoomfireAvoidanceMultiplier(ai));
    multipliers.push_back(new ArchimondeSetTremorTotemMultiplier(ai));
}
