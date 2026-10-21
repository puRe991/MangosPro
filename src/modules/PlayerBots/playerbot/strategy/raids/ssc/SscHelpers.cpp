// Ported from mod-playerbots (src/Ai/Raid/SSC/SSCHelpers.cpp, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "SscHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/Action.h"
#include <algorithm>
#include <cmath>
#include <mutex>
#include <unordered_map>

using namespace ai;

namespace ai
{
namespace ssc
{

// Hydross the Unstable <Duke of Currents>
// Frost spot is inside the Cleansing Field circle around his spawn, nature spot outside
const WorldPosition HYDROSS_FROST_TANK_POSITION(SSC_MAP_ID, -236.669f, -358.352f, -0.828f);
const WorldPosition HYDROSS_NATURE_TANK_POSITION(SSC_MAP_ID, -225.471f, -327.790f, -3.682f);

// The Lurker Below (spawn of this core: 38.46, -417.32)
const WorldPosition LURKER_MAIN_TANK_POSITION(SSC_MAP_ID, 23.706f, -406.038f, -19.686f);

// Fathom-Lord Karathress
const WorldPosition KARATHRESS_TANK_POSITION(SSC_MAP_ID, 474.403f, -531.118f, -7.548f);
const WorldPosition TIDALVESS_TANK_POSITION(SSC_MAP_ID, 511.282f, -501.162f, -13.158f);
const WorldPosition SHARKKIS_TANK_POSITION(SSC_MAP_ID, 508.057f, -541.109f, -10.133f);
const WorldPosition CARIBDIS_TANK_POSITION(SSC_MAP_ID, 464.462f, -475.820f, -13.158f);
const WorldPosition CARIBDIS_HEALER_POSITION(SSC_MAP_ID, 466.203f, -503.201f, -13.158f);
const WorldPosition CARIBDIS_RANGED_DPS_POSITION(SSC_MAP_ID, 463.197f, -501.190f, -13.158f);

// Morogrim Tidewalker
const WorldPosition TIDEWALKER_PHASE_1_TANK_POSITION(SSC_MAP_ID, 410.925f, -741.916f, -7.146f);
const WorldPosition TIDEWALKER_PHASE_TRANSITION_WAYPOINT(SSC_MAP_ID, 407.035f, -759.479f, -7.168f);
const WorldPosition TIDEWALKER_PHASE_2_TANK_POSITION(SSC_MAP_ID, 446.571f, -767.155f, -7.144f);
const WorldPosition TIDEWALKER_PHASE_2_RANGED_POSITION(SSC_MAP_ID, 432.595f, -766.288f, -7.145f);

// Lady Vashj (her phase 2 point in this core: 29.83, -923.27, 42.90)
const WorldPosition VASHJ_PLATFORM_CENTER_POSITION(SSC_MAP_ID, 29.634f, -923.541f, VASHJ_PLATFORM_CENTER_Z);

namespace
{
    std::mutex stateMutex;
    std::unordered_map<uint32, SscInstanceState> states;

    bool HasAnyAura(Unit* unit, std::initializer_list<uint32> auras)
    {
        for (uint32 aura : auras)
        {
            if (unit->HasAura(aura))
                return true;
        }

        return false;
    }

    std::vector<Player*> AssistantsFirst(Player* bot, std::vector<Player*> players)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return players;

        std::stable_sort(players.begin(), players.end(), [group](Player* a, Player* b)
        {
            return group->IsAssistant(a->GetObjectGuid()) && !group->IsAssistant(b->GetObjectGuid());
        });

        return players;
    }

    bool Contains(const std::array<Player*, 5>& players, Player* player)
    {
        return std::find(players.begin(), players.end(), player) != players.end();
    }
}

void WithState(Player* bot, const std::function<void(SscInstanceState&)>& function)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    function(states[bot->GetInstanceId()]);
}

SscInstanceState GetState(Player* bot)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    return states[bot->GetInstanceId()];
}

bool NeedsStateReset(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    if (bot->GetMapId() != SSC_MAP_ID || bot->IsInCombat())
        return false;

    const SscInstanceState state = GetState(bot);
    const bool dirty = state.hydrossFrostDpsWait || state.hydrossNatureDpsWait ||
        state.hydrossChangeToFrostPhase || state.hydrossChangeToNaturePhase || state.lurkerSpoutEnd ||
        state.leotherasHumanFormDpsWait || state.leotherasDemonFormDpsWait || state.leotherasFinalPhaseDpsWait ||
        state.karathressDpsWait || state.lastCoreThrow || !state.vashjGenerator.IsEmpty() ||
        !state.lurkerRangedPositions.empty() || !state.tidewalkerTankStep.empty() ||
        !state.tidewalkerRangedStep.empty() || !state.hasReachedVashjRangedPosition.empty() ||
        !state.intendedLineup.empty() || !state.lastCoreInInventoryTime.empty();
    if (!dirty)
        return false;

    // A resurrected or distant bot must not erase the shared state of a live pull.
    if (Group* group = bot->GetGroup())
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->getSource())
                if (member->IsInWorld() && member->GetMap() == bot->GetMap() && member->IsInCombat())
                    return false;

    for (uint32 entry : { NPC_HYDROSS, NPC_LURKER_BELOW, NPC_LEOTHERAS, NPC_KARATHRESS, NPC_MOROGRIM, NPC_LADY_VASHJ })
        if (Unit* boss = FindSscUnit(ai, entry))
            if (boss->IsInCombat())
                return false;

    return true;
}

Unit* FindSscUnit(PlayerbotAI* ai, uint32 entry, float range)
{
    struct CacheEntry
    {
        ObjectGuid guid;
        uint32 time = 0;
    };

    // The bot is always updated by the thread of its map, a thread local cache needs no lock
    thread_local std::unordered_map<uint64, CacheEntry> cache;

    Player* bot = ai->GetBot();
    const uint64 key = (uint64(bot->GetObjectGuid().GetCounter()) << 32) | entry;
    const uint32 now = WorldTimer::getMSTime();

    auto it = cache.find(key);
    if (it != cache.end() && now >= it->second.time && now - it->second.time < 500)
    {
        if (it->second.guid.IsEmpty())
            return nullptr;

        Unit* unit = ai->GetUnit(it->second.guid);
        if (unit && unit->IsInWorld() && unit->IsAlive())
            return unit;
    }

    Unit* unit = raidboss::FindUnit(ai, entry, range);
    CacheEntry& stored = cache[key];
    stored.guid = unit ? unit->GetObjectGuid() : ObjectGuid();
    stored.time = now;
    return unit;
}

std::vector<Player*> GetHealersAssistantsFirst(Player* bot)
{
    return AssistantsFirst(bot, raidboss::GetGroupHealers(bot));
}

std::vector<Player*> GetRangedDpsAssistantsFirst(Player* bot)
{
    return AssistantsFirst(bot, raidboss::GetGroupRangedDps(bot));
}

Player* GetHealerOfIndex(Player* bot, uint8 index)
{
    std::vector<Player*> healers = GetHealersAssistantsFirst(bot);
    return index < healers.size() ? healers[index] : nullptr;
}

bool StopAttacking(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    bool stopped = false;

    if (bot->GetVictim())
    {
        bot->AttackStop();
        stopped = true;
    }

    if (bot->IsNonMeleeSpellCasted(false, false, true))
    {
        bot->InterruptNonMeleeSpells(true);
        stopped = true;
    }

    if (Pet* pet = bot->GetPet())
    {
        if (pet->GetVictim())
            pet->AttackStop();
    }

    Value<ObjectGuid>* currentTarget = ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target");
    if (!currentTarget->Get().IsEmpty())
    {
        currentTarget->Set(ObjectGuid());
        stopped = true;
    }

    return stopped;
}

WorldPosition GetPointAwayFrom(Player* bot, float fromX, float fromY, float distance)
{
    float dx = bot->GetPositionX() - fromX;
    float dy = bot->GetPositionY() - fromY;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length < 0.1f)
    {
        const float angle = frand(0.0f, 2.0f * M_PI_F);
        dx = std::cos(angle);
        dy = std::sin(angle);
        length = 1.0f;
    }

    const float x = fromX + dx / length * distance;
    const float y = fromY + dy / length * distance;
    float z = bot->GetMap()->GetHeight(x, y, bot->GetPositionZ() + 5.0f);
    if (z <= INVALID_HEIGHT || std::fabs(z - bot->GetPositionZ()) > 10.0f)
        z = bot->GetPositionZ();

    return WorldPosition(bot->GetMapId(), x, y, z);
}

WorldPosition GetStepTowards(Player* bot, const WorldPosition& position, float maxStep)
{
    const float distance = raidboss::ExactDist2d(bot, position.getX(), position.getY());
    if (distance <= maxStep || distance < 0.1f)
        return position;

    const float ratio = maxStep / distance;
    const float x = bot->GetPositionX() + (position.getX() - bot->GetPositionX()) * ratio;
    const float y = bot->GetPositionY() + (position.getY() - bot->GetPositionY()) * ratio;
    return WorldPosition(position.getMapId(), x, y, position.getZ());
}

bool IsActionNamed(Action* action, std::initializer_list<const char*> names)
{
    if (!action)
        return false;

    const std::string& name = action->getName();
    for (const char* n : names)
    {
        if (name == n)
            return true;
    }

    return false;
}

bool IsTbcRaidAction(Action* action)
{
    return action && action->getName().rfind("tbc raid", 0) == 0;
}

// Hydross the Unstable

bool HasMarkOfHydrossAt100Percent(Player* bot)
{
    return HasAnyAura(bot, { SPELL_MARK_OF_HYDROSS_100, SPELL_MARK_OF_HYDROSS_250, SPELL_MARK_OF_HYDROSS_500 });
}

bool HasNoMarkOfHydross(Player* bot)
{
    return !HasAnyAura(bot, { SPELL_MARK_OF_HYDROSS_10, SPELL_MARK_OF_HYDROSS_25, SPELL_MARK_OF_HYDROSS_50,
                              SPELL_MARK_OF_HYDROSS_100, SPELL_MARK_OF_HYDROSS_250, SPELL_MARK_OF_HYDROSS_500 });
}

bool HasMarkOfCorruptionAt100Percent(Player* bot)
{
    return HasAnyAura(bot, { SPELL_MARK_OF_CORRUPTION_100, SPELL_MARK_OF_CORRUPTION_250, SPELL_MARK_OF_CORRUPTION_500 });
}

bool HasNoMarkOfCorruption(Player* bot)
{
    return !HasAnyAura(bot, { SPELL_MARK_OF_CORRUPTION_10, SPELL_MARK_OF_CORRUPTION_25, SPELL_MARK_OF_CORRUPTION_50,
                              SPELL_MARK_OF_CORRUPTION_100, SPELL_MARK_OF_CORRUPTION_250, SPELL_MARK_OF_CORRUPTION_500 });
}

bool UpdateHydrossState(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    Unit* hydross = FindSscUnit(ai, NPC_HYDROSS);
    if (!hydross || !hydross->IsInCombat())
        return false;

    const bool nature = hydross->HasAura(SPELL_CORRUPTION);
    Player* tank = nature ? raidboss::GetGroupAssistTank(bot, 0) : raidboss::GetGroupMainTank(bot);
    const bool marked = tank && (nature ? HasMarkOfCorruptionAt100Percent(tank) : HasMarkOfHydrossAt100Percent(tank));
    const time_t now = time(nullptr);
    bool changed = false;
    WithState(bot, [&](SscInstanceState& state)
    {
        time_t& started = nature ? state.hydrossNatureDpsWait : state.hydrossFrostDpsWait;
        time_t& previous = nature ? state.hydrossFrostDpsWait : state.hydrossNatureDpsWait;
        time_t& transition = nature ? state.hydrossChangeToFrostPhase : state.hydrossChangeToNaturePhase;
        time_t& previousTransition = nature ? state.hydrossChangeToNaturePhase : state.hydrossChangeToFrostPhase;
        if (!started || previous)
        {
            started = now;
            previous = previousTransition = transition = 0;
            changed = true;
        }
        // Read the active tank's marks, not the tracker bot's (which may be out of aura range).
        // Ignore marks left over from the previous visit to this form.
        if (marked && now - started >= 30 && !transition)
        {
            transition = now;
            changed = true;
        }
    });
    return changed;
}

bool ShouldPauseHydrossDps(PlayerbotAI* ai)
{
    Unit* hydross = FindSscUnit(ai, NPC_HYDROSS);
    if (!hydross || !hydross->IsInCombat())
        return false;

    UpdateHydrossState(ai);
    Player* bot = ai->GetBot();
    const bool nature = hydross->HasAura(SPELL_CORRUPTION);
    Player* tank = nature ? raidboss::GetGroupAssistTank(bot, 0) : raidboss::GetGroupMainTank(bot);
    const SscInstanceState state = GetState(bot);
    const time_t started = nature ? state.hydrossNatureDpsWait : state.hydrossFrostDpsWait;
    const time_t transition = nature ? state.hydrossChangeToFrostPhase : state.hydrossChangeToNaturePhase;
    return transition || !started || time(nullptr) - started < 5 || !tank || hydross->GetVictim() != tank;
}

// The Lurker Below
// This core casts Spout (37431) and then turns the boss with the 37429 / 37430 auras for 16 seconds
bool IsLurkerSpouting(Unit* lurker)
{
    if (!lurker)
        return false;

    if (lurker->HasAura(SPELL_SPOUT_LEFT) || lurker->HasAura(SPELL_SPOUT_RIGHT))
        return true;

    Spell* spell = lurker->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return spell && spell->m_spellInfo && spell->m_spellInfo->Id == SPELL_SPOUT;
}

// Submerge sets UNIT_STAND_STATE_CUSTOM in this core (UNIT_STAND_STATE_SUBMERGED of AzerothCore)
bool IsLurkerSubmerged(Unit* lurker)
{
    return lurker && (lurker->HasAura(SPELL_LURKER_SUBMERGE) || lurker->getStandState() == UNIT_STAND_STATE_CUSTOM);
}

// Leotheras the Blind

Unit* GetLeotherasHuman(PlayerbotAI* ai)
{
    Unit* leotheras = FindSscUnit(ai, NPC_LEOTHERAS, 100.0f);
    if (leotheras && leotheras->IsInCombat() && !leotheras->HasAura(SPELL_METAMORPHOSIS))
        return leotheras;

    return nullptr;
}

Unit* GetPhase2LeotherasDemon(PlayerbotAI* ai)
{
    Unit* leotheras = FindSscUnit(ai, NPC_LEOTHERAS, 100.0f);
    if (leotheras && leotheras->HasAura(SPELL_METAMORPHOSIS))
        return leotheras;

    return nullptr;
}

Unit* GetPhase3LeotherasDemon(PlayerbotAI* ai)
{
    return FindSscUnit(ai, NPC_SHADOW_OF_LEOTHERAS, 100.0f);
}

Unit* GetActiveLeotherasDemon(PlayerbotAI* ai)
{
    Unit* phase2 = GetPhase2LeotherasDemon(ai);
    return phase2 ? phase2 : GetPhase3LeotherasDemon(ai);
}

// (1) First priority is an assistant Warlock (real player or bot)
// (2) If no assistant Warlock, then look for any Warlock bot
Player* GetLeotherasDemonFormTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* fallbackWarlock = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_WARLOCK || member->GetMapId() != bot->GetMapId())
            continue;

        if (group->IsAssistant(member->GetObjectGuid()))
            return member;

        if (!fallbackWarlock && member->GetPlayerbotAI())
            fallbackWarlock = member;
    }

    return fallbackWarlock;
}

// The Inner Demon summoned for the bot (only its summoner can damage it)
Unit* GetMyInnerDemon(Player* bot)
{
    if (!bot->HasAura(SPELL_INSIDIOUS_WHISPER)) return nullptr;
    for (Unit* unit : raidboss::FindUnits(bot, NPC_INNER_DEMON, 80.0f))
    {
        if (unit->GetSpawnerGuid() == bot->GetObjectGuid())
            return unit;
    }

    return nullptr;
}

bool CanUseLeotherasMovement(PlayerbotAI* ai, const std::string& action)
{
    Player* bot = ai->GetBot();
    if (!bot->IsInWorld() || bot->IsBeingTeleported() || bot->GetMapId() != SSC_MAP_ID || !bot->IsInCombat()) return true;
    if (GetMyInnerDemon(bot))
        return action == "leotheras the blind destroy inner demon" || action == "reach melee" || action == "reach spell";
    Unit* boss = GetLeotherasHuman(ai);
    if (!boss || PlayerbotAI::IsTank(bot) || (!boss->HasAura(SPELL_WHIRLWIND) && !boss->HasAura(SPELL_WHIRLWIND_CHANNEL))) return true;
    return action == "leotheras the blind run away from whirlwind";
}

// Lady Vashj <Coilfang Matron>

bool IsLadyVashjInPhase1(PlayerbotAI* ai)
{
    Unit* vashj = FindSscUnit(ai, NPC_LADY_VASHJ);
    return vashj && vashj->GetHealthPercent() > 70.0f;
}

// Phase 2 starts with her walk to the platform center (point movement), then she is pacified
// and shielded by the Magic Barrier until the 4 generators are used
bool IsLadyVashjInPhase2(PlayerbotAI* ai)
{
    Unit* vashj = FindSscUnit(ai, NPC_LADY_VASHJ);
    if (!vashj || vashj->GetHealthPercent() > 70.0f)
        return false;

    return vashj->HasAura(SPELL_MAGIC_BARRIER) || vashj->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED) ||
           vashj->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE;
}

bool IsLadyVashjInPhase3(PlayerbotAI* ai)
{
    Unit* vashj = FindSscUnit(ai, NPC_LADY_VASHJ);
    return vashj && vashj->GetHealthPercent() <= 70.0f && !IsLadyVashjInPhase2(ai);
}

bool IsValidLadyVashjCombatNpc(Unit* unit, PlayerbotAI* ai)
{
    if (!unit || !unit->IsAlive())
        return false;

    const uint32 entry = unit->GetEntry();
    if (IsLadyVashjInPhase2(ai))
    {
        return entry == NPC_TAINTED_ELEMENTAL || entry == NPC_ENCHANTED_ELEMENTAL ||
               entry == NPC_COILFANG_ELITE || entry == NPC_COILFANG_STRIDER;
    }

    if (IsLadyVashjInPhase3(ai))
    {
        return entry == NPC_TAINTED_ELEMENTAL || entry == NPC_ENCHANTED_ELEMENTAL ||
               entry == NPC_COILFANG_ELITE || entry == NPC_COILFANG_STRIDER ||
               entry == NPC_TOXIC_SPOREBAT || entry == NPC_LADY_VASHJ;
    }

    return false;
}

bool IsMainTankInSameSubgroup(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !group->IsRaidGroup())
        return false;

    Player* mainTank = raidboss::GetGroupMainTank(bot);
    return mainTank && mainTank != bot && group->SameSubGroup(bot, mainTank);
}

bool HasTaintedCore(Player* player)
{
    return player && player->HasItemCount(ITEM_TAINTED_CORE, 1);
}

// Designated looter: (1) assistant melee DPS bot, (2) other melee DPS bot, (3) ranged DPS bot,
// (4) the group leader. Passers: healer bots of index 0, 1, 2 and the ranged DPS bot of index 0
// (assistants first), any other non tank bot as fallback.
std::array<Player*, 5> GetCoreHandlers(Player* bot)
{
    std::array<Player*, 5> handlers = { nullptr, nullptr, nullptr, nullptr, nullptr };
    Group* group = bot->GetGroup();
    if (!group)
        return handlers;

    std::vector<Player*> members = raidboss::GetGroupMembers(bot, 500.0f, true);

    Player* meleeDpsAssistant = nullptr;
    Player* meleeDps = nullptr;
    Player* rangedDps = nullptr;
    for (Player* member : members)
    {
        if (!member->GetPlayerbotAI() || !raidboss::IsDps(member))
            continue;

        if (raidboss::IsMeleePlayer(member))
        {
            if (!meleeDpsAssistant && group->IsAssistant(member->GetObjectGuid()))
                meleeDpsAssistant = member;

            if (!meleeDps)
                meleeDps = member;
        }
        else if (!rangedDps)
            rangedDps = member;
    }

    handlers[0] = meleeDpsAssistant ? meleeDpsAssistant : (meleeDps ? meleeDps : rangedDps);
    if (!handlers[0])
        handlers[0] = sObjectMgr.GetPlayer(group->GetLeaderGuid());

    auto fallback = [&]() -> Player*
    {
        for (Player* member : members)
        {
            if (member->GetPlayerbotAI() && !PlayerbotAI::IsTank(member) && !Contains(handlers, member))
                return member;
        }

        return nullptr;
    };

    std::vector<Player*> healers;
    for (Player* healer : GetHealersAssistantsFirst(bot))
    {
        if (healer->GetPlayerbotAI() && healer != handlers[0])
            healers.push_back(healer);
    }

    for (uint8 i = 0; i < 3; ++i)
        handlers[i + 1] = i < healers.size() ? healers[i] : fallback();

    for (Player* ranged : GetRangedDpsAssistantsFirst(bot))
    {
        if (ranged->GetPlayerbotAI() && !Contains(handlers, ranged))
        {
            handlers[4] = ranged;
            break;
        }
    }

    if (!handlers[4])
        handlers[4] = fallback();

    return handlers;
}

int GetCoreHandlerIndex(Player* bot)
{
    const std::array<Player*, 5> handlers = GetCoreHandlers(bot);
    for (int i = 0; i < 5; ++i)
    {
        if (handlers[i] == bot)
            return i;
    }

    return -1;
}

// Checks if any bot from earlier in the passing sequence has the Tainted Core or
// had it within the prior 3 seconds so the chain is not broken when the Core is in transit
bool AnyRecentCoreInInventory(Player* bot)
{
    const std::array<Player*, 5> handlers = GetCoreHandlers(bot);
    int myIndex = -1;
    for (int i = 0; i < 5; ++i)
    {
        if (handlers[i] == bot)
            myIndex = i;
    }

    if (myIndex < 0)
        return false;

    const SscInstanceState state = GetState(bot);
    const time_t now = time(nullptr);
    for (int i = 0; i <= myIndex; ++i)
    {
        Player* handler = handlers[i];
        if (!handler)
            continue;

        if (HasTaintedCore(handler))
            return true;

        auto it = state.lastCoreInInventoryTime.find(handler->GetObjectGuid());
        if (it != state.lastCoreInInventoryTime.end() && now - it->second <= 3)
            return true;
    }

    return false;
}

// A generator is active while its Tiny Trigger channels the Magic Barrier on Vashj
// (the mod-playerbots version uses AzerothCore spawn ids and World Invisible Triggers)
GameObject* GetNearestActiveShieldGenerator(Player* bot, WorldObject* from)
{
    GameObject* nearest = nullptr;
    for (uint32 entry = GO_SHIELD_GENERATOR_1; entry <= GO_SHIELD_GENERATOR_4; ++entry)
    {
        for (GameObject* generator : raidboss::FindGameObjects(bot, entry, 200.0f))
        {
            if (generator->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT))
                continue;

            bool active = false;
            for (Unit* trigger : raidboss::FindUnits(bot, NPC_TINY_TRIGGER, 200.0f))
            {
                if (generator->IsWithinDist2d(trigger->GetPositionX(), trigger->GetPositionY(), 6.0f) && trigger->IsNonMeleeSpellCasted(false))
                {
                    active = true;
                    break;
                }
            }

            if (!active)
                continue;

            if (!nearest || from->GetDistance(generator) < from->GetDistance(nearest))
                nearest = generator;
        }
    }

    return nearest;
}

// When Toxic Sporebats spit poison, they summon "Spore Drop Trigger" NPCs that create the toxic pools
std::vector<Unit*> GetSporeDropTriggers(Player* bot)
{
    return raidboss::FindUnits(bot, NPC_SPORE_DROP_TRIGGER, 50.0f);
}

}
}
