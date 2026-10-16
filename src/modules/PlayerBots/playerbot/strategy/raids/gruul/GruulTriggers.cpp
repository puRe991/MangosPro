// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "GruulTriggers.h"
#include "GruulHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;
using namespace ai::gruul;

// General

bool GruulsLairEncounterTrigger::IsActive()
{
    return raidboss::IsEncounterInProgress(bot, GRUUL_MAP_ID) && IsActiveInEncounter();
}

bool GruulsLairNoEncounterInProgressTrigger::IsActive()
{
    return bot->GetMapId() == GRUUL_MAP_ID && !raidboss::IsEncounterInProgress(bot, GRUUL_MAP_ID);
}

// High King Maulgar <Lord of the Ogres>

bool HighKingMaulgarThreeOgresNeedMeleeTanksTrigger::IsActiveInEncounter()
{
    if (IsBlindeyeTank(bot))
        return FindBlindeye(ai) != nullptr;

    if (IsOlmTank(bot))
        return FindOlm(ai) != nullptr;

    return IsMaulgarTank(bot) && FindMaulgar(ai);
}

bool HighKingMaulgarKroshNeedsMageTankTrigger::IsActiveInEncounter()
{
    return IsKroshMageTank(ai) && FindKrosh(ai);
}

bool HighKingMaulgarKigglerNeedsMoonkinTankTrigger::IsActiveInEncounter()
{
    return IsKigglerMoonkinTank(ai) && FindKiggler(ai);
}

bool HighKingMaulgarDeterminingKillOrderTrigger::IsActiveInEncounter()
{
    if (!FindMaulgar(ai) || IsMaulgarTank(bot))
        return false;

    if (IsOlmTank(bot))
        return !FindOlm(ai);

    if (IsBlindeyeTank(bot))
        return !FindBlindeye(ai);

    if (IsKroshMageTank(ai))
        return !FindKrosh(ai);

    if (IsKigglerMoonkinTank(ai))
        return !FindKiggler(ai);

    return true;
}

bool HighKingMaulgarChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    Unit* maulgar = FindMaulgar(ai);
    return maulgar && maulgar->HasAura(SPELL_WHIRLWIND) && !IsMaulgarTank(bot);
}

bool HighKingMaulgarShouldStandBackFromKroshTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot) || IsKroshMageTank(ai))
        return false;

    return FindKrosh(ai) != nullptr;
}

bool HighKingMaulgarWildFelStalkerSpawnedTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_WARLOCK && !GetNearbyWildFelStalkers(ai).empty();
}

bool HighKingMaulgarPullingOgreCouncilTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* blindeye = FindBlindeye(ai);
    return blindeye && blindeye->GetHealthPercent() > BLINDEYE_ENGAGED_HEALTH_PCT;
}

// Gruul the Dragonkiller

bool GruulTheDragonkillerShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && FindGruul(ai);
}

bool GruulTheDragonkillerRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return raidboss::IsRangedPlayer(bot) && FindGruul(ai);
}

bool GruulTheDragonkillerInCaveInTrigger::IsActiveInEncounter()
{
    Unit* gruul = FindGruul(ai);
    if (!gruul)
        return false;

    // The tank holding Gruul eats it. Moving him messes up the spread and puts other bots in danger.
    if (gruul->GetVictim() == bot)
        return false;

    return IsInCaveIn(bot);
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActiveInEncounter()
{
    return HasGroundSlam(bot);
}
