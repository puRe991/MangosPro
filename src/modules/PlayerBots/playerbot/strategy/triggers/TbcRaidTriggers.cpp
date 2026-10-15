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

bool TbcRaidTankAssignedTrigger::IsActive()
{
    if (!bot->IsAlive())
        return false;

    const auto mine = TbcRaidHelper::GetMyAssignments(ai);
    if (mine.empty())
        return false;

    const ObjectGuid current = AI_VALUE(ObjectGuid, "current target");
    bool currentIsMine = false;
    for (auto& assignment : mine)
    {
        // a loose creature has to be picked up (class tanks only need it as target)
        if (!assignment.second->playerClass && assignment.first->GetVictim() != bot)
            return true;

        if (assignment.first->GetObjectGuid() == current)
            currentIsMine = true;
    }

    return !currentIsMine;
}

bool TbcRaidTankPositionTrigger::IsActive()
{
    if (!bot->IsAlive() || !ai->CanMove())
        return false;

    const auto mine = TbcRaidHelper::GetMyAssignments(ai);
    if (mine.empty())
        return false;

    Unit* creature = mine.front().first;
    const TankAssignment& rule = *mine.front().second;

    // a tank first picks the creature up, a class tank goes to its spot right away
    if (!rule.playerClass && creature->GetVictim() != bot)
        return false;

    WorldPosition spot;
    if (!TbcRaidHelper::GetAssignmentSpot(creature, rule, spot))
        return false;

    return WorldPosition(bot).distance(spot) > (rule.playerClass ? 6.0f : 4.0f);
}

bool TbcRaidStealAuraTrigger::IsActive()
{
    for (auto& assignment : TbcRaidHelper::GetMyAssignments(ai))
    {
        if (assignment.second->stealAura && assignment.first->HasAura(assignment.second->stealAura) &&
            ai->CanCastSpell("spellsteal", assignment.first, 0))
            return true;
    }

    return false;
}

bool TbcRaidInterruptTrigger::IsActive()
{
    std::string spell;
    return bot->IsAlive() && TbcRaidHelper::GetInterruptTarget(ai, spell) != nullptr;
}

bool TbcRaidMarkTargetsTrigger::IsActive()
{
    const Encounter* encounter = TbcRaidHelper::GetEncounter(ai);
    Group* group = bot->GetGroup();
    if (!encounter || encounter->marks.empty() || !group || !TbcRaidHelper::IsRaidLeaderBot(ai))
        return false;

    // an icon used by several rules goes to the first rule with a living creature
    std::vector<uint8> claimed;
    for (size_t i = 0; i < encounter->marks.size(); ++i)
    {
        const uint8 icon = encounter->marks[i].icon;
        Unit* desired = TbcRaidHelper::GetMarkTarget(ai, encounter, i);
        if (!desired || std::find(claimed.begin(), claimed.end(), icon) != claimed.end())
            continue;

        claimed.push_back(icon);
        if (group->GetTargetIcon(icon) != desired->GetObjectGuid())
            return true;
    }

    return false;
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
