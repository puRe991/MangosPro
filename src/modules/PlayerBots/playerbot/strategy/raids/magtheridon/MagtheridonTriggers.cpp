// Ported from mod-playerbots (src/Ai/Raid/Mag, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "MagtheridonTriggers.h"
#include "MagtheridonHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;
using namespace ai::magtheridon;

bool MagtheridonEncounterTrigger::IsActive()
{
    return raidboss::IsEncounterInProgress(bot, MAG_MAP_ID) && IsActiveInEncounter();
}

bool MagtheridonNoEncounterInProgressTrigger::IsActive()
{
    if (bot->GetMapId() != MAG_MAP_ID || raidboss::IsEncounterInProgress(bot, MAG_MAP_ID))
        return false;

    return raidboss::IsMechanicTrackerBot(bot, MAG_MAP_ID);
}

bool MagtheridonMainTankShouldTankChannelersTrigger::IsActiveInEncounter()
{
    if (!raidboss::IsMainTank(bot))
        return false;

    Unit* magtheridon = FindMagtheridon(ai);
    return magtheridon && !IsMagtheridonActive(magtheridon);
}

bool MagtheridonAssistTanksShouldTankChannelersTrigger::IsActiveInEncounter()
{
    if (!FindMagtheridon(ai))
        return false;

    if (raidboss::IsAssistTank(bot, 0) && GetChanneler(ai, CHANNELER_NORTHWEST))
        return true;

    return raidboss::IsAssistTank(bot, 1) && GetChanneler(ai, CHANNELER_NORTHEAST);
}

bool MagtheridonPullingWestAndEastChannelersTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER || !FindMagtheridon(ai))
        return false;

    return GetChanneler(ai, CHANNELER_WEST) || GetChanneler(ai, CHANNELER_EAST);
}

bool MagtheridonDeterminingKillOrderTrigger::IsActiveInEncounter()
{
    return !PlayerbotAI::IsTank(bot) && FindMagtheridon(ai);
}

bool MagtheridonBurningAbyssalSpawnedTrigger::IsActiveInEncounter()
{
    std::string spell;
    return GetAssignedAbyssalCcTarget(ai, spell) != nullptr;
}

bool MagtheridonChannelerNeedsInterruptTrigger::IsActiveInEncounter()
{
    std::string spell;
    return GetChannelerInterruptTarget(ai, spell) != nullptr;
}

bool MagtheridonShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* magtheridon = FindMagtheridon(ai);
    if (!IsMagtheridonActive(magtheridon))
        return false;

    // Include an assist tank that pulls aggro
    return magtheridon->GetVictim() == bot || raidboss::IsMainTank(bot);
}

bool MagtheridonShouldSpreadRangedTrigger::IsActiveInEncounter()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    Unit* magtheridon = FindMagtheridon(ai);
    if (!IsMagtheridonActive(magtheridon) || magtheridon->GetVictim() == bot)
        return false;

    // Clickers stop spreading when they have to wait by their cube
    return !IsCubeClicker(bot) || !IsBlastNovaSoon(bot);
}

bool MagtheridonStandingInDebrisTrigger::IsActiveInEncounter()
{
    if (!IsCeilingCollapsed(bot) || !FindMagtheridon(ai))
        return false;

    return IsPositionInActiveDebris(bot, bot->GetPositionX(), bot->GetPositionY());
}

bool MagtheridonIncomingBlastNovaTrigger::IsActiveInEncounter()
{
    return NeedsCubeAction(ai);
}

bool MagtheridonShouldManageTimersAndAssignmentsTrigger::IsActiveInEncounter()
{
    return raidboss::IsMechanicTrackerBot(bot, MAG_MAP_ID) && IsMagtheridonActive(FindMagtheridon(ai));
}
