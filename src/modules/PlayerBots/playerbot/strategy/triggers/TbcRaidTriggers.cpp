#include "playerbot/playerbot.h"
#include "TbcRaidTriggers.h"
#include "playerbot/strategy/generic/TbcRaidStrategy.h"

using namespace ai;
using namespace ai::tbcraid;

bool TbcRaidEnterTrigger::IsActive()
{
    if (ai->HasStrategy("tbc raid", BotState::BOT_STATE_COMBAT))
        return false;

    if (!bot->IsInWorld() || bot->IsBeingTeleported())
        return false;

    Map* map = bot->GetMap();
    return map && (map->IsDungeon() || map->IsRaid()) && EncounterRegistry::Instance().IsRaidMap(map->GetId());
}

bool TbcRaidLeaveTrigger::IsActive()
{
    if (!ai->HasStrategy("tbc raid", BotState::BOT_STATE_COMBAT))
        return false;

    if (!bot->IsInWorld() || bot->IsBeingTeleported())
        return false;

    return !EncounterRegistry::Instance().IsRaidMap(bot->GetMapId());
}

bool TbcRaidInHazardTrigger::IsActive()
{
    if (!bot->IsAlive())
        return false;

    const std::vector<HazardZone>& hazards = TbcRaidHelper::GetHazards(ai);
    return !hazards.empty() && TbcRaidHelper::IsInHazard(WorldPosition(bot), hazards);
}

namespace
{
    // Distance to the closest other raid member
    float GetClosestMemberDistance(Player* bot, float range)
    {
        float closest = range;
        for (Player* member : TbcRaidHelper::GetRaidMembers(bot, range))
        {
            const float distance = bot->GetDistance(member, false);
            if (distance < closest)
                closest = distance;
        }

        return closest;
    }
}

bool TbcRaidDebuffRunTrigger::IsActive()
{
    if (!bot->IsAlive())
        return false;

    bool urgent = false;
    const float required = TbcRaidHelper::GetRequiredSpread(ai, &urgent);
    if (!urgent || required <= 0.0f)
        return false;

    return GetClosestMemberDistance(bot, required) < required;
}

bool TbcRaidSpreadTrigger::IsActive()
{
    if (!bot->IsAlive())
        return false;

    bool urgent = false;
    const float required = TbcRaidHelper::GetRequiredSpread(ai, &urgent);
    if (urgent || required <= 0.0f)
        return false;

    // Some tolerance to avoid moving constantly
    return GetClosestMemberDistance(bot, required) < required * 0.75f;
}

bool TbcRaidGatherTrigger::IsActive()
{
    if (!bot->IsAlive())
        return false;

    float distance = 0.0f;
    Player* target = TbcRaidHelper::GetGatherTarget(ai, distance);
    if (!target)
        return false;

    const float dx = bot->GetPositionX() - target->GetPositionX();
    const float dy = bot->GetPositionY() - target->GetPositionY();
    return (dx * dx + dy * dy) > distance * distance;
}

bool TbcRaidInFrontOfBossTrigger::IsActive()
{
    return bot->IsAlive() && TbcRaidHelper::GetFrontDanger(ai) != nullptr;
}

bool TbcRaidPriorityTargetTrigger::IsActive()
{
    if (!bot->IsAlive())
        return false;

    Unit* target = TbcRaidHelper::GetPriorityTarget(ai);
    if (!target)
        return false;

    const ObjectGuid currentTarget = AI_VALUE(ObjectGuid, "current target");
    return currentTarget != target->GetObjectGuid();
}

bool TbcRaidTauntTrigger::IsActive()
{
    return bot->IsAlive() && TbcRaidHelper::GetTauntTarget(ai) != nullptr;
}

bool TbcRaidUseCubeTrigger::IsActive()
{
    if (!bot->IsAlive() || bot->HasAura(30410))
        return false;

    Unit* magtheridon = nullptr;
    GameObject* cube = TbcRaidHelper::GetAssignedCube(ai, &magtheridon);
    if (!cube || !magtheridon)
        return false;

    // Move next to the cube, then click it as soon as Blast Nova starts
    if (!bot->IsWithinDistInMap(cube, INTERACTION_DISTANCE - 1.0f))
        return true;

    if (TbcRaidHelper::IsCastingBlastNova(magtheridon))
        return true;

    // Join the other clickers
    for (Player* member : TbcRaidHelper::GetRaidMembers(bot, 150.0f))
    {
        if (member->HasAura(30410))
            return true;
    }

    return false;
}

bool TbcRaidUseSpineTrigger::IsActive()
{
    if (!bot->IsAlive())
        return false;

    return TbcRaidHelper::GetShieldedNajentus(ai) || TbcRaidHelper::GetSpineToRemove(ai);
}
