// Ported from mod-playerbots (src/Ai/Raid/Mag, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "MagtheridonStrategy.h"
#include "MagtheridonMultipliers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;

void MagtheridonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    const float raid = raidboss::ACTION_RAID;

    triggers.push_back(new TriggerNode("magtheridon no encounter in progress",
        NextAction::array(0, new NextAction("magtheridon reset encounter states", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode("magtheridon main tank should tank channelers",
        NextAction::array(0, new NextAction("magtheridon main tank attack first three channelers", raid), NULL)));

    triggers.push_back(new TriggerNode("magtheridon assist tanks should tank channelers",
        NextAction::array(0, new NextAction("magtheridon assist tanks attack last two channelers", raid), NULL)));

    triggers.push_back(new TriggerNode("magtheridon pulling west and east channelers",
        NextAction::array(0, new NextAction("magtheridon misdirect hellfire channelers to main tank", raid + 1), NULL)));

    triggers.push_back(new TriggerNode("magtheridon determining kill order",
        NextAction::array(0, new NextAction("magtheridon assign dps priority", raid), NULL)));

    triggers.push_back(new TriggerNode("magtheridon burning abyssal spawned",
        NextAction::array(0, new NextAction("magtheridon warlock cc burning abyssal", raid + 2), NULL)));

    triggers.push_back(new TriggerNode("magtheridon should be tanked",
        NextAction::array(0, new NextAction("magtheridon main tank position boss", raid + 1), NULL)));

    triggers.push_back(new TriggerNode("magtheridon should spread ranged",
        NextAction::array(0, new NextAction("magtheridon spread ranged", raid + 1), NULL)));

    triggers.push_back(new TriggerNode("magtheridon standing in debris",
        NextAction::array(0, new NextAction("magtheridon move out of debris", ACTION_EMERGENCY + 7), NULL)));

    triggers.push_back(new TriggerNode("magtheridon incoming blast nova",
        NextAction::array(0, new NextAction("magtheridon use manticron cube", ACTION_EMERGENCY + 6), NULL)));

    triggers.push_back(new TriggerNode("magtheridon should manage timers and assignments",
        NextAction::array(0, new NextAction("magtheridon update timers and assignments", ACTION_EMERGENCY + 9), NULL)));
}

void MagtheridonStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Clean-up after a wipe or a kill (timers, cube assignment, raid icon)
    triggers.push_back(new TriggerNode("magtheridon no encounter in progress",
        NextAction::array(0, new NextAction("magtheridon reset encounter states", ACTION_EMERGENCY + 9), NULL)));
}

void MagtheridonStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new MagtheridonUseManticronCubeMultiplier(ai));
    multipliers.push_back(new MagtheridonHoldDpsMultiplier(ai));
    multipliers.push_back(new MagtheridonControlTankActionsMultiplier(ai));
    multipliers.push_back(new MagtheridonAvoidDebrisDangerMultiplier(ai));
}
