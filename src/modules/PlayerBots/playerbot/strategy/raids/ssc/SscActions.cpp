// Ported from mod-playerbots (src/Ai/Raid/SSC/SSCActions.cpp, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "SscActions.h"
#include "SscHelpers.h"
#include "playerbot/strategy/raids/RaidBossStrategies.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "Loot/LootMgr.h"
#include "Entities/DynamicObject.h"
#include "Entities/Totem.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include <cmath>

using namespace ai;
using namespace ai::ssc;

namespace
{
    // Misdirection on the tank, then a Steady Shot to transfer the threat
    bool Misdirect(PlayerbotAI* ai, Player* tank, Unit* boss)
    {
        if (!tank || !boss)
            return false;

        if (raidboss::TryCast(ai, "misdirection", tank))
            return true;

        return ai->GetBot()->HasAura(SPELL_MISDIRECTION) && raidboss::TryCast(ai, "steady shot", boss);
    }

    class ToxicPoolCheck
    {
    public:
        ToxicPoolCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) {}
        WorldObject const& GetFocusObject() const { return *i_obj; }
        bool operator()(DynamicObject* u) { return u && u->GetSpellId() == SPELL_TOXIC_POOL && i_obj->IsWithinDistInMap(u, i_range); }

    private:
        WorldObject const* i_obj;
        float i_range;
    };

    float NormalizeAngle(float angle)
    {
        while (angle < 0.0f)
            angle += 2.0f * M_PI_F;
        while (angle >= 2.0f * M_PI_F)
            angle -= 2.0f * M_PI_F;
        return angle;
    }
}

// ---------------------------------------------------------------------------
// Base classes
// ---------------------------------------------------------------------------

bool SscMovementAction::MoveToPosition(const WorldPosition& position)
{
    if (raidboss::ExactDist2d(bot, position.getX(), position.getY()) < 0.5f)
        return false;

    if (IsReaction())
        bot->InterruptNonMeleeSpells(true);
    return MoveTo(bot->GetMapId(), position.getX(), position.getY(), position.getZ(), false, IsReaction(), false, true);
}

bool SscMovementAction::MoveAwayFrom(float x, float y, float distance)
{
    return MoveToPosition(GetPointAwayFrom(bot, x, y, distance + 0.5f));
}

bool SscAttackAction::AttackUnit(Unit* target)
{
    return target && Attack(GetMaster(), target);
}

bool SscAttackAction::IsCurrentTarget(Unit* target)
{
    return target && AI_VALUE(ObjectGuid, "current target") == target->GetObjectGuid();
}

bool SscAttackAction::MoveToPosition(const WorldPosition& position)
{
    if (raidboss::ExactDist2d(bot, position.getX(), position.getY()) < 0.5f)
        return false;

    if (IsReaction())
        bot->InterruptNonMeleeSpells(true);
    return MoveTo(bot->GetMapId(), position.getX(), position.getY(), position.getZ(), false, IsReaction(), false, true);
}

bool SscAttackAction::MoveAwayFrom(WorldObject* object, float distance)
{
    return MoveToPosition(GetPointAwayFrom(bot, object->GetPositionX(), object->GetPositionY(), distance + 0.5f));
}

bool SscAttackAction::StepToPosition(const WorldPosition& position, float arrivalDistance, float maxStep)
{
    if (raidboss::ExactDist2d(bot, position.getX(), position.getY()) <= arrivalDistance)
        return false;

    return MoveToPosition(GetStepTowards(bot, position, maxStep));
}

// ---------------------------------------------------------------------------
// General
// ---------------------------------------------------------------------------

bool SscEraseTimersAndTrackersAction::Execute(Event& event)
{
    if (!NeedsStateReset(ai))
        return false;

    WithState(bot, [](SscInstanceState& state) { state = SscInstanceState(); });
    return true;
}
// ---------------------------------------------------------------------------
// Trash
// ---------------------------------------------------------------------------

// Move out of the toxic pool left behind by some colossi upon death
bool SscUnderbogColossusEscapeToxicPoolAction::Execute(Event& event)
{
    std::list<DynamicObject*> pools;
    ToxicPoolCheck check(bot, 40.0f);
    MaNGOS::DynamicObjectListSearcher<ToxicPoolCheck> searcher(pools, check);
    Cell::VisitAllObjects(bot, searcher, 40.0f);

    DynamicObject* pool = nullptr;
    for (DynamicObject* candidate : pools)
    {
        if (!pool || bot->GetDistance(candidate) < bot->GetDistance(pool))
            pool = candidate;
    }

    if (!pool)
        return false;

    const float radius = pool->GetRadius();
    if (radius <= 0.0f || raidboss::ExactDist2d(bot, pool) > radius + 1.0f)
        return false;

    constexpr float bufferDist = 3.0f;
    return MoveAwayFrom(pool, radius + bufferDist);
}

bool SscGreyheartTidecallerMarkWaterElementalTotemAction::Execute(Event& event)
{
    Unit* totem = FindSscUnit(ai, NPC_WATER_ELEMENTAL_TOTEM, 60.0f);
    return totem && raidboss::MarkTargetWithSkull(bot, totem);
}

// ---------------------------------------------------------------------------
// Hydross the Unstable <Duke of Currents>
// The boss changes form when he leaves / enters the Cleansing Field around his spawn,
// the frost spot is inside it and the nature spot outside.
// ---------------------------------------------------------------------------

// (1) When tanking, move to designated tanking spot on frost side
// (2) 1 second after 100% Mark of Hydross, move to nature tank's spot to hand off boss
// (3) When Hydross is in nature form, move back to frost tank spot and wait for transition
bool SscHydrossPositionFrostTankAction::Execute(Event& event)
{
    Unit* hydross = FindSscUnit(ai, NPC_HYDROSS);
    if (!hydross)
        return false;

    const bool corrupted = hydross->HasAura(SPELL_CORRUPTION);
    const bool tanking = hydross->GetVictim() == bot && bot->CanReachWithMeleeAttack(hydross);

    UpdateHydrossState(ai);
    const time_t changeTime = GetState(bot).hydrossChangeToNaturePhase;
    if (!corrupted && !changeTime)
    {
        if (raidboss::MarkTargetWithSquare(bot, hydross))
            return true;

        raidboss::SetRtiTarget(ai, "square");
        if (!IsCurrentTarget(hydross))
            return AttackUnit(hydross);

        if (tanking)
            return StepToPosition(HYDROSS_FROST_TANK_POSITION, 2.0f, 5.0f);
    }

    if (!corrupted && changeTime && tanking)
    {
        if (changeTime && time(nullptr) - changeTime >= 1)
        {
            if (StepToPosition(HYDROSS_NATURE_TANK_POSITION, 2.0f, 5.0f))
                return true;

            // Keep threat until the form actually changes.
            return false;
        }
    }

    if (corrupted)
    {
        if (StopAttacking(ai))
            return true;
        // Do not drag the boss back over the boundary before the other tank takes over.
        if (hydross->GetVictim() != raidboss::GetGroupAssistTank(bot, 0))
            return false;
        return MoveToPosition(HYDROSS_FROST_TANK_POSITION);
    }

    return false;
}

// (1) When tanking, move to designated tanking spot on nature side
// (2) 1 second after 100% Mark of Corruption, move to frost tank's spot to hand off boss
// (3) When Hydross is in frost form, move back to nature tank spot and wait for transition
bool SscHydrossPositionNatureTankAction::Execute(Event& event)
{
    Unit* hydross = FindSscUnit(ai, NPC_HYDROSS);
    if (!hydross)
        return false;

    const bool corrupted = hydross->HasAura(SPELL_CORRUPTION);
    const bool tanking = hydross->GetVictim() == bot && bot->CanReachWithMeleeAttack(hydross);

    UpdateHydrossState(ai);
    const time_t changeTime = GetState(bot).hydrossChangeToFrostPhase;
    if (corrupted && !changeTime)
    {
        if (raidboss::MarkTargetWithTriangle(bot, hydross))
            return true;

        raidboss::SetRtiTarget(ai, "triangle");
        if (!IsCurrentTarget(hydross))
            return AttackUnit(hydross);

        if (tanking)
            return StepToPosition(HYDROSS_NATURE_TANK_POSITION, 2.0f, 5.0f);
    }

    if (corrupted && changeTime && tanking)
    {
        if (changeTime && time(nullptr) - changeTime >= 1)
        {
            if (StepToPosition(HYDROSS_FROST_TANK_POSITION, 2.0f, 5.0f))
                return true;

            return false;
        }
    }

    if (!corrupted)
    {
        if (StopAttacking(ai))
            return true;
        if (hydross->GetVictim() != raidboss::GetGroupMainTank(bot))
            return false;
        return MoveToPosition(HYDROSS_NATURE_TANK_POSITION);
    }

    return false;
}

bool SscHydrossPrioritizeElementalAddsAction::Execute(Event& event)
{
    Unit* elemental = FindSscUnit(ai, NPC_PURE_SPAWN_OF_HYDROSS);
    if (!elemental)
        elemental = FindSscUnit(ai, NPC_TAINTED_SPAWN_OF_HYDROSS);

    if (!elemental)
        return false;

    if (raidboss::MarkTargetWithSkull(bot, elemental))
        return true;

    raidboss::SetRtiTarget(ai, "skull");
    return !IsCurrentTarget(elemental) && AttackUnit(elemental);
}

// To mitigate the effect of Water Tomb / Vile Sludge
bool SscHydrossFrostPhaseSpreadOutAction::Execute(Event& event)
{
    constexpr float safeDistance = 6.0f;
    const uint32 now = WorldTimer::getMSTime();
    if (lastMoveTime && now >= lastMoveTime && now - lastMoveTime < 1000)
        return false;

    Player* nearestPlayer = raidboss::GetNearestPlayerInRadius(bot, safeDistance);
    if (!nearestPlayer)
        return false;

    lastMoveTime = now;
    return MoveAwayFrom(nearestPlayer, safeDistance);
}

bool SscHydrossMisdirectBossToTankAction::Execute(Event& event)
{
    Unit* hydross = FindSscUnit(ai, NPC_HYDROSS);
    if (!hydross)
        return false;

    if (!hydross->HasAura(SPELL_CORRUPTION) && HasNoMarkOfHydross(bot))
        return Misdirect(ai, raidboss::GetGroupMainTank(bot), hydross);

    if (hydross->HasAura(SPELL_CORRUPTION) && HasNoMarkOfCorruption(bot))
        return Misdirect(ai, raidboss::GetGroupAssistTank(bot, 0), hydross);

    return false;
}

bool SscHydrossStopDpsUponPhaseChangeAction::Execute(Event& event)
{
    return ShouldPauseHydrossDps(ai) && StopAttacking(ai);
}

bool SscHydrossManageTimersAction::Execute(Event& event)
{
    return UpdateHydrossState(ai);
}
// ---------------------------------------------------------------------------
// The Lurker Below
// ---------------------------------------------------------------------------

// Run around behind Lurker during Spout. Bots in the water are safe already
// (the "tbc raid" strategy jumps into the water first, this is the fallback).
bool SscLurkerRunAroundBehindBossAction::Execute(Event& event)
{
    Unit* lurker = FindSscUnit(ai, NPC_LURKER_BELOW);
    if (!lurker || bot->IsInWater())
        return false;

    const float radius = frand(19.0f, 20.0f);
    const float botAngle = std::atan2(bot->GetPositionY() - lurker->GetPositionY(), bot->GetPositionX() - lurker->GetPositionX());
    const float relativeAngle = NormalizeAngle(botAngle - lurker->GetOrientation());
    constexpr float safeArc = M_PI_F / 2.0f;

    float diffFromBehind = relativeAngle - M_PI_F;
    if (!raidboss::IsMainTank(bot) && std::fabs(diffFromBehind) > safeArc / 2.0f)
    {
        const float tangentAngle = botAngle + (relativeAngle > M_PI_F ? -0.1f : 0.1f);
        return MoveToPosition(lurker->GetPositionX() + radius * std::cos(tangentAngle),
                              lurker->GetPositionY() + radius * std::sin(tangentAngle), lurker->GetPositionZ());
    }

    const float behindAngle = lurker->GetOrientation() + M_PI_F + frand(-0.5f, 0.5f) * safeArc;
    const float targetX = lurker->GetPositionX() + radius * std::cos(behindAngle);
    const float targetY = lurker->GetPositionY() + radius * std::sin(behindAngle);
    if (raidboss::ExactDist2d(bot, targetX, targetY) > 2.0f)
        return MoveToPosition(targetX, targetY, lurker->GetPositionZ());

    return false;
}

bool SscLurkerPositionMainTankAction::Execute(Event& event)
{
    Unit* lurker = FindSscUnit(ai, NPC_LURKER_BELOW);
    if (!lurker)
        return false;

    if (!IsCurrentTarget(lurker))
        return AttackUnit(lurker);

    if (raidboss::ExactDist2d(bot, LURKER_MAIN_TANK_POSITION.getX(), LURKER_MAIN_TANK_POSITION.getY()) > 1.0f)
        return MoveToPosition(LURKER_MAIN_TANK_POSITION);

    return false;
}

// Assign ranged positions within a 120-degree arc behind Lurker (against Geyser)
bool SscLurkerSpreadRangedInArcAction::Execute(Event& event)
{
    Unit* lurker = FindSscUnit(ai, NPC_LURKER_BELOW);
    if (!lurker)
        return false;

    std::vector<Player*> rangedMembers;
    for (Player* member : raidboss::GetGroupMembers(bot, 200.0f, true))
    {
        if (raidboss::IsRangedPlayer(member))
            rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    const ObjectGuid guid = bot->GetObjectGuid();
    WorldPosition position;
    bool found = false;
    WithState(bot, [&](SscInstanceState& state)
    {
        auto it = state.lurkerRangedPositions.find(guid);
        if (it == state.lurkerRangedPositions.end())
        {
            const size_t count = rangedMembers.size();
            const int index = std::max(0, raidboss::GetIndexOf(bot, rangedMembers));

            constexpr float arcSpan = 2.0f * M_PI_F / 3.0f;
            constexpr float arcCenter = 2.262f;
            constexpr float arcStart = arcCenter - arcSpan / 2.0f;
            const float angle = count == 1 ? arcCenter : arcStart + arcSpan * float(index) / float(count - 1);
            constexpr float radius = 27.0f;

            it = state.lurkerRangedPositions.emplace(guid, WorldPosition(SSC_MAP_ID,
                lurker->GetPositionX() + radius * std::sin(angle),
                lurker->GetPositionY() + radius * std::cos(angle), lurker->GetPositionZ())).first;
        }

        position = it->second;
        found = true;
    });

    if (!found || raidboss::ExactDist2d(bot, position.getX(), position.getY()) <= 2.0f)
        return false;

    return MoveToPosition(position);
}

// During the submerge phase, if there are >= 3 tanks in the raid, the first 3 will each pick up 1 Guardian
bool SscLurkerTanksPickUpAddsAction::Execute(Event& event)
{
    Player* tanks[3] = { raidboss::GetGroupMainTank(bot), raidboss::GetGroupAssistTank(bot, 0), raidboss::GetGroupAssistTank(bot, 1) };
    if (!tanks[0] || !tanks[1] || !tanks[2])
        return false;

    std::vector<Unit*> guardians = raidboss::FindUnits(bot, NPC_COILFANG_GUARDIAN, 150.0f);
    if (guardians.size() < 3)
        return false;

    // Same order for every bot
    std::sort(guardians.begin(), guardians.end(), [](Unit* a, Unit* b) { return a->GetObjectGuid().GetCounter() < b->GetObjectGuid().GetCounter(); });

    const uint8 icons[3] = { raidboss::ICON_STAR, raidboss::ICON_CIRCLE, raidboss::ICON_DIAMOND };
    const char* rtiNames[3] = { "star", "circle", "diamond" };
    for (size_t i = 0; i < 3; ++i)
    {
        if (bot != tanks[i])
            continue;

        if (raidboss::MarkTargetWithIcon(bot, guardians[i], icons[i]))
            return true;

        raidboss::SetRtiTarget(ai, rtiNames[i]);
        return !IsCurrentTarget(guardians[i]) && AttackUnit(guardians[i]);
    }

    return false;
}

bool SscLurkerManageSpoutTimerAction::Execute(Event& event)
{
    Unit* lurker = FindSscUnit(ai, NPC_LURKER_BELOW);
    if (!lurker)
        return false;

    const time_t now = time(nullptr);
    const bool spouting = IsLurkerSpouting(lurker);
    bool changed = false;
    WithState(bot, [&](SscInstanceState& state)
    {
        // Keep the Spout handling active for 2 more seconds after the rotation stopped
        if (spouting && state.lurkerSpoutEnd < now + 2)
        {
            state.lurkerSpoutEnd = now + 2;
            changed = true;
        }
        else if (!spouting && state.lurkerSpoutEnd && state.lurkerSpoutEnd <= now)
        {
            state.lurkerSpoutEnd = 0;
            changed = true;
        }
    });

    return changed;
}

// ---------------------------------------------------------------------------
// Leotheras the Blind
// ---------------------------------------------------------------------------

bool SscLeotherasTargetSpellbindersAction::Execute(Event& event)
{
    Unit* spellbinder = FindSscUnit(ai, NPC_GREYHEART_SPELLBINDER, 100.0f);
    return spellbinder && raidboss::MarkTargetWithSkull(bot, spellbinder);
}

// Warlock tank: tanks the demon form with Searing Pain (see GetLeotherasDemonFormTank)
bool SscLeotherasDemonFormTankAttackBossAction::Execute(Event& event)
{
    if (GetMyInnerDemon(bot))
        return false;

    Unit* leotherasDemon = GetActiveLeotherasDemon(ai);
    if (!leotherasDemon)
        return false;

    if (raidboss::MarkTargetWithSquare(bot, leotherasDemon))
        return true;

    raidboss::SetRtiTarget(ai, "square");
    if (!IsCurrentTarget(leotherasDemon) && AttackUnit(leotherasDemon))
        return true;

    return raidboss::TryCast(ai, "searing pain", leotherasDemon);
}

// Stop melee tanks from attacking upon transformation so they don't take aggro
// Applies only if there is a Warlock tank present
bool SscLeotherasMeleeTanksDontAttackDemonFormAction::Execute(Event& event)
{
    if (!bot->GetVictim() && !AI_VALUE(ObjectGuid, "current target")) return false;
    return StopAttacking(ai);
}

// Keep enough distance from Leotheras and spread to prepare for Whirlwind,
// and stay away from the Warlock tank to avoid Chaos Blasts
bool SscLeotherasPositionRangedAction::Execute(Event& event)
{
    const uint32 now = WorldTimer::getMSTime();
    constexpr float safeDistFromBoss = 15.0f;
    Unit* leotherasHuman = GetLeotherasHuman(ai);
    if (leotherasHuman && raidboss::ExactDist2d(bot, leotherasHuman) < safeDistFromBoss && leotherasHuman->GetVictim() != bot)
    {
        if (lastMoveTime && now >= lastMoveTime && now - lastMoveTime < 500)
            return false;

        lastMoveTime = now;
        return MoveAwayFrom(leotherasHuman, safeDistFromBoss);
    }

    if (!GetActiveLeotherasDemon(ai))
        return false;

    Player* demonTank = GetLeotherasDemonFormTank(bot);
    for (Player* member : raidboss::GetGroupMembers(bot, 30.0f, false))
    {
        const float safeDistFromMember = member == demonTank ? 10.0f : 6.0f;
        if (raidboss::ExactDist2d(bot, member) < safeDistFromMember)
            return MoveAwayFrom(member, safeDistFromMember);
    }

    return false;
}

bool SscLeotherasRunAwayFromWhirlwindAction::Execute(Event& event)
{
    Unit* leotherasHuman = GetLeotherasHuman(ai);
    constexpr float safeDistance = 25.0f;
    if (!leotherasHuman || raidboss::ExactDist2d(bot, leotherasHuman) >= safeDistance)
        return false;

    StopAttacking(ai);
    return MoveAwayFrom(leotherasHuman, safeDistance);
}

// Likely unnecessary with a Warlock tank: if a melee tank is used,
// other melee need to run away after too many Chaos Blast stacks
bool SscLeotherasMeleeDpsRunAwayFromBossAction::Execute(Event& event)
{
    if (raidboss::TryCast(ai, "cloak of shadows", bot))
        return true;

    Unit* leotheras = GetPhase2LeotherasDemon(ai);
    Unit* demonVictim = leotheras ? leotheras->GetVictim() : nullptr;
    if (!demonVictim || demonVictim == bot)
        return false;

    constexpr float safeDistance = 10.0f;
    if (raidboss::ExactDist2d(bot, demonVictim) < safeDistance)
        return MoveAwayFrom(demonVictim, safeDistance);

    return false;
}

// Hardcoded actions for healers and bear tanks to kill Inner Demons
bool SscLeotherasDestroyInnerDemonAction::Execute(Event& event)
{
    Unit* innerDemon = GetMyInnerDemon(bot);
    if (!innerDemon)
        return false;

    auto* debug = dynamic_cast<RaidBossDebugValue*>(context->GetUntypedValue("raid boss debug"));
    if (debug && debug->Get() && time(nullptr) - debug->lastLog["inner demon"] >= 2)
    {
        debug->lastLog["inner demon"] = time(nullptr);
        sLog.outString("[RaidInnerDemon] bot=%s demon=%u owner=%u hp=%.2f distance=%.2f los=%u selected=%u",
            bot->GetName(), innerDemon->GetObjectGuid().GetCounter(), innerDemon->GetSpawnerGuid().GetCounter(),
            innerDemon->GetHealthPercent(), raidboss::ExactDist2d(bot, innerDemon), bot->IsWithinLOSInMap(innerDemon), IsCurrentTarget(innerDemon));
    }
    if (!IsCurrentTarget(innerDemon)) return AttackUnit(innerDemon);
    if (raidboss::ExactDist2d(bot, innerDemon) > 22.0f || !bot->IsWithinLOSInMap(innerDemon))
    {
        const bool moved = MoveTo(SSC_MAP_ID, innerDemon->GetPositionX(), innerDemon->GetPositionY(), innerDemon->GetPositionZ(), false, false, false, true);
        if (moved) SetDuration(200);
        return moved;
    }

    if (PlayerbotAI::IsTank(bot) && bot->getClass() == CLASS_DRUID && HandleFeralTankStrategy(innerDemon))
        return true;

    if (PlayerbotAI::IsHeal(bot) && HandleHealerStrategy(innerDemon))
        return true;

    // Roles without a strategy need to affirmatively attack their Inner Demons
    // because DPS assist is disabled via multipliers
    return !IsCurrentTarget(innerDemon) && AttackUnit(innerDemon);
}

// At 50% nerfed damage, bears have trouble killing their Inner Demons: switch to cat form
bool SscLeotherasDestroyInnerDemonAction::HandleFeralTankStrategy(Unit* innerDemon)
{
    if (bot->HasAura(SPELL_DIRE_BEAR_FORM))
        bot->RemoveAurasDueToSpell(SPELL_DIRE_BEAR_FORM);

    if (bot->HasAura(SPELL_BEAR_FORM))
        bot->RemoveAurasDueToSpell(SPELL_BEAR_FORM);

    if (!IsCurrentTarget(innerDemon))
        AttackUnit(innerDemon);

    if (!bot->HasAura(SPELL_CAT_FORM))
        return raidboss::TryCast(ai, "cat form", bot);

    if (bot->GetPower(POWER_ENERGY) < 30 && raidboss::TryCast(ai, "tiger's fury", bot))
        return true;

    if (bot->GetComboPoints() >= 4 && raidboss::TryCast(ai, "ferocious bite", innerDemon))
        return true;

    if (bot->GetComboPoints() == 0 && innerDemon->GetHealthPercent() > 25.0f && raidboss::TryCast(ai, "rake", innerDemon))
        return true;

    return raidboss::TryCast(ai, "mangle (cat)", innerDemon);
}

bool SscLeotherasDestroyInnerDemonAction::HandleHealerStrategy(Unit* innerDemon)
{
    switch (bot->getClass())
    {
    case CLASS_DRUID:
        if (bot->HasAura(SPELL_TREE_OF_LIFE))
            bot->RemoveAurasDueToSpell(SPELL_TREE_OF_LIFE);

        return raidboss::TryCast(ai, "barkskin", bot) || raidboss::TryCast(ai, "wrath", innerDemon);
    case CLASS_PALADIN:
        return raidboss::TryCast(ai, "avenging wrath", bot) || raidboss::TryCast(ai, "consecration", bot) ||
               raidboss::TryCast(ai, "exorcism", innerDemon) || raidboss::TryCast(ai, "hammer of wrath", innerDemon) ||
               raidboss::TryCast(ai, "holy shock", innerDemon) || raidboss::TryCast(ai, "judgement", innerDemon) ||
               raidboss::TryCast(ai, "seal of righteousness", bot);
    case CLASS_PRIEST:
        return raidboss::TryCast(ai, "smite", innerDemon);
    case CLASS_SHAMAN:
        return raidboss::TryCast(ai, "earth shock", innerDemon) || raidboss::TryCast(ai, "chain lightning", innerDemon) ||
               raidboss::TryCast(ai, "lightning bolt", innerDemon);
    default:
        return false;
    }
}

// Everybody except the Warlock tank focuses on Leotheras in phase 3,
// the human form tank keeps him 20 yards away from the Shadow of Leotheras
bool SscLeotherasFinalPhaseAssignDpsPriorityAction::Execute(Event& event)
{
    Unit* leotherasHuman = GetLeotherasHuman(ai);
    if (!leotherasHuman)
        return false;

    if (raidboss::MarkTargetWithStar(bot, leotherasHuman))
        return true;

    raidboss::SetRtiTarget(ai, "star");
    if (!IsCurrentTarget(leotherasHuman))
        return AttackUnit(leotherasHuman);

    Unit* leotherasDemon = GetPhase3LeotherasDemon(ai);
    if (!leotherasDemon || leotherasHuman->GetVictim() != bot)
        return false;

    Unit* demonTarget = leotherasDemon->GetVictim();
    constexpr float safeDistanceFromDemon = 20.0f;
    if (demonTarget && raidboss::ExactDist2d(leotherasHuman, demonTarget) < safeDistanceFromDemon)
        return MoveAwayFrom(demonTarget, raidboss::ExactDist2d(bot, demonTarget) + safeDistanceFromDemon);

    return false;
}

// Misdirect to the Warlock tank or to the main tank if there is no Warlock tank
bool SscLeotherasMisdirectBossToDemonFormTankAction::Execute(Event& event)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(ai);
    if (!leotherasDemon)
        return false;

    Player* targetTank = GetLeotherasDemonFormTank(bot);
    if (!targetTank)
        targetTank = raidboss::GetGroupMainTank(bot);

    return Misdirect(ai, targetTank, leotherasDemon);
}

// This does not pause DPS after a Whirlwind, which is also an aggro wipe
bool SscLeotherasManageDpsWaitTimersAction::Execute(Event& event)
{
    Unit* leotheras = FindSscUnit(ai, NPC_LEOTHERAS, 100.0f);
    if (!leotheras)
        return false;

    const time_t now = time(nullptr);
    const bool banished = leotheras->HasAura(SPELL_LEOTHERAS_BANISHED);
    const bool human = GetLeotherasHuman(ai) != nullptr;
    const bool phase2Demon = GetPhase2LeotherasDemon(ai) != nullptr;
    const bool phase3Demon = GetPhase3LeotherasDemon(ai) != nullptr;
    auto* debug = dynamic_cast<RaidBossDebugValue*>(context->GetUntypedValue("raid boss debug"));
    if (debug && debug->Get() && time(nullptr) - debug->lastLog["leotheras"] >= 3)
    {
        debug->lastLog["leotheras"] = time(nullptr);
        Player* tank = GetLeotherasDemonFormTank(bot);
        sLog.outString("[RaidLeotheras] instance=%u human=%u demon=%u final=%u whirlwind=%u demonTank=%s victim=%s bossHp=%.2f",
            bot->GetInstanceId(), human, phase2Demon, phase3Demon,
            leotheras->HasAura(SPELL_WHIRLWIND) || leotheras->HasAura(SPELL_WHIRLWIND_CHANNEL),
            tank ? tank->GetName() : "none", leotheras->GetVictim() ? leotheras->GetVictim()->GetName() : "none", leotheras->GetHealthPercent());
    }


    bool changed = false;
    WithState(bot, [&](SscInstanceState& state)
    {
        // Encounter start/reset: clear all timers
        if (banished && (state.leotherasHumanFormDpsWait || state.leotherasDemonFormDpsWait || state.leotherasFinalPhaseDpsWait))
        {
            state.leotherasHumanFormDpsWait = state.leotherasDemonFormDpsWait = state.leotherasFinalPhaseDpsWait = 0;
            changed = true;
        }

        if (human && !phase3Demon)
        {
            if (!state.leotherasHumanFormDpsWait || state.leotherasDemonFormDpsWait)
                changed = true;
            if (!state.leotherasHumanFormDpsWait)
                state.leotherasHumanFormDpsWait = now;
            state.leotherasDemonFormDpsWait = 0;
        }
        else if (phase2Demon)
        {
            if (!state.leotherasDemonFormDpsWait || state.leotherasHumanFormDpsWait)
                changed = true;
            if (!state.leotherasDemonFormDpsWait)
                state.leotherasDemonFormDpsWait = now;
            state.leotherasHumanFormDpsWait = 0;
        }
        else if (human && phase3Demon)
        {
            if (!state.leotherasFinalPhaseDpsWait || state.leotherasHumanFormDpsWait || state.leotherasDemonFormDpsWait)
                changed = true;
            if (!state.leotherasFinalPhaseDpsWait)
                state.leotherasFinalPhaseDpsWait = now;
            state.leotherasHumanFormDpsWait = state.leotherasDemonFormDpsWait = 0;
        }
    });

    return changed;
}

// ---------------------------------------------------------------------------
// Fathom-Lord Karathress
// 4 tanks are required for the full strategy, at least 2 to separate Caribdis from the others
// ---------------------------------------------------------------------------

namespace
{
    template <class T>
    bool TankAddAtPosition(T* action, PlayerbotAI* ai, Player* bot, Unit* target, uint8 icon, const char* rti,
                           const WorldPosition& position, float maxStep, bool needMelee, std::function<bool(Unit*)> isCurrent,
                           std::function<bool(Unit*)> attack, std::function<bool(const WorldPosition&, float, float)> step)
    {
        if (!target)
            return false;

        if (raidboss::MarkTargetWithIcon(bot, target, icon))
            return true;

        raidboss::SetRtiTarget(ai, rti);
        if (!isCurrent(target))
            return attack(target);

        if (target->GetVictim() == bot && (!needMelee || bot->CanReachWithMeleeAttack(target)))
            return step(position, 3.0f, maxStep);

        return false;
    }
}

#define SSC_TANK_ADD(target, icon, rti, position, maxStep, needMelee) \
    TankAddAtPosition(this, ai, bot, target, icon, rti, position, maxStep, needMelee, \
        [this](Unit* u) { return IsCurrentTarget(u); }, [this](Unit* u) { return AttackUnit(u); }, \
        [this](const WorldPosition& p, float a, float s) { return StepToPosition(p, a, s); })

// Karathress is tanked near his starting position
bool SscKarathressMainTankPositionBossAction::Execute(Event& event)
{
    return SSC_TANK_ADD(FindSscUnit(ai, NPC_KARATHRESS), raidboss::ICON_TRIANGLE, "triangle", KARATHRESS_TANK_POSITION, 5.0f, true);
}

// Caribdis is pulled far to the West in the corner (best with a Warrior or Druid tank for interrupts)
bool SscKarathressFirstAssistTankPositionCaribdisAction::Execute(Event& event)
{
    return SSC_TANK_ADD(FindSscUnit(ai, NPC_CARIBDIS), raidboss::ICON_DIAMOND, "diamond", CARIBDIS_TANK_POSITION, 10.0f, false);
}

// Sharkkis is pulled North to the other side of the ramp
bool SscKarathressSecondAssistTankPositionSharkkisAction::Execute(Event& event)
{
    return SSC_TANK_ADD(FindSscUnit(ai, NPC_SHARKKIS), raidboss::ICON_STAR, "star", SHARKKIS_TANK_POSITION, 10.0f, true);
}

// Tidalvess is pulled Northwest near the pillar
bool SscKarathressThirdAssistTankPositionTidalvessAction::Execute(Event& event)
{
    return SSC_TANK_ADD(FindSscUnit(ai, NPC_TIDALVESS), raidboss::ICON_CIRCLE, "circle", TIDALVESS_TANK_POSITION, 10.0f, true);
}

#undef SSC_TANK_ADD

// Caribdis's tank spot is far away so a dedicated healer is needed (first healer, assistants first)
bool SscKarathressPositionCaribdisTankHealerAction::Execute(Event& event)
{
    if (raidboss::ExactDist2d(bot, CARIBDIS_HEALER_POSITION.getX(), CARIBDIS_HEALER_POSITION.getY()) <= 3.0f)
        return false;

    return MoveToPosition(GetStepTowards(bot, CARIBDIS_HEALER_POSITION, 10.0f));
}

// Misdirect priority: (1) Caribdis tank, (2) Tidalvess tank, (3) Sharkkis tank
bool SscKarathressMisdirectBossesToTanksAction::Execute(Event& event)
{
    std::vector<Player*> hunters;
    for (Player* member : raidboss::GetGroupMembers(bot, 200.0f, true))
    {
        if (member->getClass() == CLASS_HUNTER && member->GetPlayerbotAI())
            hunters.push_back(member);

        if (hunters.size() >= 3)
            break;
    }

    switch (raidboss::GetIndexOf(bot, hunters))
    {
    case 0: return Misdirect(ai, raidboss::GetGroupAssistTank(bot, 0), FindSscUnit(ai, NPC_CARIBDIS));
    case 1: return Misdirect(ai, raidboss::GetGroupAssistTank(bot, 2), FindSscUnit(ai, NPC_TIDALVESS));
    case 2: return Misdirect(ai, raidboss::GetGroupAssistTank(bot, 1), FindSscUnit(ai, NPC_SHARKKIS));
    default: return false;
    }
}

// Kill order is non-standard because bots handle Cyclones poorly and need more time
// to get Caribdis down than real players (standard is ranged DPS help with Sharkkis first)
bool SscKarathressAssignDpsPriorityAction::Execute(Event& event)
{
    auto focus = [&](Unit* target, uint8 icon, const char* rti) -> bool
    {
        if (raidboss::MarkTargetWithIcon(bot, target, icon))
            return true;

        raidboss::SetRtiTarget(ai, rti);
        return !IsCurrentTarget(target) && AttackUnit(target);
    };

    // Every free damage dealer focuses Spitfire Totems immediately.
    Unit* totem = FindSscUnit(ai, NPC_SPITFIRE_TOTEM, 100.0f);
    if (totem)
    {
        Unit* current = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
        if (current != totem)
            bot->InterruptNonMeleeSpells(true);
        if (focus(totem, raidboss::ICON_SKULL, "skull"))
            return true;

        // Direct movement order, the path between Sharkkis and the totem is sometimes bad
        if (raidboss::IsMeleePlayer(bot) && !bot->CanReachWithMeleeAttack(totem))
            return MoveToPosition(WorldPosition(totem));

        return false;
    }

    // Target priority 2: Tidalvess for all dps
    if (Unit* tidalvess = FindSscUnit(ai, NPC_TIDALVESS))
        return focus(tidalvess, raidboss::ICON_CIRCLE, "circle");

    // Target priority 3: Caribdis for ranged dps
    Unit* caribdis = FindSscUnit(ai, NPC_CARIBDIS);
    if (caribdis && raidboss::IsDps(bot) && raidboss::IsRangedPlayer(bot))
    {
        if (raidboss::MarkTargetWithDiamond(bot, caribdis))
            return true;

        raidboss::SetRtiTarget(ai, "diamond");
        if (raidboss::ExactDist2d(bot, CARIBDIS_RANGED_DPS_POSITION.getX(), CARIBDIS_RANGED_DPS_POSITION.getY()) > 8.0f)
            return MoveToPosition(CARIBDIS_RANGED_DPS_POSITION);

        return !IsCurrentTarget(caribdis) && AttackUnit(caribdis);
    }

    // Target priority 4: Sharkkis for melee dps and, after Caribdis is down, ranged dps also
    if (Unit* sharkkis = FindSscUnit(ai, NPC_SHARKKIS))
        return focus(sharkkis, raidboss::ICON_STAR, "star");

    // Target priority 5: Sharkkis pets for melee
    if (raidboss::IsMeleePlayer(bot))
    {
        if (Unit* sporebat = raidboss::FindUnitByName(ai, "fathom sporebat"))
            return focus(sporebat, raidboss::ICON_CROSS, "cross");

        if (Unit* lurker = raidboss::FindUnitByName(ai, "fathom lurker"))
            return focus(lurker, raidboss::ICON_SQUARE, "square");
    }

    // Target priority 6: Karathress for all dps
    if (Unit* karathress = FindSscUnit(ai, NPC_KARATHRESS))
        return focus(karathress, raidboss::ICON_TRIANGLE, "triangle");

    return false;
}

bool SscKarathressManageDpsTimerAction::Execute(Event& event)
{
    bool changed = false;
    WithState(bot, [&](SscInstanceState& state)
    {
        if (!state.karathressDpsWait)
        {
            state.karathressDpsWait = time(nullptr);
            changed = true;
        }
    });

    return changed;
}

// ---------------------------------------------------------------------------
// Morogrim Tidewalker
// ---------------------------------------------------------------------------

bool SscMorogrimMisdirectBossToMainTankAction::Execute(Event& event)
{
    return Misdirect(ai, raidboss::GetGroupMainTank(bot), FindSscUnit(ai, NPC_MOROGRIM));
}

// Separate tanking positions for phase 1 and phase 2 (Water Globules):
// phase 1 against the Northeast pillar, phase 2 around the pillar into the Northeast corner
bool SscMorogrimMoveBossToTankPositionAction::Execute(Event& event)
{
    Unit* tidewalker = FindSscUnit(ai, NPC_MOROGRIM);
    if (!tidewalker)
        return false;

    if (!IsCurrentTarget(tidewalker))
        return AttackUnit(tidewalker);

    if (tidewalker->GetVictim() != bot || !bot->CanReachWithMeleeAttack(tidewalker))
        return false;

    if (tidewalker->GetHealthPercent() > 26.0f)
        return StepToPosition(TIDEWALKER_PHASE_1_TANK_POSITION, 1.0f, 5.0f);

    const ObjectGuid guid = bot->GetObjectGuid();
    uint8 step = 0;
    WithState(bot, [&](SscInstanceState& state) { step = state.tidewalkerTankStep[guid]; });

    if (step == 0)
    {
        if (StepToPosition(TIDEWALKER_PHASE_TRANSITION_WAYPOINT, 2.0f, 5.0f))
            return true;

        WithState(bot, [&](SscInstanceState& state) { state.tidewalkerTankStep[guid] = 1; });
    }

    return StepToPosition(TIDEWALKER_PHASE_2_TANK_POSITION, 1.0f, 5.0f);
}

// Ranged stack behind the boss in the Northeast corner in phase 2
// (no corresponding action for melee since they will do so anyway)
bool SscMorogrimPhase2RepositionRangedAction::Execute(Event& event)
{
    const ObjectGuid guid = bot->GetObjectGuid();
    uint8 step = 0;
    WithState(bot, [&](SscInstanceState& state) { step = state.tidewalkerRangedStep[guid]; });

    if (step == 0)
    {
        if (raidboss::ExactDist2d(bot, TIDEWALKER_PHASE_TRANSITION_WAYPOINT.getX(), TIDEWALKER_PHASE_TRANSITION_WAYPOINT.getY()) > 2.0f)
            return MoveToPosition(GetStepTowards(bot, TIDEWALKER_PHASE_TRANSITION_WAYPOINT, 10.0f));

        WithState(bot, [&](SscInstanceState& state) { state.tidewalkerRangedStep[guid] = 1; });
    }

    if (raidboss::ExactDist2d(bot, TIDEWALKER_PHASE_2_RANGED_POSITION.getX(), TIDEWALKER_PHASE_2_RANGED_POSITION.getY()) > 1.0f)
        return MoveToPosition(GetStepTowards(bot, TIDEWALKER_PHASE_2_RANGED_POSITION, 10.0f));

    return false;
}

// ---------------------------------------------------------------------------
// Lady Vashj <Coilfang Matron>
// ---------------------------------------------------------------------------

bool SscVashjMainTankPositionBossAction::Execute(Event& event)
{
    Unit* vashj = FindSscUnit(ai, NPC_LADY_VASHJ);
    if (!vashj)
        return false;

    if (!IsCurrentTarget(vashj))
        return AttackUnit(vashj);

    if (vashj->GetVictim() != bot || !bot->CanReachWithMeleeAttack(vashj))
        return false;

    // Phase 1: Vashj in the center of the platform
    if (IsLadyVashjInPhase1(ai))
        return StepToPosition(VASHJ_PLATFORM_CENTER_POSITION, 2.0f, 5.0f);

    // Phase 3: no fixed position, but move Vashj away from the Enchanted Elementals
    if (IsLadyVashjInPhase3(ai))
    {
        Unit* enchanted = FindSscUnit(ai, NPC_ENCHANTED_ELEMENTAL, 60.0f);
        constexpr float safeDistance = 10.0f;
        if (enchanted && raidboss::ExactDist2d(bot, enchanted) < safeDistance)
            return MoveAwayFrom(enchanted, safeDistance);
    }

    return false;
}

// Semicircle around the center of the room (escape paths for Static Charged bots)
bool SscVashjPhase1SpreadRangedInArcAction::Execute(Event& event)
{
    std::vector<Player*> spreadMembers;
    for (Player* member : raidboss::GetGroupMembers(bot, 200.0f, true))
    {
        if (member->GetPlayerbotAI() && raidboss::IsRangedPlayer(member))
            spreadMembers.push_back(member);
    }

    const size_t count = spreadMembers.size();
    if (count == 0)
        return false;

    const int botIndex = std::max(0, raidboss::GetIndexOf(bot, spreadMembers));
    constexpr float arcCenter = M_PI_F / 2.0f; // North
    constexpr float arcSpan = M_PI_F;          // 180 degrees
    constexpr float arcStart = arcCenter - arcSpan / 2.0f;
    const float angle = count == 1 ? arcCenter : arcStart + float(botIndex) / float(count - 1) * arcSpan;

    constexpr float radius = 25.0f;
    const float targetX = VASHJ_PLATFORM_CENTER_POSITION.getX() + radius * std::cos(angle);
    const float targetY = VASHJ_PLATFORM_CENTER_POSITION.getY() + radius * std::sin(angle);

    const ObjectGuid guid = bot->GetObjectGuid();
    bool reached = false;
    WithState(bot, [&](SscInstanceState& state) { reached = state.hasReachedVashjRangedPosition[guid]; });
    if (reached)
        return false;

    if (raidboss::ExactDist2d(bot, targetX, targetY) > 2.0f)
        return MoveToPosition(targetX, targetY, VASHJ_PLATFORM_CENTER_Z);

    WithState(bot, [&](SscInstanceState& state) { state.hasReachedVashjRangedPosition[guid] = true; });
    return false;
}

// Grounding Totem absorbs the Shock Blast on the main tank
bool SscVashjSetGroundingTotemInMainTankGroupAction::Execute(Event& event)
{
    constexpr uint32 NPC_GROUNDING_TOTEM = 5925;
    Player* mainTank = raidboss::GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    Totem* totem = bot->GetTotem(TOTEM_SLOT_AIR);
    if (totem && totem->IsAlive() && totem->GetEntry() == NPC_GROUNDING_TOTEM && totem->IsWithinDistInMap(mainTank, 20.0f))
        return false;

    constexpr float distFromTank = 20.0f;
    if (bot->GetDistance(mainTank) > distFromTank)
        return MoveToPosition(GetStepTowards(bot, WorldPosition(mainTank), bot->GetDistance(mainTank) - distFromTank + 2.0f));

    return raidboss::TryCast(ai, "grounding totem", bot);
}

bool SscVashjMisdirectBossToMainTankAction::Execute(Event& event)
{
    return Misdirect(ai, raidboss::GetGroupMainTank(bot), FindSscUnit(ai, NPC_LADY_VASHJ));
}

bool SscVashjStaticChargeMoveAwayFromGroupAction::Execute(Event& event)
{
    constexpr float safeDistance = 11.0f;

    // If the main tank has Static Charge, other group members move away from him
    Player* mainTank = raidboss::GetGroupMainTank(bot);
    if (mainTank && bot != mainTank && mainTank->HasAura(SPELL_STATIC_CHARGE) && raidboss::ExactDist2d(bot, mainTank) < safeDistance)
        return MoveAwayFrom(mainTank, safeDistance);

    // Any other bot with Static Charge moves away from the group
    if (mainTank != bot && bot->HasAura(SPELL_STATIC_CHARGE))
    {
        if (Player* nearest = raidboss::GetNearestPlayerInRadius(bot, safeDistance))
            return MoveAwayFrom(nearest, safeDistance);
    }

    return false;
}

// Replaces the teleport cheat of mod-playerbots: the designated looter kills the
// Tainted Elemental and loots the Tainted Core (looting roots the bot with Paralyze)
bool SscVashjLootTaintedCoreAction::Execute(Event& event)
{
    for (Player* member : raidboss::GetGroupMembers(bot, 500.0f, true))
    {
        if (HasTaintedCore(member))
            return false;
    }

    Unit* tainted = nullptr;
    for (Unit* unit : raidboss::FindUnits(bot, NPC_TAINTED_ELEMENTAL, 150.0f, false))
    {
        if (!tainted || bot->GetDistance(unit) < bot->GetDistance(tainted))
            tainted = unit;
    }

    if (!tainted)
        return false;

    if (tainted->IsAlive())
    {
        if (raidboss::MarkTargetWithStar(bot, tainted))
            return true;

        if (!IsCurrentTarget(tainted))
            return AttackUnit(tainted);

        if (!bot->CanReachWithMeleeAttack(tainted) && !raidboss::IsRangedPlayer(bot))
            return MoveToPosition(WorldPosition(tainted));

        return false;
    }

    Loot* loot = sLootMgr.GetLoot(bot, tainted->GetObjectGuid());
    if (!loot || !loot->IsItemAlreadyIn(ITEM_TAINTED_CORE))
        return false;

    if (!bot->IsWithinDistInMap(tainted, INTERACTION_DISTANCE - 1.0f))
        return MoveToPosition(WorldPosition(tainted));

    if (bot->IsMoving())
        ai->StopMoving();

    const ObjectGuid corpseGuid = tainted->GetObjectGuid();
    WorldPacket lootPacket(CMSG_LOOT, 8);
    lootPacket << corpseGuid;
    bot->GetSession()->HandleLootOpcode(lootPacket);

    bool looted = false;
    if (Loot* corpseLoot = sLootMgr.GetLoot(bot, corpseGuid))
    {
        LootItemList items;
        corpseLoot->GetLootItemsListFor(bot, items);
        for (LootItem* item : items)
        {
            if (item && item->itemId == ITEM_TAINTED_CORE)
            {
                looted = corpseLoot->SendItem(bot, item) == EQUIP_ERR_OK;
                break;
            }
        }
    }

    WorldPacket releasePacket(CMSG_LOOT_RELEASE, 8);
    releasePacket << corpseGuid;
    bot->GetSession()->HandleLootReleaseOpcode(releasePacket);

    if (looted)
        WithState(bot, [&](SscInstanceState& state) { state.lastCoreInInventoryTime[bot->GetObjectGuid()] = time(nullptr); });

    return looted;
}

bool SscVashjPassTheTaintedCoreAction::MoveToLineup(const WorldPosition& position)
{
    StopAttacking(ai);
    if (raidboss::ExactDist2d(bot, position.getX(), position.getY()) <= 1.0f)
        return false;

    return MoveToPosition(position);
}

// First passer stands at the top of the stairs between the platform center and the looter
bool SscVashjPassTheTaintedCoreAction::LineUpFirstCorePasser(Player* designatedLooter)
{
    if (!designatedLooter)
        return false;

    const ObjectGuid guid = bot->GetObjectGuid();
    WorldPosition position;
    WithState(bot, [&](SscInstanceState& state)
    {
        auto it = state.intendedLineup.find(guid);
        if (it == state.intendedLineup.end())
        {
            const float centerX = VASHJ_PLATFORM_CENTER_POSITION.getX();
            const float centerY = VASHJ_PLATFORM_CENTER_POSITION.getY();
            constexpr float radius = 57.5f;
            const float angle = std::atan2(designatedLooter->GetPositionY() - centerY, designatedLooter->GetPositionX() - centerX);
            it = state.intendedLineup.emplace(guid, WorldPosition(SSC_MAP_ID, centerX + radius * std::cos(angle),
                centerY + radius * std::sin(angle), VASHJ_PLATFORM_CENTER_Z)).first;
        }

        position = it->second;
    });

    return MoveToLineup(position);
}

// Next passer: if the generator is within passing distance of the previous passer, move next
// to the generator; otherwise as close as possible while staying in passing range
bool SscVashjPassTheTaintedCoreAction::LineUpNextCorePasser(Player* previousPasser, Player* beforePreviousPasser, GameObject* generator, bool last)
{
    if (!previousPasser || !generator)
        return false;

    // Third and fourth passers are needed only when the chain does not reach the generator yet
    if (beforePreviousPasser)
    {
        const bool needed =
            (IsPasserInPosition(beforePreviousPasser) && raidboss::ExactDist2d(beforePreviousPasser, generator) > 42.0f) ||
            (IsPasserInPosition(previousPasser) && raidboss::ExactDist2d(previousPasser, generator) > 4.0f);
        if (!needed)
            return false;
    }

    const ObjectGuid guid = bot->GetObjectGuid();
    const ObjectGuid previousGuid = previousPasser->GetObjectGuid();
    bool found = false;
    WorldPosition position;
    WithState(bot, [&](SscInstanceState& state)
    {
        auto itPrevious = state.intendedLineup.find(previousGuid);
        if (itPrevious == state.intendedLineup.end())
            return;

        auto it = state.intendedLineup.find(guid);
        if (it == state.intendedLineup.end())
        {
            const float px = itPrevious->second.getX();
            const float py = itPrevious->second.getY();
            float dx = generator->GetPositionX() - px;
            float dy = generator->GetPositionY() - py;
            const float distToGenerator = std::sqrt(dx * dx + dy * dy);
            if (distToGenerator < 0.1f)
                return;

            dx /= distToGenerator;
            dy /= distToGenerator;

            constexpr float thresholdDist = 40.0f;
            constexpr float nearGeneratorDist = 1.5f;
            constexpr float farDistance = 38.0f;
            float moveDist = farDistance;
            if (last || distToGenerator <= thresholdDist)
                moveDist = std::max(distToGenerator - nearGeneratorDist, 0.0f);

            it = state.intendedLineup.emplace(guid, WorldPosition(SSC_MAP_ID, px + dx * moveDist, py + dy * moveDist, VASHJ_PLATFORM_CENTER_Z)).first;
        }

        position = it->second;
        found = true;
    });

    return found && MoveToLineup(position);
}

// The passer is <= 2 yards of its intended position
bool SscVashjPassTheTaintedCoreAction::IsPasserInPosition(Player* passer)
{
    if (!passer)
        return false;

    const SscInstanceState state = GetState(bot);
    auto it = state.intendedLineup.find(passer->GetObjectGuid());
    return it != state.intendedLineup.end() && raidboss::ExactDist2d(passer, it->second.getX(), it->second.getY()) <= 2.0f;
}

bool SscVashjPassTheTaintedCoreAction::UseCoreOnGenerator(GameObject* generator)
{
    if (!generator || !bot->IsWithinDistInMap(generator, INTERACTION_DISTANCE))
        return false;

    generator->Use(bot);
    bot->DestroyItemCount(ITEM_TAINTED_CORE, 1, true);
    bot->RemoveAurasDueToSpell(SPELL_PARALYZE);

    const std::array<Player*, 5> handlers = GetCoreHandlers(bot);
    WithState(bot, [&](SscInstanceState& state)
    {
        state.vashjGenerator = ObjectGuid();
        state.lastCoreThrow = 0;
        state.intendedLineup.clear();
        for (Player* handler : handlers)
        {
            if (handler)
                state.lastCoreInInventoryTime.erase(handler->GetObjectGuid());
        }
        state.lastCoreInInventoryTime.erase(bot->GetObjectGuid());
    });

    return true;
}

// Throw Key (item spell of the core, 40 yards) gives the core to the receiver
bool SscVashjPassTheTaintedCoreAction::ThrowCore(Player* receiver)
{
    if (!receiver || receiver == bot || HasTaintedCore(receiver) || !bot->IsWithinDistInMap(receiver, 39.0f))
        return false;

    const time_t now = time(nullptr);
    if (GetState(bot).lastCoreThrow + 2 > now)
        return false;

    if (!thrower.UseItem(GetMaster(), ITEM_TAINTED_CORE, receiver))
        return false;

    WithState(bot, [&](SscInstanceState& state)
    {
        state.lastCoreThrow = now;
        state.lastCoreInInventoryTime[bot->GetObjectGuid()] = now;
    });

    return true;
}

bool SscVashjPassTheTaintedCoreAction::Execute(Event& event)
{
    Unit* vashj = FindSscUnit(ai, NPC_LADY_VASHJ);
    if (!vashj)
        return false;

    const std::array<Player*, 5> handlers = GetCoreHandlers(bot);
    const int index = GetCoreHandlerIndex(bot);

    // Generator the chain goes to: the active one closest to the Tainted Elemental, kept until used
    GameObject* generator = nullptr;
    {
        const ObjectGuid stored = GetState(bot).vashjGenerator;
        if (!stored.IsEmpty())
        {
            generator = ai->GetGameObject(stored);
            if (generator && generator->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT))
                generator = nullptr;
        }

        if (!generator)
        {
            WorldObject* from = nullptr;
            std::vector<Unit*> tainted = raidboss::FindUnits(bot, NPC_TAINTED_ELEMENTAL, 150.0f, false);
            if (!tainted.empty())
                from = tainted.front();
            else if (handlers[0])
                from = handlers[0];
            else
                from = bot;

            generator = GetNearestActiveShieldGenerator(bot, from);
            if (generator)
            {
                const ObjectGuid generatorGuid = generator->GetObjectGuid();
                WithState(bot, [&](SscInstanceState& state) { state.vashjGenerator = generatorGuid; });
            }
        }
    }

    if (!generator)
        return false;

    if (!HasTaintedCore(bot))
    {
        switch (index)
        {
        case 1: return LineUpFirstCorePasser(handlers[0]);
        case 2: return LineUpNextCorePasser(handlers[1], nullptr, generator, false);
        case 3: return LineUpNextCorePasser(handlers[2], handlers[1], generator, false);
        case 4: return LineUpNextCorePasser(handlers[3], handlers[2], generator, true);
        default: return false;
        }
    }

    WithState(bot, [&](SscInstanceState& state) { state.lastCoreInInventoryTime[bot->GetObjectGuid()] = time(nullptr); });

    // Close enough: use the core on the generator
    if (UseCoreOnGenerator(generator))
        return true;

    // Designated chain: pass to the next passer once it is in position
    if (index >= 0 && index < 4)
    {
        Player* next = handlers[index + 1];
        if (next && next->IsAlive() && !next->HasAura(SPELL_PARALYZE))
        {
            if (IsPasserInPosition(next))
                return ThrowCore(next);

            if (GetState(bot).intendedLineup.count(next->GetObjectGuid()))
                return false;   // still walking to its spot
        }
    }

    // Fallback (no passer available, or the core is held by somebody outside of the chain):
    // throw the core to the raid member which is closest to the generator
    const float myDistance = bot->GetDistance(generator);
    Player* receiver = nullptr;
    for (Player* member : raidboss::GetGroupMembers(bot, 39.0f, false))
    {
        if (member->HasAura(SPELL_PARALYZE) || HasTaintedCore(member))
            continue;

        const float distance = member->GetDistance(generator);
        if (distance >= myDistance - 5.0f)
            continue;

        if (!receiver || distance < receiver->GetDistance(generator))
            receiver = member;
    }

    return ThrowCore(receiver);
}

// Cores left over outside of phase 2 are useless
bool SscVashjDestroyTaintedCoreAction::Execute(Event& event)
{
    if (!HasTaintedCore(bot))
        return false;

    bot->DestroyItemCount(ITEM_TAINTED_CORE, 1, true);
    bot->RemoveAurasDueToSpell(SPELL_PARALYZE);
    return true;
}

bool SscVashjAssignPhase2AndPhase3DpsPriorityAction::Execute(Event& event)
{
    Unit* vashj = FindSscUnit(ai, NPC_LADY_VASHJ);
    if (!vashj)
        return false;

    const bool phase2 = IsLadyVashjInPhase2(ai);
    const bool phase3 = IsLadyVashjInPhase3(ai);
    const bool ranged = raidboss::IsRangedPlayer(bot);
    const WorldPosition& center = VASHJ_PLATFORM_CENTER_POSITION;

    // Search and attack radius keep the bots from going down the stairs
    const float maxSearchRange = ranged ? 60.0f : 55.0f;
    const float maxPursueRange = maxSearchRange - 5.0f;

    Unit* tainted = nullptr;
    Unit* enchanted = nullptr;
    Unit* elite = nullptr;
    Unit* strider = nullptr;
    Unit* sporebat = nullptr;

    for (ObjectGuid guid : AI_VALUE(std::list<ObjectGuid>, "possible targets no los"))
    {
        Unit* unit = ai->GetUnit(guid);
        if (!IsValidLadyVashjCombatNpc(unit, ai))
            continue;

        if (phase2 && raidboss::ExactDist2d(unit, center.getX(), center.getY()) > maxSearchRange)
            continue;

        switch (unit->GetEntry())
        {
        case NPC_TAINTED_ELEMENTAL:
            tainted = unit;
            break;
        case NPC_ENCHANTED_ELEMENTAL:
            if (!enchanted || raidboss::ExactDist2d(vashj, unit) < raidboss::ExactDist2d(vashj, enchanted))
                enchanted = unit;
            break;
        case NPC_COILFANG_ELITE:
            if (!elite || unit->GetHealthPercent() < elite->GetHealthPercent())
                elite = unit;
            break;
        case NPC_COILFANG_STRIDER:
            if (!strider || unit->GetHealthPercent() < strider->GetHealthPercent())
                strider = unit;
            break;
        case NPC_TOXIC_SPOREBAT:
            if (!sporebat || bot->GetDistance(unit) < bot->GetDistance(sporebat))
                sporebat = unit;
            break;
        default:
            break;
        }
    }

    std::vector<Unit*> targets;
    if (phase2)
    {
        if (ranged)
        {
            // Hunters and Mages prioritize Enchanted Elementals, other ranged DPS the Striders.
            // Without the teleport cheat the reachable Tainted Elementals die first (Tainted Core)
            if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_MAGE)
                targets = { tainted, enchanted, strider, elite };
            else
                targets = { tainted, strider, elite, enchanted };
        }
        else if (raidboss::IsDps(bot))
            targets = { enchanted, elite };
        else if (PlayerbotAI::IsTank(bot))
            targets = { elite, strider, enchanted };
        else
            targets = { enchanted, elite, strider };
    }
    else if (phase3)
    {
        if (PlayerbotAI::IsTank(bot))
        {
            if (raidboss::IsMainTank(bot))
            {
                if (raidboss::MarkTargetWithDiamond(bot, vashj))
                    return true;

                raidboss::SetRtiTarget(ai, "diamond");
                targets = { vashj };
            }
            else
                targets = { elite, strider, enchanted, vashj };
        }
        else if (ranged)
        {
            // Hunters are assigned to kill Sporebats in phase 3
            if (bot->getClass() == CLASS_HUNTER)
                targets = { sporebat, enchanted, strider, elite, vashj };
            else
                targets = { enchanted, strider, elite, vashj };
        }
        else if (raidboss::IsDps(bot))
            targets = { enchanted, elite, vashj };
        else
            targets = { enchanted, elite, strider, vashj };
    }

    Unit* target = nullptr;
    for (Unit* candidate : targets)
    {
        if (candidate && raidboss::ExactDist2d(bot, candidate) <= maxPursueRange)
        {
            target = candidate;
            break;
        }
    }

    Unit* currentTarget = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
    if (currentTarget && !IsValidLadyVashjCombatNpc(currentTarget, ai))
    {
        StopAttacking(ai);
        currentTarget = nullptr;
    }

    if (target && currentTarget != target)
        return AttackUnit(target);

    // If bots have wandered too far from the center, move them back
    if (raidboss::ExactDist2d(bot, vashj) > maxPursueRange)
        return MoveToPosition(GetStepTowards(bot, WorldPosition(vashj), raidboss::ExactDist2d(bot, vashj) - (maxPursueRange - 10.0f)));

    return false;
}

// Striders are not tankable without the Fear Ward cheat of mod-playerbots: everybody except
// the tanks keeps away from them and the casters slow them (poor man's kiting)
bool SscVashjTankAttackAndMoveAwayStriderAction::Execute(Event& event)
{
    Unit* strider = FindSscUnit(ai, NPC_COILFANG_STRIDER);
    if (!strider)
        return false;

    constexpr float safeDistance = 20.0f;
    if (!PlayerbotAI::IsTank(bot) && raidboss::ExactDist2d(bot, strider) < safeDistance)
        return MoveAwayFrom(strider, safeDistance);

    switch (bot->getClass())
    {
    case CLASS_SHAMAN:
        return !ai->HasAura("frost shock", strider) && raidboss::TryCast(ai, "frost shock", strider);
    case CLASS_WARLOCK:
        return !strider->HasAura(SPELL_CURSE_OF_EXHAUSTION) && raidboss::TryCast(ai, "curse of exhaustion", strider);
    case CLASS_MAGE:
        return !strider->HasAura(SPELL_SLOW) && raidboss::TryCast(ai, "slow", strider);
    default:
        return false;
    }
}

// The standard "avoid aoe" works for Toxic Spores, this one keeps more distance and
// stays on the platform so that the bots do not go down the stairs
bool SscVashjAvoidToxicSporesAction::Execute(Event& event)
{
    const std::vector<Unit*> spores = GetSporeDropTriggers(bot);
    if (spores.empty())
        return false;

    constexpr float hazardRadius = 7.0f;
    bool inDanger = false;
    for (Unit* spore : spores)
    {
        if (raidboss::ExactDist2d(bot, spore) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    const WorldPosition safest = FindSafestNearbyPosition(spores, 60.0f, hazardRadius);
    if (safest.getMapId() != bot->GetMapId())
        return false;

    return MoveToPosition(safest);
}

WorldPosition SscVashjAvoidToxicSporesAction::FindSafestNearbyPosition(const std::vector<Unit*>& spores, float maxRadius, float hazardRadius)
{
    constexpr float searchStep = M_PI_F / 8.0f;
    const WorldPosition start(bot);
    WorldPosition bestPos;
    float minMoveDistance = 9999.0f;
    bool foundSafe = false;
    bool foundAny = false;

    for (float distance = 2.0f; distance <= 40.0f && !foundSafe; distance += 1.0f)
    {
        for (float angle = 0.0f; angle < 2.0f * M_PI_F; angle += searchStep)
        {
            const float x = bot->GetPositionX() + distance * std::cos(angle);
            const float y = bot->GetPositionY() + distance * std::sin(angle);
            if (std::hypot(x - VASHJ_PLATFORM_CENTER_POSITION.getX(), y - VASHJ_PLATFORM_CENTER_POSITION.getY()) > maxRadius)
                continue;

            bool safe = true;
            for (Unit* spore : spores)
            {
                if (raidboss::ExactDist2d(spore, x, y) < hazardRadius)
                {
                    safe = false;
                    break;
                }
            }

            if (!safe)
                continue;

            const WorldPosition testPos(bot->GetMapId(), x, y, bot->GetPositionZ());
            const bool pathSafe = IsPathSafeFromSpores(start, testPos, spores, hazardRadius);
            if ((pathSafe && (!foundSafe || distance < minMoveDistance)) || (!foundSafe && distance < minMoveDistance))
            {
                bestPos = testPos;
                minMoveDistance = distance;
                foundSafe = foundSafe || pathSafe;
                foundAny = true;
            }
        }
    }

    return foundAny ? bestPos : WorldPosition();
}

bool SscVashjAvoidToxicSporesAction::IsPathSafeFromSpores(const WorldPosition& start, const WorldPosition& end, const std::vector<Unit*>& spores, float hazardRadius)
{
    constexpr uint8 numChecks = 10;
    const float dx = end.getX() - start.getX();
    const float dy = end.getY() - start.getY();
    for (uint8 i = 1; i <= numChecks; ++i)
    {
        const float ratio = float(i) / numChecks;
        for (Unit* spore : spores)
        {
            if (raidboss::ExactDist2d(spore, start.getX() + dx * ratio, start.getY() + dy * ratio) < hazardRadius)
                return false;
        }
    }

    return true;
}

// Rogues: Cloak of Shadows when Entangled with Static Charge or near a spore.
// Paladins: Blessing of Freedom (Hand of Freedom in mod-playerbots) on Entangled melee
bool SscVashjUseFreeActionAbilitiesAction::Execute(Event& event)
{
    const std::vector<Unit*> spores = GetSporeDropTriggers(bot);
    constexpr float toxicSporeRadius = 6.0f;
    auto nearSpore = [&](Player* player)
    {
        for (Unit* spore : spores)
        {
            if (raidboss::ExactDist2d(player, spore) < toxicSporeRadius)
                return true;
        }
        return false;
    };

    if (bot->getClass() == CLASS_ROGUE && bot->HasAura(SPELL_ENTANGLE) && (bot->HasAura(SPELL_STATIC_CHARGE) || nearSpore(bot)))
        return raidboss::TryCast(ai, "cloak of shadows", bot);

    if (bot->getClass() != CLASS_PALADIN)
        return false;

    Player* mainTankToxic = nullptr;
    Player* anyToxic = nullptr;
    Player* mainTankStatic = nullptr;
    Player* anyStatic = nullptr;
    Player* mainTank = raidboss::GetGroupMainTank(bot);
    for (Player* member : raidboss::GetGroupMembers(bot, 40.0f, true))
    {
        if (!member->HasAura(SPELL_ENTANGLE) || !raidboss::IsMeleePlayer(member))
            continue;

        if (nearSpore(member))
        {
            if (member == mainTank)
                mainTankToxic = member;
            if (!anyToxic)
                anyToxic = member;
        }

        if (member->HasAura(SPELL_STATIC_CHARGE))
        {
            if (member == mainTank)
                mainTankStatic = member;
            if (!anyStatic)
                anyStatic = member;
        }
    }

    // Priority 1: Entangled in Toxic Spores, priority 2: Entangled with Static Charge (main tank first)
    Player* toxicTarget = mainTankToxic ? mainTankToxic : anyToxic;
    if (toxicTarget && raidboss::TryCast(ai, "blessing of freedom", toxicTarget))
        return true;

    Player* staticTarget = mainTankStatic ? mainTankStatic : anyStatic;
    return staticTarget && raidboss::TryCast(ai, "blessing of freedom", staticTarget);
}
