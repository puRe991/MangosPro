// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/Hyjal/Util/HyjalHelpers) and adapted for this core.
#include "playerbot/playerbot.h"
#include "HyjalHelpers.h"
#include "HyjalValues.h"
#include "playerbot/strategy/Action.h"
#include "playerbot/strategy/actions/AttackAction.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "Maps/MapManager.h"
#include "Spells/Spell.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace ai;
using namespace ai::raidboss;

namespace
{
    const std::vector<uint32> HYJAL_BOSSES = {
        hyjal::NPC_RAGE_WINTERCHILL, hyjal::NPC_ANETHERON, hyjal::NPC_KAZROGAL, hyjal::NPC_AZGALOR, hyjal::NPC_ARCHIMONDE };

    std::vector<WorldPosition> GetCachedHazardPositions(PlayerbotAI* ai, const char* value)
    {
        return ai->GetAiObjectContext()->GetValue<std::vector<WorldPosition>>(value)->Get();
    }

    float Dist2d(const WorldPosition& a, float x, float y)
    {
        const float dx = a.getX() - x;
        const float dy = a.getY() - y;
        return std::sqrt(dx * dx + dy * dy);
    }

    float OffsetFrom(float angle, float from)
    {
        float offset = MapManager::NormalizeOrientation(angle - from);
        if (offset > M_PI_F)
            offset -= 2.0f * M_PI_F;

        return offset;
    }

    // The span of a ring that a circular ground hazard covers
    bool GetHazardBlockedArc(const WorldPosition& ringCenter, float ringRadius, const WorldPosition& hazard, float hazardRadius,
                             hyjal::BlockedArc& arc)
    {
        const float centerToHazard = Dist2d(hazard, ringCenter.getX(), ringCenter.getY());
        if (centerToHazard <= 0.0f)
        {
            arc = { 0.0f, M_PI_F };
            return ringRadius < hazardRadius;
        }

        const float cosHalfWidth = (centerToHazard * centerToHazard + ringRadius * ringRadius - hazardRadius * hazardRadius) /
                                   (2.0f * centerToHazard * ringRadius);
        if (cosHalfWidth >= 1.0f)
            return false;

        arc.center = std::atan2(hazard.getY() - ringCenter.getY(), hazard.getX() - ringCenter.getX());
        arc.halfWidth = (cosHalfWidth <= -1.0f) ? M_PI_F : std::acos(cosHalfWidth);
        return true;
    }

    // The angle nearest to the preferred one that clears every blocked arc
    bool FindNearestUnblockedAngle(const std::vector<hyjal::BlockedArc>& blocked, float preferred, float& unblocked)
    {
        auto isUnblocked = [&blocked](float angle)
        {
            for (const hyjal::BlockedArc& arc : blocked)
            {
                if (std::fabs(OffsetFrom(angle, arc.center)) < arc.halfWidth)
                    return false;
            }

            return true;
        };

        if (isUnblocked(preferred))
        {
            unblocked = preferred;
            return true;
        }

        const float edgeNudge = 0.01f;
        bool found = false;
        float bestOffset = std::numeric_limits<float>::max();
        for (const hyjal::BlockedArc& arc : blocked)
        {
            for (int side = -1; side <= 1; side += 2)
            {
                const float edge = arc.center + side * (arc.halfWidth + edgeNudge);
                if (!isUnblocked(edge))
                    continue;

                const float offset = OffsetFrom(edge, preferred);
                if (std::fabs(offset) < std::fabs(bestOffset))
                {
                    bestOffset = offset;
                    unblocked = edge;
                    found = true;
                }
            }
        }

        return found;
    }

    // Step straight out of a hazard
    bool GetHazardEscapeStep(Player* bot, const WorldPosition& hazard, float escapeRadius, float moveDist,
                             float& stepX, float& stepY, float& stepZ, const std::function<bool(float, float)>& isAcceptable)
    {
        float escapeAngle = std::atan2(bot->GetPositionY() - hazard.getY(), bot->GetPositionX() - hazard.getX());
        if (ExactDist2d(bot, hazard.getX(), hazard.getY()) <= 0.1f)
            escapeAngle = bot->GetOrientation();

        return hyjal::FindStepToCircle(bot, hazard, escapeRadius, escapeAngle, moveDist, stepX, stepY, stepZ, isAcceptable, nullptr, nullptr);
    }

    bool NameIn(const std::string& name, std::initializer_list<const char*> names)
    {
        for (const char* n : names)
        {
            if (name == n)
                return true;
        }

        return false;
    }
}

// General

bool hyjal::IsHyjalEncounterInProgress(Player* bot)
{
    return IsEncounterInProgress(bot, HYJAL_MAP_ID, HYJAL_BOSSES);
}

Unit* hyjal::FindEngagedBoss(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    if (!IsHyjalEncounterInProgress(bot))
        return nullptr;

    for (uint32 entry : HYJAL_BOSSES)
    {
        if (Unit* boss = FindUnit(ai, entry))
            return boss;
    }

    return nullptr;
}

Unit* hyjal::GetAnyBoss(PlayerbotAI* ai)
{
    Unit* boss = ai->GetUnit(ai->GetAiObjectContext()->GetValue<ObjectGuid>("hyjal boss")->Get());
    return boss && boss->IsAlive() ? boss : nullptr;
}

Unit* hyjal::GetBoss(PlayerbotAI* ai, uint32 entry)
{
    Unit* boss = GetAnyBoss(ai);
    return boss && boss->GetEntry() == entry ? boss : nullptr;
}

Unit* hyjal::GetCurrentTarget(PlayerbotAI* ai)
{
    return ai->GetUnit(ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Get());
}

float hyjal::GetMeleeRange(Player* bot, Unit* target)
{
    return bot->GetCombinedCombatReach(target, true);
}

WorldPosition hyjal::ToPosition(const Point& point)
{
    return WorldPosition(HYJAL_MAP_ID, point.x, point.y, point.z);
}

bool hyjal::FindStepToCircle(Player* bot, const WorldPosition& center, float radius, float preferredAngle, float moveDist,
                             float& stepX, float& stepY, float& stepZ, const std::function<bool(float, float)>& isAcceptable,
                             float* chosenX, float* chosenY)
{
    const uint8 fanSteps = 8;
    const float fanStep = M_PI_F / fanSteps;
    for (uint8 step = 0; step <= fanSteps; ++step)
    {
        const float delta = fanStep * step;
        const uint8 candidates = (step == 0) ? 1 : 2;
        for (uint8 i = 0; i < candidates; ++i)
        {
            const float angle = preferredAngle + (i == 0 ? delta : -delta);
            const float targetX = center.getX() + std::cos(angle) * radius;
            const float targetY = center.getY() + std::sin(angle) * radius;

            if (isAcceptable && !isAcceptable(targetX, targetY))
                continue;

            if (!CanTakeStepTowards(bot, targetX, targetY, moveDist, stepX, stepY, stepZ))
                continue;

            if (isAcceptable && !isAcceptable(stepX, stepY))
                continue;

            if (chosenX)
                *chosenX = targetX;
            if (chosenY)
                *chosenY = targetY;

            return true;
        }
    }

    return false;
}

// Melee look for an open position within the melee range of the boss. If there is none, they take
// the shortest path out of the hazard and wait it out.
bool hyjal::GetMeleeHazardManeuverStep(Player* bot, Unit* boss, const std::vector<WorldPosition>& hazards, float hazardRadius,
                                       const std::vector<BlockedArc>& extraBlocked, float& stepX, float& stepY, float& stepZ,
                                       const std::function<bool(float, float)>& isAcceptable)
{
    const float moveDist = 10.0f;
    const float meleeRadius = std::max(1.0f, GetMeleeRange(bot, boss) - MELEE_RANGE_INSET);
    const WorldPosition bossPosition(boss);

    std::vector<BlockedArc> blocked;
    for (const WorldPosition& hazard : hazards)
    {
        BlockedArc hazardArc;
        if (GetHazardBlockedArc(bossPosition, meleeRadius, hazard, hazardRadius, hazardArc))
            blocked.push_back(hazardArc);
    }

    blocked.insert(blocked.end(), extraBlocked.begin(), extraBlocked.end());

    const float bossX = boss->GetPositionX();
    const float bossY = boss->GetPositionY();
    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();

    float standAngle;
    if (FindNearestUnblockedAngle(blocked, std::atan2(botY - bossY, botX - bossX), standAngle))
    {
        const float targetX = bossX + std::cos(standAngle) * meleeRadius;
        const float targetY = bossY + std::sin(standAngle) * meleeRadius;
        const float distToTarget = ExactDist2d(bot, targetX, targetY);

        const float minStepDistance = 0.5f;
        if (distToTarget < minStepDistance)
            return false;

        const float stepDist = std::min(moveDist, distToTarget);
        stepX = botX + ((targetX - botX) / distToTarget) * stepDist;
        stepY = botY + ((targetY - botY) / distToTarget) * stepDist;
        stepZ = bot->GetPositionZ();
        return true;
    }

    const WorldPosition* nearest = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    for (const WorldPosition& hazard : hazards)
    {
        const float distance = ExactDist2d(bot, hazard.getX(), hazard.getY());
        if (distance < nearestDistance)
        {
            nearest = &hazard;
            nearestDistance = distance;
        }
    }

    if (!nearest || nearestDistance >= hazardRadius)
        return false;

    const float escapeMargin = 2.0f;
    return GetHazardEscapeStep(bot, *nearest, hazardRadius + escapeMargin, moveDist, stepX, stepY, stepZ, isAcceptable);
}

std::vector<Player*> hyjal::GetRangedMembers(Player* bot)
{
    std::vector<Player*> members;
    for (Player* member : GetGroupMembers(bot, 500.0f, true))
    {
        if (member->GetMapId() == HYJAL_MAP_ID && member->GetPlayerbotAI() && IsRangedPlayer(member))
            members.push_back(member);
    }

    return members;
}

bool hyjal::GetRangedRingStep(Player* bot, const WorldPosition& center, float healerRadius, float dpsRadius,
                              float& stepX, float& stepY, float& stepZ, bool& reached)
{
    const bool isHealer = PlayerbotAI::IsHeal(bot);
    std::vector<Player*> sameRole;
    for (Player* member : GetRangedMembers(bot))
    {
        if (PlayerbotAI::IsHeal(member) == isHealer)
            sameRole.push_back(member);
    }

    const int index = GetIndexOf(bot, sameRole);
    const size_t count = sameRole.size();
    if (count == 0)
        return false;

    const float arcSpan = 2.0f * M_PI_F;
    const float arcCenter = 0.0f;
    const float arcStart = arcCenter - arcSpan / 2.0f;
    const float angle = (count == 1) ? arcCenter : (arcStart + arcSpan * float(std::max(index, 0)) / float(count));

    const float radius = isHealer ? healerRadius : dpsRadius;
    const float moveDist = 3.5f;
    float chosenX;
    float chosenY;
    if (!FindStepToCircle(bot, center, radius, angle, moveDist, stepX, stepY, stepZ, {}, &chosenX, &chosenY))
    {
        reached = true;
        return false;
    }

    const float arrivalDist = 2.0f;
    if (ExactDist2d(bot, chosenX, chosenY) <= arrivalDist)
    {
        reached = true;
        return false;
    }

    return true;
}

// Rage Winterchill

bool hyjal::GetDeathAndDecayPosition(PlayerbotAI* ai, WorldPosition& deathAndDecay)
{
    std::vector<WorldPosition> positions = GetCachedHazardPositions(ai, "hyjal death and decay");
    if (positions.empty())
        return false;

    deathAndDecay = positions.front();
    return true;
}

bool hyjal::IsNearDeathAndDecay(PlayerbotAI* ai, float radius)
{
    WorldPosition deathAndDecay;
    return GetDeathAndDecayPosition(ai, deathAndDecay) &&
           ExactDist2d(ai->GetBot(), deathAndDecay.getX(), deathAndDecay.getY()) < radius;
}

bool hyjal::IsInDeathAndDecay(PlayerbotAI* ai)
{
    return IsNearDeathAndDecay(ai, DEATH_AND_DECAY_RADIUS);
}

// Anetheron

Player* hyjal::GetInfernoTarget(Unit* anetheron)
{
    if (!anetheron)
        return nullptr;

    Spell* spell = anetheron->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || !spell->m_spellInfo || spell->m_spellInfo->Id != SPELL_INFERNO)
        return nullptr;

    Unit* target = spell->m_targets.getUnitTarget();
    return target && target->IsPlayer() ? static_cast<Player*>(target) : nullptr;
}

std::list<ObjectGuid> hyjal::FindInfernalGuids(Player* bot)
{
    std::vector<Unit*> alive = FindUnits(bot, NPC_TOWERING_INFERNAL, INFERNAL_SEARCH_RADIUS, true);
    std::sort(alive.begin(), alive.end(), [](const Unit* first, const Unit* second)
    {
        return first->GetObjectGuid().GetCounter() < second->GetObjectGuid().GetCounter();
    });

    std::list<ObjectGuid> guids;
    for (Unit* infernal : alive)
        guids.push_back(infernal->GetObjectGuid());

    return guids;
}

std::list<ObjectGuid> hyjal::GetInfernalGuids(PlayerbotAI* ai)
{
    return ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("hyjal infernals")->Get();
}

Unit* hyjal::GetLooseInfernal(PlayerbotAI* ai)
{
    Player* infernalTank = GetInfernalTank(ai->GetBot());
    if (!infernalTank)
        return nullptr;

    for (ObjectGuid guid : GetInfernalGuids(ai))
    {
        Unit* infernal = ai->GetUnit(guid);
        if (infernal && infernal->IsAlive() && infernal->GetVictim() != infernalTank)
            return infernal;
    }

    return nullptr;
}

Unit* hyjal::GetNearestInfernal(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    Unit* nearest = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    for (ObjectGuid guid : GetInfernalGuids(ai))
    {
        Unit* infernal = ai->GetUnit(guid);
        if (!infernal || !infernal->IsAlive())
            continue;

        const float distance = ExactDist2d(bot, infernal);
        if (distance < nearestDistance)
        {
            nearest = infernal;
            nearestDistance = distance;
        }
    }

    return nearest;
}

Unit* hyjal::GetInfernalToAttack(PlayerbotAI* ai, Unit* anetheron)
{
    if (!anetheron || anetheron->GetHealthPercent() <= BOSS_BURN_HEALTH_PCT)
        return nullptr;

    Player* bot = ai->GetBot();
    for (ObjectGuid guid : GetInfernalGuids(ai))
    {
        Unit* infernal = ai->GetUnit(guid);
        if (infernal && infernal->IsAlive() && ExactDist2d(bot, infernal) < INFERNAL_RANGED_ENGAGE_DISTANCE)
            return infernal;
    }

    return nullptr;
}

Unit* hyjal::GetInfernalTargetingBot(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    for (ObjectGuid guid : GetInfernalGuids(ai))
    {
        Unit* infernal = ai->GetUnit(guid);
        if (infernal && infernal->IsAlive() && infernal->GetVictim() == bot)
            return infernal;
    }

    return nullptr;
}

bool hyjal::IsInfernalTank(Player* bot)
{
    return PlayerbotAI::IsTank(bot) && IsAssistTank(bot, 0);
}

Player* hyjal::GetInfernalTank(Player* bot)
{
    return GetGroupAssistTank(bot, 0);
}

WorldPosition hyjal::GetInfernalTankPosition(Player* bot)
{
    Player* infernalTank = GetInfernalTank(bot);
    Player* from = infernalTank ? infernalTank : bot;

    const Point& east = ANETHERON_E_INFERNAL_POSITION;
    const Point& west = ANETHERON_W_INFERNAL_POSITION;
    return ExactDist2d(from, east.x, east.y) <= ExactDist2d(from, west.x, west.y) ? ToPosition(east) : ToPosition(west);
}

// Kaz'rogal

float hyjal::GetKazrogalRangedArcRadius(Unit* kazrogal)
{
    return (kazrogal && kazrogal->GetHealthPercent() > BOSS_ENGAGED_HEALTH_PCT) ? KAZROGAL_RANGED_ARC_APPROACH_RADIUS : KAZROGAL_RANGED_ARC_RADIUS;
}

float hyjal::GetKazrogalRangedArcSpan(float radius)
{
    const float ratio = KAZROGAL_RANGED_ARC_HALF_WIDTH / radius;
    return 2.0f * std::asin(ratio < 1.0f ? ratio : 1.0f);
}

bool hyjal::IsKazrogalManaUser(Player* bot)
{
    switch (bot->getClass())
    {
    case CLASS_WARRIOR:
    case CLASS_ROGUE:
        return false;
    case CLASS_DRUID:
        return IsRangedPlayer(bot);
    default:
        return true;
    }
}

bool hyjal::HasMarkOfKazrogal(Player* bot)
{
    return bot->HasAura(SPELL_MARK_OF_KAZROGAL);
}

// Azgalor

bool hyjal::IsSafeFromAzgalorCleave(Unit* azgalor, float x, float y)
{
    Unit* victim = azgalor->GetVictim();
    if (!victim)
        return true;

    if (ExactDist2d(victim, x, y) > CLEAVE_CHAIN_RADIUS)
        return true;

    const float angle = std::atan2(y - azgalor->GetPositionY(), x - azgalor->GetPositionX());
    return std::fabs(OffsetFrom(angle, azgalor->GetOrientation())) > CLEAVE_DANGER_ARC / 2.0f;
}

std::vector<WorldPosition> hyjal::GetRainOfFirePositions(PlayerbotAI* ai)
{
    return GetCachedHazardPositions(ai, "hyjal rain of fire");
}

bool hyjal::GetNearestRainOfFirePosition(PlayerbotAI* ai, WorldPosition& pool)
{
    Player* bot = ai->GetBot();
    bool found = false;
    float nearestDistance = std::numeric_limits<float>::max();
    for (const WorldPosition& position : GetRainOfFirePositions(ai))
    {
        const float distance = ExactDist2d(bot, position.getX(), position.getY());
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            pool = position;
            found = true;
        }
    }

    return found;
}

bool hyjal::IsNearRainOfFire(PlayerbotAI* ai, float radius)
{
    Player* bot = ai->GetBot();
    for (const WorldPosition& position : GetRainOfFirePositions(ai))
    {
        if (ExactDist2d(bot, position.getX(), position.getY()) < radius)
            return true;
    }

    return false;
}

bool hyjal::IsInRainOfFire(PlayerbotAI* ai)
{
    return IsNearRainOfFire(ai, RAIN_OF_FIRE_RADIUS);
}

bool hyjal::IsDoomed(Player* bot)
{
    return bot->HasAura(SPELL_DOOM);
}

bool hyjal::IsDoomguardTank(Player* bot)
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Player* firstAssistTank = GetGroupAssistTank(bot, 0);
    if (!firstAssistTank)
        return false;

    if (firstAssistTank == bot)
        return true;

    return IsDoomed(firstAssistTank) && GetGroupAssistTank(bot, 1) == bot;
}

bool hyjal::AnyGroupMemberHasDoom(Player* bot)
{
    for (Player* member : GetGroupMembers(bot, 500.0f, true))
    {
        if (IsDoomed(member))
            return true;
    }

    return false;
}

// Archimonde

bool hyjal::HasProtectionOfElune(Player* bot)
{
    return bot->HasAura(SPELL_PROTECTION_OF_ELUNE);
}

std::vector<WorldPosition> hyjal::GetDoomfirePositions(PlayerbotAI* ai)
{
    return GetCachedHazardPositions(ai, "hyjal doomfire trail");
}

bool hyjal::IsNearDoomfire(PlayerbotAI* ai, float radius)
{
    Player* bot = ai->GetBot();
    for (const WorldPosition& patch : GetDoomfirePositions(ai))
    {
        if (ExactDist2d(bot, patch.getX(), patch.getY()) < radius)
            return true;
    }

    return false;
}

bool hyjal::IsPositionNearDoomfire(PlayerbotAI* ai, float x, float y, float radius)
{
    for (const WorldPosition& patch : GetDoomfirePositions(ai))
    {
        if (Dist2d(patch, x, y) < radius)
            return true;
    }

    return false;
}

Player* hyjal::GetAirBurstTarget(Unit* archimonde)
{
    if (!archimonde)
        return nullptr;

    Spell* spell = archimonde->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || !spell->m_spellInfo || spell->m_spellInfo->Id != SPELL_AIR_BURST)
        return nullptr;

    Unit* target = spell->m_targets.getUnitTarget();
    return target && target->IsPlayer() ? static_cast<Player*>(target) : nullptr;
}

std::string hyjal::GetSelfImmunitySpellName(Player* bot)
{
    switch (bot->getClass())
    {
    case CLASS_MAGE:    return "ice block";
    case CLASS_PALADIN: return "divine shield";
    case CLASS_ROGUE:   return "cloak of shadows";
    default:            return "";
    }
}

uint32 hyjal::GetSelfImmunitySpellId(PlayerbotAI* ai)
{
    const std::string name = GetSelfImmunitySpellName(ai->GetBot());
    if (name.empty())
        return 0;

    return ai->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
}

// Action classification

bool hyjal::IsAvoidAoeAction(Action* action)
{
    return action && NameIn(action->getName(), { "flee", "tbc raid leave hazard" });
}

bool hyjal::IsFormationMoveAction(Action* action)
{
    return action && NameIn(action->getName(), { "tbc raid spread", "tbc raid leave front" });
}

bool hyjal::IsReachTargetAction(Action* action)
{
    return action && NameIn(action->getName(), {
        "reach melee", "reach spell", "reach pull", "charge", "intercept", "feral charge - bear", "feral charge - cat", "shadowstep" });
}

bool hyjal::IsSetBehindAction(Action* action)
{
    return action && action->getName() == "set behind";
}

bool hyjal::IsAssistTargetingAction(Action* action)
{
    return action && NameIn(action->getName(), { "dps assist", "dps aoe", "tank assist", "attack least hp target" });
}

bool hyjal::IsAttackAction(Action* action)
{
    return dynamic_cast<AttackAction*>(action) != nullptr;
}

// Values

std::vector<WorldPosition> HyjalHazardPositionsValue::Calculate()
{
    return GetDynamicObjectPositions(bot, searchRadius, spellId);
}
