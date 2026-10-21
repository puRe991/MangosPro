// Ported from mod-playerbots (AzerothCore, GPL v2+), src/Ai/Raid/ZA/ZATriggers.cpp, and adapted for this core.
#include "playerbot/playerbot.h"
#include "ZaTriggers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;
using namespace ai::za;

// General

bool ZaNoEncounterInProgressTrigger::IsActive()
{
    if (bot->GetMapId() != ZA_MAP_ID || IsEncounterInProgress(bot))
        return false;

    if (AI_VALUE(uint32, "akil'zon electrical storm timer") || AI_VALUE(uint32, "akil'zon last electrical storm"))
        return true;

    // The mechanic tracker clears the boss marks once the group is out of combat
    if (bot->IsInCombat() || !raidboss::IsMechanicTrackerBot(bot, ZA_MAP_ID))
        return false;

    return raidboss::GetIconTarget(ai, raidboss::ICON_SKULL) || raidboss::GetIconTarget(ai, raidboss::ICON_MOON);
}

bool ZaPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = GetBoss(ai, bossEntry);
    return boss && boss->GetHealthPercent() > raidboss::BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

bool ZaAmanishiMedicineManSummonedWardTrigger::IsActive()
{
    if (bot->GetMapId() != ZA_MAP_ID || !raidboss::IsMechanicTrackerBot(bot, ZA_MAP_ID))
        return false;

    if (!raidboss::FindUnit(ai, NPC_AMANISHI_MEDICINE_MAN, ZA_CREATURE_SEARCH_RADIUS))
        return false;

    return !raidboss::FindUnits(bot, NPC_AMANI_PROTECTIVE_WARD, ZA_CREATURE_SEARCH_RADIUS).empty() ||
           !raidboss::FindUnits(bot, NPC_AMANI_HEALING_WARD, ZA_CREATURE_SEARCH_RADIUS).empty();
}

// Akil'zon <Eagle Avatar>

bool ZaAkilzonShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot) || !GetBoss(ai, NPC_AKILZON))
        return false;

    return !GetElectricalStormTarget(bot);
}

bool ZaAkilzonSpreadForStaticDisruptionTrigger::IsActiveInEncounter()
{
    if (!raidboss::IsRangedPlayer(bot) || !GetBoss(ai, NPC_AKILZON))
        return false;

    return !IsInStormWindow(ai);
}

bool ZaAkilzonElectricalStormIncomingTrigger::IsActiveInEncounter()
{
    return GetBoss(ai, NPC_AKILZON) && IsInStormWindow(ai);
}

bool ZaAkilzonShouldTrackElectricalStormTrigger::IsActiveInEncounter()
{
    if (!GetBoss(ai, NPC_AKILZON))
        return false;

    if (!AI_VALUE(uint32, "akil'zon electrical storm timer"))
        return true;

    // A new storm started (the weather cycle repeats after 54-60s).
    if (!GetElectricalStormTarget(bot))
        return false;

    const uint32 lastStorm = AI_VALUE(uint32, "akil'zon last electrical storm");
    return !lastStorm || WorldTimer::getMSTimeDiff(lastStorm, WorldTimer::getMSTime()) > 20000;
}

// Nalorakk <Bear Avatar>

bool ZaNalorakkBothFormsShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!GetBoss(ai, NPC_NALORAKK))
        return false;

    return raidboss::IsMainTank(bot) || raidboss::IsAssistTank(bot, 0);
}

bool ZaNalorakkSpreadForSurgeTrigger::IsActiveInEncounter()
{
    return raidboss::IsRangedPlayer(bot) && GetBoss(ai, NPC_NALORAKK);
}

// Jan'alai <Dragonhawk Avatar>

bool ZaJanalaiShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* janalai = GetBoss(ai, NPC_JANALAI);
    return janalai && !IsJanalaiBombing(janalai);
}

bool ZaJanalaiSpreadForFlameBreathTrigger::IsActiveInEncounter()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    Unit* janalai = GetBoss(ai, NPC_JANALAI);
    if (!janalai)
        return false;

    if (raidboss::FindUnit(ai, NPC_AMANI_DRAGONHAWK_HATCHLING))
        return false;

    return !IsJanalaiBombing(janalai);
}

bool ZaJanalaiIsFireBombingTrigger::IsActiveInEncounter()
{
    return IsJanalaiBombing(GetBoss(ai, NPC_JANALAI));
}

bool ZaJanalaiAmanishiHatchersSpawnedTrigger::IsActiveInEncounter()
{
    if (!raidboss::IsDps(bot) || !raidboss::IsRangedPlayer(bot))
        return false;

    Unit* janalai = GetBoss(ai, NPC_JANALAI);
    if (!janalai || janalai->GetHealthPercent() <= JANALAI_HATCH_ALL_HEALTH_PCT)
        return false;

    return !raidboss::FindUnits(bot, NPC_AMANISHI_HATCHER_1, ZA_CREATURE_SEARCH_RADIUS).empty() ||
           !raidboss::FindUnits(bot, NPC_AMANISHI_HATCHER_2, ZA_CREATURE_SEARCH_RADIUS).empty();
}

// Halazzi <Lynx Avatar>

bool ZaHalazziShouldBeTankedTrigger::IsActiveInEncounter()
{
    return raidboss::IsMainTank(bot) && GetBoss(ai, NPC_HALAZZI);
}

bool ZaHalazziSpiritLynxHasAppearedTrigger::IsActiveInEncounter()
{
    return raidboss::IsAssistTank(bot, 0) && GetBoss(ai, NPC_HALAZZI);
}

bool ZaHalazziShouldFocusDpsTrigger::IsActiveInEncounter()
{
    return raidboss::IsDps(bot) && GetBoss(ai, NPC_HALAZZI);
}

// Hex Lord Malacrass

bool ZaHexLordMalacrassShouldPrioritizeAddsTrigger::IsActiveInEncounter()
{
    return raidboss::IsDps(bot) && GetBoss(ai, NPC_HEX_LORD_MALACRASS);
}

bool ZaHexLordMalacrassChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    Unit* malacrass = GetBoss(ai, NPC_HEX_LORD_MALACRASS);
    if (!malacrass || malacrass->GetVictim() == bot)
        return false;

    return malacrass->HasAura(SPELL_HEX_LORD_WHIRLWIND) && raidboss::ExactDist2d(bot, malacrass) < ZA_WHIRLWIND_SAFE_DISTANCE;
}

bool ZaHexLordMalacrassFreezingTrapPlacedTrigger::IsActiveInEncounter()
{
    if (!GetBoss(ai, NPC_HEX_LORD_MALACRASS))
        return false;

    GameObject* trap = GetNearbyFreezingTrap(ai);
    return trap && raidboss::ExactDist2d(bot, trap) < ZA_FREEZING_TRAP_SAFE_DISTANCE;
}

// Zul'jin

bool ZaZuljinShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* zuljin = GetBoss(ai, NPC_ZULJIN);
    if (!zuljin)
        return false;

    // Eagle can't be tanked, and the tank needs the freedom to turn Zul'jin during Dragonhawk
    return !zuljin->HasAura(SPELL_SHAPE_OF_THE_EAGLE) && !zuljin->HasAura(SPELL_SHAPE_OF_THE_DRAGONHAWK);
}

bool ZaZuljinChannelingWhirlwindInTrollFormTrigger::IsActiveInEncounter()
{
    Unit* zuljin = GetBoss(ai, NPC_ZULJIN);
    if (!zuljin || !zuljin->HasAura(SPELL_ZULJIN_WHIRLWIND))
        return false;

    if (PlayerbotAI::IsTank(bot) && zuljin->GetVictim() == bot)
        return false;

    return raidboss::ExactDist2d(bot, zuljin) < ZA_WHIRLWIND_SAFE_DISTANCE;
}

bool ZaZuljinCreepingParalysisInBearFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* zuljin = GetBoss(ai, NPC_ZULJIN);
    if (!zuljin || !zuljin->HasAura(SPELL_SHAPE_OF_THE_BEAR))
        return false;

    return GetZuljinCreepingParalysisDispelTarget(bot) != nullptr;
}

bool ZaZuljinSummoningCyclonesInEagleFormTrigger::IsActiveInEncounter()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    Unit* zuljin = GetBoss(ai, NPC_ZULJIN);
    if (!zuljin)
        return false;

    if (zuljin->HasAura(SPELL_SHAPE_OF_THE_EAGLE))
        return true;

    // The health check lets ranged head to their positions during the phase transition
    const float healthPct = zuljin->GetHealthPercent();
    return healthPct <= 60.0f && healthPct > 40.0f;
}

bool ZaZuljinSpreadForDragonhawkAoeTrigger::IsActiveInEncounter()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    Unit* zuljin = GetBoss(ai, NPC_ZULJIN);
    return zuljin && zuljin->HasAura(SPELL_SHAPE_OF_THE_DRAGONHAWK);
}
