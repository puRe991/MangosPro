// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "GruulStrategy.h"
#include "GruulMultipliers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;

void GruulStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    const float raid = raidboss::ACTION_RAID;

    // General
    triggers.push_back(new TriggerNode("gruul's lair no encounter in progress",
        NextAction::array(0, new NextAction("gruul's lair reset encounter states", ACTION_EMERGENCY + 9), NULL)));

    // High King Maulgar
    triggers.push_back(new TriggerNode("high king maulgar three ogres need melee tanks",
        NextAction::array(0, new NextAction("high king maulgar melee tanks position bosses", raid + 1), NULL)));

    triggers.push_back(new TriggerNode("high king maulgar krosh needs mage tank",
        NextAction::array(0, new NextAction("high king maulgar mage tank attack krosh", raid + 1), NULL)));

    triggers.push_back(new TriggerNode("high king maulgar kiggler needs moonkin tank",
        NextAction::array(0, new NextAction("high king maulgar moonkin tank attack kiggler", raid + 1), NULL)));

    triggers.push_back(new TriggerNode("high king maulgar determining kill order",
        NextAction::array(0, new NextAction("high king maulgar assign dps priority", raid), NULL)));

    triggers.push_back(new TriggerNode("high king maulgar channeling whirlwind",
        NextAction::array(0, new NextAction("high king maulgar run away from whirlwind", ACTION_EMERGENCY + 6), NULL)));

    triggers.push_back(new TriggerNode("high king maulgar should stand back from krosh",
        NextAction::array(0, new NextAction("high king maulgar back away from krosh", raid + 3), NULL)));

    triggers.push_back(new TriggerNode("high king maulgar wild fel stalker spawned",
        NextAction::array(0, new NextAction("high king maulgar banish fel stalker", raid + 1), NULL)));

    triggers.push_back(new TriggerNode("high king maulgar pulling ogre council",
        NextAction::array(0, new NextAction("high king maulgar misdirect ogres to tanks", raid + 1), NULL)));

    // Gruul the Dragonkiller
    triggers.push_back(new TriggerNode("gruul the dragonkiller should be tanked",
        NextAction::array(0, new NextAction("gruul the dragonkiller tanks position boss", raid), NULL)));

    triggers.push_back(new TriggerNode("gruul the dragonkiller ranged should spread",
        NextAction::array(0, new NextAction("gruul the dragonkiller spread ranged", raid), NULL)));

    triggers.push_back(new TriggerNode("gruul the dragonkiller in cave in",
        NextAction::array(0, new NextAction("gruul the dragonkiller get out of cave in", ACTION_EMERGENCY + 7), NULL)));

    triggers.push_back(new TriggerNode("gruul the dragonkiller incoming shatter",
        NextAction::array(0, new NextAction("gruul the dragonkiller shatter spread", ACTION_EMERGENCY + 6), NULL)));
}

void GruulStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Clean-up between the encounters (spread positions, raid icons)
    triggers.push_back(new TriggerNode("gruul's lair no encounter in progress",
        NextAction::array(0, new NextAction("gruul's lair reset encounter states", ACTION_EMERGENCY + 9), NULL)));
}

void GruulStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    // General
    multipliers.push_back(new GruulsLairDelayDpsCooldownsMultiplier(ai));

    // High King Maulgar
    multipliers.push_back(new HighKingMaulgarControlTankActionsMultiplier(ai));
    multipliers.push_back(new HighKingMaulgarRestrictTauntingMultiplier(ai));
    multipliers.push_back(new HighKingMaulgarDisableDpsAssistMultiplier(ai));
    multipliers.push_back(new HighKingMaulgarAvoidWhirlwindMultiplier(ai));
    multipliers.push_back(new HighKingMaulgarControlHunterActionsMultiplier(ai));
    multipliers.push_back(new HighKingMaulgarControlMageTankActionsMultiplier(ai));

    // Gruul the Dragonkiller
    multipliers.push_back(new GruulTheDragonkillerControlTankMovementMultiplier(ai));
    multipliers.push_back(new GruulTheDragonkillerStaySpreadForShatterMultiplier(ai));
    multipliers.push_back(new GruulTheDragonkillerControlAvoidanceMultiplier(ai));
    multipliers.push_back(new GruulTheDragonkillerHoldWhileSnaredMultiplier(ai));
}
