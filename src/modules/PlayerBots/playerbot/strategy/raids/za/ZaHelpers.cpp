// Ported from mod-playerbots (AzerothCore, GPL v2+), src/Ai/Raid/ZA/ZAHelpers.cpp, and adapted for this core.
#include "playerbot/playerbot.h"
#include "ZaHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/actions/MovementActions.h"
#include "playerbot/strategy/actions/AttackAction.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace ai;

namespace
{
    bool IsInsideSafeZone(const std::vector<WorldPosition>& corners, float x, float y)
    {
        // The safe zone is pentagonal, a rectangle with one corner cut out (fenced-off broken corner)
        const size_t count = corners.size();
        int8 insideSign = 0;
        for (size_t i = 0; i < count; ++i)
        {
            const WorldPosition& edgeStart = corners[i];
            const WorldPosition& edgeEnd = corners[(i + 1) % count];

            const float cross = (edgeEnd.getX() - edgeStart.getX()) * (y - edgeStart.getY()) -
                                (edgeEnd.getY() - edgeStart.getY()) * (x - edgeStart.getX());
            if (cross == 0.0f)
                continue;

            const int8 sign = cross > 0.0f ? 1 : -1;
            if (insideSign == 0)
                insideSign = sign;
            else if (insideSign != sign)
                return false;
        }

        return true;
    }

    bool IsPositionSafeFromHazards(float x, float y, const std::vector<Unit*>& hazards, float hazardRadius)
    {
        for (Unit* hazard : hazards)
        {
            if (raidboss::ExactDist2d(hazard, x, y) < hazardRadius)
                return false;
        }

        return true;
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

bool za::IsEncounterInProgress(Player* bot)
{
    return raidboss::IsEncounterInProgress(bot, ZA_MAP_ID,
        { NPC_AKILZON, NPC_NALORAKK, NPC_JANALAI, NPC_HALAZZI, NPC_HEX_LORD_MALACRASS, NPC_ZULJIN });
}

Unit* za::GetBoss(PlayerbotAI* ai, uint32 entry)
{
    return raidboss::FindUnit(ai, entry);
}

bool za::CastTaunt(PlayerbotAI* ai, Unit* target)
{
    if (!target)
        return false;

    Player* bot = ai->GetBot();
    std::string spell;
    Unit* spellTarget = target;
    switch (bot->getClass())
    {
    case CLASS_WARRIOR:
        spell = "taunt";
        break;
    case CLASS_DRUID:
        spell = "growl";
        break;
    case CLASS_PALADIN:
        // Righteous Defense is cast on the friendly target of the creature
        spell = "righteous defense";
        spellTarget = target->GetVictim();
        break;
    default:
        return false;
    }

    if (!spellTarget || !ai->CanCastSpell(spell, spellTarget, 0, nullptr, false) || !ai->CastSpell(spell, spellTarget))
        return false;

    ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Set(target->GetObjectGuid());
    bot->SetSelectionGuid(target->GetObjectGuid());
    return true;
}

// Akil'zon <Eagle Avatar>

bool za::IsInStormWindow(Player* bot, uint32 engageMs, uint32 lastStormMs)
{
    if (GetElectricalStormTarget(bot))
        return true;

    const uint32 now = WorldTimer::getMSTime();
    if (lastStormMs)
    {
        const uint32 elapsed = WorldTimer::getMSTimeDiff(lastStormMs, now);
        return elapsed + AKILZON_STORM_LEAD_MS >= AKILZON_NEXT_STORM_MIN_MS &&
               elapsed <= AKILZON_NEXT_STORM_MAX_MS + AKILZON_STORM_DURATION_MS;
    }

    if (engageMs)
    {
        const uint32 elapsed = WorldTimer::getMSTimeDiff(engageMs, now);
        return elapsed + AKILZON_STORM_LEAD_MS >= AKILZON_FIRST_STORM_MS &&
               elapsed <= AKILZON_FIRST_STORM_MS + AKILZON_STORM_DURATION_MS;
    }

    return false;
}

bool za::IsInStormWindow(PlayerbotAI* ai)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    return IsInStormWindow(ai->GetBot(), context->GetValue<uint32>("akil'zon electrical storm timer")->Get(),
                           context->GetValue<uint32>("akil'zon last electrical storm")->Get());
}

Player* za::GetElectricalStormTarget(Player* bot)
{
    for (Player* member : raidboss::GetGroupMembers(bot, 200.0f, true))
    {
        if (member->HasAura(SPELL_ELECTRICAL_STORM))
            return member;
    }

    return nullptr;
}

// Nalorakk <Bear Avatar>

bool za::IsNalorakkInBearForm(Unit* nalorakk)
{
    return nalorakk && nalorakk->HasAura(SPELL_BEARFORM);
}

Player* za::GetNalorakkFormTank(Player* bot, Unit* nalorakk)
{
    return IsNalorakkInBearForm(nalorakk) ? raidboss::GetGroupMainTank(bot) : raidboss::GetGroupAssistTank(bot, 0);
}

// Jan'alai <Dragonhawk Avatar>

std::pair<Unit*, Unit*> za::GetAmanishiHatcherPair(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    Unit* first = nullptr;
    Unit* second = nullptr;

    for (Unit* unit : raidboss::FindUnits(bot, NPC_AMANISHI_HATCHER_1, 100.0f))
    {
        first = unit;
        break;
    }

    for (Unit* unit : raidboss::FindUnits(bot, NPC_AMANISHI_HATCHER_2, 100.0f))
    {
        second = unit;
        break;
    }

    return { first, second };
}

uint32 za::CountJanalaiHatchlings(PlayerbotAI* ai)
{
    uint32 count = 0;
    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("attackers")->Get())
    {
        Unit* unit = ai->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_AMANI_DRAGONHAWK_HATCHLING)
            ++count;
    }

    return count;
}

bool za::IsJanalaiBombing(Unit* janalai)
{
    return janalai && janalai->HasAura(SPELL_FIRE_BOMB_CHANNEL);
}

std::list<ObjectGuid> za::FindNearbyFireBombGuids(Player* bot)
{
    std::list<ObjectGuid> guids;
    if (bot->GetMapId() != ZA_MAP_ID)
        return guids;

    for (Unit* bomb : raidboss::FindUnits(bot, NPC_FIRE_BOMB, JANALAI_FIRE_BOMB_SEARCH_RADIUS))
        guids.push_back(bomb->GetObjectGuid());

    return guids;
}

std::vector<Unit*> za::GetNearbyFireBombs(PlayerbotAI* ai)
{
    std::vector<Unit*> bombs;
    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("jan'alai fire bombs")->Get())
    {
        Unit* unit = ai->GetUnit(guid);
        if (unit && unit->IsAlive())
            bombs.push_back(unit);
    }

    return bombs;
}

// Shortest move to a spot that is safe from Fire Bombs, within the measured area
bool za::FindSafeStepInJanalaiZone(Player* bot, const std::vector<Unit*>& hazards, const std::vector<WorldPosition>& safeZone,
                                   float maxSearchDistance, float hazardRadius, float moveDist,
                                   float& stepX, float& stepY, float& stepZ)
{
    const uint8 angleCount = 16;
    const float angleStep = 2.0f * M_PI_F / angleCount;
    const float distanceStep = 1.0f;
    const uint32 ringCount = uint32(maxSearchDistance / distanceStep);

    for (uint32 ring = 1; ring <= ringCount; ++ring)
    {
        const float distance = ring * distanceStep;
        for (uint8 i = 0; i < angleCount; ++i)
        {
            const float angle = i * angleStep;
            const float x = bot->GetPositionX() + distance * std::cos(angle);
            const float y = bot->GetPositionY() + distance * std::sin(angle);

            if (!IsInsideSafeZone(safeZone, x, y))
                continue;

            if (!IsPositionSafeFromHazards(x, y, hazards, hazardRadius))
                continue;

            if (!raidboss::CanTakeStepTowards(bot, x, y, moveDist, stepX, stepY, stepZ))
                continue;

            return true;
        }
    }

    return false;
}

// Hex Lord Malacrass

ObjectGuid za::FindNearbyFreezingTrapGuid(Player* bot)
{
    if (bot->GetMapId() != ZA_MAP_ID)
        return ObjectGuid();

    GameObject* nearest = nullptr;
    for (GameObject* trap : raidboss::FindGameObjects(bot, GO_FREEZING_TRAP, ZA_FREEZING_TRAP_SEARCH_RADIUS))
    {
        if (!nearest || bot->GetDistance(trap) < bot->GetDistance(nearest))
            nearest = trap;
    }

    return nearest ? nearest->GetObjectGuid() : ObjectGuid();
}

GameObject* za::GetNearbyFreezingTrap(PlayerbotAI* ai)
{
    const ObjectGuid guid = ai->GetAiObjectContext()->GetValue<ObjectGuid>("hex lord malacrass freezing trap")->Get();
    if (guid.IsEmpty())
        return nullptr;

    GameObject* trap = ai->GetGameObject(guid);
    return trap && trap->IsSpawned() ? trap : nullptr;
}

bool za::HasUnstableAffliction(Unit* unit)
{
    return unit && (unit->HasAura(SPELL_UNSTABLE_AFFLICTION) || unit->HasAura(SPELL_UNSTABLE_AFFLICTION_ALT));
}

// Zul'jin

bool za::IsZuljinInAnimalForm(Unit* zuljin)
{
    return zuljin && (zuljin->HasAura(SPELL_SHAPE_OF_THE_BEAR) || zuljin->HasAura(SPELL_SHAPE_OF_THE_EAGLE) ||
                      zuljin->HasAura(SPELL_SHAPE_OF_THE_LYNX) || zuljin->HasAura(SPELL_SHAPE_OF_THE_DRAGONHAWK));
}

bool za::GetZuljinSpreadSlotIndex(Player* bot, size_t slotCount, size_t& slotIndex)
{
    if (slotCount == 0 || !bot->GetGroup())
        return false;

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (Player* member : raidboss::GetGroupMembers(bot, 500.0f, true))
    {
        if (!raidboss::IsRangedPlayer(member))
            continue;

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    const int healerIndex = raidboss::GetIndexOf(bot, healers);
    if (healerIndex >= 0)
    {
        slotIndex = size_t(healerIndex) % slotCount;
        return true;
    }

    const int dpsIndex = raidboss::GetIndexOf(bot, rangedDps);
    if (dpsIndex < 0)
        return false;

    slotIndex = (healers.size() + size_t(dpsIndex)) % slotCount;
    return true;
}

Player* za::GetZuljinCreepingParalysisDispelTarget(Player* bot)
{
    Player* closestTarget = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (Player* member : raidboss::GetGroupMembers(bot, 100.0f, true))
    {
        if (!member->HasAura(SPELL_CREEPING_PARALYSIS))
            continue;

        const float distance = bot->GetDistance(member);
        if (distance < closestDistance)
        {
            closestTarget = member;
            closestDistance = distance;
        }
    }

    return closestTarget;
}

// Action classification

bool za::IsApproachMovement(Action* action)
{
    // "set behind" circles around targets when they are near, even if the bot is held in position
    return action && NameIn(action->getName(), {
        "reach melee", "reach spell", "reach pull", "reach party member to heal", "set behind" });
}

bool za::IsHazardousMovement(Action* action)
{
    if (!action)
        return false;

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return true;

    // Spells which move the bot
    return NameIn(action->getName(), { "charge", "intercept", "feral charge - bear", "feral charge", "blink" });
}
