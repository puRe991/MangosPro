// Ported from mod-playerbots (src/Ai/Raid/SSC/SSCTriggers.cpp, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "SscTriggers.h"
#include "SscHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "Loot/LootMgr.h"

using namespace ai;
using namespace ai::ssc;

namespace
{
    // A boss which is fighting (the creature also exists out of combat while the raid clears trash)
    Unit* GetEngaged(PlayerbotAI* ai, uint32 entry)
    {
        Unit* unit = FindSscUnit(ai, entry);
        return unit && unit->IsInCombat() ? unit : nullptr;
    }

    bool InSsc(Player* bot)
    {
        return bot->GetMapId() == SSC_MAP_ID;
    }

    bool IsHunter(Player* bot)
    {
        return bot->getClass() == CLASS_HUNTER;
    }
}

// General

bool SscBotIsNotInCombatTrigger::IsActive()
{
    return NeedsStateReset(ai);
}

// Trash

bool SscUnderbogColossusSpawnedToxicPoolAfterDeathTrigger::IsActive()
{
    return InSsc(bot) && bot->HasAura(SPELL_TOXIC_POOL);
}

bool SscGreyheartTidecallerWaterElementalTotemSpawnedTrigger::IsActive()
{
    return InSsc(bot) && raidboss::IsDps(bot) && FindSscUnit(ai, NPC_WATER_ELEMENTAL_TOTEM, 60.0f) &&
           raidboss::FindUnitByName(ai, "greyheart tidecaller");
}

// Hydross the Unstable <Duke of Currents>

bool SscHydrossBotIsFrostTankTrigger::IsActive()
{
    return raidboss::IsMainTank(bot) && GetEngaged(ai, NPC_HYDROSS);
}

bool SscHydrossBotIsNatureTankTrigger::IsActive()
{
    return raidboss::IsAssistTank(bot, 0) && GetEngaged(ai, NPC_HYDROSS);
}

bool SscHydrossElementalsSpawnedTrigger::IsActive()
{
    if (PlayerbotAI::IsHeal(bot) || raidboss::IsMainTank(bot) || raidboss::IsAssistTank(bot, 0))
        return false;

    Unit* hydross = GetEngaged(ai, NPC_HYDROSS);
    if (!hydross || hydross->GetHealthPercent() < 10.0f)
        return false;

    return FindSscUnit(ai, NPC_PURE_SPAWN_OF_HYDROSS) || FindSscUnit(ai, NPC_TAINTED_SPAWN_OF_HYDROSS);
}

bool SscHydrossDangerFromWaterTombsTrigger::IsActive()
{
    return raidboss::IsRangedPlayer(bot) && GetEngaged(ai, NPC_HYDROSS);
}

bool SscHydrossTankNeedsAggroUponPhaseChangeTrigger::IsActive()
{
    return IsHunter(bot) && GetEngaged(ai, NPC_HYDROSS);
}

bool SscHydrossAggroResetsUponPhaseChangeTrigger::IsActive()
{
    if (PlayerbotAI::IsHeal(bot) || raidboss::IsMainTank(bot) || raidboss::IsAssistTank(bot, 0))
        return false;

    return GetEngaged(ai, NPC_HYDROSS);
}

bool SscHydrossNeedToManageTimersTrigger::IsActive()
{
    return raidboss::IsMechanicTrackerBot(bot, SSC_MAP_ID) && GetEngaged(ai, NPC_HYDROSS);
}

// The Lurker Below

bool SscLurkerSpoutIsActiveTrigger::IsActive()
{
    Unit* lurker = GetEngaged(ai, NPC_LURKER_BELOW);
    if (!lurker)
        return false;

    return IsLurkerSpouting(lurker) || GetState(bot).lurkerSpoutEnd > time(nullptr);
}

bool SscLurkerBossIsActiveForMainTankTrigger::IsActive()
{
    if (!raidboss::IsMainTank(bot))
        return false;

    Unit* lurker = GetEngaged(ai, NPC_LURKER_BELOW);
    return lurker && !IsLurkerSubmerged(lurker) && !IsLurkerSpouting(lurker) && GetState(bot).lurkerSpoutEnd <= time(nullptr);
}

bool SscLurkerBossCastsGeyserTrigger::IsActive()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    Unit* lurker = GetEngaged(ai, NPC_LURKER_BELOW);
    return lurker && !IsLurkerSubmerged(lurker) && !IsLurkerSpouting(lurker) && GetState(bot).lurkerSpoutEnd <= time(nullptr);
}

// Active only if there are at least 3 tanks in the raid
bool SscLurkerBossIsSubmergedTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* lurker = GetEngaged(ai, NPC_LURKER_BELOW);
    if (!IsLurkerSubmerged(lurker))
        return false;

    Player* mainTank = raidboss::GetGroupMainTank(bot);
    Player* firstAssistTank = raidboss::GetGroupAssistTank(bot, 0);
    Player* secondAssistTank = raidboss::GetGroupAssistTank(bot, 1);
    if (!mainTank || !firstAssistTank || !secondAssistTank)
        return false;

    return bot == mainTank || bot == firstAssistTank || bot == secondAssistTank;
}

bool SscLurkerNeedToPrepareTimerForSpoutTrigger::IsActive()
{
    return raidboss::IsMechanicTrackerBot(bot, SSC_MAP_ID) && GetEngaged(ai, NPC_LURKER_BELOW);
}

// Leotheras the Blind

bool SscLeotherasBossIsInactiveTrigger::IsActive()
{
    return raidboss::IsMechanicTrackerBot(bot, SSC_MAP_ID) && FindSscUnit(ai, NPC_GREYHEART_SPELLBINDER, 100.0f);
}

bool SscLeotherasBossTransformedIntoDemonFormTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK || !FindSscUnit(ai, NPC_LEOTHERAS, 100.0f))
        return false;

    return GetLeotherasDemonFormTank(bot) == bot && GetActiveLeotherasDemon(ai);
}

bool SscLeotherasOnlyWarlockShouldTankDemonFormTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot) || bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    if (!FindSscUnit(ai, NPC_LEOTHERAS, 100.0f) || !GetLeotherasDemonFormTank(bot))
        return false;

    return GetPhase2LeotherasDemon(ai);
}

bool SscLeotherasBossEngagedByRangedTrigger::IsActive()
{
    if (!raidboss::IsRangedPlayer(bot) || bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    Unit* leotheras = FindSscUnit(ai, NPC_LEOTHERAS, 100.0f);
    if (!leotheras)
        return false;

    return !leotheras->HasAura(SPELL_LEOTHERAS_BANISHED) && !leotheras->HasAura(SPELL_WHIRLWIND) &&
           !leotheras->HasAura(SPELL_WHIRLWIND_CHANNEL);
}

bool SscLeotherasBossChannelingWhirlwindTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot) || bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    Unit* leotheras = FindSscUnit(ai, NPC_LEOTHERAS, 100.0f);
    return leotheras && (leotheras->HasAura(SPELL_WHIRLWIND) || leotheras->HasAura(SPELL_WHIRLWIND_CHANNEL));
}

bool SscLeotherasBotHasTooManyChaosBlastStacksTrigger::IsActive()
{
    if (raidboss::IsRangedPlayer(bot) || bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    if (!FindSscUnit(ai, NPC_LEOTHERAS, 100.0f) || bot->GetAuraCount(SPELL_CHAOS_BLAST) < 5)
        return false;

    if (!GetLeotherasDemonFormTank(bot) && raidboss::IsMainTank(bot))
        return false;

    return GetPhase2LeotherasDemon(ai);
}

bool SscLeotherasInnerDemonHasAwakenedTrigger::IsActive()
{
    return bot->HasAura(SPELL_INSIDIOUS_WHISPER) && GetMyInnerDemon(bot);
}

bool SscLeotherasEnteredFinalPhaseTrigger::IsActive()
{
    if (PlayerbotAI::IsHeal(bot) || bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return false;

    if (!FindSscUnit(ai, NPC_LEOTHERAS, 100.0f))
        return false;

    if (bot->getClass() == CLASS_WARLOCK && GetLeotherasDemonFormTank(bot) == bot)
        return false;

    return GetPhase3LeotherasDemon(ai);
}

bool SscLeotherasDemonFormTankNeedsAggroTrigger::IsActive()
{
    return IsHunter(bot) && !bot->HasAura(SPELL_INSIDIOUS_WHISPER) && FindSscUnit(ai, NPC_LEOTHERAS, 100.0f) &&
           GetActiveLeotherasDemon(ai);
}

bool SscLeotherasBossWipesAggroUponPhaseChangeTrigger::IsActive()
{
    return raidboss::IsMechanicTrackerBot(bot, SSC_MAP_ID) && FindSscUnit(ai, NPC_LEOTHERAS, 100.0f);
}

// Fathom-Lord Karathress

bool SscKarathressBossEngagedByMainTankTrigger::IsActive()
{
    return raidboss::IsMainTank(bot) && GetEngaged(ai, NPC_KARATHRESS);
}

bool SscKarathressCaribdisEngagedByFirstAssistTankTrigger::IsActive()
{
    return raidboss::IsAssistTank(bot, 0) && GetEngaged(ai, NPC_CARIBDIS);
}

bool SscKarathressSharkkisEngagedBySecondAssistTankTrigger::IsActive()
{
    return raidboss::IsAssistTank(bot, 1) && GetEngaged(ai, NPC_SHARKKIS);
}

bool SscKarathressTidalvessEngagedByThirdAssistTankTrigger::IsActive()
{
    return raidboss::IsAssistTank(bot, 2) && GetEngaged(ai, NPC_TIDALVESS);
}

bool SscKarathressCaribdisTankNeedsDedicatedHealerTrigger::IsActive()
{
    return PlayerbotAI::IsHeal(bot) && GetHealerOfIndex(bot, 0) == bot && GetEngaged(ai, NPC_CARIBDIS);
}

bool SscKarathressPullingBossesTrigger::IsActive()
{
    if (!IsHunter(bot))
        return false;

    Unit* karathress = GetEngaged(ai, NPC_KARATHRESS);
    return karathress && karathress->GetHealthPercent() > 98.0f;
}

bool SscKarathressDeterminingKillOrderTrigger::IsActive()
{
    if (raidboss::IsDps(bot) && FindSscUnit(ai, NPC_SPITFIRE_TOTEM, 100.0f))
        return true;

    if (PlayerbotAI::IsHeal(bot) || !GetEngaged(ai, NPC_KARATHRESS))
        return false;

    if (raidboss::IsDps(bot))
        return true;

    if (raidboss::IsAssistTank(bot, 0))
        return !FindSscUnit(ai, NPC_CARIBDIS);

    if (raidboss::IsAssistTank(bot, 1))
        return !FindSscUnit(ai, NPC_SHARKKIS);

    if (raidboss::IsAssistTank(bot, 2))
        return !FindSscUnit(ai, NPC_TIDALVESS);

    return false;
}

bool SscKarathressTanksNeedToEstablishAggroTrigger::IsActive()
{
    return raidboss::IsMechanicTrackerBot(bot, SSC_MAP_ID) && GetEngaged(ai, NPC_KARATHRESS);
}

// Morogrim Tidewalker

bool SscMorogrimPullingBossTrigger::IsActive()
{
    if (!IsHunter(bot))
        return false;

    Unit* tidewalker = GetEngaged(ai, NPC_MOROGRIM);
    return tidewalker && tidewalker->GetHealthPercent() > 95.0f;
}

bool SscMorogrimBossEngagedByMainTankTrigger::IsActive()
{
    return raidboss::IsMainTank(bot) && GetEngaged(ai, NPC_MOROGRIM);
}

bool SscMorogrimWaterGlobulesAreIncomingTrigger::IsActive()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    // The boss summons the Water Globules below 25% health
    Unit* tidewalker = GetEngaged(ai, NPC_MOROGRIM);
    return tidewalker && tidewalker->GetHealthPercent() < 25.0f;
}

// Lady Vashj <Coilfang Matron>

bool SscVashjBossEngagedByMainTankTrigger::IsActive()
{
    return raidboss::IsMainTank(bot) && GetEngaged(ai, NPC_LADY_VASHJ) && !IsLadyVashjInPhase2(ai);
}

bool SscVashjBossEngagedByRangedInPhase1Trigger::IsActive()
{
    return raidboss::IsRangedPlayer(bot) && GetEngaged(ai, NPC_LADY_VASHJ) && IsLadyVashjInPhase1(ai);
}

bool SscVashjCastsShockBlastOnHighestAggroTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN || !GetEngaged(ai, NPC_LADY_VASHJ) || IsLadyVashjInPhase2(ai))
        return false;

    return IsMainTankInSameSubgroup(bot);
}

bool SscVashjBotHasStaticChargeTrigger::IsActive()
{
    if (!GetEngaged(ai, NPC_LADY_VASHJ))
        return false;

    for (Player* member : raidboss::GetGroupMembers(bot, 100.0f, true))
    {
        if (member->HasAura(SPELL_STATIC_CHARGE))
            return true;
    }

    return false;
}

bool SscVashjPullingBossInPhase1AndPhase3Trigger::IsActive()
{
    if (!IsHunter(bot))
        return false;

    Unit* vashj = GetEngaged(ai, NPC_LADY_VASHJ);
    if (!vashj)
        return false;

    const float health = vashj->GetHealthPercent();
    return health > 90.0f || (IsLadyVashjInPhase3(ai) && health <= 50.0f && health > 40.0f);
}

// Replaces "lady vashj tainted elemental cheat": the designated looter kills the Tainted Elemental
// and loots the core without the raid cheats of mod-playerbots (teleport, full heal)
bool SscVashjTaintedCoreCanBeLootedTrigger::IsActive()
{
    if (!GetEngaged(ai, NPC_LADY_VASHJ) || !IsLadyVashjInPhase2(ai) || HasTaintedCore(bot))
        return false;

    if (GetCoreHandlers(bot)[0] != bot)
        return false;

    for (Unit* tainted : raidboss::FindUnits(bot, NPC_TAINTED_ELEMENTAL, 150.0f, false))
    {
        if (tainted->IsAlive())
            return true;

        Loot* loot = sLootMgr.GetLoot(bot, tainted->GetObjectGuid());
        if (loot && loot->IsItemAlreadyIn(ITEM_TAINTED_CORE))
            return true;
    }

    return false;
}

bool SscVashjTaintedCoreWasLootedTrigger::IsActive()
{
    if (!GetEngaged(ai, NPC_LADY_VASHJ) || !IsLadyVashjInPhase2(ai))
        return false;

    // Any bot holding a core passes it (also a core looted by somebody else)
    if (HasTaintedCore(bot))
        return true;

    const std::array<Player*, 5> handlers = GetCoreHandlers(bot);
    if (std::find(handlers.begin(), handlers.end(), bot) == handlers.end())
        return false;

    // First and second passers move to positions as soon as the looter reached the elemental
    Unit* tainted = FindSscUnit(ai, NPC_TAINTED_ELEMENTAL);
    if (tainted && handlers[0] && raidboss::ExactDist2d(handlers[0], tainted) < 5.0f &&
        (bot == handlers[1] || bot == handlers[2]))
        return true;

    // Main logic: run if core is in play for this bot or a prior handler
    return AnyRecentCoreInInventory(bot);
}

bool SscVashjTaintedCoreIsUnusableTrigger::IsActive()
{
    return InSsc(bot) && HasTaintedCore(bot) && !IsLadyVashjInPhase2(ai);
}

bool SscVashjAddsSpawnInPhase2AndPhase3Trigger::IsActive()
{
    return !PlayerbotAI::IsHeal(bot) && GetEngaged(ai, NPC_LADY_VASHJ) && !IsLadyVashjInPhase1(ai);
}

bool SscVashjCoilfangStriderIsApproachingTrigger::IsActive()
{
    return FindSscUnit(ai, NPC_COILFANG_STRIDER) && GetEngaged(ai, NPC_LADY_VASHJ);
}

bool SscVashjToxicSporebatsAreSpewingPoisonCloudsTrigger::IsActive()
{
    return GetEngaged(ai, NPC_LADY_VASHJ) && IsLadyVashjInPhase3(ai);
}

bool SscVashjBotIsEntangledInToxicSporesOrStaticChargeTrigger::IsActive()
{
    if (!GetEngaged(ai, NPC_LADY_VASHJ))
        return false;

    for (Player* member : raidboss::GetGroupMembers(bot, 100.0f, true))
    {
        if (member->HasAura(SPELL_ENTANGLE) && raidboss::IsMeleePlayer(member))
            return true;
    }

    return false;
}
