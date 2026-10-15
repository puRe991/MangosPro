#include "playerbot/playerbot.h"
#include "RaidBossHelpers.h"
#include "playerbot/strategy/Action.h"
#include "playerbot/strategy/actions/MovementActions.h"
#include "playerbot/strategy/generic/TbcRaidStrategy.h"
#include "Entities/DynamicObject.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "Maps/InstanceData.h"
#include <algorithm>
#include <cmath>

using namespace ai;

namespace
{
    class DynamicObjectOfSpellInRangeCheck
    {
    public:
        DynamicObjectOfSpellInRangeCheck(WorldObject const* obj, uint32 spellId, float range) : i_obj(obj), i_spellId(spellId), i_range(range) {}
        WorldObject const& GetFocusObject() const { return *i_obj; }
        bool operator()(DynamicObject* u) { return u && u->GetSpellId() == i_spellId && i_obj->IsWithinDistInMap(u, i_range); }

    private:
        WorldObject const* i_obj;
        uint32 i_spellId;
        float i_range;
    };

    bool NameIn(const std::string& name, std::initializer_list<const char*> names)
    {
        for (const char* n : names)
        {
            if (name == n)
                return true;
        }

        return false;
    }

    std::string ToLower(std::string text)
    {
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        return text;
    }
}

bool raidboss::IsEncounterInProgress(Player* bot, uint32 mapId, const std::vector<uint32>& bossEntries)
{
    if (!bot || !bot->IsInWorld() || bot->GetMapId() != mapId)
        return false;

    if (InstanceData* instance = bot->GetInstanceData())
    {
        if (instance->IsEncounterInProgress())
            return true;
    }

    if (!bot->IsInCombat())
        return false;

    PlayerbotAI* ai = bot->GetPlayerbotAI();
    if (!ai)
        return false;

    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("attackers")->Get())
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->IsPlayer())
            continue;

        if (bossEntries.empty() || std::find(bossEntries.begin(), bossEntries.end(), unit->GetEntry()) != bossEntries.end())
            return true;
    }

    return false;
}

Unit* raidboss::FindUnit(PlayerbotAI* ai, uint32 entry, float range)
{
    Player* bot = ai->GetBot();
    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("attackers")->Get())
    {
        Unit* unit = ai->GetUnit(guid);
        if (unit && unit->IsAlive() && !unit->IsPlayer() && unit->GetEntry() == entry)
            return unit;
    }

    Unit* nearest = nullptr;
    for (Unit* unit : FindUnits(bot, entry, range))
    {
        if (!nearest || bot->GetDistance(unit) < bot->GetDistance(nearest))
            nearest = unit;
    }

    return nearest;
}

Unit* raidboss::FindUnitByName(PlayerbotAI* ai, const std::string& name, float range)
{
    Player* bot = ai->GetBot();
    const std::string wanted = ToLower(name);

    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("attackers")->Get())
    {
        Unit* unit = ai->GetUnit(guid);
        if (unit && unit->IsAlive() && !unit->IsPlayer() && ToLower(unit->GetName()) == wanted)
            return unit;
    }

    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible targets no los")->Get())
    {
        Unit* unit = ai->GetUnit(guid);
        if (unit && unit->IsAlive() && !unit->IsPlayer() && bot->IsWithinDistInMap(unit, range) && ToLower(unit->GetName()) == wanted)
            return unit;
    }

    return nullptr;
}

std::vector<Unit*> raidboss::FindUnits(Player* bot, uint32 entry, float range, bool aliveOnly)
{
    std::vector<Unit*> result;
    for (Unit* unit : TbcRaidHelper::FindCreatures(bot, entry, range))
    {
        if (unit && (!aliveOnly || unit->IsAlive()))
            result.push_back(unit);
    }

    return result;
}

std::vector<GameObject*> raidboss::FindGameObjects(Player* bot, uint32 entry, float range)
{
    std::list<GameObject*> objects = TbcRaidHelper::FindGameObjects(bot, entry, range);
    return std::vector<GameObject*>(objects.begin(), objects.end());
}

bool raidboss::CanTakeStepTowards(Player* bot, float destinationX, float destinationY, float moveDist,
                                  float& stepX, float& stepY, float& stepZ)
{
    const float minMoveDistance = 0.5f;
    const float distance = ExactDist2d(bot, destinationX, destinationY);
    if (distance < minMoveDistance)
        return false;

    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();
    const float botZ = bot->GetPositionZ();

    const float ratio = std::min(moveDist, distance) / distance;
    const float candidateX = botX + (destinationX - botX) * ratio;
    const float candidateY = botY + (destinationY - botY) * ratio;
    float candidateZ = bot->GetMap()->GetHeight(candidateX, candidateY, botZ + 4.0f);
    if (candidateZ <= INVALID_HEIGHT)
        candidateZ = botZ;

    // Too steep upwards, or a wall in between: try another path
    if (candidateZ - botZ > moveDist * 1.5f)
        return false;

    if (!bot->GetMap()->IsInLineOfSight(botX, botY, botZ + 2.0f, candidateX, candidateY, candidateZ + 2.0f, true))
        return false;

    stepX = candidateX;
    stepY = candidateY;
    stepZ = candidateZ;
    return true;
}

bool raidboss::GetStepToPosition(Player* bot, const WorldPosition& position, float arrivalDist, Unit* facing,
                                 float& stepX, float& stepY, bool& backwards)
{
    const float distToPosition = ExactDist2d(bot, position.getX(), position.getY());
    if (distToPosition <= arrivalDist)
        return false;

    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();
    const float toPosX = position.getX() - botX;
    const float toPosY = position.getY() - botY;

    // Tanks walk backwards when the destination is behind them and they hold aggro in melee range
    backwards = false;
    if (facing && facing->GetVictim() == bot && bot->CanReachWithMeleeAttack(facing))
    {
        const float toFacingX = facing->GetPositionX() - botX;
        const float toFacingY = facing->GetPositionY() - botY;
        backwards = (toPosX * toFacingX + toPosY * toFacingY) < 0.0f;
    }

    // Short steps, so the movement is re-evaluated every tick
    const float backwardDistancePerStep = 2.25f;
    const float forwardDistancePerStep = 3.5f;
    const float maxMoveDist = backwards ? backwardDistancePerStep : forwardDistancePerStep;
    const float ratio = std::min(maxMoveDist, distToPosition) / distToPosition;

    stepX = botX + toPosX * ratio;
    stepY = botY + toPosY * ratio;
    return true;
}

bool raidboss::MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId)
{
    if (!target)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (group->GetTargetIcon(iconId) != target->GetObjectGuid())
    {
        group->SetTargetIcon(iconId, target->GetObjectGuid());
        return true;
    }

    return false;
}

bool raidboss::ClearTargetIcon(Player* bot, uint8 iconId)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (!group->GetTargetIcon(iconId).IsEmpty())
    {
        group->SetTargetIcon(iconId, ObjectGuid());
        return true;
    }

    return false;
}

Unit* raidboss::GetIconTarget(PlayerbotAI* ai, uint8 iconId)
{
    Group* group = ai->GetBot()->GetGroup();
    if (!group)
        return nullptr;

    Unit* unit = ai->GetUnit(group->GetTargetIcon(iconId));
    return unit && unit->IsAlive() ? unit : nullptr;
}

void raidboss::SetRtiTarget(PlayerbotAI* ai, const std::string& rtiName)
{
    Value<std::string>* rtiValue = ai->GetAiObjectContext()->GetValue<std::string>("rti");
    if (rtiValue->Get() != rtiName)
        rtiValue->Set(rtiName);
}

bool raidboss::IsMechanicTrackerBot(Player* bot, uint32 mapId)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || !member->IsAlive() || !member->IsInWorld() || member->GetMapId() != mapId || !member->GetPlayerbotAI())
            continue;

        return member == bot;
    }

    return false;
}

std::vector<Player*> raidboss::GetGroupTanks(Player* bot)
{
    std::vector<Player*> assistants;
    std::vector<Player*> others;

    Group* group = bot->GetGroup();
    if (!group)
    {
        if (PlayerbotAI::IsTank(bot))
            others.push_back(bot);

        return others;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || !member->IsAlive() || !member->IsInWorld() || member->GetMapId() != bot->GetMapId() || !PlayerbotAI::IsTank(member))
            continue;

        if (group->IsAssistant(member->GetObjectGuid()) || group->IsLeader(member->GetObjectGuid()))
            assistants.push_back(member);
        else
            others.push_back(member);
    }

    assistants.insert(assistants.end(), others.begin(), others.end());
    return assistants;
}

namespace
{
    std::vector<Player*> GetMembersMatching(Player* bot, const std::function<bool(Player*)>& match)
    {
        std::vector<Player*> result;
        for (Player* member : raidboss::GetGroupMembers(bot, 500.0f, true))
        {
            if (match(member))
                result.push_back(member);
        }

        return result;
    }
}

std::vector<Player*> raidboss::GetGroupHealers(Player* bot)
{
    return GetMembersMatching(bot, [](Player* member) { return PlayerbotAI::IsHeal(member); });
}

std::vector<Player*> raidboss::GetGroupRangedDps(Player* bot)
{
    return GetMembersMatching(bot, [](Player* member) { return IsDps(member) && IsRangedPlayer(member); });
}

std::vector<Player*> raidboss::GetGroupMeleeDps(Player* bot)
{
    return GetMembersMatching(bot, [](Player* member) { return IsDps(member) && IsMeleePlayer(member); });
}

int raidboss::GetIndexOf(Player* bot, const std::vector<Player*>& players)
{
    for (size_t i = 0; i < players.size(); ++i)
    {
        if (players[i] == bot)
            return int(i);
    }

    return -1;
}

Player* raidboss::GetGroupMainTank(Player* bot)
{
    std::vector<Player*> tanks = GetGroupTanks(bot);
    return tanks.empty() ? nullptr : tanks.front();
}

Player* raidboss::GetGroupAssistTank(Player* bot, uint8 index)
{
    std::vector<Player*> tanks = GetGroupTanks(bot);
    return size_t(index) + 1 < tanks.size() ? tanks[index + 1] : nullptr;
}

bool raidboss::IsMainTank(Player* bot)
{
    return GetGroupMainTank(bot) == bot;
}

bool raidboss::IsAssistTank(Player* bot, uint8 index)
{
    return GetGroupAssistTank(bot, index) == bot;
}

bool raidboss::IsDps(Player* player)
{
    return !PlayerbotAI::IsTank(player) && !PlayerbotAI::IsHeal(player);
}

bool raidboss::IsRangedPlayer(Player* player)
{
    if (PlayerbotAI* ai = player->GetPlayerbotAI())
        return ai->IsRanged(player);

    switch (player->getClass())
    {
    case CLASS_PALADIN:
    case CLASS_WARRIOR:
    case CLASS_ROGUE:
        return false;
    case CLASS_DRUID:
        return !player->HasAura(768) && !player->HasAura(5487) && !player->HasAura(9634);  // cat / bear / dire bear form
    default:
        return true;
    }
}

bool raidboss::IsMeleePlayer(Player* player)
{
    return !IsRangedPlayer(player);
}

std::vector<Player*> raidboss::GetGroupMembers(Player* bot, float range, bool includeSelf)
{
    return TbcRaidHelper::GetRaidMembers(bot, range, includeSelf);
}

Player* raidboss::GetNearestPlayerInRadius(Player* bot, float radius)
{
    Player* nearestPlayer = nullptr;
    float nearestDistance = radius;
    for (Player* member : GetGroupMembers(bot, radius, false))
    {
        const float distance = ExactDist2d(bot, member);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestPlayer = member;
        }
    }

    return nearestPlayer;
}

std::vector<WorldPosition> raidboss::GetDynamicObjectPositions(Player* bot, float searchRadius, uint32 spellId)
{
    std::list<DynamicObject*> objects;
    DynamicObjectOfSpellInRangeCheck check(bot, spellId, searchRadius);
    MaNGOS::DynamicObjectListSearcher<DynamicObjectOfSpellInRangeCheck> searcher(objects, check);
    Cell::VisitAllObjects(bot, searcher, searchRadius);

    std::vector<WorldPosition> positions;
    for (DynamicObject* object : objects)
        positions.push_back(WorldPosition(object));

    return positions;
}

uint32 raidboss::GetSelfImmunitySpell(Player* bot)
{
    switch (bot->getClass())
    {
    case CLASS_MAGE:    return 45438;   // Ice Block
    case CLASS_PALADIN: return 642;     // Divine Shield
    case CLASS_ROGUE:   return 31224;   // Cloak of Shadows
    default:            return 0;
    }
}

bool raidboss::IsDpsCooldownAction(Player* bot, Action* action)
{
    if (!action)
        return false;

    const std::string& name = action->getName();

    // Before the dps check, Bloodlust / Heroism of a restoration shaman counts too
    if (bot->getClass() == CLASS_SHAMAN && (name == "bloodlust" || name == "heroism"))
        return true;

    if (!IsDps(bot))
        return false;

    return NameIn(name, {
        "use trinket", "arcane power", "combustion", "icy veins", "presence of mind", "cold snap",
        "summon water elemental", "rapid fire", "readiness", "bestial wrath", "kill command",
        "death wish", "recklessness", "adrenaline rush", "blade flurry", "cold blood",
        "avenging wrath", "elemental mastery", "fire elemental totem", "power infusion",
        "force of nature", "blood fury", "berserking", "arcane torrent" });
}

bool raidboss::IsTauntAction(Player* bot, Action* action)
{
    if (!action || !PlayerbotAI::IsTank(bot))
        return false;

    return NameIn(action->getName(), {
        "taunt", "growl", "righteous defense", "hand of reckoning", "mocking blow",
        "challenging shout", "challenging roar", "taunt on snare target", "tbc raid taunt" });
}

bool raidboss::IsAoeThreatAction(Player* bot, Action* action)
{
    if (!action || !PlayerbotAI::IsTank(bot))
        return false;

    return NameIn(action->getName(), {
        "swipe", "swipe (bear)", "consecration", "avenger's shield", "thunder clap", "cleave",
        "demoralizing shout", "demoralizing roar", "challenging shout", "challenging roar" });
}

bool raidboss::IsMovementAction(Action* action)
{
    if (!action)
        return false;

    const std::string& name = action->getName();
    return dynamic_cast<MovementAction*>(action) != nullptr || NameIn(name, {
        "reach melee", "reach spell", "reach target", "follow", "flee", "runaway", "move out of enemy contact",
        "move to loot", "set facing", "set behind", "move random", "return", "tank face", "avoid aoe",
        "move away from creature", "move from group", "disperse set", "go away" });
}

bool raidboss::IsAttackTargetAction(Action* action)
{
    if (!action)
        return false;

    return NameIn(action->getName(), {
        "dps assist", "dps aoe", "tank assist", "attack least hp target", "attack anything", "attack rti target",
        "attack enemy player", "attack enemy flag carrier", "select new target", "pull rti target" });
}

float raidboss::ExactDist2d(const WorldObject* a, const WorldObject* b)
{
    return ExactDist2d(a, b->GetPositionX(), b->GetPositionY());
}

float raidboss::ExactDist2d(const WorldObject* a, float x, float y)
{
    const float dx = a->GetPositionX() - x;
    const float dy = a->GetPositionY() - y;
    return std::sqrt(dx * dx + dy * dy);
}

bool raidboss::TryCast(PlayerbotAI* ai, uint32 spellId, Unit* target)
{
    return spellId && target && ai->CanCastSpell(spellId, target, 0) && ai->CastSpell(spellId, target);
}

bool raidboss::TryCast(PlayerbotAI* ai, const std::string& spell, Unit* target)
{
    return target && ai->CanCastSpell(spell, target, 0) && ai->CastSpell(spell, target);
}
