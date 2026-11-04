// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/BT/BTHelpers) and adapted for this core.
#include "playerbot/playerbot.h"
#include "BtHelpers.h"
#include "BtCouncilRules.h"
#include "BtGurtoggRules.h"
#include "BtSupremusRules.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/Action.h"
#include "playerbot/strategy/actions/MovementActions.h"
#include "playerbot/strategy/actions/AttackAction.h"
#include "playerbot/strategy/actions/FollowActions.h"
#include "playerbot/strategy/actions/ReachTargetActions.h"
#include "playerbot/strategy/actions/GenericSpellActions.h"
#include "Combat/HostileRefManager.h"
#include "Combat/ThreatManager.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>

using namespace ai;

namespace
{
    std::mutex stateMutex;
    std::map<std::pair<uint8, uint64>, int64> stateStore;

    bool NameIn(const std::string& name, std::initializer_list<const char*> names)
    {
        for (const char* n : names)
        {
            if (name == n)
                return true;
        }

        return false;
    }

    // Healers / tanks flagged as raid assistant first, then the others (IsAssistHealOfIndex of mod-playerbots)
    std::vector<Player*> AssistantsFirst(Player* bot, const std::vector<Player*>& members)
    {
        std::vector<Player*> assistants;
        std::vector<Player*> others;
        Group* group = bot->GetGroup();
        for (Player* member : members)
        {
            if (group && (group->IsAssistant(member->GetObjectGuid()) || group->IsLeader(member->GetObjectGuid())))
                assistants.push_back(member);
            else
                others.push_back(member);
        }

        assistants.insert(assistants.end(), others.begin(), others.end());
        return assistants;
    }
}

namespace ai
{
namespace bt
{

// ---------------------------------------------------------------------------
// State store
// ---------------------------------------------------------------------------

bool GetState(BtState type, uint64 key, int64& value)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    auto it = stateStore.find(std::make_pair(uint8(type), key));
    if (it == stateStore.end())
        return false;

    value = it->second;
    return true;
}

void SetState(BtState type, uint64 key, int64 value)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    stateStore[std::make_pair(uint8(type), key)] = value;
}

bool SetStateIfAbsent(BtState type, uint64 key, int64 value)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    return stateStore.emplace(std::make_pair(uint8(type), key), value).second;
}

bool EraseState(BtState type, uint64 key)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    return stateStore.erase(std::make_pair(uint8(type), key)) > 0;
}

int64 GetTimerElapsed(BtState type, uint32 instanceId)
{
    int64 start = 0;
    if (!GetState(type, instanceId, start))
        return -1;

    return int64(time(nullptr)) - start;
}

// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------

Unit* FindBoss(PlayerbotAI* ai, uint32 entry)
{
    Player* bot = ai->GetBot();
    if (!bot->IsInWorld() || bot->GetMapId() != BLACK_TEMPLE_MAP_ID)
        return nullptr;

    // Creatures which have the bot on their threat list
    for (HostileReference* ref = bot->getHostileRefManager().getFirst(); ref; ref = ref->next())
    {
        Unit* owner = ref->getSource() ? ref->getSource()->getOwner() : nullptr;
        if (owner && owner->IsAlive() && !owner->IsPlayer() && owner->GetEntry() == entry)
            return owner;
    }

    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("attackers")->Get())
    {
        Unit* unit = ai->GetUnit(guid);
        if (unit && unit->IsAlive() && !unit->IsPlayer() && unit->GetEntry() == entry)
            return unit;
    }

    return nullptr;
}

Unit* FindNearest(Player* bot, uint32 entry, float range)
{
    Unit* nearest = nullptr;
    for (Unit* unit : raidboss::FindUnits(bot, entry, range))
    {
        if (!nearest || raidboss::ExactDist2d(bot, unit) < raidboss::ExactDist2d(bot, nearest))
            nearest = unit;
    }

    return nearest;
}

bool IsBossInCombat(Player* bot, uint32 entry)
{
    for (Unit* unit : raidboss::FindUnits(bot, entry, 250.0f))
    {
        if (unit->IsInCombat())
            return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// Action classification
// ---------------------------------------------------------------------------

// Formation moves of this core (spread, stack, approach) which would fight the positions of the port.
// "set behind" stays allowed like in mod-playerbots.
bool IsFormationMoveAction(Action* action)
{
    return action && NameIn(action->getName(), {
        "move out of enemy contact", "tbc raid spread", "tbc raid approach boss", "tbc raid stack on tank",
        "tbc raid soak front" });
}

bool IsFleeOrFollowAction(Action* action)
{
    if (!action)
        return false;

    return dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action) ||
           NameIn(action->getName(), { "flee", "follow", "runaway", "flee with pet", "disengage", "blink" });
}

bool IsReachAction(Action* action)
{
    return action && (dynamic_cast<ReachTargetAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action));
}

bool IsAttackOrSpellAction(Action* action)
{
    if (!action)
        return false;

    if (dynamic_cast<AttackAction*>(action))
        return true;

    return dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action);
}

// ---------------------------------------------------------------------------
// Positions
// ---------------------------------------------------------------------------

const WorldPosition NAJENTUS_TANK_POSITION(BLACK_TEMPLE_MAP_ID, 438.515f, 772.436f, 11.931f);
const WorldPosition GOREFIEND_TANK_POSITION(BLACK_TEMPLE_MAP_ID, 597.653f, 402.284f, 187.090f);
const WorldPosition GOREFIEND_DIE_POSITION(BLACK_TEMPLE_MAP_ID, 525.709f, 377.177f, 193.203f);
const WorldPosition GURTOGG_TANK_POSITION(BLACK_TEMPLE_MAP_ID, 735.987f, 272.451f, 63.554f);
const WorldPosition GURTOGG_RANGED_POSITION(BLACK_TEMPLE_MAP_ID, 762.265f, 277.183f, 63.781f);
const WorldPosition GURTOGG_SOAKER_POSITION(BLACK_TEMPLE_MAP_ID, 769.348f, 280.116f, 63.780f);
const WorldPosition SHAHRAZ_TANK_POSITION(BLACK_TEMPLE_MAP_ID, 960.438f, 178.989f, 192.826f);
const WorldPosition SHAHRAZ_TRANSITION_POSITION(BLACK_TEMPLE_MAP_ID, 951.327f, 179.550f, 192.550f);
const WorldPosition SHAHRAZ_RANGED_POSITION(BLACK_TEMPLE_MAP_ID, 935.267f, 175.459f, 192.821f);

const std::array<WorldPosition, 4> GATHIOS_TANK_POSITIONS = {{
    WorldPosition(BLACK_TEMPLE_MAP_ID, 662.977f, 296.246f, 271.688f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 636.238f, 283.719f, 271.629f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 655.571f, 261.377f, 271.687f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 673.789f, 274.139f, 271.689f)
}};
const WorldPosition ZEREVOR_TANK_POSITION(BLACK_TEMPLE_MAP_ID, 686.219f, 377.644f, 271.689f);
const std::array<WorldPosition, 2> ZEREVOR_HEALER_POSITIONS = {{
    WorldPosition(BLACK_TEMPLE_MAP_ID, 661.385f, 351.219f, 271.690f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 667.003f, 363.768f, 271.690f)
}};
const WorldPosition MALANDE_TANK_POSITION(BLACK_TEMPLE_MAP_ID, 690.590f, 299.790f, 277.443f);

// Same landing point as aCenterLoc of boss_illidan.cpp
const WorldPosition ILLIDAN_LANDING_POSITION(BLACK_TEMPLE_MAP_ID, 676.648f, 304.761f, 354.189f);
const WorldPosition ILLIDAN_N_GRATE_POSITION(BLACK_TEMPLE_MAP_ID, 682.100f, 306.000f, 353.192f);
const WorldPosition ILLIDAN_E_GRATE_POSITION(BLACK_TEMPLE_MAP_ID, 673.500f, 298.500f, 353.192f);
const WorldPosition ILLIDAN_W_GRATE_POSITION(BLACK_TEMPLE_MAP_ID, 672.400f, 312.500f, 353.192f);
const std::array<WorldPosition, 3> GRATE_POSITIONS = {{ ILLIDAN_N_GRATE_POSITION, ILLIDAN_E_GRATE_POSITION, ILLIDAN_W_GRATE_POSITION }};

const WorldPosition ILLIDAN_E_GLAIVE_WAITING_POSITION(BLACK_TEMPLE_MAP_ID, 677.656f, 294.066f, 353.192f);
const std::array<WorldPosition, 7> E_GLAIVE_TANK_POSITIONS = {{
    WorldPosition(BLACK_TEMPLE_MAP_ID, 683.000f, 295.000f, 354.000f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 696.969f, 300.982f, 354.302f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 691.112f, 287.461f, 354.363f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 676.674f, 280.797f, 354.268f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 664.414f, 284.834f, 354.271f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 656.826f, 295.113f, 354.165f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 665.000f, 304.000f, 354.000f)
}};

const WorldPosition ILLIDAN_W_GLAIVE_WAITING_POSITION(BLACK_TEMPLE_MAP_ID, 676.102f, 316.305f, 353.192f);
const std::array<WorldPosition, 7> W_GLAIVE_TANK_POSITIONS = {{
    WorldPosition(BLACK_TEMPLE_MAP_ID, 697.208f, 313.475f, 354.234f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 681.000f, 318.000f, 354.000f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 664.000f, 307.000f, 354.000f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 656.161f, 314.132f, 354.092f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 665.080f, 326.905f, 354.128f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 678.809f, 329.968f, 354.387f),
    WorldPosition(BLACK_TEMPLE_MAP_ID, 690.889f, 324.277f, 354.204f)
}};

float Dist2d(float x1, float y1, float x2, float y2)
{
    const float dx = x1 - x2;
    const float dy = y1 - y2;
    return std::sqrt(dx * dx + dy * dy);
}

// ---------------------------------------------------------------------------
// Supremus
// ---------------------------------------------------------------------------

std::vector<Unit*> GetSupremusVolcanos(Player* bot, float range)
{
    return raidboss::FindUnits(bot, NPC_SUPREMUS_VOLCANO, range);
}

bool HasSupremusVolcanoNearby(Player* bot)
{
    return !GetSupremusVolcanos(bot, 20.0f).empty();
}

// Centre distance at which Supremus hits a running player (his combat reach alone is 24 yards)
float GetSupremusMeleeReach(Player* bot, Unit* supremus)
{
    return supremus->GetCombinedCombatReach(bot, true) + MELEE_LEEWAY;
}

// The Molten Punch stalker gets a huge amount of threat on its target and leaves the flame trail behind it
Unit* GetSupremusMoltenFlameChasingBot(Player* bot)
{
    for (Unit* flame : raidboss::FindUnits(bot, NPC_SUPREMUS_MOLTEN_FLAME, 50.0f))
    {
        if (flame->GetVictim() == bot)
            return flame;
    }

    return nullptr;
}

bool IsSupremusTankPhase(Unit* supremus)
{
    return supremus && supremus->IsInCombat() && !supremus->HasAura(SPELL_SNARE_SELF);
}

namespace
{
    // Tanks besides the boss' victim in his melee reach take the Hateful Strikes instead of the melee dps
    int CountSupremusHatefulSoakers(Player* bot, Unit* supremus)
    {
        int soakers = 0;
        for (Player* tank : raidboss::GetGroupTanks(bot))
        {
            if (tank != supremus->GetVictim() && supremus->CanReachWithMeleeAttack(tank))
                ++soakers;
        }

        return soakers;
    }

    float NormalizeSlotAngle(float angle)
    {
        while (angle > M_PI_F)
            angle -= 2.0f * M_PI_F;
        while (angle < -M_PI_F)
            angle += 2.0f * M_PI_F;
        return angle;
    }
}

bool MustAvoidSupremusHatefulStrike(Player* bot, Unit* supremus)
{
    if (!IsSupremusTankPhase(supremus) || !bot->IsAlive() || PlayerbotAI::IsTank(bot) || supremus->GetVictim() == bot)
        return false;

    if (PlayerbotAI::IsHeal(bot) || raidboss::IsRangedPlayer(bot))
        return true;

    return !supremus::MeleeMayEngage(CountSupremusHatefulSoakers(bot, supremus));
}

bool GetSupremusSafeSlot(Player* bot, Unit* supremus, float& x, float& y, float& z, float& angle)
{
    // Healers and the other players waiting outside get separate lists, both in group order
    std::vector<Player*> healers;
    std::vector<Player*> others;
    for (Player* member : raidboss::GetGroupMembers(bot, 200.0f, true))
    {
        if (!member->GetPlayerbotAI() || !MustAvoidSupremusHatefulStrike(member, supremus))
            continue;

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else
            others.push_back(member);
    }

    const bool healer = PlayerbotAI::IsHeal(bot);
    const std::vector<Player*>& list = healer ? healers : others;
    const int index = raidboss::GetIndexOf(bot, list);
    if (index < 0)
        return false;

    // The healer arc points at the tank of this tank phase; kept for the whole phase so a moving tank
    // doesn't rotate every slot
    const uint32 instanceId = supremus->GetInstanceId();
    int64 anchorMilli = 0;
    if (!GetState(BtState::SupremusSlotAnchor, instanceId, anchorMilli))
    {
        Unit* tank = supremus->GetVictim();
        if (!tank || !tank->IsPlayer() || !PlayerbotAI::IsTank(static_cast<Player*>(tank)))
            tank = raidboss::GetGroupMainTank(bot);

        const float anchor = tank ? std::atan2(tank->GetPositionY() - supremus->GetPositionY(), tank->GetPositionX() - supremus->GetPositionX())
                                  : supremus->GetOrientation();
        SetStateIfAbsent(BtState::SupremusSlotAnchor, instanceId, int64(anchor * 1000.0f));
        GetState(BtState::SupremusSlotAnchor, instanceId, anchorMilli);
    }

    const float anchor = float(anchorMilli) / 1000.0f;
    const float slotAngle = healer ? supremus::HealerSlotAngle(index, int(list.size()), anchor)
                                   : supremus::DpsSlotAngle(index, int(list.size()), anchor);
    const float radius = supremus::SafeRadius(GetSupremusMeleeReach(bot, supremus));

    const float bossX = supremus->GetPositionX();
    const float bossY = supremus->GetPositionY();
    const float bossZ = supremus->GetPositionZ();
    const std::vector<Unit*> volcanos = GetSupremusVolcanos(bot, 100.0f);

    // Walls and volcanos: shift the slot along the ring, then a bit further out
    const float offsets[] = { 0.0f, 0.15f, -0.15f, 0.3f, -0.3f, 0.45f, -0.45f, 0.6f, -0.6f };
    for (float extra : { 0.0f, 5.0f })
    {
        for (float offset : offsets)
        {
            const float candidateAngle = slotAngle + offset;
            const float candidateX = bossX + (radius + extra) * std::cos(candidateAngle);
            const float candidateY = bossY + (radius + extra) * std::sin(candidateAngle);
            const float candidateZ = bot->GetMap()->GetHeight(candidateX, candidateY, bossZ + 5.0f);
            if (candidateZ <= INVALID_HEIGHT || std::fabs(candidateZ - bossZ) > 8.0f)
                continue;

            if (!bot->GetMap()->IsInLineOfSight(bossX, bossY, bossZ + 2.0f, candidateX, candidateY, candidateZ + 2.0f, true))
                continue;

            bool inVolcano = false;
            for (Unit* volcano : volcanos)
            {
                if (raidboss::ExactDist2d(volcano, candidateX, candidateY) < 16.0f)
                {
                    inVolcano = true;
                    break;
                }
            }

            if (inVolcano)
                continue;

            x = candidateX;
            y = candidateY;
            z = candidateZ;
            angle = NormalizeSlotAngle(candidateAngle);
            return true;
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
// Shade of Akama
// ---------------------------------------------------------------------------

bool IsShadeOfAkamaChannelTarget(Player* bot, Unit* target)
{
    if (!bot->IsInWorld() || bot->GetMapId() != BLACK_TEMPLE_MAP_ID || !target || !target->IsAlive() ||
        !target->IsInWorld() || target->GetInstanceId() != bot->GetInstanceId())
    {
        return false;
    }

    if (target->GetEntry() != NPC_ASHTONGUE_CHANNELER && target->GetEntry() != NPC_ASHTONGUE_SORCERER)
        return false;

    // Channelers become attackable when Akama starts his channel
    return !target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
}

namespace
{
    // Lowest health first, ties by guid, so every bot picks the same one
    Unit* GetFocusTarget(Player* bot, uint32 entry)
    {
        Unit* best = nullptr;
        for (Unit* unit : raidboss::FindUnits(bot, entry, 80.0f))
        {
            if (!IsShadeOfAkamaChannelTarget(bot, unit))
                continue;

            if (!best || unit->GetHealthPercent() < best->GetHealthPercent() ||
                (unit->GetHealthPercent() == best->GetHealthPercent() && unit->GetObjectGuid().GetCounter() < best->GetObjectGuid().GetCounter()))
            {
                best = unit;
            }
        }

        return best;
    }
}

// Melee stand at the Shade with the Channelers around it, the Sorcerers run in from the side door
Unit* GetShadeOfAkamaKillTarget(Player* bot)
{
    const bool melee = raidboss::IsMeleePlayer(bot);
    if (Unit* first = GetFocusTarget(bot, melee ? NPC_ASHTONGUE_CHANNELER : NPC_ASHTONGUE_SORCERER))
        return first;

    return GetFocusTarget(bot, melee ? NPC_ASHTONGUE_SORCERER : NPC_ASHTONGUE_CHANNELER);
}

Unit* GetAddAttackingAkama(Player* bot)
{
    Unit* akama = FindNearest(bot, NPC_AKAMA_SHADE, 100.0f);
    if (!akama)
        return nullptr;

    Unit* closest = nullptr;
    for (uint32 entry : { NPC_ASHTONGUE_DEFENDER, NPC_ASHTONGUE_ROGUE, NPC_ASHTONGUE_ELEMENTALIST, NPC_ASHTONGUE_SPIRITBINDER })
    {
        for (Unit* add : raidboss::FindUnits(bot, entry, 80.0f))
        {
            if (add->GetVictim() != akama)
                continue;

            if (!closest || raidboss::ExactDist2d(bot, add) < raidboss::ExactDist2d(bot, closest))
                closest = add;
        }
    }

    return closest;
}

// ---------------------------------------------------------------------------
// Gurtogg Bloodboil
// ---------------------------------------------------------------------------

// The five ranged bots with the fewest Bloodboil stacks stand at the far position. They change right after
// each Bloodboil lands, which leaves the whole 11 seconds until the next cast to swap places.
std::vector<Player*> GetGurtoggBloodboilSoakers(Player* bot)
{
    std::vector<Player*> rangedMembers;
    Group* group = bot->GetGroup();
    if (!group)
        return rangedMembers;

    std::vector<gurtogg::SoakCandidate> candidates;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || !member->IsAlive() || !member->IsInWorld() || member->GetMapId() != bot->GetMapId() ||
            !member->GetPlayerbotAI() || !raidboss::IsRangedPlayer(member))
        {
            continue;
        }

        gurtogg::SoakCandidate candidate;
        candidate.key = member->GetObjectGuid().GetRawValue();
        if (SpellAuraHolder* holder = member->GetSpellAuraHolder(SPELL_BLOODBOIL))
        {
            candidate.stacks = holder->GetStackAmount();
            candidate.remainingMs = holder->GetAuraDuration();
        }

        rangedMembers.push_back(member);
        candidates.push_back(candidate);
    }

    std::vector<Player*> soakers;
    for (size_t index : gurtogg::PickSoakers(candidates))
        soakers.push_back(rangedMembers[index]);

    return soakers;
}

// ---------------------------------------------------------------------------
// Mother Shahraz
// ---------------------------------------------------------------------------

TankPositionState GetShahrazTankPositionState(Player* bot)
{
    Player* mainTank = raidboss::GetGroupMainTank(bot);
    if (!mainTank)
        return TankPositionState::Unknown;

    int64 value = 0;
    if (GetState(BtState::ShahrazTankStep, mainTank->GetObjectGuid().GetRawValue(), value))
        return TankPositionState(value);

    return TankPositionState::Unknown;
}

std::vector<Player*> GetFatalAttractionPlayers(Player* bot)
{
    std::vector<Player*> attracted;
    for (Player* member : raidboss::GetGroupMembers(bot, 200.0f, true))
    {
        if (member->HasAura(SPELL_FATAL_ATTRACTION))
            attracted.push_back(member);
    }

    std::sort(attracted.begin(), attracted.end(), [](Player* a, Player* b)
    {
        return a->GetObjectGuid().GetCounter() < b->GetObjectGuid().GetCounter();
    });

    return attracted;
}

// ---------------------------------------------------------------------------
// Illidari Council
// ---------------------------------------------------------------------------

// (1) an assistant Mage (real player or bot), (2) otherwise the first Mage bot
Player* GetZerevorMageTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* fallbackMage = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_MAGE)
            continue;

        if (group->IsAssistant(member->GetObjectGuid()))
            return member;

        if (!fallbackMage && member->GetPlayerbotAI())
            fallbackMage = member;
    }

    return fallbackMage;
}

// First assist healer: the healer of the mage tank
Player* GetZerevorHealer(Player* bot)
{
    std::vector<Player*> healers = AssistantsFirst(bot, raidboss::GetGroupHealers(bot));
    for (Player* healer : healers)
    {
        if (healer->GetPlayerbotAI())
            return healer;
    }

    return nullptr;
}

Player* GetVerasTank(Player* bot)
{
    if (Player* tank = raidboss::GetGroupAssistTank(bot, 1))
        return tank;

    return raidboss::GetGroupMainTank(bot);
}

// Hunters and the melee classes deal physical damage (Retribution and Enhancement mostly through their weapon)
bool IsPhysicalDamageDealer(Player* bot)
{
    switch (bot->getClass())
    {
    case CLASS_WARRIOR:
    case CLASS_ROGUE:
    case CLASS_HUNTER:
        return true;
    case CLASS_DRUID:
    case CLASS_SHAMAN:
    case CLASS_PALADIN:
        return raidboss::IsMeleePlayer(bot);
    default:
        return false;
    }
}

// Gathios blesses a random council member: Protection stops physical, Spell Warding magical damage.
// Malande's Reflective Shield sends the damage back, Veras is gone while he vanishes.
bool CanDamageCouncilMember(Player* bot, Unit* member)
{
    if (!member || !member->IsAlive())
        return false;

    if (member->HasAura(SPELL_VANISH) || member->HasAura(SPELL_REFLECTIVE_SHIELD))
        return false;

    return !member->HasAura(IsPhysicalDamageDealer(bot) ? SPELL_BLESSING_OF_PROTECTION : SPELL_BLESSING_OF_SPELL_WARDING);
}

Unit* GetMalandeInterrupt(PlayerbotAI* ai, std::string& spell)
{
    std::vector<std::string> interrupts;
    bool physical = false;
    switch (ai->GetBot()->getClass())
    {
    case CLASS_ROGUE:   interrupts = { "kick" }; physical = true; break;
    case CLASS_WARRIOR: interrupts = { "shield bash", "pummel" }; physical = true; break;
    case CLASS_MAGE:    interrupts = { "counterspell" }; break;
    case CLASS_SHAMAN:  interrupts = { "earth shock" }; break;
    case CLASS_PRIEST:  interrupts = { "silence" }; break;
    default: return nullptr;
    }

    // Circle of Healing heals the whole council, Divine Wrath hits a random player for 5000 + 10000.
    // Empowered Smite on her tank is left alone so the interrupts are ready for those two.
    Unit* malande = FindBoss(ai, NPC_MALANDE);
    if (!malande || (!malande->FindCurrentSpellBySpellId(SPELL_CIRCLE_OF_HEALING) && !malande->FindCurrentSpellBySpellId(SPELL_DIVINE_WRATH)))
        return nullptr;

    if (!council::InterruptLands(physical, malande->HasAura(SPELL_BLESSING_OF_PROTECTION), malande->HasAura(SPELL_BLESSING_OF_SPELL_WARDING)))
        return nullptr;

    for (const std::string& interrupt : interrupts)
    {
        if (ai->IsInterruptableSpellCasting(malande, interrupt) && ai->CanCastSpell(interrupt, malande, 0))
        {
            spell = interrupt;
            return malande;
        }
    }

    return nullptr;
}

bool HasDangerousCouncilAura(Unit* unit)
{
    return unit->HasAura(SPELL_CONSECRATION) || unit->HasAura(SPELL_BLIZZARD) || unit->HasAura(SPELL_FLAMESTRIKE);
}

// ---------------------------------------------------------------------------
// Illidan Stormrage
// ---------------------------------------------------------------------------

bool IsIllidanDefeated(Unit* illidan)
{
    // Death prevention keeps Illidan at 1 hp for the outro (SPELL_DEATH 41218)
    return !illidan || illidan->GetHealth() <= 1 || illidan->HasAura(41218);
}

int GetIllidanPhase(Unit* illidan)
{
    if (IsIllidanDefeated(illidan) || illidan->HasAura(SPELL_SHADOW_PRISON))
        return -1;

    const bool uninteractible = illidan->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
    if (uninteractible)
    {
        // Transition from Phase 2 to Phase 3: flies to (or stands on) the landing point
        float x = 0.0f, y = 0.0f, z = 0.0f;
        const bool hasDestination = illidan->GetMotionMaster()->GetDestination(x, y, z);
        const float landX = ILLIDAN_LANDING_POSITION.getX();
        const float landY = ILLIDAN_LANDING_POSITION.getY();
        if ((hasDestination && Dist2d(x, y, landX, landY) < 0.5f) || raidboss::ExactDist2d(illidan, landX, landY) < 0.5f)
            return 0;

        // Phase 2: flying
        if (illidan->IsLevitating() || illidan->IsHovering())
            return 2;

        // Maiev transition (Phase 4/3 -> 5)
        return -1;
    }

    if (illidan->GetHealthPercent() > 65.0f)
        return 1;

    if (!illidan->HasAura(SPELL_CAGED) &&
        (illidan->HasAura(SPELL_DEMON_FORM) || illidan->HasAura(SPELL_DEMON_TRANSFORM_1) ||
         illidan->HasAura(SPELL_DEMON_TRANSFORM_2) || illidan->HasAura(SPELL_DEMON_TRANSFORM_3)))
    {
        return 4;
    }

    if (illidan->GetHealthPercent() > 30.0f)
        return 3;

    return 5;
}

std::vector<Unit*> GetAllFlameCrashes(Player* bot)
{
    return raidboss::FindUnits(bot, NPC_FLAME_CRASH, 30.0f);
}

void GetFlamesOfAzzinoth(Player* bot, Unit*& eastFlame, Unit*& westFlame)
{
    eastFlame = nullptr;
    westFlame = nullptr;

    PlayerbotAI* ai = bot->GetPlayerbotAI();
    if (!ai)
        return;

    const uint32 instanceId = bot->GetInstanceId();
    int64 raw = 0;
    if (GetState(BtState::IllidanEastFlame, instanceId, raw))
    {
        Unit* unit = ai->GetUnit(ObjectGuid(uint64(raw)));
        if (unit && unit->IsAlive())
            eastFlame = unit;
    }

    if (GetState(BtState::IllidanWestFlame, instanceId, raw))
    {
        Unit* unit = ai->GetUnit(ObjectGuid(uint64(raw)));
        if (unit && unit->IsAlive())
            westFlame = unit;
    }
}

// (1) an assistant Warlock (real player or bot), (2) otherwise the first Warlock bot
Player* GetIllidanWarlockTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* fallbackWarlock = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_WARLOCK)
            continue;

        if (group->IsAssistant(member->GetObjectGuid()))
            return member;

        if (!fallbackWarlock && member->GetPlayerbotAI())
            fallbackWarlock = member;
    }

    return fallbackWarlock;
}

bool HasParasiticShadowfiend(Player* member)
{
    return member && (member->HasAura(SPELL_PARASITIC_SHADOWFIEND_1) || member->HasAura(SPELL_PARASITIC_SHADOWFIEND_2));
}

// First hunter bot without Parasitic Shadowfiend
Player* GetIllidanTrapperHunter(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && member->GetPlayerbotAI() &&
            !HasParasiticShadowfiend(member))
        {
            return member;
        }
    }

    return nullptr;
}

Player* GetBotWithParasiticShadowfiend(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (member && member->IsAlive() && member->GetPlayerbotAI() && HasParasiticShadowfiend(member))
            return member;
    }

    return nullptr;
}

EyeBlastDangerArea GetEyeBlastDangerArea(Player* bot)
{
    Unit* trigger = FindNearest(bot, NPC_ILLIDAN_DB_TARGET, 100.0f);
    if (!trigger)
        return {};

    float destX = 0.0f, destY = 0.0f, destZ = 0.0f;
    if (!trigger->GetMotionMaster()->GetDestination(destX, destY, destZ))
        return {};

    EyeBlastDangerArea area;
    area.startX = trigger->GetPositionX();
    area.startY = trigger->GetPositionY();
    area.endX = destX;
    area.endY = destY;
    if (Dist2d(area.startX, area.startY, area.endX, area.endY) < 0.1f)
        return {};

    area.width = 9.0f;
    return area;
}

bool IsPositionInEyeBlastDangerArea(float x, float y, const EyeBlastDangerArea& area)
{
    const float dx = area.endX - area.startX;
    const float dy = area.endY - area.startY;
    const float length = Dist2d(area.startX, area.startY, area.endX, area.endY);
    if (length < 0.1f)
        return false;

    const float projection = std::clamp(((x - area.startX) * dx + (y - area.startY) * dy) / (length * length), 0.0f, 1.0f);
    const float closestX = area.startX + projection * dx;
    const float closestY = area.startY + projection * dy;
    return Dist2d(x, y, closestX, closestY) < area.width;
}

GameObject* FindNearestTrap(PlayerbotAI* ai)
{
    std::vector<GameObject*> traps = raidboss::FindGameObjects(ai->GetBot(), GO_SHADOW_TRAP, 60.0f);
    GameObject* nearest = nullptr;
    for (GameObject* trap : traps)
    {
        if (!nearest || ai->GetBot()->GetDistance(trap) < ai->GetBot()->GetDistance(nearest))
            nearest = trap;
    }

    return nearest;
}

}
}
