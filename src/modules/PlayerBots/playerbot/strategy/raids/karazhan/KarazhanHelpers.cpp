// Ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara/KaraHelpers) and adapted for this core.
#include "playerbot/playerbot.h"
#include "KarazhanHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/generic/TbcRaidStrategy.h"
#include "playerbot/strategy/values/PossibleTargetsValue.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <mutex>

using namespace ai;

namespace
{
    // Timers and beam blockers are shared by all bots of an instance. Instances can be updated by
    // different map threads, so the access is serialized.
    std::mutex karazhanStateMutex;
    std::map<std::pair<uint32, uint8>, time_t> karazhanTimers;

    struct BlockerState
    {
        ObjectGuid red;
        ObjectGuid green;
        ObjectGuid blue;
    };
    std::map<uint32, BlockerState> karazhanBlockers;

    Player* PickSticky(const std::vector<Player*>& candidates, ObjectGuid& current)
    {
        if (candidates.empty())
        {
            current = ObjectGuid();
            return nullptr;
        }

        for (Player* player : candidates)
        {
            if (player->GetObjectGuid() == current)
                return player;
        }

        current = candidates.front()->GetObjectGuid();
        return candidates.front();
    }

    uint32 GetAuraStacks(Unit* unit, uint32 spellId)
    {
        SpellAuraHolder* holder = unit->GetSpellAuraHolder(spellId);
        return holder ? holder->GetStackAmount() : 0;
    }

    bool IsKarazhanBot(Player* member)
    {
        return member && member->IsAlive() && member->IsInWorld() && member->GetMapId() == karazhan::KARAZHAN_MAP_ID &&
               member->GetPlayerbotAI();
    }
}

// ---------------------------------------------------------------------------
// General
// ---------------------------------------------------------------------------

bool karazhan::IsSafePosition(float x, float y, const std::vector<Unit*>& hazards, float hazardRadius)
{
    for (Unit* hazard : hazards)
    {
        if (raidboss::ExactDist2d(hazard, x, y) < hazardRadius)
            return false;
    }

    return true;
}

bool karazhan::GetValidGroundPoint(Player* bot, float x, float y, float& z)
{
    const float botZ = bot->GetPositionZ();
    float groundZ = bot->GetMap()->GetHeight(x, y, botZ + 5.0f);
    if (groundZ <= INVALID_HEIGHT || std::fabs(groundZ - botZ) > 6.0f)
        return false;

    if (!bot->GetMap()->IsInLineOfSight(bot->GetPositionX(), bot->GetPositionY(), botZ + 2.0f, x, y, groundZ + 2.0f, true))
        return false;

    z = groundZ;
    return true;
}

void karazhan::StopCasting(Player* bot)
{
    if (bot->IsNonMeleeSpellCasted(true))
        bot->InterruptNonMeleeSpells(true);
}

bool karazhan::IsTrackedEntry(uint32 entry)
{
    switch (entry)
    {
    case NPC_MANA_WARP: case NPC_SPECTRAL_CHARGER:
    case NPC_ATTUMEN: case NPC_MIDNIGHT: case NPC_ATTUMEN_MOUNTED:
    case NPC_MOROES: case NPC_LADY_KEIRA_BERRYBUCK: case NPC_LADY_CATRIONA_VON_INDI: case NPC_LORD_CRISPIN_FERENCE:
    case NPC_BARON_RAFE_DREUGER: case NPC_BARONESS_DOROTHEA_MILLSTIPE: case NPC_LORD_ROBIN_DARIS:
    case NPC_MAIDEN_OF_VIRTUE:
    case NPC_BIG_BAD_WOLF: case NPC_ROMULO: case NPC_JULIANNE: case NPC_DOROTHEE: case NPC_TITO: case NPC_ROAR:
    case NPC_STRAWMAN: case NPC_TINHEAD: case NPC_CRONE:
    case NPC_CURATOR: case NPC_ASTRAL_FLARE_1: case NPC_ASTRAL_FLARE_2: case NPC_ASTRAL_FLARE_3: case NPC_ASTRAL_FLARE_4:
    case NPC_TERESTIAN_ILLHOOF: case NPC_DEMON_CHAINS: case NPC_KILREK: case NPC_FIENDISH_IMP:
    case NPC_SHADE_OF_ARAN: case NPC_CONJURED_ELEMENTAL:
    case NPC_NETHERSPITE: case NPC_VOID_ZONE: case NPC_GREEN_PORTAL: case NPC_BLUE_PORTAL: case NPC_RED_PORTAL:
    case NPC_PRINCE_MALCHEZAAR: case NPC_NETHERSPITE_INFERNAL:
    case NPC_NIGHTBANE: case NPC_RESTLESS_SKELETON:
        return true;
    default:
        return false;
    }
}

std::vector<Unit*> karazhan::GetKaraUnits(PlayerbotAI* ai, uint32 entry)
{
    std::vector<Unit*> units;
    Player* bot = ai->GetBot();
    if (bot->GetMapId() != KARAZHAN_MAP_ID)
        return units;

    const std::list<ObjectGuid> guids = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("karazhan nearby creatures")->Get();
    for (const ObjectGuid& guid : guids)
    {
        if (guid.GetEntry() != entry)
            continue;

        Unit* unit = ai->GetUnit(guid);
        if (unit && unit->IsInWorld() && unit->IsAlive() && unit->GetMapId() == bot->GetMapId())
            units.push_back(unit);
    }

    // Environmental summons can be absent from the cached GUID list (and
    // need not be ordinary attackable/alive combat targets). Resolve them by
    // entry directly before concluding that a beam or hazard does not exist.
    const bool portal = entry == NPC_RED_PORTAL || entry == NPC_GREEN_PORTAL || entry == NPC_BLUE_PORTAL;
    if (units.empty() && (portal || entry == NPC_VOID_ZONE || entry == NPC_NETHERSPITE_INFERNAL))
        for (Unit* unit : TbcRaidHelper::FindCreatures(bot, entry, 120.0f))
            if (unit && unit->IsInWorld() && unit->GetInstanceId() == bot->GetInstanceId() && (portal || unit->IsAlive()))
                units.push_back(unit);
    return units;
}

Unit* karazhan::GetKaraUnit(PlayerbotAI* ai, uint32 entry)
{
    Player* bot = ai->GetBot();
    Unit* nearest = nullptr;
    for (Unit* unit : GetKaraUnits(ai, entry))
    {
        if (!nearest || raidboss::ExactDist2d(bot, unit) < raidboss::ExactDist2d(bot, nearest))
            nearest = unit;
    }

    return nearest;
}

bool karazhan::IsBotCurrentTarget(PlayerbotAI* ai, Unit* unit)
{
    return unit && ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Get() == unit->GetObjectGuid();
}

bool karazhan::IsLiveDemonChains(PlayerbotAI* ai, Unit* target)
{
    Player* bot = ai->GetBot();
    return bot->IsInWorld() && bot->GetMapId() == KARAZHAN_MAP_ID && bot->IsInCombat() &&
        target && target->GetEntry() == NPC_DEMON_CHAINS && target->IsAlive() && target->IsInWorld() &&
        target->GetInstanceId() == bot->GetInstanceId() &&
        PossibleTargetsValue::IsValid(target, bot) && GetKaraUnit(ai, NPC_TERESTIAN_ILLHOOF);
}

Unit* karazhan::GetIllhoofTarget(PlayerbotAI* ai, bool tank)
{
    Player* bot = ai->GetBot();
    if (!bot->IsInWorld() || bot->GetMapId() != KARAZHAN_MAP_ID || !bot->IsInCombat())
        return nullptr;
    Unit* boss = GetKaraUnit(ai, NPC_TERESTIAN_ILLHOOF);
    if (!boss)
        return nullptr;
    if (tank)
    {
        const auto tanks = raidboss::GetGroupTanks(bot);
        if (tanks.empty() || tanks.front() == bot)
            return boss;
    }
    else if (Unit* chains = GetKaraUnit(ai, NPC_DEMON_CHAINS))
        if (IsLiveDemonChains(ai, chains))
            return chains;
    if (Unit* kilrek = GetKaraUnit(ai, NPC_KILREK))
        if (!ai->HasAura("banish", kilrek))
            return kilrek;
    return boss;
}

// ---------------------------------------------------------------------------
// Timers
// ---------------------------------------------------------------------------

bool karazhan::GetTimer(uint32 instanceId, KaraTimer timer, time_t& start)
{
    std::lock_guard<std::mutex> lock(karazhanStateMutex);
    auto it = karazhanTimers.find({ instanceId, uint8(timer) });
    if (it == karazhanTimers.end())
        return false;

    start = it->second;
    return true;
}

bool karazhan::StartTimer(uint32 instanceId, KaraTimer timer, time_t now)
{
    std::lock_guard<std::mutex> lock(karazhanStateMutex);
    return karazhanTimers.emplace(std::make_pair(instanceId, uint8(timer)), now).second;
}

bool karazhan::ClearTimer(uint32 instanceId, KaraTimer timer)
{
    std::lock_guard<std::mutex> lock(karazhanStateMutex);
    return karazhanTimers.erase({ instanceId, uint8(timer) }) > 0;
}

int64 karazhan::GetTimerElapsed(uint32 instanceId, KaraTimer timer)
{
    time_t start = 0;
    if (!GetTimer(instanceId, timer, start))
        return -1;

    return int64(std::time(nullptr) - start);
}

// ---------------------------------------------------------------------------
// Attumen the Huntsman
// ---------------------------------------------------------------------------

Unit* karazhan::GetAttumenMounted(PlayerbotAI* ai)
{
    return GetKaraUnit(ai, NPC_ATTUMEN_MOUNTED);
}

bool karazhan::IsAttumenEncounterActive(PlayerbotAI* ai)
{
    return GetKaraUnit(ai, NPC_MIDNIGHT) || GetKaraUnit(ai, NPC_ATTUMEN_MOUNTED);
}

// ---------------------------------------------------------------------------
// The Big Bad Wolf
// ---------------------------------------------------------------------------

bool karazhan::HasLittleRedRidingHood(Player* bot)
{
    return bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD) || bot->HasAura(SPELL_RED_RIDING_HOOD_FIXATE) ||
           bot->HasAura(SPELL_RED_RIDING_HOOD_TRANSFORM);
}

// ---------------------------------------------------------------------------
// The Curator
// ---------------------------------------------------------------------------

Unit* karazhan::GetAstralFlare(PlayerbotAI* ai)
{
    for (uint32 entry : { NPC_ASTRAL_FLARE_1, NPC_ASTRAL_FLARE_2, NPC_ASTRAL_FLARE_3, NPC_ASTRAL_FLARE_4 })
    {
        if (Unit* flare = GetKaraUnit(ai, entry))
            return flare;
    }

    return nullptr;
}

// ---------------------------------------------------------------------------
// Shade of Aran
// ---------------------------------------------------------------------------

bool karazhan::IsAranCastingArcaneExplosion(Unit* aran)
{
    return aran && TbcRaidHelper::IsCreatureCasting(aran, SPELL_ARCANE_EXPLOSION);
}

bool karazhan::IsFlameWreathActive(PlayerbotAI* ai)
{
    Unit* aran = GetKaraUnit(ai, NPC_SHADE_OF_ARAN);
    if (!aran)
        return false;

    if (TbcRaidHelper::IsCreatureCasting(aran, SPELL_FLAME_WREATH_CAST))
        return true;

    for (Player* member : raidboss::GetGroupMembers(ai->GetBot(), 200.0f, true))
    {
        if (member->HasAura(SPELL_FLAME_WREATH_AURA))
            return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// Netherspite
// ---------------------------------------------------------------------------

bool karazhan::IsBanishPhase(Unit* netherspite)
{
    return netherspite && (netherspite->HasAura(SPELL_NETHERSPITE_BANISH_VISUAL) || netherspite->HasAura(SPELL_NETHERSPITE_BANISHED));
}

karazhan::BeamBlockers karazhan::GetCurrentBeamBlockers(Player* bot)
{
    BeamBlockers result;
    Group* group = bot->GetGroup();
    if (!group)
        return result;

    // Red beam blockers: tank bots without Nether Exhaustion Red (main tank first)
    std::vector<Player*> redBlockers;
    for (Player* tank : raidboss::GetGroupTanks(bot))
    {
        if (IsKarazhanBot(tank) && !tank->HasAura(SPELL_NETHER_EXHAUSTION_RED))
            redBlockers.push_back(tank);
    }

    // Blue beam blockers: mana using dps bots (includes balance druids),
    // no Nether Exhaustion Blue and less than 25 stacks of the blue beam buff
    // Green beam blockers: (1) rogues, cat druids and dps warriors without Nether Exhaustion Green,
    // (2) then healer bots without Nether Exhaustion Green and less than 25 stacks of the green beam buff
    std::vector<Player*> blueBlockers;
    std::vector<Player*> greenDps;
    std::vector<Player*> greenHealers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!IsKarazhanBot(member) || member->GetInstanceId() != bot->GetInstanceId())
            continue;

        if (raidboss::IsDps(member))
        {
            if (member->GetPowerType() == POWER_MANA)
            {
                if (!member->HasAura(SPELL_NETHER_EXHAUSTION_BLUE) && GetAuraStacks(member, SPELL_BLUE_BEAM_DEBUFF) < 25)
                    blueBlockers.push_back(member);
            }
            else if (!member->HasAura(SPELL_NETHER_EXHAUSTION_GREEN))
                greenDps.push_back(member);
        }
        else if (PlayerbotAI::IsHeal(member))
        {
            if (!member->HasAura(SPELL_NETHER_EXHAUSTION_GREEN) && GetAuraStacks(member, SPELL_GREEN_BEAM_DEBUFF) < 25)
                greenHealers.push_back(member);
        }
    }

    std::vector<Player*> greenBlockers = greenDps;
    greenBlockers.insert(greenBlockers.end(), greenHealers.begin(), greenHealers.end());

    std::lock_guard<std::mutex> lock(karazhanStateMutex);
    BlockerState& state = karazhanBlockers[bot->GetInstanceId()];
    result.red = PickSticky(redBlockers, state.red);
    result.green = PickSticky(greenBlockers, state.green);
    result.blue = PickSticky(blueBlockers, state.blue);
    return result;
}

void karazhan::ClearBeamBlockers(uint32 instanceId, bool& changed)
{
    std::lock_guard<std::mutex> lock(karazhanStateMutex);
    if (karazhanBlockers.erase(instanceId) > 0)
        changed = true;
}

std::vector<Unit*> karazhan::GetAllVoidZones(PlayerbotAI* ai)
{
    std::vector<Unit*> voidZones;
    for (Unit* voidZone : GetKaraUnits(ai, NPC_VOID_ZONE))
    {
        if (raidboss::ExactDist2d(ai->GetBot(), voidZone) <= 30.0f)
            voidZones.push_back(voidZone);
    }

    return voidZones;
}

bool karazhan::FindBeamPosition(Unit* netherspite, Unit* portal, const std::vector<Unit*>& voidZones,
                                float idealDistance, float& outX, float& outY)
{
    const float bossX = netherspite->GetPositionX();
    const float bossY = netherspite->GetPositionY();
    const float length = raidboss::ExactDist2d(netherspite, portal);
    if (length == 0.0f)
        return false;

    const float dx = (portal->GetPositionX() - bossX) / length;
    const float dy = (portal->GetPositionY() - bossY) / length;

    const float voidZoneRadius = 4.0f;
    const float searchMinDist = 2.0f;
    const float searchStep = 0.5f;
    const uint8 numSteps = 56;

    float bestDist = std::numeric_limits<float>::max();
    bool found = false;
    for (uint8 i = 0; i <= numSteps; ++i)
    {
        const float dist = searchMinDist + i * searchStep;
        // Stay between the boss and the portal, otherwise the beam is not blocked
        if (dist >= length)
            break;

        const float candidateX = bossX + dx * dist;
        const float candidateY = bossY + dy * dist;
        if (!IsSafePosition(candidateX, candidateY, voidZones, voidZoneRadius))
            continue;

        const float distToIdeal = std::fabs(dist - idealDistance);
        if (!found || distToIdeal < bestDist)
        {
            bestDist = distToIdeal;
            outX = candidateX;
            outY = candidateY;
            found = true;
        }
    }

    return found;
}

// ---------------------------------------------------------------------------
// Prince Malchezaar
// ---------------------------------------------------------------------------

std::vector<Unit*> karazhan::GetSpawnedInfernals(PlayerbotAI* ai)
{
    std::vector<Unit*> infernals;
    for (Unit* infernal : GetKaraUnits(ai, NPC_NETHERSPITE_INFERNAL))
    {
        if (raidboss::ExactDist2d(ai->GetBot(), infernal) <= 100.0f)
            infernals.push_back(infernal);
    }

    return infernals;
}

bool karazhan::IsStraightPathSafe(float sx, float sy, float tx, float ty, const std::vector<Unit*>& hazards, float hazardRadius)
{
    if (!IsSafePosition(tx, ty, hazards, hazardRadius))
        return false;
    const float totalDistX = tx - sx;
    const float totalDistY = ty - sy;
    const float totalDist = std::sqrt(totalDistX * totalDistX + totalDistY * totalDistY);
    if (totalDist == 0.0f)
        return true;

    const float stepSize = 0.5f;
    const int numSteps = static_cast<int>(totalDist / stepSize);
    for (int i = 0; i <= numSteps; ++i)
    {
        const float t = (i * stepSize) / totalDist;
        const float checkX = sx + totalDistX * t;
        const float checkY = sy + totalDistY * t;
        for (Unit* hazard : hazards)
        {
            const float hx = checkX - hazard->GetPositionX();
            const float hy = checkY - hazard->GetPositionY();
            const float startX = sx - hazard->GetPositionX();
            const float startY = sy - hazard->GetPositionY();
            const float startSq = startX * startX + startY * startY;
            const float currentSq = hx * hx + hy * hy;
            // Allow leaving an existing hazard, but never moving deeper into
            // it or entering another one. The start point itself is not a veto.
            if (currentSq < hazardRadius * hazardRadius &&
                (startSq >= hazardRadius * hazardRadius || currentSq + 0.01f < startSq))
                return false;
        }
    }

    return true;
}

bool karazhan::NeedsEmergencyEscape(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    if (!bot->IsInWorld() || bot->IsBeingTeleported() || bot->GetMapId() != KARAZHAN_MAP_ID)
        return false;
    if (Unit* aran = GetKaraUnit(ai, NPC_SHADE_OF_ARAN))
        if (IsAranCastingArcaneExplosion(aran) && !IsFlameWreathActive(ai) &&
            raidboss::ExactDist2d(bot, aran) < ARAN_ESCAPE_DISTANCE)
            return true;
    if (bot->HasAura(SPELL_ENFEEBLE))
        if (Unit* prince = GetKaraUnit(ai, NPC_PRINCE_MALCHEZAAR))
            return raidboss::ExactDist2d(bot, prince) < MALCHEZAAR_ESCAPE_DISTANCE ||
                !IsSafePosition(bot->GetPositionX(), bot->GetPositionY(), GetSpawnedInfernals(ai), 22.0f);
    return false;
}

bool karazhan::CanUseMechanicMovement(PlayerbotAI* ai, const std::string& action)
{
    Player* bot = ai->GetBot();
    if (!bot->IsInWorld() || bot->IsBeingTeleported() || bot->GetMapId() != KARAZHAN_MAP_ID)
        return true;
    if (IsFlameWreathActive(ai))
        return action == "shade of aran stop moving during flame wreath";
    if (NeedsEmergencyEscape(ai))
        return action == "shade of aran run away from arcane explosion" ||
            action == "prince malchezaar enfeebled bot avoid hazard";
    if (Unit* prince = GetKaraUnit(ai, NPC_PRINCE_MALCHEZAAR))
        if (PlayerbotAI::IsTank(bot) && prince->GetVictim() == bot &&
            !IsSafePosition(bot->GetPositionX(), bot->GetPositionY(), GetSpawnedInfernals(ai), 30.0f))
            return action == "prince malchezaar tanks position boss";
    if (Unit* boss = GetKaraUnit(ai, NPC_NETHERSPITE))
    {
        if (!IsBanishPhase(boss))
        {
            const auto blockers = GetCurrentBeamBlockers(bot);
            if (bot == blockers.red && GetKaraUnit(ai, NPC_RED_PORTAL)) return action == "netherspite block red beam";
            if (bot == blockers.blue && GetKaraUnit(ai, NPC_BLUE_PORTAL)) return action == "netherspite block blue beam";
            if (bot == blockers.green && GetKaraUnit(ai, NPC_GREEN_PORTAL)) return action == "netherspite block green beam";
        }
    }
    return true;
}

bool karazhan::TryFindSafePositionWithSafePath(Player* bot, float originX, float originY, float centerX, float centerY,
                                               const std::vector<Unit*>& hazards, float safeDistance, float maxSampleDist,
                                               float& outX, float& outY)
{
    struct Candidate
    {
        float x;
        float y;
        float moveDistSq;
    };

    // Safe points around the center, closest to the origin first
    const uint8 numAngles = 32;
    const float stepSize = 1.0f;
    const int numSteps = static_cast<int>(maxSampleDist / stepSize);
    std::vector<Candidate> candidates;
    for (int i = 0; i < numAngles; ++i)
    {
        const float angle = (2.0f * M_PI_F * i) / numAngles;
        const float dx = std::cos(angle);
        const float dy = std::sin(angle);
        for (int j = 1; j <= numSteps; ++j)
        {
            const float destX = centerX + dx * j * stepSize;
            const float destY = centerY + dy * j * stepSize;
            if (!IsSafePosition(destX, destY, hazards, safeDistance))
                continue;

            const float toDestX = destX - originX;
            const float toDestY = destY - originY;
            candidates.push_back({ destX, destY, toDestX * toDestX + toDestY * toDestY });
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) { return a.moveDistSq < b.moveDistSq; });

    // Attempt to find a safe path, but take any path to a safe position if no safe path is found
    for (bool requireSafePath : { true })
    {
        uint8 groundChecks = 0;
        for (const Candidate& candidate : candidates)
        {
            if (requireSafePath && !IsStraightPathSafe(originX, originY, candidate.x, candidate.y, hazards, safeDistance))
                continue;

            // Ground / line of sight checks are expensive, only check the best points
            if (++groundChecks > 20)
                break;

            float destZ = 0.0f;
            if (!GetValidGroundPoint(bot, candidate.x, candidate.y, destZ))
                continue;

            outX = candidate.x;
            outY = candidate.y;
            return true;
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
// Nightbane
// ---------------------------------------------------------------------------

bool karazhan::IsNightbaneFlying(Unit* nightbane)
{
    return nightbane && nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z;
}

bool karazhan::IsNightbaneFlightPhaseActive(Unit* nightbane)
{
    if (!IsNightbaneFlying(nightbane))
        return false;

    // After 35s Nightbane goes to land, and bots freely follow their master
    const int64 elapsed = GetTimerElapsed(nightbane->GetInstanceId(), KaraTimer::NIGHTBANE_FLIGHT_START);
    return elapsed >= 0 && elapsed < NIGHTBANE_FLIGHT_PHASE_SECONDS;
}

bool karazhan::AreFearCastersPresent(PlayerbotAI* ai)
{
    Unit* nightbane = GetKaraUnit(ai, NPC_NIGHTBANE);
    if (nightbane && !IsNightbaneFlying(nightbane))
        return true;

    return GetKaraUnit(ai, NPC_SPECTRAL_CHARGER) || GetKaraUnit(ai, NPC_BIG_BAD_WOLF) || GetKaraUnit(ai, NPC_ROAR);
}
