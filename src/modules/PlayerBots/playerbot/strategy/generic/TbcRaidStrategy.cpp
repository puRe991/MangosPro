#include "playerbot/playerbot.h"
#include "TbcRaidStrategy.h"
#include "playerbot/strategy/values/AttackersValue.h"
#include "Entities/DynamicObject.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;
using namespace ai::tbcraid;

namespace
{
    enum TbcRaidIds : uint32
    {
        // Magtheridon
        NPC_MAGTHERIDON             = 17257,
        GO_MANTICRON_CUBE           = 181713,
        SPELL_SHADOW_CAGE_DUMMY     = 30205,    // Magtheridon is still banished (phase 1)
        SPELL_BLAST_NOVA            = 30616,
        SPELL_SHADOW_GRASP          = 30410,    // channeled by the cube clickers
        SPELL_MIND_EXHAUSTION       = 44032,

        // Naj'entus
        NPC_NAJENTUS                = 22887,
        GO_IMPALING_SPINE           = 185584,
        SPELL_IMPALING_SPINE        = 39837,
        SPELL_TIDAL_SHIELD          = 39872,
        ITEM_NAJENTUS_SPINE         = 32408,
    };

    const uint32 ENCOUNTER_CACHE_MS = 1000;
    const uint32 HAZARD_CACHE_MS = 300;
    const uint32 PRIORITY_CACHE_MS = 500;

    class AnyDynamicObjectInRangeCheck
    {
    public:
        AnyDynamicObjectInRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) {}
        WorldObject const& GetFocusObject() const { return *i_obj; }
        bool operator()(DynamicObject* u) { return u && i_obj->IsWithinDistInMap(u, i_range); }

    private:
        WorldObject const* i_obj;
        float i_range;
    };

    class GameObjectOfEntryInRangeCheck
    {
    public:
        GameObjectOfEntryInRangeCheck(WorldObject const* obj, uint32 entry, float range) : i_obj(obj), i_entry(entry), i_range(range) {}
        WorldObject const& GetFocusObject() const { return *i_obj; }
        bool operator()(GameObject* go) { return go && go->GetEntry() == i_entry && go->IsSpawned() && i_obj->IsWithinDistInMap(go, i_range); }

    private:
        WorldObject const* i_obj;
        uint32 i_entry;
        float i_range;
    };

    bool IsExpired(uint32 checkTime, uint32 cacheTime)
    {
        const uint32 now = WorldTimer::getMSTime();
        return checkTime == 0 || now < checkTime || (now - checkTime) >= cacheTime;
    }

    bool IsTauntAction(const std::string& name)
    {
        return name == "taunt" || name == "growl" || name == "righteous defense" || name == "hand of reckoning" ||
               name == "mocking blow" || name == "challenging shout" || name == "challenging roar" ||
               name == "taunt on snare target" || name == "dark command" || name == "death grip";
    }

    uint8 RoleSortOrder(uint8 role)
    {
        switch (role)
        {
        case ROLE_RANGED: return 0;
        case ROLE_HEAL:   return 1;
        case ROLE_MELEE:  return 2;
        default:          return 3;
        }
    }
}

// ---------------------------------------------------------------------------
// Strategy
// ---------------------------------------------------------------------------

TbcRaidStrategy* TbcRaidStrategy::Get(PlayerbotAI* ai)
{
    return ai ? ai->GetStrategy<TbcRaidStrategy>("tbc raid", BotState::BOT_STATE_COMBAT) : nullptr;
}

void TbcRaidStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "tbc raid possessing",
        NextAction::array(0, new NextAction("tbc raid control possessed", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid has core",
        NextAction::array(0, new NextAction("tbc raid use core", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid lurker spout",
        NextAction::array(0, new NextAction("tbc raid jump into water", ACTION_EMERGENCY + 8), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid darkness",
        NextAction::array(0, new NextAction("tbc raid gather under dragon", ACTION_EMERGENCY + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid use blue orb",
        NextAction::array(0, new NextAction("tbc raid use blue orb", ACTION_EMERGENCY + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid hydross tank",
        NextAction::array(0, new NextAction("tbc raid hydross tank", ACTION_EMERGENCY + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid loot core",
        NextAction::array(0, new NextAction("tbc raid loot core", ACTION_CRITICAL_HEAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid stack on tank",
        NextAction::array(0, new NextAction("tbc raid stack on tank", ACTION_MEDIUM_HEAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid soak front",
        NextAction::array(0, new NextAction("tbc raid soak front", ACTION_MEDIUM_HEAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid approach boss",
        NextAction::array(0, new NextAction("tbc raid approach boss", ACTION_MEDIUM_HEAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid use cube",
        NextAction::array(0, new NextAction("tbc raid use cube", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid in hazard",
        NextAction::array(0, new NextAction("tbc raid leave hazard", ACTION_EMERGENCY + 8), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid debuff run",
        NextAction::array(0, new NextAction("tbc raid spread", ACTION_EMERGENCY + 6), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid gather",
        NextAction::array(0, new NextAction("tbc raid gather", ACTION_EMERGENCY + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid taunt",
        NextAction::array(0, new NextAction("tbc raid taunt", ACTION_EMERGENCY + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid interrupt",
        NextAction::array(0, new NextAction("tbc raid interrupt", ACTION_EMERGENCY + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid mark targets",
        NextAction::array(0, new NextAction("tbc raid mark targets", ACTION_EMERGENCY), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid tank assigned",
        NextAction::array(0, new NextAction("tbc raid tank assigned", ACTION_CRITICAL_HEAL + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid steal aura",
        NextAction::array(0, new NextAction("tbc raid steal aura", ACTION_CRITICAL_HEAL + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid tank position",
        NextAction::array(0, new NextAction("tbc raid tank position", ACTION_MEDIUM_HEAL + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid use spine",
        NextAction::array(0,
            new NextAction("tbc raid use spine", ACTION_CRITICAL_HEAL + 2),
            new NextAction("tbc raid move to spine", ACTION_CRITICAL_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid spread",
        NextAction::array(0, new NextAction("tbc raid spread", ACTION_MEDIUM_HEAL + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid in front of boss",
        NextAction::array(0, new NextAction("tbc raid leave front", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid priority target",
        NextAction::array(0, new NextAction("tbc raid attack priority target", ACTION_LIGHT_HEAL + 5), NULL)));
}

void TbcRaidStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "tbc raid possessing",
        NextAction::array(0, new NextAction("tbc raid control possessed", ACTION_EMERGENCY + 9), NULL)));

    // Ground effects stay after the fight / between boss phases
    triggers.push_back(new TriggerNode(
        "tbc raid in hazard",
        NextAction::array(0, new NextAction("tbc raid leave hazard", ACTION_EMERGENCY + 8), NULL)));
}

void TbcRaidStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    // Reactions are checked while casting, so the bot stops casting to leave fire or click a cube
    triggers.push_back(new TriggerNode(
        "tbc raid possessing",
        NextAction::array(0, new NextAction("tbc raid control possessed", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid has core",
        NextAction::array(0, new NextAction("tbc raid use core", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid lurker spout",
        NextAction::array(0, new NextAction("tbc raid jump into water", ACTION_EMERGENCY + 8), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid use cube",
        NextAction::array(0, new NextAction("tbc raid use cube", ACTION_EMERGENCY + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid in hazard",
        NextAction::array(0, new NextAction("tbc raid leave hazard", ACTION_EMERGENCY + 8), NULL)));

    triggers.push_back(new TriggerNode(
        "tbc raid debuff run",
        NextAction::array(0, new NextAction("tbc raid spread", ACTION_EMERGENCY + 6), NULL)));
}

void TbcRaidStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new TbcRaidMultiplier(ai));
}

void TbcRaidStrategy::InitReactionMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new TbcRaidMultiplier(ai));
}

// ---------------------------------------------------------------------------
// Multiplier
// ---------------------------------------------------------------------------

float TbcRaidMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    Player* bot = ai->GetBot();

    // Never break the Shadow Grasp channel of a Manticron Cube
    if (bot->HasAura(SPELL_SHADOW_GRASP))
        return 0.0f;

    const std::string& name = action->getName();
    if (name.rfind("tbc raid", 0) == 0)
        return 1.0f;

    // The bot controls a Vengeful Spirit / Blue Dragon, its own body does nothing
    if (TbcRaidHelper::IsPossessingBossCreature(bot))
        return 0.0f;

    // Paralyzed by a Tainted Core: only pass the core
    if (bot->HasAura(38132))
        return 0.0f;

    if (!TbcRaidHelper::GetEncounter(ai))
        return 1.0f;

    // A bot with a fixed assignment keeps its creature, no other target selection
    if (name == "tank assist" || name == "dps assist" || name == "dps aoe" || name == "attack least hp target" ||
        name == "tbc raid attack priority target" || name == "tank aoe" || name == "attack anything")
    {
        if (!TbcRaidHelper::GetMyAssignments(ai).empty())
            return 0.0f;
    }

    // Keep attacking the priority target instead of switching to the lowest health enemy
    if (name == "dps assist" || name == "dps aoe" || name == "attack least hp target")
    {
        Unit* priority = TbcRaidHelper::GetPriorityTarget(ai);
        if (priority)
        {
            Unit* current = ai->GetUnit(ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Get());
            if (current == priority || name != "dps aoe")
                return 0.0f;
        }
    }

    // A tank with too many stacks (or the wrong tank for the boss form) must not taunt back
    if (IsTauntAction(name) && TbcRaidHelper::ShouldHoldTaunt(ai))
        return 0.0f;

    // Cube clickers wait at their cube
    if ((name == "reach melee" || name == "reach spell") && TbcRaidHelper::GetAssignedCube(ai))
        return 0.0f;

    // Don't run back into a whirlwinding / exploding target
    if (name == "reach melee")
    {
        const ObjectGuid currentTarget = ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Get();
        if (currentTarget)
        {
            for (const HazardZone& hazard : TbcRaidHelper::GetHazards(ai))
            {
                if (hazard.guid == currentTarget)
                    return 0.0f;
            }
        }
    }

    return 1.0f;
}

// ---------------------------------------------------------------------------
// Helper
// ---------------------------------------------------------------------------

std::list<Unit*> TbcRaidHelper::FindCreatures(Player* bot, uint32 entry, float range)
{
    std::list<Unit*> creatures;
    MaNGOS::AllCreaturesOfEntryInRangeCheck check(bot, entry, range);
    MaNGOS::UnitListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(creatures, check);
    Cell::VisitAllObjects(bot, searcher, range);
    return creatures;
}

std::list<GameObject*> TbcRaidHelper::FindGameObjects(Player* bot, uint32 entry, float range)
{
    std::list<GameObject*> gameObjects;
    GameObjectOfEntryInRangeCheck check(bot, entry, range);
    MaNGOS::GameObjectListSearcher<GameObjectOfEntryInRangeCheck> searcher(gameObjects, check);
    Cell::VisitGridObjects(bot, searcher, range);
    return gameObjects;
}

std::vector<Player*> TbcRaidHelper::GetRaidMembers(Player* bot, float range, bool includeSelf)
{
    std::vector<Player*> members;
    Group* group = bot->GetGroup();
    if (!group)
    {
        if (includeSelf)
            members.push_back(bot);

        return members;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || !member->IsAlive() || !member->IsInWorld() || member->GetMapId() != bot->GetMapId())
            continue;

        if (member == bot)
        {
            if (includeSelf)
                members.push_back(member);

            continue;
        }

        if (member->GetInstanceId() != bot->GetInstanceId() || !bot->IsWithinDistInMap(member, range))
            continue;

        members.push_back(member);
    }

    return members;
}

bool TbcRaidHelper::IsCreatureCasting(Unit* unit, uint32 spellId)
{
    if (!unit)
        return false;

    const CurrentSpellTypes types[] = { CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL };
    for (CurrentSpellTypes type : types)
    {
        Spell* spell = unit->GetCurrentSpell(type);
        if (spell && spell->m_spellInfo && spell->m_spellInfo->Id == spellId)
            return true;
    }

    return false;
}

const Encounter* TbcRaidHelper::GetEncounter(PlayerbotAI* ai, Unit** boss)
{
    if (boss)
        *boss = nullptr;

    TbcRaidStrategy* strategy = TbcRaidStrategy::Get(ai);
    if (!strategy)
        return nullptr;

    Player* bot = ai->GetBot();
    RaidState& state = strategy->GetState();
    if (IsExpired(state.encounterCheckTime, ENCOUNTER_CACHE_MS))
    {
        state.encounterCheckTime = WorldTimer::getMSTime();
        state.encounter = nullptr;
        state.bossGuid = ObjectGuid();

        const EncounterRegistry& registry = EncounterRegistry::Instance();
        if (bot->IsInWorld() && !bot->IsBeingTeleported() && registry.IsRaidMap(bot->GetMapId()))
        {
            const std::list<ObjectGuid> attackers = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("attackers")->Get();
            for (const ObjectGuid& guid : attackers)
            {
                Unit* unit = ai->GetUnit(guid);
                if (!unit || !unit->IsAlive())
                    continue;

                const Encounter* encounter = registry.FindByCreature(bot->GetMapId(), unit->GetEntry());
                if (!encounter)
                    continue;

                state.encounter = encounter;
                if (unit->GetEntry() == encounter->entries.front())
                {
                    state.bossGuid = unit->GetObjectGuid();
                    break;
                }
            }

            // The boss itself may not be attacking yet (banished Magtheridon, submerged Lurker, ...)
            if (state.encounter && !state.bossGuid)
            {
                for (Unit* unit : FindCreatures(bot, state.encounter->entries.front(), 150.0f))
                {
                    if (unit->IsAlive())
                    {
                        state.bossGuid = unit->GetObjectGuid();
                        break;
                    }
                }
            }
        }

        // Force the other caches to be refreshed for the new state
        state.hazardCheckTime = 0;
        state.priorityCheckTime = 0;
        state.assignCheckTime = 0;

        // Announce the tactic once per pull in the raid chat (one bot of the group)
        if (state.encounter && state.encounter != state.announcedEncounter)
        {
            state.announcedEncounter = state.encounter;
            if (IsRaidLeaderBot(ai))
                ai->TellPlayerNoFacing(bot, DescribeTactic(ai, state.encounter), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        }
        else if (!state.encounter && !bot->IsInCombat())
        {
            state.announcedEncounter = nullptr;
        }
    }

    if (boss && state.bossGuid)
        *boss = ai->GetUnit(state.bossGuid);

    return state.encounter;
}

uint8 TbcRaidHelper::GetRole(PlayerbotAI* ai)
{
    return tbcraid::GetRole(ai, ai->GetBot());
}

bool TbcRaidHelper::IsMainTank(PlayerbotAI* ai, Unit* boss)
{
    return boss && boss->GetVictim() == ai->GetBot();
}

bool TbcRaidHelper::IsInHazard(const WorldPosition& point, const std::vector<HazardZone>& hazards, float margin)
{
    for (const HazardZone& hazard : hazards)
    {
        if (fabs(point.getZ() - hazard.position.getZ()) > 12.0f)
            continue;

        const float radius = hazard.radius + margin;
        if (point.sqDistance2d(hazard.position) < radius * radius)
            return true;
    }

    return false;
}

const std::vector<HazardZone>& TbcRaidHelper::GetHazards(PlayerbotAI* ai)
{
    static const std::vector<HazardZone> noHazards;

    TbcRaidStrategy* strategy = TbcRaidStrategy::Get(ai);
    if (!strategy)
        return noHazards;

    const Encounter* encounter = GetEncounter(ai);
    RaidState& state = strategy->GetState();
    if (!IsExpired(state.hazardCheckTime, HAZARD_CACHE_MS))
        return state.hazards;

    state.hazardCheckTime = WorldTimer::getMSTime();
    state.hazards.clear();

    Player* bot = ai->GetBot();
    if (!encounter || !bot->IsInWorld() || bot->IsBeingTeleported())
        return state.hazards;

    const uint8 role = GetRole(ai);
    const float searchExtra = 30.0f;

    for (const AvoidCreature& rule : encounter->avoidCreatures)
    {
        if (!HasRole(rule.roles, role))
            continue;

        for (Unit* unit : FindCreatures(bot, rule.entry, rule.radius + searchExtra))
        {
            if (!unit->IsAlive())
                continue;

            if (rule.requiredAura && !unit->HasAura(rule.requiredAura))
                continue;

            if (rule.requiredCast && !IsCreatureCasting(unit, rule.requiredCast))
                continue;

            if (rule.onlyIfTargetingMe)
            {
                if (unit->GetVictim() != bot)
                    continue;
            }
            // The tank of a creature keeps tanking it
            else if (role == ROLE_TANK && unit->GetVictim() == bot)
            {
                continue;
            }

            state.hazards.push_back({ WorldPosition(unit), rule.radius + unit->GetObjectBoundingRadius(), unit->GetObjectGuid() });
        }
    }

    for (const AvoidGameObject& rule : encounter->avoidObjects)
    {
        if (!HasRole(rule.roles, role))
            continue;

        for (GameObject* gameObject : FindGameObjects(bot, rule.entry, rule.radius + searchExtra))
        {
            state.hazards.push_back({ WorldPosition(gameObject), rule.radius, gameObject->GetObjectGuid() });
        }
    }

    if (!encounter->avoidAreas.empty())
    {
        std::list<DynamicObject*> dynamicObjects;
        AnyDynamicObjectInRangeCheck check(bot, 50.0f);
        MaNGOS::DynamicObjectListSearcher<AnyDynamicObjectInRangeCheck> searcher(dynamicObjects, check);
        Cell::VisitAllObjects(bot, searcher, 50.0f);

        for (DynamicObject* dynamicObject : dynamicObjects)
        {
            for (const AvoidAreaSpell& rule : encounter->avoidAreas)
            {
                if (dynamicObject->GetSpellId() != rule.spellId || !HasRole(rule.roles, role))
                    continue;

                // Ignore effects created by players (e.g. a paladin's own Consecration)
                Unit* caster = dynamicObject->GetCaster();
                if (caster && caster->GetTypeId() == TYPEID_PLAYER)
                    continue;

                const float radius = (rule.radius > 0.0f ? rule.radius : dynamicObject->GetRadius()) + 1.0f;
                state.hazards.push_back({ WorldPosition(dynamicObject), radius, ObjectGuid() });
            }
        }
    }

    return state.hazards;
}

Player* TbcRaidHelper::GetGatherTarget(PlayerbotAI* ai, float& distance)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || encounter->gather.empty())
        return nullptr;

    Player* bot = ai->GetBot();
    for (const GatherRule& rule : encounter->gather)
    {
        // The target itself is lifted into the air and can't move
        if (bot->HasAura(rule.spellId))
            return nullptr;

        for (Player* member : GetRaidMembers(bot, 100.0f))
        {
            if (member->HasAura(rule.spellId))
            {
                distance = rule.distance;
                return member;
            }
        }
    }

    return nullptr;
}

float TbcRaidHelper::GetRequiredSpread(PlayerbotAI* ai, bool* isDebuffRun)
{
    if (isDebuffRun)
        *isDebuffRun = false;

    Unit* boss = nullptr;
    const Encounter* encounter = GetEncounter(ai, &boss);
    if (!encounter)
        return 0.0f;

    Player* bot = ai->GetBot();
    float required = 0.0f;
    bool urgent = false;

    for (const DebuffRunRule& rule : encounter->debuffRun)
    {
        if (!bot->HasAura(rule.spellId))
            continue;

        if (rule.maxManaPct < 100.0f)
        {
            if (bot->GetPowerType() != POWER_MANA || bot->GetMaxPower(POWER_MANA) == 0)
                continue;

            const float manaPct = bot->GetPower(POWER_MANA) * 100.0f / bot->GetMaxPower(POWER_MANA);
            if (manaPct > rule.maxManaPct)
                continue;
        }

        required = std::max(required, rule.distance);
        urgent = true;
    }

    if (!urgent)
    {
        // Cube clickers stay at their cube
        if ((encounter->special & SPECIAL_MANTICRON_CUBES) && GetAssignedCube(ai))
            return 0.0f;

        // Stacking on a target has priority over spreading
        float gatherDistance = 0.0f;
        if (GetGatherTarget(ai, gatherDistance))
            return 0.0f;

        const uint8 role = GetRole(ai);
        const bool mainTank = IsMainTank(ai, boss);
        for (const SpreadRule& rule : encounter->spread)
        {
            if (!HasRole(rule.roles, role))
                continue;

            if (rule.requiredAuraOnSelf && !bot->HasAura(rule.requiredAuraOnSelf))
                continue;

            if (rule.requiredAuraOnBoss && (!boss || !boss->HasAura(rule.requiredAuraOnBoss)))
                continue;

            // The main tank is the anchor, everybody else moves away from him
            if (mainTank)
                continue;

            required = std::max(required, rule.distance);
            if (rule.requiredAuraOnSelf)
                urgent = true;
        }
    }

    if (isDebuffRun)
        *isDebuffRun = urgent;

    return required;
}

Unit* TbcRaidHelper::GetPriorityTarget(PlayerbotAI* ai)
{
    TbcRaidStrategy* strategy = TbcRaidStrategy::Get(ai);
    if (!strategy)
        return nullptr;

    const Encounter* encounter = GetEncounter(ai);
    RaidState& state = strategy->GetState();
    if (IsExpired(state.priorityCheckTime, PRIORITY_CACHE_MS))
    {
        state.priorityCheckTime = WorldTimer::getMSTime();
        state.priorityTarget = ObjectGuid();

        Player* bot = ai->GetBot();
        if (encounter && !encounter->priority.empty())
        {
            const uint8 role = GetRole(ai);
            for (const PriorityTarget& rule : encounter->priority)
            {
                if (!HasRole(rule.roles, role))
                    continue;

                if (rule.requiredAuraOnSelf && !bot->HasAura(rule.requiredAuraOnSelf))
                    continue;

                if (rule.excludedAuraOnSelf && bot->HasAura(rule.excludedAuraOnSelf))
                    continue;

                Unit* best = nullptr;
                for (Unit* unit : FindCreatures(bot, rule.entry, 70.0f))
                {
                    if (!unit->IsAlive())
                        continue;

                    if (rule.onlyIfTargetingMe && unit->GetVictim() != bot)
                        continue;

                    if (!AttackersValue::IsValid(unit, bot, nullptr, false))
                        continue;

                    // Focus the same target as the rest of the raid
                    if (!best || unit->GetHealthPercent() < best->GetHealthPercent() ||
                        (unit->GetHealthPercent() == best->GetHealthPercent() && unit->GetObjectGuid().GetCounter() < best->GetObjectGuid().GetCounter()))
                    {
                        best = unit;
                    }
                }

                if (best)
                {
                    state.priorityTarget = best->GetObjectGuid();
                    break;
                }
            }
        }
    }

    if (!state.priorityTarget)
        return nullptr;

    Unit* target = ai->GetUnit(state.priorityTarget);
    return (target && target->IsAlive()) ? target : nullptr;
}

Unit* TbcRaidHelper::GetTauntTarget(PlayerbotAI* ai)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || (encounter->tankSwap.empty() && encounter->formTank.empty()))
        return nullptr;

    Player* bot = ai->GetBot();
    if (GetRole(ai) != ROLE_TANK)
        return nullptr;

    for (const TankSwapRule& rule : encounter->tankSwap)
    {
        for (Unit* boss : FindCreatures(bot, rule.bossEntry, 40.0f))
        {
            if (!boss->IsAlive() || !boss->IsInCombat())
                continue;

            Unit* victim = boss->GetVictim();
            if (!victim || victim == bot || victim->GetTypeId() != TYPEID_PLAYER)
                continue;

            if (victim->GetAuraCount(rule.spellId) < rule.stacks)
                continue;

            // Only the first eligible off tank taunts
            Player* chosen = nullptr;
            for (Player* member : GetRaidMembers(bot, 60.0f, true))
            {
                if (member == victim || !PlayerbotAI::IsTank(member))
                    continue;

                if (member->GetAuraCount(rule.spellId) >= rule.stacks || !member->IsWithinDistInMap(boss, 40.0f))
                    continue;

                if (!chosen || member->GetObjectGuid().GetCounter() < chosen->GetObjectGuid().GetCounter())
                    chosen = member;
            }

            if (chosen == bot)
                return boss;
        }
    }

    for (const FormTankRule& rule : encounter->formTank)
    {
        for (Unit* boss : FindCreatures(bot, rule.bossEntry, 40.0f))
        {
            if (!boss->IsAlive() || !boss->IsInCombat() || boss->GetVictim() == bot)
                continue;

            std::vector<Player*> tanks;
            for (Player* member : GetRaidMembers(bot, 60.0f, true))
            {
                if (PlayerbotAI::IsTank(member))
                    tanks.push_back(member);
            }

            if (tanks.size() < 2)
                continue;

            std::sort(tanks.begin(), tanks.end(), [](Player* a, Player* b) { return a->GetObjectGuid().GetCounter() < b->GetObjectGuid().GetCounter(); });
            Player* assigned = boss->HasAura(rule.auraId) ? tanks[1] : tanks[0];
            if (assigned == bot)
                return boss;
        }
    }

    return nullptr;
}

bool TbcRaidHelper::ShouldHoldTaunt(PlayerbotAI* ai)
{
    Unit* boss = nullptr;
    const Encounter* encounter = GetEncounter(ai, &boss);
    if (!encounter)
        return false;

    Player* bot = ai->GetBot();

    // The off tanks stay second on threat, only the main tank taunts
    if ((encounter->special & SPECIAL_OFFTANK_NO_TAUNT) && boss && boss->IsAlive())
    {
        Unit* victim = boss->GetVictim();
        if (victim && victim != bot && victim->GetTypeId() == TYPEID_PLAYER && victim->IsAlive() && PlayerbotAI::IsTank((Player*)victim))
            return true;
    }

    if (encounter->tankSwap.empty() && encounter->formTank.empty())
        return false;
    for (const TankSwapRule& rule : encounter->tankSwap)
    {
        if (bot->GetAuraCount(rule.spellId) >= rule.stacks)
            return true;
    }

    for (const FormTankRule& rule : encounter->formTank)
    {
        for (Unit* boss : FindCreatures(bot, rule.bossEntry, 60.0f))
        {
            if (!boss->IsAlive())
                continue;

            std::vector<Player*> tanks;
            for (Player* member : GetRaidMembers(bot, 60.0f, true))
            {
                if (PlayerbotAI::IsTank(member))
                    tanks.push_back(member);
            }

            if (tanks.size() < 2)
                continue;

            std::sort(tanks.begin(), tanks.end(), [](Player* a, Player* b) { return a->GetObjectGuid().GetCounter() < b->GetObjectGuid().GetCounter(); });
            Player* assigned = boss->HasAura(rule.auraId) ? tanks[1] : tanks[0];
            if (assigned != bot)
                return true;
        }
    }

    return false;
}

Unit* TbcRaidHelper::GetFrontDanger(PlayerbotAI* ai)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || encounter->avoidFront.empty())
        return nullptr;

    Player* bot = ai->GetBot();
    const uint8 role = GetRole(ai);
    for (const AvoidFrontRule& rule : encounter->avoidFront)
    {
        if (!HasRole(rule.roles, role))
            continue;

        for (Unit* boss : FindCreatures(bot, rule.bossEntry, rule.range + 5.0f))
        {
            if (!boss->IsAlive() || !boss->IsInCombat() || boss->GetVictim() == bot)
                continue;

            if (!bot->IsWithinDistInMap(boss, rule.range))
                continue;

            // Soakers stand in front on purpose
            if (ShouldSoakFront(ai, boss))
                continue;

            // 120 degree cone in front of the boss
            if (boss->HasInArc(bot, M_PI_F * 2.0f / 3.0f))
                return boss;
        }
    }

    return nullptr;
}

bool TbcRaidHelper::IsCastingBlastNova(Unit* magtheridon)
{
    return IsCreatureCasting(magtheridon, SPELL_BLAST_NOVA);
}

GameObject* TbcRaidHelper::GetAssignedCube(PlayerbotAI* ai, Unit** magtheridon)
{
    if (magtheridon)
        *magtheridon = nullptr;

    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || !(encounter->special & SPECIAL_MANTICRON_CUBES))
        return nullptr;

    Player* bot = ai->GetBot();
    if (bot->HasAura(SPELL_MIND_EXHAUSTION) || PlayerbotAI::IsTank(bot))
        return nullptr;

    Unit* boss = nullptr;
    for (Unit* unit : FindCreatures(bot, NPC_MAGTHERIDON, 150.0f))
    {
        if (unit->IsAlive())
        {
            boss = unit;
            break;
        }
    }

    // Phase 1: Magtheridon is still banished by the channelers
    if (!boss || !boss->IsInCombat() || boss->HasAura(SPELL_SHADOW_CAGE_DUMMY))
        return nullptr;

    if (magtheridon)
        *magtheridon = boss;

    std::list<GameObject*> cubeList = FindGameObjects(bot, GO_MANTICRON_CUBE, 150.0f);
    if (cubeList.empty())
        return nullptr;

    std::vector<GameObject*> cubes(cubeList.begin(), cubeList.end());
    std::sort(cubes.begin(), cubes.end(), [](GameObject* a, GameObject* b) { return a->GetObjectGuid().GetCounter() < b->GetObjectGuid().GetCounter(); });

    // Every bot computes the same assignment: ranged first, then healers, then melee
    std::vector<std::pair<uint32, Player*>> clickers;
    for (Player* member : GetRaidMembers(bot, 150.0f, true))
    {
        if (!member->GetPlayerbotAI() || PlayerbotAI::IsTank(member) || member->HasAura(SPELL_MIND_EXHAUSTION))
            continue;

        const uint32 order = RoleSortOrder(tbcraid::GetRole(ai, member));
        clickers.push_back(std::make_pair(order, member));
    }

    std::sort(clickers.begin(), clickers.end(), [](const std::pair<uint32, Player*>& a, const std::pair<uint32, Player*>& b)
    {
        if (a.first != b.first)
            return a.first < b.first;

        return a.second->GetObjectGuid().GetCounter() < b.second->GetObjectGuid().GetCounter();
    });

    for (size_t i = 0; i < clickers.size() && i < cubes.size(); ++i)
    {
        if (clickers[i].second == bot)
            return cubes[i];
    }

    return nullptr;
}

GameObject* TbcRaidHelper::GetSpineToRemove(PlayerbotAI* ai)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || !(encounter->special & SPECIAL_NAJENTUS_SPINES))
        return nullptr;

    Player* bot = ai->GetBot();
    if (PlayerbotAI::IsTank(bot) || bot->HasAura(SPELL_IMPALING_SPINE) || bot->HasItemCount(ITEM_NAJENTUS_SPINE, 1))
        return nullptr;

    const std::vector<Player*> members = GetRaidMembers(bot, 80.0f, true);
    bool someoneImpaled = false;
    for (Player* member : members)
    {
        if (member->HasAura(SPELL_IMPALING_SPINE))
        {
            someoneImpaled = true;
            break;
        }
    }

    if (!someoneImpaled)
        return nullptr;

    for (GameObject* spine : FindGameObjects(bot, GO_IMPALING_SPINE, 60.0f))
    {
        // The closest free bot takes care of the spine
        Player* closest = nullptr;
        for (Player* member : members)
        {
            if (!member->GetPlayerbotAI() || PlayerbotAI::IsTank(member) || member->HasAura(SPELL_IMPALING_SPINE) || member->HasItemCount(ITEM_NAJENTUS_SPINE, 1))
                continue;

            if (!closest || member->GetDistance(spine) < closest->GetDistance(spine))
                closest = member;
        }

        if (closest == bot)
            return spine;
    }

    return nullptr;
}

Unit* TbcRaidHelper::GetShieldedNajentus(PlayerbotAI* ai)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || !(encounter->special & SPECIAL_NAJENTUS_SPINES))
        return nullptr;

    Player* bot = ai->GetBot();
    if (!bot->HasItemCount(ITEM_NAJENTUS_SPINE, 1))
        return nullptr;

    for (Unit* boss : FindCreatures(bot, NPC_NAJENTUS, 60.0f))
    {
        if (boss->IsAlive() && boss->HasAura(SPELL_TIDAL_SHIELD))
            return boss;
    }

    return nullptr;
}

// ---------------------------------------------------------------------------
// Tank assignments, interrupters, raid icons, tactic announcement
// ---------------------------------------------------------------------------

namespace
{
    const uint32 ASSIGN_CACHE_MS = 1000;

    // Interrupts by class (first known spell is used)
    const std::vector<std::string> interruptSpells = { "kick", "pummel", "shield bash", "earth shock", "counterspell", "silencing shot", "silence" };

    bool KnowsSpell(Player* player, const std::string& name)
    {
        for (uint32 spellId : ChatHelper::SpellIds(name))
        {
            if (player->HasSpell(spellId))
                return true;
        }

        return false;
    }

    bool KnowsInterrupt(Player* player)
    {
        for (const std::string& name : interruptSpells)
        {
            if (KnowsSpell(player, name))
                return true;
        }

        return false;
    }

    bool LowerGuid(Player* a, Player* b)
    {
        return a->GetObjectGuid().GetCounter() < b->GetObjectGuid().GetCounter();
    }

    std::string CreatureName(uint32 entry)
    {
        CreatureInfo const* info = sObjectMgr.GetCreatureTemplate(entry);
        return info ? info->Name : std::to_string(entry);
    }

    const char* IconName(uint8 icon)
    {
        static const char* names[] = { "Stern", "Kreis", "Diamant", "Dreieck", "Mond", "Viereck", "Kreuz", "Totenkopf" };
        return icon < 8 ? names[icon] : "?";
    }

    // Who holds which creature: computed the same way by every bot (sorted by guid)
    void ComputeAssignments(Player* bot, const Encounter* encounter, std::vector<Assignment>& assignments, std::vector<ObjectGuid>& interrupters)
    {
        assignments.clear();
        interrupters.clear();
        if (!encounter)
            return;

        std::vector<Player*> members = TbcRaidHelper::GetRaidMembers(bot, 150.0f, true);
        std::sort(members.begin(), members.end(), LowerGuid);

        std::vector<Player*> tanks;
        for (Player* member : members)
        {
            if (PlayerbotAI::IsTank(member))
                tanks.push_back(member);
        }

        std::vector<Player*> classHolders;
        std::vector<ObjectGuid> usedCreatures;
        size_t roundRobin = 0;
        for (const TankAssignment& rule : encounter->tankAssignments)
        {
            // several rules of the same entry take different creatures (lowest guid first)
            Unit* creature = nullptr;
            for (Unit* unit : TbcRaidHelper::GetSortedCreatures(bot, rule.entry, 150.0f))
            {
                if (std::find(usedCreatures.begin(), usedCreatures.end(), unit->GetObjectGuid()) == usedCreatures.end())
                {
                    creature = unit;
                    break;
                }
            }

            if (!creature)
                continue;

            usedCreatures.push_back(creature->GetObjectGuid());

            Player* holder = nullptr;
            if (rule.playerClass)
            {
                for (Player* member : members)
                {
                    if (member->getClass() == rule.playerClass && !PlayerbotAI::IsTank(member) &&
                        std::find(classHolders.begin(), classHolders.end(), member) == classHolders.end())
                    {
                        holder = member;
                        classHolders.push_back(member);
                        break;
                    }
                }
            }

            // Fewer tanks than slots: the last off tank takes the rest, a class tank falls back to it too
            if (!holder && !tanks.empty())
            {
                if (rule.slot == SLOT_ROUND_ROBIN && !rule.playerClass)
                    holder = tanks[roundRobin++ % tanks.size()];
                else
                {
                    const uint8 slot = rule.playerClass ? uint8(tanks.size() - 1) : rule.slot;
                    holder = tanks[std::min<size_t>(slot, tanks.size() - 1)];
                }
            }

            if (holder)
                assignments.push_back({ creature->GetObjectGuid(), holder->GetObjectGuid(), &rule });
        }

        for (const InterruptRule& rule : encounter->interrupts)
        {
            uint8 count = 0;
            for (Player* member : members)
            {
                if (count >= rule.count)
                    break;

                if (KnowsInterrupt(member) && std::find(interrupters.begin(), interrupters.end(), member->GetObjectGuid()) == interrupters.end())
                {
                    interrupters.push_back(member->GetObjectGuid());
                    count++;
                }
            }
        }
    }
}

const std::vector<Assignment>& TbcRaidHelper::GetAssignments(PlayerbotAI* ai)
{
    static const std::vector<Assignment> none;
    TbcRaidStrategy* strategy = TbcRaidStrategy::Get(ai);
    if (!strategy)
        return none;

    const Encounter* encounter = GetEncounter(ai);
    RaidState& state = strategy->GetState();
    if (IsExpired(state.assignCheckTime, ASSIGN_CACHE_MS))
    {
        state.assignCheckTime = WorldTimer::getMSTime();
        ComputeAssignments(ai->GetBot(), encounter, state.assignments, state.interrupters);
    }

    return state.assignments;
}

std::vector<std::pair<Unit*, const TankAssignment*>> TbcRaidHelper::GetMyAssignments(PlayerbotAI* ai)
{
    std::vector<std::pair<Unit*, const TankAssignment*>> mine;
    for (const Assignment& assignment : GetAssignments(ai))
    {
        if (assignment.holder != ai->GetBot()->GetObjectGuid())
            continue;

        Unit* creature = ai->GetUnit(assignment.creature);
        if (creature && creature->IsAlive())
            mine.push_back({ creature, assignment.rule });
    }

    return mine;
}

bool TbcRaidHelper::IsAssignedToMe(PlayerbotAI* ai, Unit* creature)
{
    if (!creature)
        return false;

    for (auto& mine : GetMyAssignments(ai))
    {
        if (mine.first == creature)
            return true;
    }

    return false;
}

bool TbcRaidHelper::GetAssignmentSpot(Unit* creature, const TankAssignment& rule, WorldPosition& spot)
{
    if (!creature || creature->GetTypeId() != TYPEID_UNIT)
        return false;

    float x, y, z, o;
    ((Creature*)creature)->GetRespawnCoord(x, y, z, &o);
    spot = WorldPosition(creature->GetMapId(), x, y, z);

    if (rule.forward == 0.0f && rule.side == 0.0f)
        return true;

    // Shorten the offset until the spot is on the ground and visible from the spawn point (walls, ledges)
    Map* map = creature->GetMap();
    for (float scale : { 1.0f, 0.7f, 0.4f })
    {
        const float px = x + scale * (rule.forward * cos(o) + rule.side * cos(o + M_PI_F / 2));
        const float py = y + scale * (rule.forward * sin(o) + rule.side * sin(o + M_PI_F / 2));
        const float pz = map->GetHeight(px, py, z + 5.0f);
        if (pz <= INVALID_HEIGHT || fabs(pz - z) > 6.0f)
            continue;

        if (!map->IsInLineOfSight(x, y, z + 2.0f, px, py, pz + 2.0f, true))
            continue;

        spot = WorldPosition(creature->GetMapId(), px, py, pz);
        return true;
    }

    return true;
}

bool TbcRaidHelper::IsInterrupter(PlayerbotAI* ai, Player* member)
{
    GetAssignments(ai);
    TbcRaidStrategy* strategy = TbcRaidStrategy::Get(ai);
    if (!strategy || !member)
        return false;

    const std::vector<ObjectGuid>& interrupters = strategy->GetState().interrupters;
    return std::find(interrupters.begin(), interrupters.end(), member->GetObjectGuid()) != interrupters.end();
}

Unit* TbcRaidHelper::GetInterruptTarget(PlayerbotAI* ai, std::string& spellName)
{
    const Encounter* encounter = GetEncounter(ai);
    Player* bot = ai->GetBot();
    if (!encounter || encounter->interrupts.empty() || !IsInterrupter(ai, bot))
        return nullptr;

    for (const InterruptRule& rule : encounter->interrupts)
    {
        for (Unit* creature : FindCreatures(bot, rule.entry, 40.0f))
        {
            if (!creature->IsAlive())
                continue;

            bool casting = false;
            for (uint32 spellId : rule.spells)
            {
                if (IsCreatureCasting(creature, spellId))
                {
                    casting = true;
                    break;
                }
            }

            if (!casting)
                continue;

            for (const std::string& name : interruptSpells)
            {
                if (KnowsSpell(bot, name) && ai->CanCastSpell(name, creature, 0))
                {
                    spellName = name;
                    return creature;
                }
            }
        }
    }

    return nullptr;
}

std::vector<Unit*> TbcRaidHelper::GetSortedCreatures(Player* bot, uint32 entry, float range)
{
    std::vector<Unit*> creatures;
    for (Unit* unit : FindCreatures(bot, entry, range))
    {
        // banished / not yet active creatures (Magtheridon in phase 1) are nobody's job
        if (!unit->IsAlive() || unit->IsImmuneToPlayer() || unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE))
            continue;

        creatures.push_back(unit);
    }

    std::sort(creatures.begin(), creatures.end(), [](Unit* a, Unit* b) { return a->GetObjectGuid().GetCounter() < b->GetObjectGuid().GetCounter(); });
    return creatures;
}

Unit* TbcRaidHelper::GetMarkTarget(PlayerbotAI* ai, const Encounter* encounter, size_t markIndex)
{
    if (!encounter || markIndex >= encounter->marks.size())
        return nullptr;

    // n-th mark of this entry -> n-th living creature of this entry
    const uint32 entry = encounter->marks[markIndex].entry;
    size_t nth = 0;
    for (size_t i = 0; i < markIndex; ++i)
    {
        if (encounter->marks[i].entry == entry)
            nth++;
    }

    std::vector<Unit*> creatures = GetSortedCreatures(ai->GetBot(), entry, 100.0f);
    return nth < creatures.size() ? creatures[nth] : nullptr;
}

bool TbcRaidHelper::IsRaidLeaderBot(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* leader = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId() || !member->GetPlayerbotAI() || member->GetPlayerbotAI()->IsRealPlayer())
            continue;

        if (!leader || LowerGuid(member, leader))
            leader = member;
    }

    return leader == bot;
}

std::string TbcRaidHelper::DescribeTactic(PlayerbotAI* ai, const Encounter* encounter)
{
    if (!encounter)
        return "Keine Boss-Taktik aktiv.";

    std::ostringstream out;
    out << "[Taktik] " << CreatureName(encounter->entries.front()) << " geladen.";

    std::vector<uint32> order;
    for (const PriorityTarget& rule : encounter->priority)
    {
        if (std::find(order.begin(), order.end(), rule.entry) == order.end())
            order.push_back(rule.entry);
    }

    if (!order.empty())
    {
        out << " Reihenfolge:";
        for (size_t i = 0; i < order.size(); ++i)
        {
            out << (i ? " >" : "") << " " << CreatureName(order[i]);
            for (const MarkRule& mark : encounter->marks)
            {
                if (mark.entry == order[i])
                    out << " (" << IconName(mark.icon) << ")";
            }
        }
        out << ".";
    }

    std::vector<Assignment> assignments;
    std::vector<ObjectGuid> interrupters;
    ComputeAssignments(ai->GetBot(), encounter, assignments, interrupters);

    if (!encounter->tankAssignments.empty())
    {
        out << " Tanks:";
        if (assignments.empty())
            out << " keine Tanks gefunden";

        for (size_t i = 0; i < assignments.size(); ++i)
        {
            Player* holder = sObjectMgr.GetPlayer(assignments[i].holder);
            out << (i ? "," : "") << " " << (holder ? holder->GetName() : "?") << " -> " << CreatureName(assignments[i].rule->entry);
        }
        out << ".";
    }

    if (!encounter->interrupts.empty())
    {
        out << " Unterbrecher:";
        if (interrupters.empty())
            out << " keine";

        for (size_t i = 0; i < interrupters.size(); ++i)
        {
            Player* member = sObjectMgr.GetPlayer(interrupters[i]);
            out << (i ? "," : "") << " " << (member ? member->GetName() : "?");
        }
        out << ".";
    }

    if (encounter->lustBelowHealthPct < 100.0f)
        out << " Kampfrausch unter " << uint32(encounter->lustBelowHealthPct) << "%.";

    if (!encounter->resistanceAura.empty())
        out << " Aura: " << encounter->resistanceAura << ".";

    return out.str();
}

std::string TbcRaidHelper::DescribeMyTask(PlayerbotAI* ai)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter)
        encounter = EncounterRegistry::Instance().FindNearby(ai);

    if (!encounter)
        return std::string("tbc raid ") + (ai->HasStrategy("tbc raid", BotState::BOT_STATE_COMBAT) ? "aktiv" : "AUS") + ", kein Boss in der Naehe.";

    std::vector<Assignment> assignments;
    std::vector<ObjectGuid> interrupters;
    ComputeAssignments(ai->GetBot(), encounter, assignments, interrupters);

    static const char* roleNames[] = { "", "Tank", "Heiler", "", "Nahkampf", "", "", "", "Fernkampf" };
    const uint8 role = GetRole(ai);

    std::ostringstream out;
    out << CreatureName(encounter->entries.front()) << ": " << (role < 9 ? roleNames[role] : "?");

    for (const Assignment& assignment : assignments)
    {
        if (assignment.holder == ai->GetBot()->GetObjectGuid())
            out << ", halte " << CreatureName(assignment.rule->entry);
    }

    if (std::find(interrupters.begin(), interrupters.end(), ai->GetBot()->GetObjectGuid()) != interrupters.end())
        out << ", unterbreche " << CreatureName(encounter->interrupts.front().entry);

    if (Unit* priority = GetPriorityTarget(ai))
        out << ", Ziel " << priority->GetName();

    if (!ai->HasStrategy("tbc raid", BotState::BOT_STATE_COMBAT))
        out << " (tbc raid AUS!)";

    return out.str();
}