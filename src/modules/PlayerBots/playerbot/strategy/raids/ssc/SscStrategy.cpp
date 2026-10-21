// Ported from mod-playerbots (src/Ai/Raid/SSC/SSCStrategy.cpp, GPL v2+) and adapted for this core.
// Priorities: mod ACTION_RAID -> raidboss::ACTION_RAID (85), mod ACTION_EMERGENCY + n -> 90 + n (max 99).
#include "playerbot/playerbot.h"
#include "SscStrategy.h"
#include "SscMultipliers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;

void SscStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "serpent shrine cavern bot is not in combat",
        NextAction::array(0, new NextAction("serpent shrine cavern erase timers and trackers", ACTION_EMERGENCY + 9), NULL)));

    // Left over Tainted Cores (after a wipe or a finished phase 2)
    triggers.push_back(new TriggerNode(
        "lady vashj tainted core is unusable",
        NextAction::array(0, new NextAction("lady vashj destroy tainted core", ACTION_EMERGENCY + 1), NULL)));
}

void SscStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "underbog colossus spawned toxic pool after death",
        NextAction::array(0, new NextAction("underbog colossus escape toxic pool", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "greyheart tidecaller water elemental totem spawned",
        NextAction::array(0, new NextAction("greyheart tidecaller mark water elemental totem", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "hydross the unstable bot is frost tank",
        NextAction::array(0, new NextAction("hydross the unstable position frost tank", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "hydross the unstable bot is nature tank",
        NextAction::array(0, new NextAction("hydross the unstable position nature tank", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "hydross the unstable elementals spawned",
        NextAction::array(0, new NextAction("hydross the unstable prioritize elemental adds", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "hydross the unstable danger from water tombs",
        NextAction::array(0, new NextAction("hydross the unstable frost phase spread out", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "hydross the unstable tank needs aggro upon phase change",
        NextAction::array(0, new NextAction("hydross the unstable misdirect boss to tank", ACTION_EMERGENCY + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "hydross the unstable aggro resets upon phase change",
        NextAction::array(0, new NextAction("hydross the unstable stop dps upon phase change", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "hydross the unstable need to manage timers",
        NextAction::array(0, new NextAction("hydross the unstable manage timers", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "the lurker below spout is active",
        NextAction::array(0, new NextAction("the lurker below run around behind boss", ACTION_EMERGENCY + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "the lurker below boss is active for main tank",
        NextAction::array(0, new NextAction("the lurker below position main tank", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "the lurker below boss casts geyser",
        NextAction::array(0, new NextAction("the lurker below spread ranged in arc", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "the lurker below boss is submerged",
        NextAction::array(0, new NextAction("the lurker below tanks pick up adds", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "the lurker below need to prepare timer for spout",
        NextAction::array(0, new NextAction("the lurker below manage spout timer", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "leotheras the blind boss is inactive",
        NextAction::array(0, new NextAction("leotheras the blind target spellbinders", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "leotheras the blind boss transformed into demon form",
        NextAction::array(0, new NextAction("leotheras the blind demon form tank attack boss", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "leotheras the blind only warlock should tank demon form",
        NextAction::array(0, new NextAction("leotheras the blind melee tanks don't attack demon form", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "leotheras the blind boss engaged by ranged",
        NextAction::array(0, new NextAction("leotheras the blind position ranged", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "leotheras the blind boss channeling whirlwind",
        NextAction::array(0, new NextAction("leotheras the blind run away from whirlwind", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "leotheras the blind bot has too many chaos blast stacks",
        NextAction::array(0, new NextAction("leotheras the blind melee dps run away from boss", ACTION_EMERGENCY + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "leotheras the blind inner demon has awakened",
        NextAction::array(0, new NextAction("leotheras the blind destroy inner demon", ACTION_EMERGENCY + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "leotheras the blind entered final phase",
        NextAction::array(0, new NextAction("leotheras the blind final phase assign dps priority", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "leotheras the blind demon form tank needs aggro",
        NextAction::array(0, new NextAction("leotheras the blind misdirect boss to demon form tank", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "leotheras the blind boss wipes aggro upon phase change",
        NextAction::array(0, new NextAction("leotheras the blind manage dps wait timers", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "fathom-lord karathress boss engaged by main tank",
        NextAction::array(0, new NextAction("fathom-lord karathress main tank position boss", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fathom-lord karathress caribdis engaged by first assist tank",
        NextAction::array(0, new NextAction("fathom-lord karathress first assist tank position caribdis", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fathom-lord karathress sharkkis engaged by second assist tank",
        NextAction::array(0, new NextAction("fathom-lord karathress second assist tank position sharkkis", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fathom-lord karathress tidalvess engaged by third assist tank",
        NextAction::array(0, new NextAction("fathom-lord karathress third assist tank position tidalvess", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fathom-lord karathress caribdis tank needs dedicated healer",
        NextAction::array(0, new NextAction("fathom-lord karathress position caribdis tank healer", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "fathom-lord karathress pulling bosses",
        NextAction::array(0, new NextAction("fathom-lord karathress misdirect bosses to tanks", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "fathom-lord karathress determining kill order",
        NextAction::array(0, new NextAction("fathom-lord karathress assign dps priority", ACTION_EMERGENCY + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "fathom-lord karathress tanks need to establish aggro",
        NextAction::array(0, new NextAction("fathom-lord karathress manage dps timer", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "morogrim tidewalker boss engaged by main tank",
        NextAction::array(0, new NextAction("morogrim tidewalker move boss to tank position", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "morogrim tidewalker water globules are incoming",
        NextAction::array(0, new NextAction("morogrim tidewalker phase 2 reposition ranged", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "morogrim tidewalker pulling boss",
        NextAction::array(0, new NextAction("morogrim tidewalker misdirect boss to main tank", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lady vashj boss engaged by main tank",
        NextAction::array(0, new NextAction("lady vashj main tank position boss", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lady vashj boss engaged by ranged in phase 1",
        NextAction::array(0, new NextAction("lady vashj phase 1 spread ranged in arc", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lady vashj casts shock blast on highest aggro",
        NextAction::array(0, new NextAction("lady vashj set grounding totem in main tank group", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lady vashj bot has static charge",
        NextAction::array(0, new NextAction("lady vashj static charge move away from group", ACTION_EMERGENCY + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "lady vashj pulling boss in phase 1 and phase 3",
        NextAction::array(0, new NextAction("lady vashj misdirect boss to main tank", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "lady vashj tainted core can be looted",
        NextAction::array(0, new NextAction("lady vashj loot tainted core", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "lady vashj tainted core was looted",
        NextAction::array(0, new NextAction("lady vashj pass the tainted core", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "lady vashj tainted core is unusable",
        NextAction::array(0, new NextAction("lady vashj destroy tainted core", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lady vashj adds spawn in phase 2 and phase 3",
        NextAction::array(0, new NextAction("lady vashj assign phase 2 and phase 3 dps priority", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lady vashj coilfang strider is approaching",
        NextAction::array(0, new NextAction("lady vashj tank attack and move away strider", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "lady vashj toxic sporebats are spewing poison clouds",
        NextAction::array(0, new NextAction("lady vashj avoid toxic spores", ACTION_EMERGENCY + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "lady vashj bot is entangled in toxic spores or static charge",
        NextAction::array(0, new NextAction("lady vashj use free action abilities", ACTION_EMERGENCY + 7), NULL)));
}

void SscStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new SscUnderbogColossusEscapeToxicPoolMultiplier(ai));
    multipliers.push_back(new SscHydrossDisableTankActionsMultiplier(ai));
    multipliers.push_back(new SscHydrossWaitForDpsMultiplier(ai));
    multipliers.push_back(new SscHydrossControlMisdirectionMultiplier(ai));
    multipliers.push_back(new SscLurkerStayAwayFromSpoutMultiplier(ai));
    multipliers.push_back(new SscLurkerMaintainRangedSpreadMultiplier(ai));
    multipliers.push_back(new SscLurkerDisableTankAssistMultiplier(ai));
    multipliers.push_back(new SscLeotherasAvoidWhirlwindMultiplier(ai));
    multipliers.push_back(new SscLeotherasDisableTankActionsMultiplier(ai));
    multipliers.push_back(new SscLeotherasMeleeDpsAvoidChaosBlastMultiplier(ai));
    multipliers.push_back(new SscLeotherasFocusOnInnerDemonMultiplier(ai));
    multipliers.push_back(new SscLeotherasWaitForDpsMultiplier(ai));
    multipliers.push_back(new SscLeotherasDelayBloodlustAndHeroismMultiplier(ai));
    multipliers.push_back(new SscKarathressDisableTankActionsMultiplier(ai));
    multipliers.push_back(new SscKarathressDisableAoeMultiplier(ai));
    multipliers.push_back(new SscKarathressControlMisdirectionMultiplier(ai));
    multipliers.push_back(new SscKarathressWaitForDpsMultiplier(ai));
    multipliers.push_back(new SscKarathressCaribdisTankHealerMaintainPositionMultiplier(ai));
    multipliers.push_back(new SscMorogrimDelayBloodlustAndHeroismMultiplier(ai));
    multipliers.push_back(new SscMorogrimDisableTankActionsMultiplier(ai));
    multipliers.push_back(new SscMorogrimMaintainPhase2StackingMultiplier(ai));
    multipliers.push_back(new SscVashjDelayCooldownsMultiplier(ai));
    multipliers.push_back(new SscVashjMainTankGroupShamanUseGroundingTotemMultiplier(ai));
    multipliers.push_back(new SscVashjMaintainPhase1RangedSpreadMultiplier(ai));
    multipliers.push_back(new SscVashjStaticChargeStayAwayFromGroupMultiplier(ai));
    multipliers.push_back(new SscVashjDoNotLootTheTaintedCoreMultiplier(ai));
    multipliers.push_back(new SscVashjCorePassersPrioritizePositioningMultiplier(ai));
    multipliers.push_back(new SscVashjDisableAutomaticTargetingAndMovementMultiplier(ai));
}

void SscStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "underbog colossus spawned toxic pool after death",
        NextAction::array(0, new NextAction("underbog colossus escape toxic pool", ACTION_EMERGENCY + 9), NULL)));
    triggers.push_back(new TriggerNode(
        "the lurker below spout is active",
        NextAction::array(0, new NextAction("the lurker below run around behind boss", ACTION_EMERGENCY + 6), NULL)));
    triggers.push_back(new TriggerNode(
        "leotheras the blind boss channeling whirlwind",
        NextAction::array(0, new NextAction("leotheras the blind run away from whirlwind", ACTION_EMERGENCY + 1), NULL)));
    triggers.push_back(new TriggerNode(
        "leotheras the blind bot has too many chaos blast stacks",
        NextAction::array(0, new NextAction("leotheras the blind melee dps run away from boss", ACTION_EMERGENCY + 6), NULL)));
    triggers.push_back(new TriggerNode(
        "lady vashj bot has static charge",
        NextAction::array(0, new NextAction("lady vashj static charge move away from group", ACTION_EMERGENCY + 7), NULL)));
}

void SscStrategy::InitReactionMultipliers(std::list<Multiplier*>& multipliers)
{
    InitCombatMultipliers(multipliers);
}
