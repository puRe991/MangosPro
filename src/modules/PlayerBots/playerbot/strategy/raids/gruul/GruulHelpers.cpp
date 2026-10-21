// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "GruulHelpers.h"
#include "playerbot/AiFactory.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include "MotionGenerators/PathFinder.h"

using namespace ai;

namespace
{
    const uint32 DRUID_TAB_BALANCE = 0;
}

namespace ai
{
namespace gruul
{

// High King Maulgar

Unit* FindMaulgar(PlayerbotAI* ai) { return raidboss::FindUnit(ai, NPC_HIGH_KING_MAULGAR); }
Unit* FindKrosh(PlayerbotAI* ai) { return raidboss::FindUnit(ai, NPC_KROSH_FIREHAND); }
Unit* FindOlm(PlayerbotAI* ai) { return raidboss::FindUnit(ai, NPC_OLM_THE_SUMMONER); }
Unit* FindKiggler(PlayerbotAI* ai) { return raidboss::FindUnit(ai, NPC_KIGGLER_THE_CRAZED); }
Unit* FindBlindeye(PlayerbotAI* ai) { return raidboss::FindUnit(ai, NPC_BLINDEYE_THE_SEER); }
Unit* FindGruul(PlayerbotAI* ai) { return raidboss::FindUnit(ai, NPC_GRUUL_THE_DRAGONKILLER); }

bool IsGruulsLairEncounterInProgress(Player* bot)
{
    // An empty entry list treats any trash attacker as an encounter. Nearby,
    // unpulled council members must not take over targeting or suppress DPS.
    static const std::vector<uint32> bosses = { NPC_HIGH_KING_MAULGAR, NPC_KROSH_FIREHAND,
        NPC_OLM_THE_SUMMONER, NPC_KIGGLER_THE_CRAZED, NPC_BLINDEYE_THE_SEER, NPC_GRUUL_THE_DRAGONKILLER };
    return raidboss::IsEncounterInProgress(bot, GRUUL_MAP_ID, bosses);
}

bool IsMaulgarTank(Player* bot)
{
    return PlayerbotAI::IsTank(bot) && raidboss::IsMainTank(bot);
}

bool IsCouncilMember(Unit* unit)
{
    if (!unit || unit->GetMapId() != GRUUL_MAP_ID)
        return false;
    const uint32 entry = unit->GetEntry();
    return entry == NPC_HIGH_KING_MAULGAR || entry == NPC_KROSH_FIREHAND ||
        entry == NPC_KIGGLER_THE_CRAZED || entry == NPC_OLM_THE_SUMMONER || entry == NPC_BLINDEYE_THE_SEER;
}

Unit* GetCasterRoleTarget(PlayerbotAI* ai)
{
    if (PlayerbotAI::IsTank(ai->GetBot()))
        return nullptr;
    if (IsKroshMageTank(ai))
        return FindKrosh(ai);
    if (IsKigglerMoonkinTank(ai))
        return FindKiggler(ai);
    return nullptr;
}

Unit* GetPrepareTarget(PlayerbotAI* ai, float& range)
{
    Player* bot = ai->GetBot();
    Player* master = ai->GetMaster();
    if (bot->GetMapId() != GRUUL_MAP_ID || !bot->IsAlive() || bot->IsInCombat() ||
        !master || master->GetMap() != bot->GetMap() || !master->IsAlive() || master->IsInCombat() ||
        master->IsMoving() || !bot->IsWithinDistInMap(master, 60.0f) ||
        !ai->HasStrategy("boss gruul", BotState::BOT_STATE_NON_COMBAT) ||
        ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) ||
        ai->HasStrategy("passive", BotState::BOT_STATE_NON_COMBAT) ||
        IsGruulsLairEncounterInProgress(bot))
        return nullptr;

    // Selecting a council member arms preparation; moving away or clearing selection releases it.
    Unit* selected = ai->GetUnit(master->GetSelectionGuid());
    if (!IsCouncilMember(selected) || !selected->IsAlive() || !master->IsWithinDistInMap(selected, 60.0f))
        return nullptr;
    if (Group* group = bot->GetGroup())
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->getSource())
                if (member->GetMap() == bot->GetMap() && member->IsInCombat())
                    return nullptr;

    Unit* target = GetCasterRoleTarget(ai);
    if (!target || !target->IsAlive() || !bot->IsWithinDistInMap(target, 75.0f))
        return nullptr;
    if (!ai->GetSpellRange(target->GetEntry() == NPC_KROSH_FIREHAND ? "frostbolt" : "wrath", &range))
        return nullptr;
    if (target->GetEntry() == NPC_KROSH_FIREHAND)
    {
        float stealRange = 0;
        if (!ai->GetSpellRange("spellsteal", &stealRange))
            return nullptr;
        range = std::min(range, stealRange);
    }
    range -= 1.5f; // Keep some room inside actual spell range.
    return range > target->GetAttackDistance(bot) + 3.0f ? target : nullptr;
}

namespace
{
    bool SafePreparationSegment(Player* bot, const std::vector<Unit*>& council,
        float ax, float ay, float bx, float by)
    {
        const float dx = bx - ax, dy = by - ay;
        const float length2 = dx * dx + dy * dy;
        for (Unit* member : council)
        {
            if (!member || !member->IsAlive() || member->IsInCombat())
                return false;
            const float t = length2 > 0.001f ? std::max(0.0f, std::min(1.0f,
                ((member->GetPositionX() - ax) * dx + (member->GetPositionY() - ay) * dy) / length2)) : 0.0f;
            if (std::hypot(member->GetPositionX() - ax - t * dx, member->GetPositionY() - ay - t * dy) <
                member->GetAttackDistance(bot) + 3.0f)
                return false;
        }
        return true;
    }
}

bool IsPrepared(Player* bot, Unit* target, float range)
{
    return target && raidboss::ExactDist2d(bot, target) <= range &&
        raidboss::ExactDist2d(bot, target) > target->GetAttackDistance(bot) + 3.0f &&
        bot->IsWithinLOSInMap(target);
}

bool GetPreparePosition(PlayerbotAI* ai, Unit* target, float range, float& x, float& y, float& z)
{
    Player* bot = ai->GetBot();
    const std::vector<Unit*> council = { FindMaulgar(ai), FindKrosh(ai), FindOlm(ai), FindKiggler(ai), FindBlindeye(ai) };
    const float angle = std::atan2(bot->GetPositionY() - target->GetPositionY(), bot->GetPositionX() - target->GetPositionX());
    const float offsets[] = { 0, 0.2f, -0.2f, 0.4f, -0.4f, 0.6f, -0.6f, 0.8f, -0.8f };
    for (float offset : offsets)
    {
        x = target->GetPositionX() + std::cos(angle + offset) * (range - 0.5f);
        y = target->GetPositionY() + std::sin(angle + offset) * (range - 0.5f);
        z = target->GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, z);
        if (!target->IsWithinLOS(x, y, z + bot->GetCollisionHeight(), true))
            continue;
        PathFinder path(bot);
        path.calculate(x, y, z, false);
        if (path.getPathType() != PATHFIND_NORMAL || path.getPath().size() < 2)
            continue;
        const auto& points = path.getPath();
        bool safe = SafePreparationSegment(bot, council, bot->GetPositionX(), bot->GetPositionY(), points.front().x, points.front().y);
        for (size_t i = 1; safe && i < points.size(); ++i)
            safe = SafePreparationSegment(bot, council, points[i - 1].x, points[i - 1].y, points[i].x, points[i].y);
        if (safe && std::hypot(points.back().x - x, points.back().y - y) < 1.0f)
            return true;
    }
    return false;
}

bool IsOlmTank(Player* bot)
{
    // Only living tanks are counted, so a death swaps the Blindeye tank to Olm. This is intended
    // since Blindeye dies first and Olm is more important to control anyway.
    // With only two tanks a melee dps stand-in takes Blindeye (or Olm too with a single tank).
    return raidboss::IsAssistTankOrStandIn(bot, 0);
}

bool IsBlindeyeTank(Player* bot)
{
    return raidboss::IsAssistTankOrStandIn(bot, 1);
}

namespace
{
    // An assistant of the class (player or bot) is returned immediately, otherwise the bot of the
    // class with the highest health.
    ObjectGuid FindCasterTank(Player* bot, uint8 classId, bool balanceOnly)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return ObjectGuid();

        Player* highestHpBot = nullptr;
        uint32 highestHp = 0;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->getSource();
            if (!member || !member->IsAlive() || !member->IsInWorld() || member->GetMapId() != GRUUL_MAP_ID ||
                member->getClass() != classId)
            {
                continue;
            }

            if (balanceOnly && (AiFactory::GetPlayerSpecTab(member) != int(DRUID_TAB_BALANCE) || PlayerbotAI::IsHeal(member)))
                continue;

            if (group->IsAssistant(member->GetObjectGuid()))
                return member->GetObjectGuid();

            if (!member->GetPlayerbotAI())
                continue;

            const uint32 hp = member->GetMaxHealth();
            if (!highestHpBot || hp > highestHp)
            {
                highestHpBot = member;
                highestHp = hp;
            }
        }

        return highestHpBot ? highestHpBot->GetObjectGuid() : ObjectGuid();
    }

    Player* GetPlayerFromValue(PlayerbotAI* ai, const char* valueName)
    {
        const ObjectGuid guid = ai->GetAiObjectContext()->GetValue<ObjectGuid>(valueName)->Get();
        if (guid.IsEmpty())
            return nullptr;

        Player* player = sObjectMgr.GetPlayer(guid);
        return player && player->IsInWorld() && player->GetMapId() == ai->GetBot()->GetMapId() ? player : nullptr;
    }
}

ObjectGuid FindKroshMageTankGuid(Player* bot)
{
    return FindCasterTank(bot, CLASS_MAGE, false);
}

Player* GetKroshMageTank(PlayerbotAI* ai)
{
    return GetPlayerFromValue(ai, "high king maulgar krosh mage tank");
}

bool IsKroshMageTank(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    return bot->getClass() == CLASS_MAGE && GetKroshMageTank(ai) == bot;
}

ObjectGuid FindKigglerMoonkinTankGuid(Player* bot)
{
    return FindCasterTank(bot, CLASS_DRUID, true);
}

Player* GetKigglerMoonkinTank(PlayerbotAI* ai)
{
    return GetPlayerFromValue(ai, "high king maulgar kiggler moonkin tank");
}

bool IsKigglerMoonkinTank(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    return bot->getClass() == CLASS_DRUID && GetKigglerMoonkinTank(ai) == bot;
}

std::list<ObjectGuid> FindNearbyWildFelStalkerGuids(Player* bot)
{
    std::list<ObjectGuid> guids;
    if (bot->GetMapId() != GRUUL_MAP_ID)
        return guids;

    for (Unit* felStalker : raidboss::FindUnits(bot, NPC_WILD_FEL_STALKER, WILD_FEL_STALKER_SEARCH_RADIUS))
        guids.push_back(felStalker->GetObjectGuid());

    // Ordered by guid so that every bot warlock indexes the same list
    guids.sort([](const ObjectGuid& lhs, const ObjectGuid& rhs) { return lhs.GetCounter() < rhs.GetCounter(); });
    return guids;
}

std::vector<Unit*> GetNearbyWildFelStalkers(PlayerbotAI* ai)
{
    std::vector<Unit*> felStalkers;
    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("high king maulgar wild fel stalkers")->Get())
    {
        Unit* felStalker = ai->GetUnit(guid);
        if (felStalker && felStalker->IsAlive())
            felStalkers.push_back(felStalker);
    }

    return felStalkers;
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

// Gruul the Dragonkiller

bool GetNearestCaveInPosition(Player* bot, WorldPosition& pool)
{
    bool found = false;
    float nearestDistance = std::numeric_limits<float>::max();
    for (const WorldPosition& position : raidboss::GetDynamicObjectPositions(bot, CAVE_IN_SEARCH_RADIUS, SPELL_CAVE_IN))
    {
        const float distance = raidboss::ExactDist2d(bot, position.getX(), position.getY());
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            pool = position;
            found = true;
        }
    }

    return found;
}

bool IsNearCaveIn(Player* bot, float radius)
{
    WorldPosition pool;
    return GetNearestCaveInPosition(bot, pool) && raidboss::ExactDist2d(bot, pool.getX(), pool.getY()) < radius;
}

bool IsInCaveIn(Player* bot)
{
    return IsNearCaveIn(bot, CAVE_IN_RADIUS);
}

bool HasGroundSlam(Player* bot)
{
    // The Ground Slam aura expires before Stoned/Shatter. Keep normal chasing
    // suppressed until the actual Shatter removes Grasp and Stoned as well.
    return bot->HasAura(SPELL_GROUND_SLAM_1) || bot->HasAura(SPELL_GROUND_SLAM_2) ||
        bot->HasAura(SPELL_GRONN_LORDS_GRASP) || bot->HasAura(SPELL_STONED);
}

bool GetShatterSpreadPosition(Player* bot, float& x, float& y, float& z)
{
    if (!HasGroundSlam(bot) || bot->HasAura(SPELL_STONED) || bot->GetSpeed(MOVE_RUN) <= 0.0f)
        return false;

    const std::vector<Player*> members = raidboss::GetGroupMembers(bot, 60.0f, false);
    // Moving away from only the nearest player can move straight into the next
    // player and reverse direction every tick. Minimize exposure to the whole group.
    const auto exposure = [&members](float px, float py)
    {
        float score = 0.0f;
        for (Player* member : members)
        {
            const float distance = raidboss::ExactDist2d(member, px, py);
            const float overlap = std::max(0.0f, GRUUL_SHATTER_SAFE_DISTANCE - distance);
            score += overlap * overlap;
        }
        return score;
    };

    float bestScore = exposure(bot->GetPositionX(), bot->GetPositionY());
    bool found = false;
    const std::vector<WorldPosition> pools = raidboss::GetDynamicObjectPositions(bot, CAVE_IN_SEARCH_RADIUS, SPELL_CAVE_IN);
    for (unsigned i = 0; i < 16; ++i)
    {
        const float angle = bot->GetOrientation() + i * M_PI_F / 8.0f;
        const float px = bot->GetPositionX() + std::cos(angle) * 3.5f;
        const float py = bot->GetPositionY() + std::sin(angle) * 3.5f;
        const float score = exposure(px, py);
        if (score >= bestScore - 0.01f)
            continue;

        bool inPool = false;
        for (const WorldPosition& pool : pools)
            if (std::hypot(px - pool.getX(), py - pool.getY()) < CAVE_IN_RADIUS)
                inPool = true;
        if (inPool)
            continue;

        float candidateX, candidateY, candidateZ;
        if (!raidboss::CanTakeStepTowards(bot, px, py, 3.5f, candidateX, candidateY, candidateZ))
            continue;

        bestScore = score;
        x = candidateX;
        y = candidateY;
        z = candidateZ;
        found = true;
    }
    return found;
}

bool GetPointAwayFrom(Player* bot, float centerX, float centerY, float safeDistance, float& x, float& y, float& z)
{
    const float dx = bot->GetPositionX() - centerX;
    const float dy = bot->GetPositionY() - centerY;
    const float baseAngle = (std::fabs(dx) < 0.01f && std::fabs(dy) < 0.01f) ? bot->GetOrientation() + M_PI_F : std::atan2(dy, dx);

    // Straight away first, then fan out to both sides when a wall blocks the way
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

}
}
