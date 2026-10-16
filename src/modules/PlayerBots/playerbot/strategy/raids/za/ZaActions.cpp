// Ported from mod-playerbots (AzerothCore, GPL v2+), src/Ai/Raid/ZA/ZAActions.cpp, and adapted for this core.
#include "playerbot/playerbot.h"
#include "ZaActions.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include <algorithm>
#include <array>
#include <cmath>

using namespace ai;
using namespace ai::za;

namespace
{
    // Ground height at a step position (the bosses stand on stairs and platforms)
    float GroundZ(Player* bot, float x, float y, float z)
    {
        const float groundZ = bot->GetMap()->GetHeight(x, y, z + 4.0f);
        return groundZ > INVALID_HEIGHT ? groundZ : z;
    }

    float MinDistanceToOthers(float x, float y, const std::vector<Player*>& members)
    {
        float closest = 9999.0f;
        for (Player* member : members)
            closest = std::min(closest, raidboss::ExactDist2d(member, x, y));

        return closest;
    }
}

// General

bool ZaResetEncounterStatesAction::Execute(Event& event)
{
    bool reset = false;
    if (AI_VALUE(uint32, "akil'zon electrical storm timer") || AI_VALUE(uint32, "akil'zon last electrical storm"))
    {
        SET_AI_VALUE(uint32, "akil'zon electrical storm timer", 0);
        SET_AI_VALUE(uint32, "akil'zon last electrical storm", 0);
        reset = true;
    }

    if (!bot->IsInCombat() && raidboss::IsMechanicTrackerBot(bot, ZA_MAP_ID))
    {
        reset |= raidboss::ClearTargetIcon(bot, raidboss::ICON_SKULL);
        reset |= raidboss::ClearTargetIcon(bot, raidboss::ICON_MOON);
    }

    return reset;
}

bool ZaMisdirectBossToMainTankAction::Execute(Event& event)
{
    Unit* boss = GetBoss(ai, bossEntry);
    if (!boss)
        return false;

    Player* mainTank = raidboss::GetGroupMainTank(bot);
    if (!mainTank || mainTank == bot || !mainTank->IsAlive())
        return false;

    if (raidboss::TryCast(ai, "misdirection", mainTank))
        return true;

    if (!bot->HasAura(SPELL_MISDIRECTION))
        return false;

    return raidboss::TryCast(ai, "steady shot", boss);
}

bool ZaTanksPositionBossAction::Execute(Event& event)
{
    Unit* boss = GetBoss(ai, bossEntry);
    if (!boss)
        return false;

    if (AI_VALUE(ObjectGuid, "current target") != boss->GetObjectGuid() && raidboss::IsMainTank(bot))
        return Attack(GetMaster(), boss);

    if (boss->GetVictim() != bot || !bot->CanReachWithMeleeAttack(boss))
        return false;

    const float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!raidboss::GetStepToPosition(bot, position, arrivalDist, boss, moveX, moveY, backwards))
        return false;

    return MoveTo(ZA_MAP_ID, moveX, moveY, GroundZ(bot, moveX, moveY, bot->GetPositionZ()), false, false, false, true);
}

bool ZaSpreadRangedAction::Execute(Event& event)
{
    Player* nearestPlayer = raidboss::GetNearestPlayerInRadius(bot, minDistance);
    if (!nearestPlayer)
        return false;

    const std::vector<Player*> members = raidboss::GetGroupMembers(bot, minDistance * 3.0f, false);
    const float currentMinDistance = MinDistanceToOthers(bot->GetPositionX(), bot->GetPositionY(), members);
    const WorldPosition botPosition(bot);

    // Flee from the nearest player: any point which improves the spacing, the closest first
    return MoveToBestPoint(2.0f, minDistance, [&](const WorldPosition& point) -> float
    {
        const float spacing = MinDistanceToOthers(point.getX(), point.getY(), members);
        if (spacing <= currentMinDistance + 0.5f)
            return -1.0f;

        const float missing = std::max(0.0f, minDistance - spacing);
        return missing * 10.0f + sqrt(point.sqDistance2d(botPosition));
    });
}

bool ZaRunAwayFromWhirlwindAction::Execute(Event& event)
{
    Unit* boss = GetBoss(ai, bossEntry);
    if (!boss)
        return false;

    const float currentDistance = raidboss::ExactDist2d(bot, boss);
    if (currentDistance >= ZA_WHIRLWIND_SAFE_DISTANCE)
        return false;

    if (bot->IsNonMeleeSpellCasted(true))
        bot->InterruptNonMeleeSpells(true);

    const WorldPosition botPosition(bot);
    return MoveToBestPoint(std::max(2.0f, ZA_WHIRLWIND_SAFE_DISTANCE - currentDistance), ZA_WHIRLWIND_SAFE_DISTANCE + 6.0f,
        [&](const WorldPosition& point) -> float
    {
        if (raidboss::ExactDist2d(boss, point.getX(), point.getY()) < ZA_WHIRLWIND_SAFE_DISTANCE)
            return -1.0f;

        return sqrt(point.sqDistance2d(botPosition));
    });
}

// Trash

bool ZaAmanishiMedicineManMarkWardAction::Execute(Event& event)
{
    if (Unit* protectiveWard = raidboss::FindUnit(ai, NPC_AMANI_PROTECTIVE_WARD, ZA_CREATURE_SEARCH_RADIUS))
        return raidboss::MarkTargetWithSkull(bot, protectiveWard);

    Unit* healingWard = raidboss::FindUnit(ai, NPC_AMANI_HEALING_WARD, ZA_CREATURE_SEARCH_RADIUS);
    return healingWard && raidboss::MarkTargetWithSkull(bot, healingWard);
}

// Akil'zon <Eagle Avatar>

bool ZaAkilzonMoveToEyeOfTheStormAction::Execute(Event& event)
{
    // Stack under the Electrical Storm target, before the cast on the main tank
    Player* target = GetElectricalStormTarget(bot);
    if (target == bot)
        return false;

    if (!target && !raidboss::IsMainTank(bot))
        target = raidboss::GetGroupMainTank(bot);

    const float arrivalDist = 3.0f;
    if (!target || raidboss::ExactDist2d(bot, target) <= arrivalDist)
        return false;

    if (bot->IsNonMeleeSpellCasted(true))
        bot->InterruptNonMeleeSpells(true);

    // The storm target is lifted into the air, stay on the ground below it
    const float z = GroundZ(bot, target->GetPositionX(), target->GetPositionY(), bot->GetPositionZ());
    return MoveTo(ZA_MAP_ID, target->GetPositionX(), target->GetPositionY(), z, false, false, false, true);
}

bool ZaAkilzonStartElectricalStormTimerAction::Execute(Event& event)
{
    const uint32 now = WorldTimer::getMSTime();
    bool changed = false;

    if (!AI_VALUE(uint32, "akil'zon electrical storm timer"))
    {
        SET_AI_VALUE(uint32, "akil'zon electrical storm timer", now ? now : 1);
        changed = true;
    }

    if (GetElectricalStormTarget(bot))
    {
        const uint32 lastStorm = AI_VALUE(uint32, "akil'zon last electrical storm");
        if (!lastStorm || WorldTimer::getMSTimeDiff(lastStorm, now) > 20000)
        {
            SET_AI_VALUE(uint32, "akil'zon last electrical storm", now ? now : 1);
            changed = true;
        }
    }

    return changed;
}

// Nalorakk <Bear Avatar>

bool ZaNalorakkTanksPositionBossAction::Execute(Event& event)
{
    Unit* nalorakk = GetBoss(ai, NPC_NALORAKK);
    if (!nalorakk)
        return false;

    if (AI_VALUE(ObjectGuid, "current target") != nalorakk->GetObjectGuid())
        return Attack(GetMaster(), nalorakk);

    // Main tank takes bear, assist tank takes troll
    Player* nalorakkTank = GetNalorakkFormTank(bot, nalorakk);
    if (nalorakkTank && nalorakkTank == bot)
    {
        if (nalorakk->GetVictim() != bot)
            return CastTaunt(ai, nalorakk);

        if (!bot->CanReachWithMeleeAttack(nalorakk))
            return false;
    }

    // Both tanks move to the position, the one holding Nalorakk drags him there
    const float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!raidboss::GetStepToPosition(bot, NALORAKK_TANK_POSITION, arrivalDist, nalorakk, moveX, moveY, backwards))
        return false;

    return MoveTo(ZA_MAP_ID, moveX, moveY, GroundZ(bot, moveX, moveY, bot->GetPositionZ()), false, false, false, true);
}

// Jan'alai <Dragonhawk Avatar>

bool ZaJanalaiSpreadRangedInCircleAction::Execute(Event& event)
{
    std::vector<Player*> rangedMembers;
    for (Player* member : raidboss::GetGroupMembers(bot, 500.0f, true))
    {
        if (raidboss::IsRangedPlayer(member))
            rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    const int index = raidboss::GetIndexOf(bot, rangedMembers);
    const size_t botIndex = index >= 0 ? size_t(index) : 0;
    const size_t count = rangedMembers.size();
    const float angle = (count == 1) ? 0.0f : (2.0f * M_PI_F * float(botIndex) / float(count));

    const float targetX = JANALAI_TANK_POSITION.getX() + JANALAI_RANGED_SPREAD_RADIUS * std::cos(angle);
    const float targetY = JANALAI_TANK_POSITION.getY() + JANALAI_RANGED_SPREAD_RADIUS * std::sin(angle);

    const float arrivalDist = 2.0f;
    if (raidboss::ExactDist2d(bot, targetX, targetY) <= arrivalDist)
        return false;

    return MoveTo(ZA_MAP_ID, targetX, targetY, GroundZ(bot, targetX, targetY, JANALAI_PLATFORM_Z), false, false, false, true);
}

bool ZaJanalaiAvoidFireBombsAction::Execute(Event& event)
{
    const std::vector<Unit*> bombs = GetNearbyFireBombs(ai);
    if (bombs.empty())
        return false;

    bool inDanger = false;
    for (Unit* bomb : bombs)
    {
        if (raidboss::ExactDist2d(bot, bomb) < JANALAI_FIRE_BOMB_SAFE_DISTANCE)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    const float maxFloorDeviation = 5.0f;
    if (std::fabs(bot->GetPositionZ() - JANALAI_PLATFORM_Z) > maxFloorDeviation)
        return false;

    const float moveDist = 3.5f;
    float stepX;
    float stepY;
    float stepZ;
    if (!FindSafeStepInJanalaiZone(bot, bombs, JANALAI_SAFE_ZONE, JANALAI_FIRE_BOMB_MAX_SEARCH_DISTANCE,
                                   JANALAI_FIRE_BOMB_SAFE_DISTANCE, moveDist, stepX, stepY, stepZ))
    {
        return false;
    }

    if (bot->IsNonMeleeSpellCasted(true))
        bot->InterruptNonMeleeSpells(true);

    return MoveTo(ZA_MAP_ID, stepX, stepY, stepZ, false, false, false, true);
}

bool ZaJanalaiMarkAmanishiHatchersAction::Execute(Event& event)
{
    // One hatcher per side: the first one gets the skull, the second one the moon
    std::pair<Unit*, Unit*> hatchers = GetAmanishiHatcherPair(ai);
    Unit* skullHatcher = hatchers.first ? hatchers.first : hatchers.second;
    Unit* moonHatcher = hatchers.first ? hatchers.second : nullptr;

    if (moonHatcher && raidboss::MarkTargetWithMoon(bot, moonHatcher))
        return true;

    return skullHatcher && raidboss::MarkTargetWithSkull(bot, skullHatcher);
}

// Halazzi <Lynx Avatar>

bool ZaHalazziFirstAssistTankAttackSpiritLynxAction::Execute(Event& event)
{
    if (Unit* lynx = raidboss::FindUnit(ai, NPC_SPIRIT_OF_THE_LYNX))
    {
        if (AI_VALUE(ObjectGuid, "current target") != lynx->GetObjectGuid())
            return Attack(GetMaster(), lynx);

        if (lynx->GetVictim() != bot && CastTaunt(ai, lynx))
            return true;
    }

    const float arrivalDist = 2.0f;
    if (raidboss::ExactDist2d(bot, HALAZZI_TANK_POSITION.getX(), HALAZZI_TANK_POSITION.getY()) <= arrivalDist)
        return false;

    return MoveTo(ZA_MAP_ID, HALAZZI_TANK_POSITION.getX(), HALAZZI_TANK_POSITION.getY(), HALAZZI_TANK_POSITION.getZ(), false, false, false, true);
}

bool ZaHalazziDpsAttackTotemAndBossAction::Execute(Event& event)
{
    const ObjectGuid currentTarget = AI_VALUE(ObjectGuid, "current target");

    // Target priority 1: Corrupted Lightning Totems
    if (Unit* totem = raidboss::FindUnit(ai, NPC_CORRUPTED_LIGHTNING_TOTEM, ZA_CREATURE_SEARCH_RADIUS))
    {
        if (raidboss::MarkTargetWithSkull(bot, totem))
            return true;

        return currentTarget != totem->GetObjectGuid() && Attack(GetMaster(), totem);
    }

    // Target priority 2: Halazzi (don't attack the Lynx)
    if (Unit* halazzi = GetBoss(ai, NPC_HALAZZI))
        return currentTarget != halazzi->GetObjectGuid() && Attack(GetMaster(), halazzi);

    return false;
}

// Hex Lord Malacrass

bool ZaHexLordMalacrassAssignDpsPriorityAction::Execute(Event& event)
{
    static const std::array<uint32, 9> hexLordAdds = {
        NPC_LORD_RAADAN,
        NPC_ALYSON_ANTILLE,
        NPC_KORAGG,
        NPC_DARKHEART,
        NPC_FENSTALKER,
        NPC_GAZAKROTH,
        NPC_THURG,
        NPC_SLITHER,
        NPC_HEX_LORD_MALACRASS
    };

    Unit* priorityTarget = nullptr;
    size_t bestRank = hexLordAdds.size();
    for (ObjectGuid guid : AI_VALUE(std::list<ObjectGuid>, "possible targets no los"))
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        const size_t rank = size_t(std::distance(hexLordAdds.begin(), std::find(hexLordAdds.begin(), hexLordAdds.end(), unit->GetEntry())));
        if (rank < bestRank)
        {
            bestRank = rank;
            priorityTarget = unit;
        }
    }

    return priorityTarget && raidboss::MarkTargetWithSkull(bot, priorityTarget);
}

bool ZaHexLordMalacrassMoveAwayFromFreezingTrapAction::Execute(Event& event)
{
    GameObject* trap = GetNearbyFreezingTrap(ai);
    if (!trap)
        return false;

    const float currentDistance = raidboss::ExactDist2d(bot, trap);
    if (currentDistance >= ZA_FREEZING_TRAP_SAFE_DISTANCE)
        return false;

    const WorldPosition botPosition(bot);
    return MoveToBestPoint(std::max(2.0f, ZA_FREEZING_TRAP_SAFE_DISTANCE - currentDistance), ZA_FREEZING_TRAP_SAFE_DISTANCE + 4.0f,
        [&](const WorldPosition& point) -> float
    {
        if (raidboss::ExactDist2d(trap, point.getX(), point.getY()) < ZA_FREEZING_TRAP_SAFE_DISTANCE)
            return -1.0f;

        return sqrt(point.sqDistance2d(botPosition));
    });
}

// Zul'jin

bool ZaZuljinMassDispelCreepingParalysisAction::Execute(Event& event)
{
    // Mass Dispel is cast on a location
    Player* paralysisTarget = GetZuljinCreepingParalysisDispelTarget(bot);
    if (!paralysisTarget)
        return false;

    const float x = paralysisTarget->GetPositionX();
    const float y = paralysisTarget->GetPositionY();
    const float z = paralysisTarget->GetPositionZ();
    return ai->CanCastSpell(SPELL_MASS_DISPEL, x, y, z, 0) && ai->CastSpell(SPELL_MASS_DISPEL, x, y, z);
}

bool ZaZuljinPositionRangedForCyclonesAction::Execute(Event& event)
{
    size_t slotIndex;
    if (!GetZuljinSpreadSlotIndex(bot, ZULJIN_SPREAD_POSITIONS.size(), slotIndex))
        return false;

    const WorldPosition& position = ZULJIN_SPREAD_POSITIONS[slotIndex];

    const float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!raidboss::GetStepToPosition(bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(ZA_MAP_ID, moveX, moveY, GroundZ(bot, moveX, moveY, ZULJIN_SPREAD_Z), false, false, false, true);
}
