// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/TK/TKStrategy) and adapted for this core.
#include "playerbot/playerbot.h"
#include "TkStrategy.h"
#include "TkMultipliers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;

namespace
{
    constexpr float ACTION_RAID = raidboss::ACTION_RAID;
    // mod-playerbots ACTION_EMERGENCY + n, kept below 100
    constexpr float Emergency(int n) { return std::min(99.0f, float(ACTION_EMERGENCY + n)); }

    TriggerNode* Node(const char* trigger, const char* action, float relevance)
    {
        return new TriggerNode(trigger, NextAction::array(0, new NextAction(action, relevance), NULL));
    }
}

void TkStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(Node("tempest keep no encounter in progress", "tempest keep reset encounter states", Emergency(10)));
    triggers.push_back(Node("tempest keep stuck falling", "tempest keep clear stale falling flag", Emergency(10)));

    // Trash
    triggers.push_back(Node("crimson hand centurion casts arcane flurry", "crimson hand centurion cast polymorph", ACTION_RAID));

    // Al'ar <Phoenix God>
    triggers.push_back(Node("al'ar pulling boss", "al'ar misdirect boss to main tank", Emergency(1)));
    triggers.push_back(new TriggerNode("al'ar flies between platforms", NextAction::array(0,
        new NextAction("al'ar boss tanks move between platforms", ACTION_RAID),
        new NextAction("al'ar melee dps move between platforms", ACTION_RAID),
        new NextAction("al'ar ranged and ember tank move under platforms", ACTION_RAID + 3), NULL)));
    triggers.push_back(Node("al'ar embers explode upon death", "al'ar assist tanks pick up embers", ACTION_RAID + 2));
    triggers.push_back(Node("al'ar should assign non-tank target", "al'ar assign non-tank target", ACTION_RAID + 1));
    triggers.push_back(Node("al'ar incoming flame quills", "al'ar jump from platform", Emergency(7)));
    triggers.push_back(Node("al'ar rising from the ashes", "al'ar move away from rebirth", Emergency(7)));
    triggers.push_back(new TriggerNode("al'ar in phase 2", NextAction::array(0,
        new NextAction("al'ar swap tanks on boss", Emergency(2)),
        new NextAction("al'ar avoid flame patches and dive bombs", Emergency(1)), NULL)));
    triggers.push_back(Node("al'ar should manage phase tracker", "al'ar manage phase tracker", Emergency(10)));

    // Void Reaver
    triggers.push_back(Node("void reaver should be tanked", "void reaver tanks position boss", ACTION_RAID));
    triggers.push_back(Node("void reaver knock away pulls aggro to non-tanks", "void reaver use aggro dump ability", Emergency(6)));
    triggers.push_back(Node("void reaver ranged should stand back", "void reaver ranged back off and spread", ACTION_RAID));
    triggers.push_back(Node("void reaver arcane orb is incoming", "void reaver avoid arcane orb", Emergency(1)));

    // High Astromancer Solarian
    triggers.push_back(Node("high astromancer solarian should be tanked", "high astromancer solarian main tank pick up boss", ACTION_RAID));
    triggers.push_back(Node("high astromancer solarian wrath of the astromancer", "high astromancer solarian move away from group", Emergency(6)));
    triggers.push_back(Node("high astromancer solarian solarium priests spawned", "high astromancer solarian target solarium priests", ACTION_RAID + 1));

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    triggers.push_back(Node("kael'thas sunstrider chased by thaladred", "kael'thas sunstrider kite thaladred", Emergency(6)));
    triggers.push_back(Node("kael'thas sunstrider pulling tankable advisors", "kael'thas sunstrider misdirect advisors to tanks", Emergency(2)));
    triggers.push_back(Node("kael'thas sunstrider sanguinar or telonicus should be tanked", "kael'thas sunstrider melee tanks position advisors", ACTION_RAID));
    triggers.push_back(Node("kael'thas sunstrider capernian should be tanked by warlock", "kael'thas sunstrider warlock tank position capernian", ACTION_RAID));
    triggers.push_back(Node("kael'thas sunstrider should stand back from capernian", "kael'thas sunstrider spread and move away from capernian", ACTION_RAID + 2));
    triggers.push_back(Node("kael'thas sunstrider should hold phase 3 positions", "kael'thas sunstrider handle advisor roles in phase 3", ACTION_RAID + 1));
    triggers.push_back(Node("kael'thas sunstrider determining advisor kill order", "kael'thas sunstrider assign advisor dps priority", ACTION_RAID));
    triggers.push_back(Node("kael'thas sunstrider should manage advisor dps timer", "kael'thas sunstrider manage advisor dps timer", Emergency(10)));
    triggers.push_back(Node("kael'thas sunstrider legendary weapons are alive", "kael'thas sunstrider assign legendary weapon dps priority", ACTION_RAID));
    triggers.push_back(Node("kael'thas sunstrider legendary axe casts whirlwind", "kael'thas sunstrider move devastation away", Emergency(1)));
    triggers.push_back(Node("kael'thas sunstrider legendary weapons are dead", "kael'thas sunstrider loot legendary weapons", ACTION_NORMAL));
    triggers.push_back(Node("kael'thas sunstrider legendary weapons are equipped", "kael'thas sunstrider use legendary weapons", Emergency(6)));
    triggers.push_back(Node("kael'thas sunstrider legendary weapons were lost", "kael'thas sunstrider reequip gear", Emergency(11)));
    triggers.push_back(new TriggerNode("kael'thas sunstrider has entered the fight", NextAction::array(0,
        new NextAction("kael'thas sunstrider tanks position boss", ACTION_RAID),
        new NextAction("kael'thas sunstrider avoid flame strike", Emergency(8)), NULL)));
    triggers.push_back(Node("kael'thas sunstrider should assign final phase target", "kael'thas sunstrider assign final phase target", ACTION_RAID));
    triggers.push_back(Node("kael'thas sunstrider raid member is mind controlled", "kael'thas sunstrider break mind control", Emergency(1)));
    triggers.push_back(Node("kael'thas sunstrider in gravity lapse phase", "kael'thas sunstrider spread out in midair", ACTION_RAID + 1));
}

// Out of combat: clean-up between encounters, re-equipping after Kael'thas, and Kael's power-up scene
void TkStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(Node("tempest keep no encounter in progress", "tempest keep reset encounter states", Emergency(10)));
    triggers.push_back(Node("tempest keep stuck falling", "tempest keep clear stale falling flag", Emergency(10)));
    triggers.push_back(Node("kael'thas sunstrider legendary weapons were lost", "kael'thas sunstrider reequip gear", Emergency(11)));
    triggers.push_back(Node("kael'thas sunstrider in gravity lapse phase", "kael'thas sunstrider spread out in midair", ACTION_RAID + 1));
}

void TkStrategy::AddMultipliers(PlayerbotAI* ai, std::list<Multiplier*>& multipliers)
{
    // Al'ar <Phoenix God>
    multipliers.push_back(new TkAlarSuppressGapClosersMultiplier(ai));
    multipliers.push_back(new TkAlarControlMovementMultiplier(ai));
    multipliers.push_back(new TkAlarDisableAutomaticTargetingMultiplier(ai));
    multipliers.push_back(new TkAlarStayAwayFromRebirthMultiplier(ai));
    multipliers.push_back(new TkAlarControlTauntingMultiplier(ai));

    // Void Reaver
    multipliers.push_back(new TkVoidReaverMaintainPositionsMultiplier(ai));

    // High Astromancer Solarian
    multipliers.push_back(new TkSolarianDisableMeleeTargetingMultiplier(ai));
    multipliers.push_back(new TkSolarianWrathStayAwayMultiplier(ai));

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    multipliers.push_back(new TkKaelthasWaitForDpsMultiplier(ai));
    multipliers.push_back(new TkKaelthasKiteThaladredMultiplier(ai));
    multipliers.push_back(new TkKaelthasControlMisdirectionMultiplier(ai));
    multipliers.push_back(new TkKaelthasKeepDistanceFromCapernianMultiplier(ai));
    multipliers.push_back(new TkKaelthasManageWeaponTankingMultiplier(ai));
    multipliers.push_back(new TkKaelthasSuppressEquipUpgradeMultiplier(ai));
    multipliers.push_back(new TkKaelthasManageAutomaticTargetingMultiplier(ai));
    multipliers.push_back(new TkKaelthasDisableDisperseMultiplier(ai));
    multipliers.push_back(new TkKaelthasPrepareForPhase3Multiplier(ai));
    multipliers.push_back(new TkKaelthasDelayCooldownsMultiplier(ai));
    multipliers.push_back(new TkKaelthasStaySpreadDuringGravityLapseMultiplier(ai));
}

void TkStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    AddMultipliers(ai, multipliers);
}

void TkStrategy::InitNonCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    AddMultipliers(ai, multipliers);
}
