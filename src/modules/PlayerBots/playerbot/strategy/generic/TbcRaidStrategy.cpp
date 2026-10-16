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
    if (bot->HasAura(SPELL_SHADOW_GRASP) && action->getName().rfind("magtheridon", 0) != 0)
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
