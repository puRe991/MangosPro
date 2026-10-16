// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/TK/Util/TKHelpers) and adapted for this core.
#include "playerbot/playerbot.h"
#include "TkHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/generic/TbcRaidStrategy.h"
#include "Spells/Spell.h"
#include "Movement/MoveSpline.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>
#include <unordered_map>

using namespace ai;

namespace
{
    // Encounter states shared by the bots of an instance. The maps of different instances are
    // updated by different threads, so every access is locked.
    struct AlarState
    {
        bool lastRebirthState = false;
        bool inPhase2 = false;
    };

    struct KaelthasState
    {
        uint32 phase = tk::PHASE_NONE;
        uint32 weaponPhaseStart = 0;
        uint32 advisorDpsWaitStart = 0;     // 0 = not started
        bool advisorDpsWaitKnown = false;
    };

    std::mutex tkStateLock;
    std::unordered_map<uint32, AlarState> alarStates;
    std::unordered_map<uint32, KaelthasState> kaelthasStates;

    constexpr uint32 ADVISOR_ENTRIES[4] = { tk::NPC_THALADRED, tk::NPC_SANGUINAR, tk::NPC_CAPERNIAN, tk::NPC_TELONICUS };
    constexpr uint32 WEAPON_ENTRIES[7] =
    {
        tk::NPC_STAFF_OF_DISINTEGRATION, tk::NPC_COSMIC_INFUSER, tk::NPC_INFINITY_BLADES, tk::NPC_WARP_SLICER,
        tk::NPC_PHASESHIFT_BULWARK, tk::NPC_NETHERSTRAND_LONGBOW, tk::NPC_DEVASTATION,
    };

    int8 GetNearestLocationIndex(float x, float y, float maxDist)
    {
        const tk::TkPos locations[6] =
        {
            tk::ALAR_LANDING_PLATFORM_POSITIONS[0], tk::ALAR_LANDING_PLATFORM_POSITIONS[1],
            tk::ALAR_LANDING_PLATFORM_POSITIONS[2], tk::ALAR_LANDING_PLATFORM_POSITIONS[3],
            tk::ALAR_POINT_QUILLS, tk::ALAR_POINT_MIDDLE,
        };

        float minDist = std::numeric_limits<float>::max();
        int8 locationIndex = tk::LOCATION_NONE;
        for (int8 i = 0; i < 6; ++i)
        {
            const float dist = std::sqrt((locations[i].x - x) * (locations[i].x - x) + (locations[i].y - y) * (locations[i].y - y));
            if (dist < minDist)
            {
                minDist = dist;
                locationIndex = i;
            }
        }

        // The Dive Bomb point is next to the Flame Quills point
        const float diveDist = std::sqrt((tk::ALAR_POINT_DIVE_BOMB.x - x) * (tk::ALAR_POINT_DIVE_BOMB.x - x) +
                                         (tk::ALAR_POINT_DIVE_BOMB.y - y) * (tk::ALAR_POINT_DIVE_BOMB.y - y));
        if (diveDist < minDist)
        {
            minDist = diveDist;
            locationIndex = tk::POINT_QUILL_OR_DIVE_IDX;
        }

        return minDist > maxDist ? tk::LOCATION_NONE : locationIndex;
    }

    bool IsTankOfIndex(Player* bot, int index)
    {
        return index == 0 ? raidboss::IsMainTank(bot) : raidboss::IsAssistTank(bot, uint8(index - 1));
    }
}

// General

bool tk::IsTkEncounterInProgress(Player* bot)
{
    return raidboss::IsEncounterInProgress(bot, TK_MAP_ID);
}

std::pair<Unit*, Unit*> tk::GetTargetUnitPair(PlayerbotAI* ai, uint32 entry)
{
    Unit* lowest = nullptr;
    Unit* highest = nullptr;

    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("attackers")->Get())
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != entry)
            continue;

        if (!lowest || unit->GetObjectGuid().GetRawValue() < lowest->GetObjectGuid().GetRawValue())
            lowest = unit;

        if (!highest || unit->GetObjectGuid().GetRawValue() > highest->GetObjectGuid().GetRawValue())
            highest = unit;
    }

    return { lowest, highest };
}

Player* tk::GetNearestNonTankPlayerInRadius(Player* bot, float radius)
{
    Player* nearestPlayer = nullptr;
    float nearestDistance = radius;
    for (Player* member : raidboss::GetGroupMembers(bot, radius + 10.0f, false))
    {
        if (PlayerbotAI::IsTank(member))
            continue;

        const float distance = raidboss::ExactDist2d(bot, member);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestPlayer = member;
        }
    }

    return nearestPlayer;
}

float tk::Dist2d(const WorldObject* a, const TkPos& pos)
{
    return raidboss::ExactDist2d(a, pos.x, pos.y);
}

Unit* tk::GetCurrentTarget(PlayerbotAI* ai)
{
    return ai->GetUnit(ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Get());
}

bool tk::CastTaunt(PlayerbotAI* ai, Unit* target)
{
    if (!target)
        return false;

    if (raidboss::TryCast(ai, "taunt", target) || raidboss::TryCast(ai, "growl", target))
        return true;

    // Righteous Defense is cast on the friendly target of the mob
    Unit* victim = target->GetVictim();
    return victim && victim->IsPlayer() && victim != ai->GetBot() && raidboss::TryCast(ai, "righteous defense", victim);
}

void tk::ClearTarget(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Set(ObjectGuid());
    bot->SetSelectionGuid(ObjectGuid());
}

bool tk::IsCaster(Player* player)
{
    return raidboss::IsRangedPlayer(player) && player->getClass() != CLASS_HUNTER;
}

bool tk::IsRangedDps(Player* player)
{
    return raidboss::IsDps(player) && raidboss::IsRangedPlayer(player);
}

bool tk::IsMeleeDps(Player* player)
{
    return raidboss::IsDps(player) && raidboss::IsMeleePlayer(player);
}

bool tk::IsFirstAssistHealer(Player* bot)
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* first = nullptr;
    for (Player* healer : raidboss::GetGroupHealers(bot))
    {
        if (group->IsAssistant(healer->GetObjectGuid()) || group->IsLeader(healer->GetObjectGuid()))
            return healer == bot;

        if (!first)
            first = healer;
    }

    return first == bot;
}

bool tk::GetPointAwayFrom(Player* bot, float fromX, float fromY, float distance, WorldPosition& out)
{
    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();
    float baseAngle = std::atan2(botY - fromY, botX - fromX);
    if (std::fabs(botX - fromX) < 0.01f && std::fabs(botY - fromY) < 0.01f)
        baseAngle = bot->GetOrientation() + M_PI_F;

    // Straight away first, then turn more and more to the sides (walls, ledges)
    const float offsets[] = { 0.0f, 0.4f, -0.4f, 0.8f, -0.8f, 1.2f, -1.2f, 1.6f, -1.6f };
    for (float offset : offsets)
    {
        const float angle = baseAngle + offset;
        const float destX = botX + std::cos(angle) * distance;
        const float destY = botY + std::sin(angle) * distance;

        float stepX, stepY, stepZ;
        if (!raidboss::CanTakeStepTowards(bot, destX, destY, distance, stepX, stepY, stepZ))
            continue;

        out = WorldPosition(bot->GetMapId(), stepX, stepY, stepZ);
        return true;
    }

    return false;
}

bool tk::GetPointAwayFromGroup(Player* bot, float distance, bool ignoreTanks, WorldPosition& out)
{
    std::vector<Player*> members;
    for (Player* member : raidboss::GetGroupMembers(bot, distance * 3.0f + 10.0f, false))
    {
        if (!ignoreTanks || !PlayerbotAI::IsTank(member))
            members.push_back(member);
    }

    if (members.empty())
        return false;

    auto minDistance = [&](float x, float y)
    {
        float closest = std::numeric_limits<float>::max();
        for (Player* member : members)
            closest = std::min(closest, raidboss::ExactDist2d(member, x, y));

        return closest;
    };

    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();
    const float current = minDistance(botX, botY);
    if (current >= distance)
        return false;

    float bestScore = current;
    bool found = false;
    const float radii[] = { distance * 0.5f, distance, distance * 1.5f };
    for (float radius : radii)
    {
        for (uint8 i = 0; i < 16; ++i)
        {
            const float angle = i * (2.0f * M_PI_F / 16.0f);
            const float destX = botX + std::cos(angle) * radius;
            const float destY = botY + std::sin(angle) * radius;

            // Prefer the shortest move that gets far enough away
            const float score = std::min(minDistance(destX, destY), distance) - radius * 0.05f;
            if (score <= bestScore + 0.5f)
                continue;

            float stepX, stepY, stepZ;
            if (!raidboss::CanTakeStepTowards(bot, destX, destY, radius, stepX, stepY, stepZ))
                continue;

            bestScore = score;
            out = WorldPosition(bot->GetMapId(), stepX, stepY, stepZ);
            found = true;
        }
    }

    return found;
}

bool tk::GetPointInside(Player* bot, const TkPos& center, float radius, WorldPosition& out)
{
    const float distance = Dist2d(bot, center);
    if (distance <= radius)
        return false;

    // Closest point of the circle (a bit inside)
    const float ratio = std::max(0.0f, radius * 0.8f) / distance;
    const float x = center.x + (bot->GetPositionX() - center.x) * ratio;
    const float y = center.y + (bot->GetPositionY() - center.y) * ratio;
    float z = bot->GetMap()->GetHeight(x, y, center.z + 4.0f);
    if (z <= INVALID_HEIGHT)
        z = center.z;

    out = WorldPosition(bot->GetMapId(), x, y, z);
    return true;
}

// Trash

// "attackers" already drops polymorphed units so no check for it is necessary.
Unit* tk::GetCenturionCastingArcaneFlurry(PlayerbotAI* ai)
{
    Unit* target = nullptr;
    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("attackers")->Get())
    {
        Unit* attacker = ai->GetUnit(guid);
        if (!attacker || attacker->GetEntry() != NPC_CRIMSON_HAND_CENTURION || !attacker->IsAlive())
            continue;

        if (!attacker->HasAura(SPELL_ARCANE_FLURRY) && !TbcRaidHelper::IsCreatureCasting(attacker, SPELL_ARCANE_FLURRY))
            continue;

        if (!target || attacker->GetObjectGuid() < target->GetObjectGuid())
            target = attacker;
    }

    return target;
}

// Al'ar <Phoenix God>

bool tk::IsAlarInPhase2(uint32 instanceId)
{
    std::lock_guard<std::mutex> guard(tkStateLock);
    auto it = alarStates.find(instanceId);
    return it != alarStates.end() && it->second.inPhase2;
}

bool tk::UpdateAlarPhaseTracker(Unit* alar)
{
    const bool rebirthActive = TbcRaidHelper::IsCreatureCasting(alar, SPELL_REBIRTH_PHASE2);

    std::lock_guard<std::mutex> guard(tkStateLock);
    AlarState& state = alarStates[alar->GetInstanceId()];
    if (!state.inPhase2 && state.lastRebirthState && !rebirthActive)
    {
        state.inPhase2 = true;
        return true;
    }

    state.lastRebirthState = rebirthActive;
    return false;
}

int8 tk::GetAlarCurrentLocationIndex(Unit* alar)
{
    if (!alar)
        return LOCATION_NONE;

    return GetNearestLocationIndex(alar->GetPositionX(), alar->GetPositionY(), 1.0f);
}

int8 tk::GetAlarDestinationLocationIndex(Unit* alar)
{
    if (!alar || !alar->movespline || alar->movespline->Finalized())
        return LOCATION_NONE;

    const auto destination = alar->movespline->FinalDestination();
    return GetNearestLocationIndex(destination.x, destination.y, 1.0f);
}

int8 tk::GetAlarPlatformIndex(Unit* alar)
{
    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
        locationIndex = GetAlarDestinationLocationIndex(alar);

    if (locationIndex < PLATFORM_0_IDX || locationIndex > PLATFORM_3_IDX)
        return LOCATION_NONE;

    return locationIndex;
}

bool tk::IsAlarPassive(Unit* alar)
{
    return alar && alar->AI() && alar->AI()->GetReactState() == REACT_PASSIVE;
}

// The nearest platform's index is also the ground (landing) spot beneath it.
const tk::TkPos& tk::GetClosestGroundPosition(Player* bot)
{
    size_t closestPlatform = 0;
    float minDist = Dist2d(bot, ALAR_LANDING_PLATFORM_POSITIONS[0]);
    for (size_t i = 1; i < 4; ++i)
    {
        const float dist = Dist2d(bot, ALAR_LANDING_PLATFORM_POSITIONS[i]);
        if (dist < minDist)
        {
            minDist = dist;
            closestPlatform = i;
        }
    }

    return ALAR_GROUND_POSITIONS[closestPlatform];
}

// The main tank rotates between the W (where Al'ar initially lands) and NE platforms in phase 1
// and starts on Al'ar in phase 2.
bool tk::IsFirstAlarTank(Player* bot)
{
    return IsTankOfIndex(bot, 0);
}

// The first assist tank rotates between the NW and E platforms in phase 1.
bool tk::IsSecondAlarTank(Player* bot)
{
    return IsTankOfIndex(bot, 1);
}

// The second assist tank is the primary ember tank.
bool tk::IsPrimaryEmberTank(Player* bot)
{
    return IsTankOfIndex(bot, 2);
}

// The secondary Ember Tank is needed only during phase 2, and it is initially the first assist
// tank (i.e., the SecondAlarTank). When Al'ar melts the armor of the main tank, then the main
// tank becomes the secondary Ember tank. The two tanks swap from then on.
Player* tk::GetSecondaryEmberTank(Player* bot)
{
    Player* mainTank = raidboss::GetGroupMainTank(bot);
    Player* assistTank = raidboss::GetGroupAssistTank(bot, 0);
    if (!mainTank || !assistTank)
        return nullptr;

    if (mainTank->HasAura(SPELL_MELT_ARMOR))
        return mainTank;

    return assistTank;
}

std::list<ObjectGuid> tk::FindFlamePatchGuids(Player* bot)
{
    std::list<ObjectGuid> guids;
    for (Unit* flamePatch : raidboss::FindUnits(bot, NPC_FLAME_PATCH, ALAR_FLAME_PATCH_SEARCH_DISTANCE))
        guids.push_back(flamePatch->GetObjectGuid());

    return guids;
}

std::vector<Unit*> tk::GetFlamePatches(PlayerbotAI* ai)
{
    std::vector<Unit*> flamePatches;
    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("tk flame patches")->Get())
    {
        Unit* flamePatch = ai->GetUnit(guid);
        if (flamePatch && flamePatch->IsAlive())
            flamePatches.push_back(flamePatch);
    }

    return flamePatches;
}

// Void Reaver

// The Arcane Orb is a triggered missile without a cast bar. The flying spells are found in the
// event list of Void Reaver, their target spot is where the orb explodes.
std::vector<WorldPosition> tk::GetActiveArcaneOrbs(Unit* voidReaver)
{
    std::vector<WorldPosition> orbs;
    if (!voidReaver)
        return orbs;

    for (auto& entry : voidReaver->m_events.GetEvents())
    {
        SpellEvent* spellEvent = dynamic_cast<SpellEvent*>(entry.second);
        if (!spellEvent || spellEvent->to_Abort)
            continue;

        Spell* spell = spellEvent->GetSpell();
        if (!spell || !spell->m_spellInfo || spell->m_spellInfo->Id != SPELL_ARCANE_ORB || spell->getState() == SPELL_STATE_FINISHED)
            continue;

        float x, y, z;
        spell->m_targets.getDestination(x, y, z);
        if (x == 0.0f && y == 0.0f)
        {
            Unit* target = voidReaver->GetMap()->GetUnit(spell->m_targets.getUnitTargetGuid());
            if (!target)
                continue;

            x = target->GetPositionX();
            y = target->GetPositionY();
            z = target->GetPositionZ();
        }

        orbs.push_back(WorldPosition(voidReaver->GetMapId(), x, y, z));
    }

    return orbs;
}

bool tk::IsNearArcaneOrb(Player* bot, const std::vector<WorldPosition>& orbs, float radius)
{
    for (const WorldPosition& orb : orbs)
    {
        if (raidboss::ExactDist2d(bot, orb.getX(), orb.getY()) < radius)
            return true;
    }

    return false;
}

bool tk::IsNearActiveArcaneOrb(PlayerbotAI* ai, float radius)
{
    Unit* voidReaver = raidboss::FindUnit(ai, NPC_VOID_REAVER);
    return voidReaver && IsNearArcaneOrb(ai->GetBot(), GetActiveArcaneOrbs(voidReaver), radius);
}

// High Astromancer Solarian

bool tk::HasWrathOfTheAstromancer(Player* bot)
{
    return bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER);
}

bool tk::IsSolarianTankable(Unit* astromancer)
{
    return astromancer && astromancer->GetVisibility() != VISIBILITY_OFF;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

Unit* tk::FindKaelthas(PlayerbotAI* ai)
{
    return raidboss::FindUnit(ai, NPC_KAELTHAS, KAELTHAS_ROOM_SEARCH_DISTANCE + 25.0f);
}

uint32 tk::GetKaelthasTkPhase(PlayerbotAI* ai, Unit* kaelthas)
{
    if (!kaelthas)
        return PHASE_NONE;

    const uint32 instanceId = kaelthas->GetInstanceId();
    if (!kaelthas->IsAlive() || !kaelthas->IsInCombat())
    {
        std::lock_guard<std::mutex> guard(tkStateLock);
        auto it = kaelthasStates.find(instanceId);
        if (it != kaelthasStates.end())
            it->second.phase = PHASE_NONE;

        return PHASE_NONE;
    }

    uint32 phase;
    {
        std::lock_guard<std::mutex> guard(tkStateLock);
        phase = kaelthasStates[instanceId].phase;
    }

    if (phase == PHASE_FINAL)
        return phase;

    Player* bot = ai->GetBot();
    uint32 newPhase = phase == PHASE_NONE ? PHASE_SINGLE_ADVISOR : phase;

    // Kael'thas is uninteractible (and at full health) until he fights himself
    if (!kaelthas->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE) || kaelthas->GetHealthPercent() < 100.0f)
    {
        newPhase = PHASE_FINAL;
    }
    else if (newPhase < PHASE_WEAPONS)
    {
        for (uint32 entry : WEAPON_ENTRIES)
        {
            if (!raidboss::FindUnits(bot, entry, KAELTHAS_ROOM_SEARCH_DISTANCE).empty())
            {
                newPhase = PHASE_WEAPONS;
                break;
            }
        }
    }
    else if (newPhase == PHASE_WEAPONS || newPhase == PHASE_TRANSITION)
    {
        // All advisors were feigning death during the weapon phase, a revived one starts phase 3
        bool revived = TbcRaidHelper::IsCreatureCasting(kaelthas, SPELL_RESURRECTION);
        for (uint32 entry : ADVISOR_ENTRIES)
        {
            Unit* advisor = raidboss::FindUnit(ai, entry, KAELTHAS_ROOM_SEARCH_DISTANCE);
            if (advisor && !advisor->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
                revived = true;
        }

        if (revived)
            newPhase = PHASE_ALL_ADVISORS;
    }

    std::lock_guard<std::mutex> guard(tkStateLock);
    KaelthasState& state = kaelthasStates[instanceId];
    if (newPhase > state.phase)
    {
        state.phase = newPhase;
        if (newPhase == PHASE_WEAPONS)
            state.weaponPhaseStart = WorldTimer::getMSTime();
    }

    if (state.phase == PHASE_WEAPONS && state.weaponPhaseStart &&
        WorldTimer::getMSTimeDiff(state.weaponPhaseStart, WorldTimer::getMSTime()) >= KAELTHAS_WEAPON_PHASE_MS)
    {
        state.phase = PHASE_TRANSITION;
    }

    return state.phase;
}

void tk::ResetAdvisorDpsTimer(uint32 instanceId)
{
    std::lock_guard<std::mutex> guard(tkStateLock);
    KaelthasState& state = kaelthasStates[instanceId];
    state.advisorDpsWaitStart = 0;
    state.advisorDpsWaitKnown = true;
}

bool tk::UpdateAdvisorDpsTimer(uint32 instanceId, bool advisorAtFullHp)
{
    std::lock_guard<std::mutex> guard(tkStateLock);
    KaelthasState& state = kaelthasStates[instanceId];
    if (advisorAtFullHp)
    {
        state.advisorDpsWaitStart = 0;
        state.advisorDpsWaitKnown = true;
        return false;
    }

    if (!state.advisorDpsWaitKnown || state.advisorDpsWaitStart)
        return false;

    state.advisorDpsWaitStart = WorldTimer::getMSTime();
    return true;
}

bool tk::IsAdvisorDpsWaitOver(uint32 instanceId, uint32 waitMs)
{
    std::lock_guard<std::mutex> guard(tkStateLock);
    auto it = kaelthasStates.find(instanceId);
    return it != kaelthasStates.end() && it->second.advisorDpsWaitStart &&
        WorldTimer::getMSTimeDiff(it->second.advisorDpsWaitStart, WorldTimer::getMSTime()) >= waitMs;
}

// Waiting advisors are immune to players, "killed" advisors of phase 1 have Permanent Feign Death
// until they are revived in phase 3 (and stay uninteractible while they talk).
bool tk::IsAdvisorActive(Unit* advisor)
{
    return advisor && advisor->IsAlive() &&
        !advisor->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER) &&
        !advisor->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE) &&
        !advisor->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
}

bool tk::IsAdvisorReviving(Unit* advisor)
{
    return advisor && advisor->IsAlive() && advisor->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
}

// (1) First priority is an assistant Warlock (real player or bot).
// (2) If no assistant Warlock, then look for any Warlock bot.
Player* tk::GetCapernianTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* fallbackWarlock = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || member->getClass() != CLASS_WARLOCK || !member->IsAlive() || member->GetMapId() != TK_MAP_ID)
            continue;

        if (group->IsAssistant(member->GetObjectGuid()))
            return member;

        if (!fallbackWarlock && member->GetPlayerbotAI())
            fallbackWarlock = member;
    }

    return fallbackWarlock;
}

bool tk::IsCapernianTank(Player* bot)
{
    return bot->getClass() == CLASS_WARLOCK && GetCapernianTank(bot) == bot;
}

// One Hunter will start on Sanguinar in phase 3 (with melee) to apply Armor Disruption from the
// Netherstrand Longbow.
// (1) First priority is an assistant Hunter (real player or bot).
// (2) If no assistant Hunter, then look for any Hunter bot.
bool tk::IsSanguinarDebuffHunter(Player* bot)
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* fallbackHunter = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || member->getClass() != CLASS_HUNTER || !member->IsAlive() || member->GetMapId() != TK_MAP_ID)
            continue;

        if (group->IsAssistant(member->GetObjectGuid()))
            return member == bot;

        if (!fallbackHunter && member->GetPlayerbotAI())
            fallbackHunter = member;
    }

    return fallbackHunter == bot;
}

std::list<ObjectGuid> tk::FindLegendaryWeaponGuids(Player* bot)
{
    std::list<ObjectGuid> guids;
    for (uint32 entry : WEAPON_ENTRIES)
    {
        for (Unit* weapon : raidboss::FindUnits(bot, entry, KAELTHAS_ROOM_SEARCH_DISTANCE, false))
            guids.push_back(weapon->GetObjectGuid());
    }

    return guids;
}

namespace
{
    Creature* GetLegendaryWeaponByState(PlayerbotAI* ai, uint32 weaponEntry, bool alive)
    {
        for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("tk legendary weapons")->Get())
        {
            Creature* weapon = ai->GetCreature(guid);
            if (weapon && weapon->GetEntry() == weaponEntry && weapon->IsAlive() == alive)
                return weapon;
        }

        return nullptr;
    }
}

Unit* tk::GetLegendaryWeapon(PlayerbotAI* ai, uint32 weaponEntry)
{
    return GetLegendaryWeaponByState(ai, weaponEntry, true);
}

Creature* tk::GetDeadLegendaryWeapon(PlayerbotAI* ai, uint32 weaponEntry)
{
    return GetLegendaryWeaponByState(ai, weaponEntry, false);
}

bool tk::HasDeadLegendaryWeapon(PlayerbotAI* ai)
{
    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("tk legendary weapons")->Get())
    {
        Creature* weapon = ai->GetCreature(guid);
        if (weapon && !weapon->IsAlive())
            return true;
    }

    return false;
}

bool tk::IsLegendaryWeaponItem(uint32 itemId)
{
    switch (itemId)
    {
    case ITEM_WARP_SLICER:
    case ITEM_INFINITY_BLADE:
    case ITEM_STAFF_OF_DISINTEGRATION:
    case ITEM_PHASESHIFT_BULWARK:
    case ITEM_DEVASTATION:
    case ITEM_COSMIC_INFUSER:
    case ITEM_NETHERSTRAND_LONGBOW:
        return true;
    default:
        return false;
    }
}

Item* tk::GetEquippableItemForSlot(Player* bot, uint8 slot)
{
    Item* best = nullptr;
    auto check = [&](Item* item)
    {
        if (!item || !item->GetProto() || IsLegendaryWeaponItem(item->GetEntry()))
            return;

        uint16 dest = 0;
        if (bot->CanEquipItem(slot, dest, item, true) != EQUIP_ERR_OK || (dest & 0xFF) != slot)
            return;

        if (!best || item->GetProto()->ItemLevel > best->GetProto()->ItemLevel)
            best = item;
    };

    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        check(bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i));

    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        Item* bagItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (!bagItem || !bagItem->IsBag())
            continue;

        Bag* container = static_cast<Bag*>(bagItem);
        for (uint8 i = 0; i < container->GetBagSize(); ++i)
            check(container->GetItemByPos(i));
    }

    return best;
}

Item* tk::GetEquippedItemInSlot(Player* bot, uint8 slot, uint32 itemId)
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    return item && item->GetEntry() == itemId ? item : nullptr;
}

Creature* tk::GetNearestFlameStrikeInRadius(Player* bot, float radius)
{
    Creature* nearest = nullptr;
    for (Unit* unit : raidboss::FindUnits(bot, NPC_FLAME_STRIKE_TRIGGER, radius))
    {
        if (unit->GetTypeId() == TYPEID_UNIT && (!nearest || bot->GetDistance(unit) < bot->GetDistance(nearest)))
            nearest = static_cast<Creature*>(unit);
    }

    return nearest;
}

Creature* tk::GetPhoenixEgg(Player* bot)
{
    constexpr float searchRadius = 75.0f;
    Creature* nearest = nullptr;
    for (Unit* unit : raidboss::FindUnits(bot, NPC_PHOENIX_EGG, searchRadius))
    {
        if (unit->GetTypeId() == TYPEID_UNIT && (!nearest || bot->GetDistance(unit) < bot->GetDistance(nearest)))
            nearest = static_cast<Creature*>(unit);
    }

    return nearest;
}

bool tk::ResetEncounterStates(uint32 instanceId)
{
    std::lock_guard<std::mutex> guard(tkStateLock);
    bool reset = alarStates.erase(instanceId) > 0;
    reset |= kaelthasStates.erase(instanceId) > 0;
    return reset;
}
