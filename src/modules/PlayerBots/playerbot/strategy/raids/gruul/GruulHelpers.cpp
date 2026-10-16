// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "GruulHelpers.h"
#include "playerbot/AiFactory.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include <algorithm>
#include <cmath>
#include <limits>

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

bool IsMaulgarTank(Player* bot)
{
    return PlayerbotAI::IsTank(bot) && raidboss::IsMainTank(bot);
}

bool IsOlmTank(Player* bot)
{
    // Only living tanks are counted, so a death swaps the Blindeye tank to Olm. This is intended
    // since Blindeye dies first and Olm is more important to control anyway.
    return raidboss::IsAssistTank(bot, 0);
}

bool IsBlindeyeTank(Player* bot)
{
    return raidboss::IsAssistTank(bot, 1);
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

            if (balanceOnly && AiFactory::GetPlayerSpecTab(member) != int(DRUID_TAB_BALANCE))
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
    return bot->HasAura(SPELL_GROUND_SLAM_1) || bot->HasAura(SPELL_GROUND_SLAM_2);
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
