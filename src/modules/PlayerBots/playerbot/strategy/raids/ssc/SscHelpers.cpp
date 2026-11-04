// Ported from mod-playerbots (src/Ai/Raid/SSC/SSCHelpers.cpp, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "SscHelpers.h"
#include "SscHydrossRules.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/Action.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
#include <unordered_map>

using namespace ai;

namespace ai
{
namespace ssc
{

// Hydross the Unstable <Duke of Currents>
// Frost spot is inside the Cleansing Field circle around the helper at -239.7, -366.4, nature spot outside.
// The boss leaves the field at 28 yards and trails a tank by about 8 yards (SscHydrossRules.h): the nature spot
// lies 41 yards out. Spots at 30 and 36 yards left the boss in water form behind the tank in the raid logs.
const WorldPosition HYDROSS_FROST_TANK_POSITION(SSC_MAP_ID, -235.2f, -354.2f, -1.2f);
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

namespace
{
    // Latched Karathress roles per instance: mob entry -> tank. The raid log of 2026-10-30 had nine tank
    // deaths; recomputing the roles from the living tanks swapped every role on each death and revive.
    std::mutex karathressRolesMutex;
    std::map<uint32, std::map<uint32, ObjectGuid>> karathressRoles;
    // Council member guid -> first moment it was seen on a non-tank (dps hold)
    std::map<uint32, std::map<ObjectGuid, time_t>> karathressLooseSince;
    // Resolved roles per instance for a short time: every tank and dps action asks for them
    struct KarathressRolesCache
    {
        uint32 time = 0;
        std::map<uint32, ObjectGuid> tanks;
    };
    std::map<uint32, KarathressRolesCache> karathressRolesCache;

    // Assist tank order: Caribdis is separated (Cyclones, heals), Tidalvess is the first dps target and
    // needs its own threat, Sharkkis dies last of the three and stays with Karathress when tanks are short.
    const uint32 KARATHRESS_ASSIST_ORDER[] = { NPC_CARIBDIS, NPC_TIDALVESS, NPC_SHARKKIS };

    Player* FindTank(const std::vector<Player*>& tanks, const ObjectGuid& guid)
    {
        for (Player* tank : tanks)
        {
            if (tank->GetObjectGuid() == guid)
                return tank;
        }

        return nullptr;
    }

    // Resolves the tank of every living mob. Callers hold karathressRolesMutex.
    std::map<uint32, Player*> ResolveKarathressTanks(PlayerbotAI* ai)
    {
        Player* bot = ai->GetBot();
        const std::vector<Player*> tanks = raidboss::GetGroupTanks(bot);   // living tanks only
        std::map<uint32, ObjectGuid>& latched = karathressRoles[bot->GetInstanceId()];
        std::map<uint32, Player*> result;

        std::vector<uint32> mobs;
        std::map<uint32, Unit*> units;
        for (uint32 entry : { uint32(NPC_KARATHRESS), uint32(NPC_CARIBDIS), uint32(NPC_TIDALVESS), uint32(NPC_SHARKKIS) })
        {
            Unit* unit = FindSscUnit(ai, entry);
            if (unit && unit->IsAlive())
            {
                mobs.push_back(entry);
                units[entry] = unit;
            }
        }

        auto isUsed = [&](Player* tank)
        {
            for (const auto& entry : result)
            {
                if (entry.second == tank)
                    return true;
            }

            return false;
        };

        // A tank latched to a living mob keeps it while he lives and is close: a tank revived at the
        // entrance takes his mob back from the stand-in only once he got there
        for (uint32 entry : mobs)
        {
            auto it = latched.find(entry);
            if (it != latched.end())
                if (Player* tank = FindTank(tanks, it->second))
                    if (!isUsed(tank) && tank->IsWithinDistInMap(units[entry], 45.0f))
                        result[entry] = tank;
        }

        // Unused living tank in group order; Caribdis prefers a warrior or druid (Shield Bash for Healing Wave)
        auto takeFreeTank = [&](bool preferInterrupt) -> Player*
        {
            Player* found = nullptr;
            for (Player* tank : tanks)
            {
                if (isUsed(tank))
                    continue;

                if (!found || (preferInterrupt && found->getClass() == CLASS_PALADIN && tank->getClass() != CLASS_PALADIN))
                    found = tank;
            }

            return found;
        };

        // Karathress first, then the council members in assist order. A mob whose latched tank is dead gets
        // a stand-in that is not latched: the tank gets his mob back on revive.
        std::vector<uint32> order = { NPC_KARATHRESS };
        order.insert(order.end(), std::begin(KARATHRESS_ASSIST_ORDER), std::end(KARATHRESS_ASSIST_ORDER));
        for (uint32 entry : order)
        {
            if (std::find(mobs.begin(), mobs.end(), entry) == mobs.end() || result.count(entry))
                continue;

            Player* tank = takeFreeTank(entry == NPC_CARIBDIS);
            if (!tank)
                break;

            result[entry] = tank;
            ObjectGuid& role = latched[entry];
            if (role.IsEmpty())
                role = tank->GetObjectGuid();
        }

        // Members without a tank stay with the Karathress tank. Without one, a tank other than Caribdis' holds
        // them (Caribdis must stay in her corner), Sharkkis' or Tidalvess' tank first.
        Player* holder = result.count(NPC_KARATHRESS) ? result[NPC_KARATHRESS] : nullptr;
        for (uint32 entry : { uint32(NPC_SHARKKIS), uint32(NPC_TIDALVESS) })
        {
            if (!holder && result.count(entry))
                holder = result[entry];
        }
        for (Player* tank : tanks)
        {
            if (!holder && (!result.count(NPC_CARIBDIS) || result[NPC_CARIBDIS] != tank))
                holder = tank;
        }
        if (!holder && !tanks.empty())
            holder = tanks.front();

        for (uint32 entry : mobs)
        {
            if (!result.count(entry) && holder)
                result[entry] = holder;
        }

        return result;
    }

    // Resolved roles, reused for 250 ms. Callers hold karathressRolesMutex.
    std::map<uint32, Player*> GetKarathressTanks(PlayerbotAI* ai)
    {
        Player* bot = ai->GetBot();
        const uint32 now = WorldTimer::getMSTime();
        KarathressRolesCache& cache = karathressRolesCache[bot->GetInstanceId()];
        std::map<uint32, Player*> result;
        if (cache.time && now >= cache.time && now - cache.time < 250)
        {
            bool valid = true;
            for (const auto& entry : cache.tanks)
            {
                Player* tank = bot->GetMap()->GetPlayer(entry.second);
                if (!tank || !tank->IsAlive())
                {
                    valid = false;
                    break;
                }

                result[entry.first] = tank;
            }

            if (valid)
                return result;
        }

        result = ResolveKarathressTanks(ai);
        cache.time = now;
        cache.tanks.clear();
        for (const auto& entry : result)
            cache.tanks[entry.first] = entry.second->GetObjectGuid();

        return result;
    }
}

Player* GetKarathressTank(Player* bot, uint32 entry)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    if (!ai)
        return nullptr;

    std::lock_guard<std::mutex> lock(karathressRolesMutex);
    const std::map<uint32, Player*> tanks = GetKarathressTanks(ai);
    auto it = tanks.find(entry);
    return it != tanks.end() ? it->second : nullptr;
}

std::vector<Unit*> GetKarathressTankUnits(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    std::map<uint32, Player*> tanks;
    {
        std::lock_guard<std::mutex> lock(karathressRolesMutex);
        tanks = GetKarathressTanks(ai);
    }

    std::vector<Unit*> units;
    for (uint32 entry : { uint32(NPC_KARATHRESS), uint32(NPC_TIDALVESS), uint32(NPC_CARIBDIS), uint32(NPC_SHARKKIS) })
    {
        auto it = tanks.find(entry);
        if (it == tanks.end() || it->second != bot)
            continue;

        if (Unit* unit = FindSscUnit(ai, entry))
            units.push_back(unit);
    }

    return units;
}

void ResetKarathressRoles(Player* bot)
{
    std::lock_guard<std::mutex> lock(karathressRolesMutex);
    karathressRoles.erase(bot->GetInstanceId());
    karathressLooseSince.erase(bot->GetInstanceId());
    karathressRolesCache.erase(bot->GetInstanceId());
}

bool ShouldHoldKarathressDps(PlayerbotAI* ai, Unit* target)
{
    if (!target || !target->IsAlive())
        return false;

    const uint32 entry = target->GetEntry();
    if (entry != NPC_KARATHRESS && entry != NPC_CARIBDIS && entry != NPC_TIDALVESS && entry != NPC_SHARKKIS)
        return false;

    Player* bot = ai->GetBot();
    Unit* victim = target->GetVictim();
    const bool loose = victim && victim->GetTypeId() == TYPEID_PLAYER && !PlayerbotAI::IsTank(static_cast<Player*>(victim));

    std::lock_guard<std::mutex> lock(karathressRolesMutex);
    std::map<ObjectGuid, time_t>& looseSince = karathressLooseSince[bot->GetInstanceId()];
    if (!loose)
    {
        looseSince.erase(target->GetObjectGuid());
        return false;
    }

    // Only while the mob's tank is alive and close enough to take it back
    const std::map<uint32, Player*> tanks = GetKarathressTanks(ai);
    auto it = tanks.find(entry);
    if (it == tanks.end() || !it->second || !it->second->IsWithinDistInMap(target, 40.0f))
        return false;

    const time_t now = time(nullptr);
    time_t& since = looseSince[target->GetObjectGuid()];
    if (!since || now - since > 60)
        since = now;

    // A taunt that misses must not stop the dps for good
    return now - since < 5;
}

Unit* GetCaribdisInterruptTarget(PlayerbotAI* ai, std::string& spell)
{
    Player* bot = ai->GetBot();
    if (bot->GetMapId() != SSC_MAP_ID || !bot->IsInCombat())
        return nullptr;

    std::vector<std::string> interrupts;
    switch (bot->getClass())
    {
    case CLASS_ROGUE:   interrupts = { "kick" }; break;
    case CLASS_WARRIOR: interrupts = { "shield bash", "pummel" }; break;
    case CLASS_MAGE:    interrupts = { "counterspell" }; break;
    case CLASS_SHAMAN:  interrupts = { "earth shock" }; break;
    default: return nullptr;
    }

    Unit* caribdis = FindSscUnit(ai, NPC_CARIBDIS);
    if (!caribdis || !caribdis->IsInCombat() || !caribdis->FindCurrentSpellBySpellId(SPELL_CARIBDIS_HEALING_WAVE))
        return nullptr;

    for (const std::string& interrupt : interrupts)
    {
        if (ai->IsInterruptableSpellCasting(caribdis, interrupt) && ai->CanCastSpell(interrupt, caribdis, 0))
        {
            spell = interrupt;
            return caribdis;
        }
    }

    return nullptr;
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
        state.leotherasWhirlwindEnd || state.leotherasWhirlwindActive || !state.leotherasDemonTank.IsEmpty() ||
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

bool GetRangedSpreadPosition(Player* bot, Unit* anchor, float spreadDistance, float minAnchorDistance,
                             float maxAnchorDistance, WorldPosition& position)
{
    // Minimize the overlap with the whole group: moving away from only the nearest player
    // can move straight into the next one.
    const std::vector<Player*> members = raidboss::GetGroupMembers(bot, 40.0f, false);
    const auto exposure = [&members, spreadDistance](float px, float py)
    {
        float score = 0.0f;
        for (Player* member : members)
        {
            const float overlap = std::max(0.0f, spreadDistance - raidboss::ExactDist2d(member, px, py));
            score += overlap * overlap;
        }
        return score;
    };

    const float currentScore = exposure(bot->GetPositionX(), bot->GetPositionY());
    if (currentScore <= 0.0f)
        return false;

    constexpr unsigned candidates = 16;
    constexpr float stepDistance = 5.0f;
    float bestScore = currentScore * 0.75f;  // hysteresis: no shuffling for small gains
    bool found = false;
    for (unsigned i = 0; i < candidates; ++i)
    {
        const float angle = i * 2.0f * M_PI_F / candidates;
        const float px = bot->GetPositionX() + std::cos(angle) * stepDistance;
        const float py = bot->GetPositionY() + std::sin(angle) * stepDistance;
        if (anchor)
        {
            const float anchorDistance = raidboss::ExactDist2d(anchor, px, py);
            if (anchorDistance < minAnchorDistance || anchorDistance > maxAnchorDistance)
                continue;
        }

        const float score = exposure(px, py);
        if (score >= bestScore)
            continue;

        float x, y, z;
        if (!raidboss::CanTakeStepTowards(bot, px, py, stepDistance, x, y, z))
            continue;

        bestScore = score;
        position = WorldPosition(bot->GetMapId(), x, y, z);
        found = true;
    }

    return found;
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

namespace
{
    // The boss counts as leaderless when no tank held him for this long (dead or resurrected tank)
    constexpr time_t HYDROSS_LEADERLESS_SECONDS = 4;

    std::mutex hydrossTankedMutex;
    std::map<uint32, time_t> hydrossLastTanked;   // by instance id

    bool IsHydrossOnTank(Unit* hydross)
    {
        Unit* victim = hydross->GetVictim();
        return victim && victim->GetTypeId() == TYPEID_PLAYER && PlayerbotAI::IsTank(static_cast<Player*>(victim));
    }
}

namespace
{
    // Frost and nature tank of the current pull per instance. A role only moves on when its tank dies:
    // re-ranking every tick gave a resurrected tank his role back and took the boss off his replacement.
    std::mutex hydrossRolesMutex;
    std::map<uint32, std::pair<ObjectGuid, ObjectGuid>> hydrossRoles;   // instance id -> frost, nature

    Player* GetLivingTank(const std::vector<Player*>& tanks, const ObjectGuid& guid)
    {
        for (Player* tank : tanks)
        {
            if (tank->GetObjectGuid() == guid)
                return tank;
        }

        return nullptr;
    }

    // The frost tank pulls inside the Cleansing Field, a warrior gets frost whenever there is one: with a
    // paladin frost tank the boss stayed on the extra tank and the warlocks for 40 seconds in the raid log.
    // Callers hold hydrossRolesMutex.
    void UpdateHydrossRoles(Player* bot, Player*& frost, Player*& nature)
    {
        const std::vector<Player*> tanks = raidboss::GetGroupTanks(bot);
        std::pair<ObjectGuid, ObjectGuid>& roles = hydrossRoles[bot->GetInstanceId()];
        frost = roles.first.IsEmpty() ? nullptr : GetLivingTank(tanks, roles.first);
        nature = roles.second.IsEmpty() ? nullptr : GetLivingTank(tanks, roles.second);

        if (!frost)
        {
            for (Player* tank : tanks)
            {
                if (tank != nature && tank->getClass() == CLASS_WARRIOR)
                {
                    frost = tank;
                    break;
                }
            }

            for (Player* tank : tanks)
            {
                if (!frost && tank != nature)
                    frost = tank;
            }
        }

        if (!nature)
        {
            for (Player* tank : tanks)
            {
                if (!nature && tank != frost)
                    nature = tank;
            }
        }

        roles.first = frost ? frost->GetObjectGuid() : ObjectGuid();
        roles.second = nature ? nature->GetObjectGuid() : ObjectGuid();

        // The last living tank takes both forms
        if (!frost)
            frost = nature;
        if (!nature)
            nature = frost;
    }
}

Player* GetHydrossFrostTank(Player* bot)
{
    std::lock_guard<std::mutex> lock(hydrossRolesMutex);
    Player* frost = nullptr;
    Player* nature = nullptr;
    UpdateHydrossRoles(bot, frost, nature);
    return frost;
}

Player* GetHydrossNatureTank(Player* bot)
{
    std::lock_guard<std::mutex> lock(hydrossRolesMutex);
    Player* frost = nullptr;
    Player* nature = nullptr;
    UpdateHydrossRoles(bot, frost, nature);
    return nature;
}

// No tank held Hydross for a few seconds: the additional tank may take him
bool IsHydrossLeaderless(Unit* hydross)
{
    if (!hydross)
        return false;

    // Out of combat (wipe, reset): the next pull starts with a fresh grace period
    if (!hydross->IsInCombat())
    {
        std::lock_guard<std::mutex> lock(hydrossTankedMutex);
        hydrossLastTanked.erase(hydross->GetInstanceId());
        return false;
    }

    const time_t now = time(nullptr);
    std::lock_guard<std::mutex> lock(hydrossTankedMutex);
    time_t& lastTanked = hydrossLastTanked[hydross->GetInstanceId()];
    if (IsHydrossOnTank(hydross) || !lastTanked || now - lastTanked > 600)
    {
        lastTanked = now;
        return false;
    }

    return now - lastTanked >= HYDROSS_LEADERLESS_SECONDS;
}

namespace
{
    // Seconds since a tank held Hydross, 0 while a tank has him
    time_t HydrossSecondsWithoutTank(Unit* hydross)
    {
        IsHydrossLeaderless(hydross);
        std::lock_guard<std::mutex> lock(hydrossTankedMutex);
        auto lastTanked = hydrossLastTanked.find(hydross->GetInstanceId());
        if (lastTanked == hydrossLastTanked.end() || IsHydrossOnTank(hydross))
            return 0;

        return time(nullptr) - lastTanked->second;
    }
}

bool UpdateHydrossState(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    Unit* hydross = FindSscUnit(ai, NPC_HYDROSS);
    if (!hydross || !hydross->IsInCombat())
        return false;

    const bool nature = hydross->HasAura(SPELL_CORRUPTION);
    Player* tank = nature ? GetHydrossNatureTank(bot) : GetHydrossFrostTank(bot);
    const bool marked = tank && (nature ? HasMarkOfCorruptionAt100Percent(tank) : HasMarkOfHydrossAt100Percent(tank));
    const time_t now = time(nullptr);
    bool firstUpdate = false;
    bool changed = false;
    WithState(bot, [&](SscInstanceState& state)
    {
        firstUpdate = !state.hydrossFrostDpsWait && !state.hydrossNatureDpsWait;
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
        // Hand off by time; the active tank's 100% mark only matters if the timer was missed.
        // Ignore marks left over from the previous visit to this form.
        if ((hydross::IsHandoffTime(now - started) || (marked && now - started >= 30)) && !transition)
        {
            transition = now;
            changed = true;
        }
    });

    // New pull: roles and the "without tank" timer start fresh, nothing carries over from a wipe
    if (firstUpdate)
    {
        {
            std::lock_guard<std::mutex> lock(hydrossRolesMutex);
            hydrossRoles.erase(hydross->GetInstanceId());
        }
        std::lock_guard<std::mutex> lock(hydrossTankedMutex);
        hydrossLastTanked[hydross->GetInstanceId()] = now;
    }

    IsHydrossLeaderless(hydross);
    return changed;
}

bool ShouldPauseHydrossDps(PlayerbotAI* ai)
{
    Unit* hydross = FindSscUnit(ai, NPC_HYDROSS);
    if (!hydross || !hydross->IsInCombat())
        return false;

    UpdateHydrossState(ai);
    const bool nature = hydross->HasAura(SPELL_CORRUPTION);
    const SscInstanceState state = GetState(ai->GetBot());
    const time_t started = nature ? state.hydrossNatureDpsWait : state.hydrossFrostDpsWait;
    const time_t transition = nature ? state.hydrossChangeToFrostPhase : state.hydrossChangeToNaturePhase;
    const time_t now = time(nullptr);

    // The tank needs a few seconds to drag the boss over the boundary
    if (transition)
        return now - transition < 10;

    // The boss chases a damage dealer (dead or resurrected tank): stop feeding his threat so the tank's
    // taunt sticks. In the 2.4.3 raid log a taunted boss went straight back to the warlocks for 35 seconds.
    const time_t withoutTank = HydrossSecondsWithoutTank(hydross);
    if (withoutTank >= 1 && withoutTank <= 8)
        return true;

    if (!started || now - started < 5)
        return true;

    // Threat was wiped by the form change: wait until the tank of the new form holds the boss (the tank who
    // dragged him has no threat left), but never stall the raid for longer than 15 seconds (a dead or out of
    // range tank used to pause the dps for good)
    Unit* victim = hydross->GetVictim();
    Player* formTank = nature ? GetHydrossNatureTank(ai->GetBot()) : GetHydrossFrostTank(ai->GetBot());
    const bool tankHasBoss = victim && (victim == formTank || (!formTank && victim->GetTypeId() == TYPEID_PLAYER &&
                                                                  PlayerbotAI::IsTank(static_cast<Player*>(victim))));
    return !tankHasBoss && now - started < 15;
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
// (2) If no assistant Warlock, the Warlock bot with the most Fire Resistance (Chaos Blast is fire
//     damage), then the most health. The choice is kept for the fight while that Warlock lives.
Player* GetLeotherasDemonFormTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    const auto isCandidate = [bot, group](Player* member)
    {
        return member && member->IsAlive() && member->IsInWorld() && member->getClass() == CLASS_WARLOCK &&
               member->GetMapId() == bot->GetMapId() && group->IsMember(member->GetObjectGuid());
    };

    const ObjectGuid chosen = GetState(bot).leotherasDemonTank;
    if (!chosen.IsEmpty())
    {
        Player* member = sObjectMgr.GetPlayer(chosen);
        if (isCandidate(member))
            return member;
    }

    Player* tank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!isCandidate(member))
            continue;

        if (group->IsAssistant(member->GetObjectGuid()))
        {
            tank = member;
            break;
        }

        if (!member->GetPlayerbotAI())
            continue;

        const int32 fireResistance = member->GetResistance(SPELL_SCHOOL_FIRE);
        if (!tank || fireResistance > tank->GetResistance(SPELL_SCHOOL_FIRE) ||
            (fireResistance == tank->GetResistance(SPELL_SCHOOL_FIRE) && member->GetMaxHealth() > tank->GetMaxHealth()))
            tank = member;
    }

    if (tank && bot->IsInCombat())
        WithState(bot, [tank](SscInstanceState& state) { state.leotherasDemonTank = tank->GetObjectGuid(); });

    return tank;
}

Player* GetLeotherasMindControlledPlayer(Player* bot)
{
    Player* nearest = nullptr;
    float nearestDistance = 0.0f;
    for (Player* member : raidboss::GetGroupMembers(bot, 60.0f, false))
    {
        if (!member->IsAlive() || !member->HasAura(SPELL_CONSUMING_MADNESS))
            continue;

        const float distance = raidboss::ExactDist2d(bot, member);
        if (!nearest || distance < nearestDistance)
        {
            nearest = member;
            nearestDistance = distance;
        }
    }

    return nearest;
}

bool IsLeotherasWhirlwindAggroReset(Player* bot)
{
    // Long enough for the tank (and a Misdirection) to build a threat lead again
    constexpr time_t aggroResetSeconds = 4;
    const time_t end = GetState(bot).leotherasWhirlwindEnd;
    return end && time(nullptr) - end < aggroResetSeconds;
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
