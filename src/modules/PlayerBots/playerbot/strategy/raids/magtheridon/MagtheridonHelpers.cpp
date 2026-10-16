// Ported from mod-playerbots (src/Ai/Raid/Mag, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "MagtheridonHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
#include <unordered_map>

using namespace ai;

namespace ai
{
namespace magtheridon
{

namespace
{
    struct EncounterState
    {
        bool hasBlastNovaTimer = false;
        uint32 blastNovaTime = 0;
        bool hasDpsWaitTimer = false;
        uint32 dpsWaitTime = 0;
        bool lastBlastNovaState = false;
        bool lastQuakeState = false;
        bool ceilingCollapsed = false;
        std::map<ObjectGuid, CubeInfo> cubeAssignments;
    };

    std::mutex stateMutex;
    std::unordered_map<uint32, EncounterState> encounterStates;

    bool IsQuaking(Unit* magtheridon)
    {
        // The boss script stuns Magtheridon for 7s when he casts Quake
        return magtheridon->HasAura(SPELL_QUAKE) || magtheridon->FindCurrentSpellBySpellId(SPELL_QUAKE) ||
               (magtheridon->hasUnitState(UNIT_STAT_STUNNED) && !magtheridon->HasAura(SPELL_SHADOW_CAGE));
    }

    // A clicker who has Mind Exhaustion and does not hold the cube cannot click on the next Blast Nova
    bool CanClickCube(Player* member)
    {
        return member->HasAura(SPELL_SHADOW_GRASP) || !member->HasAura(SPELL_MIND_EXHAUSTION);
    }

    bool IsAssigned(const std::map<ObjectGuid, CubeInfo>& assignment, Player* member)
    {
        return assignment.find(member->GetObjectGuid()) != assignment.end();
    }
}

Unit* FindMagtheridon(PlayerbotAI* ai)
{
    return raidboss::FindUnit(ai, NPC_MAGTHERIDON);
}

bool IsMagtheridonActive(Unit* magtheridon)
{
    return magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE_DUMMY);
}

bool IsBlastNovaCasting(Unit* magtheridon)
{
    return magtheridon && magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA);
}

ChannelerSlot GetChannelerSlot(Creature* channeler)
{
    float x, y, z;
    channeler->GetRespawnCoord(x, y, z);
    const float angle = std::atan2(y - ROOM_CENTER_Y, x - ROOM_CENTER_X) * 180.0f / M_PI_F;
    const float absAngle = std::fabs(angle);
    if (absAngle <= 72.0f)
        return angle >= 0.0f ? CHANNELER_NORTHWEST : CHANNELER_NORTHEAST;

    if (absAngle >= 144.0f)
        return CHANNELER_SOUTH;

    return angle > 0.0f ? CHANNELER_WEST : CHANNELER_EAST;
}

std::list<ObjectGuid> FindChannelerGuids(Player* bot)
{
    std::list<ObjectGuid> guids;
    for (Unit* channeler : raidboss::FindUnits(bot, NPC_HELLFIRE_CHANNELER, 150.0f))
        guids.push_back(channeler->GetObjectGuid());

    return guids;
}

Unit* GetChanneler(PlayerbotAI* ai, ChannelerSlot slot)
{
    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("magtheridon hellfire channelers")->Get())
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetTypeId() != TYPEID_UNIT)
            continue;

        if (GetChannelerSlot(static_cast<Creature*>(unit)) == slot)
            return unit;
    }

    return nullptr;
}

std::list<ObjectGuid> FindBurningAbyssalGuids(Player* bot)
{
    std::list<ObjectGuid> guids;
    for (Unit* abyssal : raidboss::FindUnits(bot, NPC_BURNING_ABYSSAL, 100.0f))
        guids.push_back(abyssal->GetObjectGuid());

    guids.sort([](const ObjectGuid& lhs, const ObjectGuid& rhs) { return lhs.GetCounter() < rhs.GetCounter(); });
    return guids;
}

std::vector<Unit*> GetBurningAbyssals(PlayerbotAI* ai)
{
    std::vector<Unit*> abyssals;
    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("magtheridon burning abyssals")->Get())
    {
        Unit* abyssal = ai->GetUnit(guid);
        if (abyssal && abyssal->IsAlive())
            abyssals.push_back(abyssal);
    }

    return abyssals;
}

std::vector<Player*> GetGroupBotsOfClass(Player* bot, uint8 classId)
{
    std::vector<Player*> result;
    Group* group = bot->GetGroup();
    if (!group)
        return result;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (member && member->IsAlive() && member->IsInWorld() && member->GetMapId() == bot->GetMapId() &&
            member->getClass() == classId && member->GetPlayerbotAI())
        {
            result.push_back(member);
        }
    }

    return result;
}

std::vector<CubeInfo> FindCubes(Player* bot)
{
    std::vector<GameObject*> objects = raidboss::FindGameObjects(bot, GO_MANTICRON_CUBE, 150.0f);
    std::sort(objects.begin(), objects.end(), [](GameObject* a, GameObject* b) { return a->GetObjectGuid().GetCounter() < b->GetObjectGuid().GetCounter(); });

    std::vector<CubeInfo> cubes;
    for (GameObject* go : objects)
    {
        CubeInfo info;
        info.guid = go->GetObjectGuid();
        info.x = go->GetPositionX();
        info.y = go->GetPositionY();
        info.z = go->GetPositionZ();
        cubes.push_back(info);
    }

    return cubes;
}

std::vector<WorldPosition> FindDebrisPositions(Player* bot)
{
    return raidboss::GetDynamicObjectPositions(bot, 150.0f, SPELL_DEBRIS_MARKER);
}

bool IsPositionInDebris(const std::vector<WorldPosition>& debris, float x, float y, float radius)
{
    for (const WorldPosition& position : debris)
    {
        const float dx = position.getX() - x;
        const float dy = position.getY() - y;
        if (std::sqrt(dx * dx + dy * dy) <= radius)
            return true;
    }

    return false;
}

bool IsPositionInActiveDebris(Player* bot, float x, float y, float radius)
{
    return IsPositionInDebris(FindDebrisPositions(bot), x, y, radius);
}

std::vector<GameObject*> GetActiveConflagrations(Player* bot)
{
    return raidboss::FindGameObjects(bot, GO_BLAZE, 80.0f);
}

bool IsPositionInConflagration(const std::vector<GameObject*>& blazes, float x, float y)
{
    for (GameObject* blaze : blazes)
    {
        if (raidboss::ExactDist2d(blaze, x, y) < CONFLAGRATION_HAZARD_RADIUS)
            return true;
    }

    return false;
}

bool IsPositionInActiveConflagration(Player* bot, float x, float y)
{
    return IsPositionInConflagration(GetActiveConflagrations(bot), x, y);
}

bool GetPointAwayFrom(Player* bot, float centerX, float centerY, float safeDistance, float& x, float& y, float& z)
{
    const float dx = bot->GetPositionX() - centerX;
    const float dy = bot->GetPositionY() - centerY;
    const float baseAngle = (std::fabs(dx) < 0.01f && std::fabs(dy) < 0.01f) ? bot->GetOrientation() + M_PI_F : std::atan2(dy, dx);

    const float offsets[] = { 0.0f, M_PI_F / 6, -M_PI_F / 6, M_PI_F / 3, -M_PI_F / 3, M_PI_F / 2, -M_PI_F / 2,
                              2 * M_PI_F / 3, -2 * M_PI_F / 3 };
    for (float offset : offsets)
    {
        const float angle = baseAngle + offset;
        const float pointX = centerX + std::cos(angle) * (safeDistance + 0.5f);
        const float pointY = centerY + std::sin(angle) * (safeDistance + 0.5f);
        const float distance = raidboss::ExactDist2d(bot, pointX, pointY);
        if (distance < 0.5f)
            continue;

        if (raidboss::CanTakeStepTowards(bot, pointX, pointY, distance, x, y, z))
            return true;
    }

    return false;
}

// Shared encounter state

bool GetBlastNovaTime(uint32 instanceId, uint32& time)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    auto it = encounterStates.find(instanceId);
    if (it == encounterStates.end() || !it->second.hasBlastNovaTimer)
        return false;

    time = it->second.blastNovaTime;
    return true;
}

bool GetDpsWaitTime(uint32 instanceId, uint32& time)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    auto it = encounterStates.find(instanceId);
    if (it == encounterStates.end() || !it->second.hasDpsWaitTimer)
        return false;

    time = it->second.dpsWaitTime;
    return true;
}

bool IsCeilingCollapsed(Player* bot)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    auto it = encounterStates.find(bot->GetInstanceId());
    return it != encounterStates.end() && it->second.ceilingCollapsed;
}

bool IsCubeClicker(Player* bot)
{
    CubeInfo cube;
    return GetAssignedCube(bot, cube);
}

bool GetAssignedCube(Player* bot, CubeInfo& cube)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    auto it = encounterStates.find(bot->GetInstanceId());
    if (it == encounterStates.end())
        return false;

    auto assigned = it->second.cubeAssignments.find(bot->GetObjectGuid());
    if (assigned == it->second.cubeAssignments.end())
        return false;

    cube = assigned->second;
    return true;
}

bool IsBlastNovaSoon(Player* bot)
{
    uint32 time = 0;
    return GetBlastNovaTime(bot->GetInstanceId(), time) &&
           WorldTimer::getMSTimeDiff(time, WorldTimer::getMSTime()) >= BLAST_NOVA_INTERIM_MS;
}

bool ResetEncounterState(uint32 instanceId)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    return encounterStates.erase(instanceId) > 0;
}

bool UpdateEncounterState(Player* tracker, Unit* magtheridon)
{
    if (!IsMagtheridonActive(magtheridon))
        return false;

    // Everything read from the world is collected before taking the lock
    const uint32 now = WorldTimer::getMSTime();
    const bool isCasting = IsBlastNovaCasting(magtheridon);
    const bool isQuaking = IsQuaking(magtheridon);
    const bool belowCollapseHealth = magtheridon->GetHealthPercent() < CEILING_COLLAPSE_HP_PCT;
    Unit* victim = magtheridon->GetVictim();
    Player* mainTank = raidboss::GetGroupMainTank(tracker);
    const std::vector<CubeInfo> cubes = FindCubes(tracker);
    const std::vector<Player*> members = raidboss::GetGroupMembers(tracker, 500.0f, true);

    std::lock_guard<std::mutex> lock(stateMutex);
    EncounterState& state = encounterStates[magtheridon->GetInstanceId()];
    bool updated = false;

    if (isCasting && !state.lastBlastNovaState)
    {
        state.blastNovaTime = now;
        state.hasBlastNovaTimer = true;
    }
    state.lastBlastNovaState = isCasting;

    // Replaces the Quake spell listener of mod-playerbots: each Quake delays Blast Nova
    if (isQuaking && !state.lastQuakeState && state.hasBlastNovaTimer)
        state.blastNovaTime += QUAKE_DELAY_MS;
    state.lastQuakeState = isQuaking;

    if (!state.hasBlastNovaTimer)
    {
        state.blastNovaTime = now;
        state.hasBlastNovaTimer = true;
        updated = true;
    }

    if (!state.hasDpsWaitTimer)
    {
        state.dpsWaitTime = now;
        state.hasDpsWaitTimer = true;
        updated = true;
    }

    if (belowCollapseHealth && !state.ceilingCollapsed)
    {
        state.blastNovaTime += CEILING_COLLAPSE_DELAY_MS;
        state.ceilingCollapsed = true;
        updated = true;
    }

    // Cube assignment
    auto& assignment = state.cubeAssignments;
    if (cubes.empty())
    {
        updated |= !assignment.empty();
        assignment.clear();
        return updated;
    }

    // Prune dead, absent and exhausted clickers (this core: Mind Exhaustion blocks the next click,
    // so the next Blast Nova needs a fresh set of clickers)
    for (auto it = assignment.begin(); it != assignment.end();)
    {
        Player* clicker = nullptr;
        for (Player* member : members)
        {
            if (member->GetObjectGuid() == it->first)
            {
                clicker = member;
                break;
            }
        }

        if (!clicker || !CanClickCube(clicker))
        {
            it = assignment.erase(it);
            updated = true;
        }
        else
            ++it;
    }

    // Fill unassigned cubes
    for (const CubeInfo& cube : cubes)
    {
        bool alreadyAssigned = false;
        for (const auto& pair : assignment)
        {
            if (pair.second.guid == cube.guid)
            {
                alreadyAssigned = true;
                break;
            }
        }

        if (alreadyAssigned)
            continue;

        Player* candidate = nullptr;

        // Pass 1: ranged dps bots, excluding warlocks (they handle the Burning Abyssals)
        for (Player* member : members)
        {
            if (member->GetPlayerbotAI() && member->getClass() != CLASS_WARLOCK && raidboss::IsDps(member) &&
                raidboss::IsRangedPlayer(member) && CanClickCube(member) && !IsAssigned(assignment, member))
            {
                candidate = member;
                break;
            }
        }

        // Pass 2: any bot other than the main tank or whoever Magtheridon is attacking
        if (!candidate)
        {
            for (Player* member : members)
            {
                if (member->GetPlayerbotAI() && member != victim && member != mainTank && CanClickCube(member) &&
                    !IsAssigned(assignment, member))
                {
                    candidate = member;
                    break;
                }
            }
        }

        if (candidate)
        {
            assignment[candidate->GetObjectGuid()] = cube;
            updated = true;
        }
    }

    return updated;
}

}
}
