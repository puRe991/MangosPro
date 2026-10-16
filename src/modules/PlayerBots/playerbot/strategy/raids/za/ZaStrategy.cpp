// Ported from mod-playerbots (AzerothCore, GPL v2+), src/Ai/Raid/ZA/ZAStrategy.cpp, and adapted for this core.
#include "playerbot/playerbot.h"
#include "ZaStrategy.h"
#include "ZaMultipliers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;

namespace
{
    // mod-playerbots ACTION_EMERGENCY + n, capped below 100
    constexpr float ZA_EMERGENCY_1 = 91.0f;
    constexpr float ZA_EMERGENCY_6 = 96.0f;
    constexpr float ZA_EMERGENCY_9 = 99.0f;     // mod-playerbots: ACTION_EMERGENCY + 10
}

void ZaStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "zul'aman no encounter in progress",
        NextAction::array(0, new NextAction("zul'aman reset encounter states", ZA_EMERGENCY_9), NULL)));

    triggers.push_back(new TriggerNode(
        "amani'shi medicine man summoned ward",
        NextAction::array(0, new NextAction("amani'shi medicine man mark ward", raidboss::ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode(
        "akil'zon pulling boss",
        NextAction::array(0, new NextAction("akil'zon misdirect boss to main tank", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "akil'zon should be tanked",
        NextAction::array(0, new NextAction("akil'zon tanks position boss", raidboss::ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode(
        "akil'zon spread for static disruption",
        NextAction::array(0, new NextAction("akil'zon spread ranged", raidboss::ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode(
        "akil'zon electrical storm incoming",
        NextAction::array(0, new NextAction("akil'zon move to eye of the storm", ZA_EMERGENCY_6), NULL)));

    triggers.push_back(new TriggerNode(
        "akil'zon should track electrical storm",
        NextAction::array(0, new NextAction("akil'zon start electrical storm timer", ZA_EMERGENCY_9), NULL)));

    triggers.push_back(new TriggerNode(
        "nalorakk pulling boss",
        NextAction::array(0, new NextAction("nalorakk misdirect boss to main tank", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "nalorakk both forms should be tanked",
        NextAction::array(0, new NextAction("nalorakk tanks position boss", ZA_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "nalorakk spread for surge",
        NextAction::array(0, new NextAction("nalorakk spread ranged", raidboss::ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode(
        "jan'alai pulling boss",
        NextAction::array(0, new NextAction("jan'alai misdirect boss to main tank", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "jan'alai should be tanked",
        NextAction::array(0, new NextAction("jan'alai tanks position boss", raidboss::ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode(
        "jan'alai spread for flame breath",
        NextAction::array(0, new NextAction("jan'alai spread ranged in circle", raidboss::ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode(
        "jan'alai is fire bombing",
        NextAction::array(0, new NextAction("jan'alai avoid fire bombs", ZA_EMERGENCY_6), NULL)));

    triggers.push_back(new TriggerNode(
        "jan'alai amani'shi hatchers spawned",
        NextAction::array(0, new NextAction("jan'alai mark amani'shi hatchers", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "halazzi pulling boss",
        NextAction::array(0, new NextAction("halazzi misdirect boss to main tank", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "halazzi should be tanked",
        NextAction::array(0, new NextAction("halazzi main tank position boss", raidboss::ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode(
        "halazzi spirit lynx has appeared",
        NextAction::array(0, new NextAction("halazzi first assist tank attack spirit lynx", raidboss::ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode(
        "halazzi should focus dps",
        NextAction::array(0, new NextAction("halazzi dps attack totem and boss", raidboss::ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode(
        "hex lord malacrass pulling boss",
        NextAction::array(0, new NextAction("hex lord malacrass misdirect boss to main tank", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "hex lord malacrass should prioritize adds",
        NextAction::array(0, new NextAction("hex lord malacrass assign dps priority", raidboss::ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode(
        "hex lord malacrass channeling whirlwind",
        NextAction::array(0, new NextAction("hex lord malacrass run away from whirlwind", ZA_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "hex lord malacrass freezing trap placed",
        NextAction::array(0, new NextAction("hex lord malacrass move away from freezing trap", ZA_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "zul'jin pulling boss",
        NextAction::array(0, new NextAction("zul'jin misdirect boss to main tank", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "zul'jin should be tanked",
        NextAction::array(0, new NextAction("zul'jin tanks position boss", raidboss::ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode(
        "zul'jin channeling whirlwind in troll form",
        NextAction::array(0, new NextAction("zul'jin run away from whirlwind", ZA_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "zul'jin creeping paralysis in bear form",
        NextAction::array(0, new NextAction("zul'jin mass dispel creeping paralysis", ZA_EMERGENCY_6), NULL)));

    triggers.push_back(new TriggerNode(
        "zul'jin summoning cyclones in eagle form",
        NextAction::array(0, new NextAction("zul'jin position ranged for cyclones", raidboss::ACTION_RAID), NULL)));

    triggers.push_back(new TriggerNode(
        "zul'jin spread for dragonhawk aoe",
        NextAction::array(0, new NextAction("zul'jin spread ranged", raidboss::ACTION_RAID), NULL)));
}

// Out of combat: reset the Akil'zon storm timers and the boss marks
void ZaStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "zul'aman no encounter in progress",
        NextAction::array(0, new NextAction("zul'aman reset encounter states", ZA_EMERGENCY_9), NULL)));
}

// Checked while casting: storm, Fire Bombs, Whirlwind and Freezing Trap
void ZaStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "akil'zon electrical storm incoming",
        NextAction::array(0, new NextAction("akil'zon move to eye of the storm", ZA_EMERGENCY_6), NULL)));

    triggers.push_back(new TriggerNode(
        "jan'alai is fire bombing",
        NextAction::array(0, new NextAction("jan'alai avoid fire bombs", ZA_EMERGENCY_6), NULL)));

    triggers.push_back(new TriggerNode(
        "hex lord malacrass channeling whirlwind",
        NextAction::array(0, new NextAction("hex lord malacrass run away from whirlwind", ZA_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "hex lord malacrass freezing trap placed",
        NextAction::array(0, new NextAction("hex lord malacrass move away from freezing trap", ZA_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "zul'jin channeling whirlwind in troll form",
        NextAction::array(0, new NextAction("zul'jin run away from whirlwind", ZA_EMERGENCY_1), NULL)));
}

void ZaStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    // General
    multipliers.push_back(new ZaDelayDpsCooldownsMultiplier(ai));
    multipliers.push_back(new ZaDisableTankActionsMultiplier(ai));
    multipliers.push_back(new ZaControlMisdirectionMultiplier(ai));
    multipliers.push_back(new ZaDisableCombatFormationMoveMultiplier(ai));
    multipliers.push_back(new ZaAvoidWhirlwindMultiplier(ai));
    multipliers.push_back(new ZaTbcRaidOverlapMultiplier(ai));

    // Akil'zon <Eagle Avatar>
    multipliers.push_back(new ZaAkilzonStayInEyeOfTheStormMultiplier(ai));

    // Jan'alai <Dragonhawk Avatar>
    multipliers.push_back(new ZaJanalaiStayAwayFromFireBombsMultiplier(ai));
    multipliers.push_back(new ZaJanalaiDoNotCrowdControlHatchersMultiplier(ai));

    // Halazzi <Lynx Avatar>
    multipliers.push_back(new ZaHalazziDisableAutoDpsTargetingMultiplier(ai));

    // Hex Lord Malacrass
    multipliers.push_back(new ZaHexLordMalacrassUnstableAfflictionMultiplier(ai));
    multipliers.push_back(new ZaHexLordMalacrassSpellReflectionMultiplier(ai));
    multipliers.push_back(new ZaHexLordMalacrassStayAwayFromFreezingTrapMultiplier(ai));

    // Zul'jin
    multipliers.push_back(new ZaZuljinStopAttackingDuringPhaseChangeMultiplier(ai));
    multipliers.push_back(new ZaZuljinEagleDisableAvoidAoeMultiplier(ai));
}
