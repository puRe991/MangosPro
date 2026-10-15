#include "playerbot/playerbot.h"
#include "TbcRaidActions.h"
#include "playerbot/strategy/generic/TbcRaidStrategy.h"

using namespace ai;
using namespace ai::tbcraid;

namespace
{
    const uint32 ITEM_NAJENTUS_SPINE = 32408;
    const uint32 SPELL_SHADOW_GRASP = 30410;

    float MinDistanceToMembers(const WorldPosition& point, const std::vector<Player*>& members)
    {
        float closest = 9999.0f;
        for (Player* member : members)
        {
            const float distance = sqrt(point.sqDistance2d(WorldPosition(member)));
            if (distance < closest)
                closest = distance;
        }

        return closest;
    }

    float Distance2d(const WorldPosition& a, const WorldPosition& b)
    {
        return sqrt(a.sqDistance2d(b));
    }
}

// ---------------------------------------------------------------------------
// Movement base
// ---------------------------------------------------------------------------

bool TbcRaidMovementAction::MoveToPoint(const WorldPosition& point)
{
    if (ai->HasStrategy("debug move", BotState::BOT_STATE_COMBAT))
        bot->SummonCreature(15631, point.getX(), point.getY(), point.getZ(), 0.0f, TEMPSPAWN_TIMED_DESPAWN, 5000.0f);

    return MoveTo(bot->GetMapId(), point.getX(), point.getY(), point.getZ(), false, IsReaction(), false, true);
}

bool TbcRaidMovementAction::MoveToBestPoint(float minRadius, float maxRadius, const std::function<float(const WorldPosition&)>& score)
{
    struct Candidate
    {
        WorldPosition position;
        float score;
    };

    const WorldPosition botPosition(bot);
    std::vector<Candidate> candidates;

    const uint8 angles = 16;
    const float radiusStep = std::max(2.0f, (maxRadius - minRadius) / 4.0f);
    for (float radius = minRadius; radius <= maxRadius + 0.01f; radius += radiusStep)
    {
        for (uint8 i = 0; i < angles; ++i)
        {
            const float angle = i * (2.0f * M_PI_F / angles);
            const float x = botPosition.getX() + radius * cos(angle);
            const float y = botPosition.getY() + radius * sin(angle);
            const float z = bot->GetMap()->GetHeight(x, y, botPosition.getZ() + 5.0f);
            if (z <= INVALID_HEIGHT || fabs(z - botPosition.getZ()) > 8.0f)
                continue;

            const WorldPosition point(bot->GetMapId(), x, y, z);
            const float pointScore = score(point);
            if (pointScore < 0.0f)
                continue;

            candidates.push_back({ point, pointScore });
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) { return a.score < b.score; });

    // Pathfinding is expensive, only check the best candidates
    uint8 attempts = 0;
    for (const Candidate& candidate : candidates)
    {
        if (++attempts > 10)
            break;

        const WorldPosition& point = candidate.position;
        if (!bot->IsWithinLOS(point.getX(), point.getY(), point.getZ() + bot->GetCollisionHeight()))
            continue;

        if (!botPosition.canPathTo(point, bot))
            continue;

        if (MoveToPoint(point))
        {
            if (!IsReaction())
                WaitForReach(botPosition.distance(point));

            return true;
        }
    }

    return false;
}

Unit* TbcRaidMovementAction::GetAnchor()
{
    const uint8 role = TbcRaidHelper::GetRole(ai);
    if (role == ROLE_HEAL)
    {
        Unit* boss = nullptr;
        TbcRaidHelper::GetEncounter(ai, &boss);
        if (boss && boss->GetVictim())
            return boss->GetVictim();

        return boss;
    }

    return ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
}

float TbcRaidMovementAction::GetAnchorRange()
{
    const uint8 role = TbcRaidHelper::GetRole(ai);
    if (role == ROLE_HEAL)
        return 36.0f;

    if (role == ROLE_RANGED)
        return 33.0f;

    return 0.0f;
}

// ---------------------------------------------------------------------------
// Leave hazard
// ---------------------------------------------------------------------------

bool TbcRaidLeaveHazardAction::Execute(Event& event)
{
    const std::vector<HazardZone>& hazards = TbcRaidHelper::GetHazards(ai);
    const WorldPosition botPosition(bot);

    float maxRadius = 0.0f;
    for (const HazardZone& hazard : hazards)
    {
        if (botPosition.sqDistance2d(hazard.position) < hazard.radius * hazard.radius)
            maxRadius = std::max(maxRadius, hazard.radius * 2.0f + 4.0f);
    }

    if (maxRadius <= 0.0f)
        return false;

    Unit* anchor = GetAnchor();
    const float anchorRange = GetAnchorRange();
    const bool melee = TbcRaidHelper::GetRole(ai) == ROLE_MELEE || TbcRaidHelper::GetRole(ai) == ROLE_TANK;

    // Copy: the cache can be refreshed while scoring
    const std::vector<HazardZone> zones = hazards;
    return MoveToBestPoint(3.0f, maxRadius, [&](const WorldPosition& point) -> float
    {
        if (TbcRaidHelper::IsInHazard(point, zones, 1.5f))
            return -1.0f;

        float score = Distance2d(botPosition, point);
        if (anchor)
        {
            const float anchorDistance = Distance2d(point, WorldPosition(anchor));
            if (anchorRange > 0.0f && anchorDistance > anchorRange)
                score += (anchorDistance - anchorRange) * 3.0f;
            else if (melee)
                score += anchorDistance * 0.5f;
        }

        return score;
    });
}

// ---------------------------------------------------------------------------
// Spread / debuff run
// ---------------------------------------------------------------------------

bool TbcRaidSpreadAction::Execute(Event& event)
{
    bool urgent = false;
    const float required = TbcRaidHelper::GetRequiredSpread(ai, &urgent);
    if (required <= 0.0f)
        return false;

    const std::vector<Player*> members = TbcRaidHelper::GetRaidMembers(bot, required + 40.0f);
    if (members.empty())
        return false;

    const WorldPosition botPosition(bot);
    const float currentDistance = MinDistanceToMembers(botPosition, members);
    if (currentDistance >= required)
        return false;

    const std::vector<HazardZone> zones = TbcRaidHelper::GetHazards(ai);
    Unit* anchor = GetAnchor();
    float anchorRange = GetAnchorRange();
    if (urgent && anchorRange > 0.0f)
        anchorRange += 8.0f;    // getting away has priority over staying in range

    return MoveToBestPoint(required * 0.5f, required * 2.0f + 5.0f, [&](const WorldPosition& point) -> float
    {
        if (TbcRaidHelper::IsInHazard(point, zones, 1.0f))
            return -1.0f;

        const float memberDistance = MinDistanceToMembers(point, members);
        if (memberDistance <= currentDistance + 1.0f)
            return -1.0f;

        if (anchor && anchorRange > 0.0f && Distance2d(point, WorldPosition(anchor)) > anchorRange)
            return -1.0f;

        float score = Distance2d(botPosition, point);
        if (memberDistance < required)
            score += (required - memberDistance) * 20.0f;

        return score;
    });
}

// ---------------------------------------------------------------------------
// Gather
// ---------------------------------------------------------------------------

bool TbcRaidGatherAction::Execute(Event& event)
{
    float distance = 0.0f;
    Player* target = TbcRaidHelper::GetGatherTarget(ai, distance);
    if (!target)
        return false;

    // The target may be lifted into the air, stand on the ground below it
    const float x = target->GetPositionX();
    const float y = target->GetPositionY();
    float z = bot->GetMap()->GetHeight(x, y, target->GetPositionZ());
    if (z <= INVALID_HEIGHT)
        z = target->GetPositionZ();

    return MoveToPoint(WorldPosition(bot->GetMapId(), x, y, z));
}

// ---------------------------------------------------------------------------
// Leave front
// ---------------------------------------------------------------------------

bool TbcRaidLeaveFrontAction::Execute(Event& event)
{
    Unit* boss = TbcRaidHelper::GetFrontDanger(ai);
    if (!boss)
        return false;

    const WorldPosition botPosition(bot);
    const WorldPosition bossPosition(boss);
    const uint8 role = TbcRaidHelper::GetRole(ai);

    float distance = Distance2d(botPosition, bossPosition);
    if (role == ROLE_MELEE || role == ROLE_TANK)
        distance = std::max(2.0f, boss->GetCombatReach());
    else
        distance = std::min(30.0f, std::max(8.0f, distance));

    const std::vector<HazardZone> zones = TbcRaidHelper::GetHazards(ai);
    const float facing = boss->GetOrientation();
    const float offsets[] = { M_PI_F + 0.8f, M_PI_F - 0.8f, M_PI_F, M_PI_F / 2.0f + 0.4f, -(M_PI_F / 2.0f + 0.4f) };

    const WorldPosition* best = nullptr;
    std::vector<WorldPosition> points;
    for (float offset : offsets)
    {
        const float angle = facing + offset;
        const float x = bossPosition.getX() + distance * cos(angle);
        const float y = bossPosition.getY() + distance * sin(angle);
        const float z = bot->GetMap()->GetHeight(x, y, bossPosition.getZ() + 5.0f);
        if (z <= INVALID_HEIGHT)
            continue;

        points.emplace_back(bot->GetMapId(), x, y, z);
    }

    std::sort(points.begin(), points.end(), [&](const WorldPosition& a, const WorldPosition& b) { return botPosition.sqDistance2d(a) < botPosition.sqDistance2d(b); });
    for (const WorldPosition& point : points)
    {
        if (TbcRaidHelper::IsInHazard(point, zones, 1.0f))
            continue;

        if (!bot->IsWithinLOS(point.getX(), point.getY(), point.getZ() + bot->GetCollisionHeight()))
            continue;

        if (!botPosition.canPathTo(point, bot))
            continue;

        best = &point;
        break;
    }

    return best ? MoveToPoint(*best) : false;
}

// ---------------------------------------------------------------------------
// Magtheridon - Manticron Cubes
// ---------------------------------------------------------------------------

bool TbcRaidUseCubeAction::Execute(Event& event)
{
    if (bot->HasAura(SPELL_SHADOW_GRASP))
        return false;

    Unit* magtheridon = nullptr;
    GameObject* cube = TbcRaidHelper::GetAssignedCube(ai, &magtheridon);
    if (!cube || !magtheridon)
        return false;

    if (!bot->IsWithinDistInMap(cube, INTERACTION_DISTANCE - 1.0f))
    {
        // Stand next to the cube, on the side facing the bot
        const float angle = cube->GetAngle(bot);
        const float x = cube->GetPositionX() + 2.0f * cos(angle);
        const float y = cube->GetPositionY() + 2.0f * sin(angle);
        float z = bot->GetMap()->GetHeight(x, y, cube->GetPositionZ() + 5.0f);
        if (z <= INVALID_HEIGHT)
            z = cube->GetPositionZ();

        return MoveToPoint(WorldPosition(bot->GetMapId(), x, y, z));
    }

    bool shouldClick = TbcRaidHelper::IsCastingBlastNova(magtheridon);
    if (!shouldClick)
    {
        // Join the other clickers (players may click before the bots notice the cast)
        for (Player* member : TbcRaidHelper::GetRaidMembers(bot, 150.0f))
        {
            if (member->HasAura(SPELL_SHADOW_GRASP))
            {
                shouldClick = true;
                break;
            }
        }
    }

    if (!shouldClick)
        return false;

    if (bot->IsNonMeleeSpellCasted(true))
        bot->InterruptNonMeleeSpells(true);

    if (bot->IsMoving())
        bot->StopMoving();

    bot->SetFacingTo(bot->GetAngle(cube));
    cube->Use(bot);
    return true;
}

// ---------------------------------------------------------------------------
// Naj'entus - spines
// ---------------------------------------------------------------------------

bool TbcRaidMoveToSpineAction::Execute(Event& event)
{
    GameObject* spine = TbcRaidHelper::GetSpineToRemove(ai);
    if (!spine || bot->IsWithinDistInMap(spine, INTERACTION_DISTANCE - 1.0f))
        return false;

    const float angle = spine->GetAngle(bot);
    const float x = spine->GetPositionX() + 1.5f * cos(angle);
    const float y = spine->GetPositionY() + 1.5f * sin(angle);
    float z = bot->GetMap()->GetHeight(x, y, spine->GetPositionZ() + 5.0f);
    if (z <= INVALID_HEIGHT)
        z = spine->GetPositionZ();

    return MoveToPoint(WorldPosition(bot->GetMapId(), x, y, z));
}

bool TbcRaidUseSpineAction::isUseful()
{
    if (Unit* najentus = TbcRaidHelper::GetShieldedNajentus(ai))
        return bot->IsWithinDistInMap(najentus, 30.0f);

    GameObject* spine = TbcRaidHelper::GetSpineToRemove(ai);
    return spine && bot->IsWithinDistInMap(spine, INTERACTION_DISTANCE);
}

bool TbcRaidUseSpineAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    // Throw the spine at Naj'entus to pop the Tidal Shield
    if (Unit* najentus = TbcRaidHelper::GetShieldedNajentus(ai))
    {
        if (bot->IsWithinDistInMap(najentus, 30.0f))
            return UseItem(requester, ITEM_NAJENTUS_SPINE, najentus);

        return false;
    }

    // Pull the spine out of an impaled player
    if (GameObject* spine = TbcRaidHelper::GetSpineToRemove(ai))
    {
        if (bot->IsWithinDistInMap(spine, INTERACTION_DISTANCE))
        {
            if (bot->IsMoving())
                bot->StopMoving();

            spine->Use(bot);
            return true;
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
// Priority target
// ---------------------------------------------------------------------------

bool TbcRaidAttackPriorityTargetAction::isUseful()
{
    Unit* target = TbcRaidHelper::GetPriorityTarget(ai);
    return target && AI_VALUE(ObjectGuid, "current target") != target->GetObjectGuid();
}

bool TbcRaidAttackPriorityTargetAction::Execute(Event& event)
{
    Unit* target = TbcRaidHelper::GetPriorityTarget(ai);
    if (!target || !target->IsInWorld() || target->GetMapId() != bot->GetMapId())
        return false;

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    return Attack(requester, target);
}

// ---------------------------------------------------------------------------
// Taunt
// ---------------------------------------------------------------------------

bool TbcRaidTauntAction::isUseful()
{
    return TbcRaidHelper::GetTauntTarget(ai) != nullptr;
}

bool TbcRaidTauntAction::Execute(Event& event)
{
    Unit* boss = TbcRaidHelper::GetTauntTarget(ai);
    if (!boss)
        return false;

    std::string spell;
    Unit* target = boss;
    switch (bot->getClass())
    {
    case CLASS_WARRIOR:
        spell = "taunt";
        break;
    case CLASS_DRUID:
        spell = "growl";
        break;
    case CLASS_PALADIN:
        // Righteous Defense is cast on the friendly target of the boss
        spell = "righteous defense";
        target = boss->GetVictim();
        break;
    default:
        return false;
    }

    if (!target || !ai->CanCastSpell(spell, target, 0, nullptr, false))
        return false;

    if (!ai->CastSpell(spell, target))
        return false;

    // Keep tanking the boss after the taunt
    SET_AI_VALUE(ObjectGuid, "current target", boss->GetObjectGuid());
    bot->SetSelectionGuid(boss->GetObjectGuid());
    return true;
}
