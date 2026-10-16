// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/Hyjal/HyjalActions) and adapted for this core.
#include "playerbot/playerbot.h"
#include "HyjalActions.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "Util/Timer.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace ai;
using namespace ai::hyjal;
using namespace ai::raidboss;

// Movement helpers

template<class Base>
bool HyjalMoveHelper<Base>::MoveStep(float x, float y, float z)
{
    Player* bot = this->bot;
    float groundZ = bot->GetMap()->GetHeight(x, y, z + 4.0f);
    if (groundZ <= INVALID_HEIGHT || std::fabs(groundZ - z) > 10.0f)
        groundZ = z;

    return this->MoveTo(HYJAL_MAP_ID, x, y, groundZ, false, false, false, true);
}

template<class Base>
bool HyjalMoveHelper<Base>::FleePosition(const WorldPosition& from, float radius, uint32 minIntervalMs)
{
    Player* bot = this->bot;
    const float distance = ExactDist2d(bot, from.getX(), from.getY());
    if (distance >= radius)
        return false;

    const uint32 now = WorldTimer::getMSTime();
    if (minIntervalMs && lastFleeTime && WorldTimer::getMSTimeDiff(lastFleeTime, now) < minIntervalMs)
        return false;

    float angle = std::atan2(bot->GetPositionY() - from.getY(), bot->GetPositionX() - from.getX());
    if (distance <= 0.1f)
        angle = bot->GetOrientation();

    const float escapeRadius = radius + 1.0f;
    const float moveDist = escapeRadius - distance;
    float stepX, stepY, stepZ;
    if (!FindStepToCircle(bot, from, escapeRadius, angle, moveDist, stepX, stepY, stepZ, {}, nullptr, nullptr))
        return false;

    lastFleeTime = now;
    return MoveStep(stepX, stepY, stepZ);
}

template<class Base>
bool HyjalMoveHelper<Base>::MoveAway(Unit* from, float distance)
{
    if (!from || distance <= 0.0f)
        return false;

    return FleePosition(WorldPosition(from), ExactDist2d(this->bot, from) + distance);
}

template class ai::HyjalMoveHelper<MovementAction>;
template class ai::HyjalMoveHelper<AttackAction>;

// General

bool HyjalResetEncounterStatesAction::Execute(Event& /*event*/)
{
    bool reset = false;
    for (const char* name : { "rage winterchill ranged position reached", "anetheron ranged position reached", "kaz'rogal below mana threshold" })
    {
        if (AI_VALUE(bool, name))
        {
            context->GetValue<bool>(name)->Reset();
            reset = true;
        }
    }

    return reset;
}

bool HyjalMisdirectBossToMainTankAction::Execute(Event& /*event*/)
{
    Unit* boss = GetBoss(ai, bossEntry);
    if (!boss)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || !mainTank->IsAlive() || mainTank == bot)
        return false;

    if (TryCast(ai, "misdirection", mainTank))
        return true;

    if (!bot->HasAura(SPELL_MISDIRECTION_AURA))
        return false;

    return TryCast(ai, "steady shot", boss);
}

bool HyjalMainTankPositionBossAction::Execute(Event& /*event*/)
{
    Unit* boss = GetBoss(ai, bossEntry);
    if (!boss)
        return false;

    if (GetCurrentTarget(ai) != boss)
        return Attack(GetMaster(), boss);

    if (boss->GetVictim() != bot || !bot->CanReachWithMeleeAttack(boss))
        return false;

    if (bot->GetHealthPercent() < bailBelowHealthPct)
        return false;

    const float arrivalDist = 4.0f;
    float moveX, moveY;
    bool backwards;
    if (!GetStepToPosition(bot, ToPosition(position), arrivalDist, boss, moveX, moveY, backwards))
        return false;

    return MoveStep(moveX, moveY, bot->GetPositionZ());
}

bool HyjalRemoveDangerousDotAction::Execute(Event& /*event*/)
{
    return TryCast(ai, GetSelfImmunitySpellId(ai), bot);
}

// Rage Winterchill

// A forced "avoid aoe": Death and Decay is larger than the default avoid aoe radius
bool RageWinterchillRangedGetOutOfDeathAndDecayAction::Execute(Event& /*event*/)
{
    WorldPosition pool;
    if (!GetDeathAndDecayPosition(ai, pool))
        return false;

    return FleePosition(pool, DEATH_AND_DECAY_RADIUS);
}

// Spread the ranged in a circle initially. After the initial spread, movement is free.
bool RageWinterchillSpreadRangedInCircleAction::Execute(Event& /*event*/)
{
    const char* reachedValue = "rage winterchill ranged position reached";
    if (AI_VALUE(bool, reachedValue))
        return false;

    const float healerRadius = 25.0f;
    const float dpsRadius = 35.0f;
    float moveX, moveY, moveZ;
    bool reached = false;
    const bool step = GetRangedRingStep(bot, ToPosition(WINTERCHILL_TANK_POSITION), healerRadius, dpsRadius, moveX, moveY, moveZ, reached);
    if (reached)
        SET_AI_VALUE(bool, reachedValue, true);

    return step && MoveStep(moveX, moveY, moveZ);
}

// Melee look for an open position within melee range of the boss. If there is none (likely when
// Death and Decay lands on the melee), they take the shortest path out and wait it out.
bool RageWinterchillMeleeManeuverThroughDeathAndDecayAction::Execute(Event& /*event*/)
{
    Unit* winterchill = GetBoss(ai, NPC_RAGE_WINTERCHILL);
    if (!winterchill)
        return false;

    WorldPosition pool;
    if (!GetDeathAndDecayPosition(ai, pool))
        return false;

    float moveX, moveY, moveZ;
    if (!GetMeleeHazardManeuverStep(bot, winterchill, { pool }, DEATH_AND_DECAY_RADIUS, {}, moveX, moveY, moveZ))
        return false;

    return MoveStep(moveX, moveY, moveZ);
}

// Anetheron

bool AnetheronMisdirectBossAndInfernalsToTanksAction::Execute(Event& /*event*/)
{
    Unit* anetheron = GetBoss(ai, NPC_ANETHERON);
    if (!anetheron)
        return false;

    Player* tank = nullptr;
    Unit* enemy = nullptr;
    if (anetheron->GetHealthPercent() > BOSS_ENGAGED_HEALTH_PCT)
    {
        tank = GetGroupMainTank(bot);
        enemy = anetheron;
    }
    else if (Unit* infernal = GetLooseInfernal(ai))
    {
        tank = GetInfernalTank(bot);
        enemy = infernal;
    }

    if (!enemy || !tank || !tank->IsAlive() || tank == bot)
        return false;

    if (TryCast(ai, "misdirection", tank))
        return true;

    if (!bot->HasAura(SPELL_MISDIRECTION_AURA))
        return false;

    return TryCast(ai, "steady shot", enemy);
}

// An initial spread like for Winterchill, but the bots keep a bit of distance from each other
// throughout the fight because of Carrion Swarm
bool AnetheronSpreadRangedInCircleAction::Execute(Event& /*event*/)
{
    const char* reachedValue = "anetheron ranged position reached";
    if (AI_VALUE(bool, reachedValue))
    {
        const float safeDistFromPlayer = 6.0f;
        const uint32 minInterval = 2000;
        if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
            return FleePosition(WorldPosition(nearestPlayer), safeDistFromPlayer, minInterval);

        return false;
    }

    const float healerRadius = 27.0f;
    const float dpsRadius = 34.0f;
    float moveX, moveY, moveZ;
    bool reached = false;
    const bool step = GetRangedRingStep(bot, ToPosition(ANETHERON_TANK_POSITION), healerRadius, dpsRadius, moveX, moveY, moveZ, reached);
    if (reached)
        SET_AI_VALUE(bool, reachedValue, true);

    return step && MoveStep(moveX, moveY, moveZ);
}

// Everyone near the Inferno target is about to be stunned, the Infernal lands at that player's feet.
// The 3.5s cast is the whole window, and stepping out also keeps the bot clear of the Immolation.
bool AnetheronMoveAwayFromInfernoTargetAction::Execute(Event& /*event*/)
{
    Unit* anetheron = GetBoss(ai, NPC_ANETHERON);
    if (!anetheron)
        return false;

    Player* infernoTarget = GetInfernoTarget(anetheron);
    if (!infernoTarget || infernoTarget == bot)
        return false;

    return FleePosition(WorldPosition(infernoTarget), INFERNAL_ESCAPE_DISTANCE);
}

bool AnetheronBringInfernalToInfernalTankAction::Execute(Event& /*event*/)
{
    const float arrivalDist = 2.0f;
    float moveX, moveY;
    bool backwards;
    if (!GetStepToPosition(bot, GetInfernalTankPosition(bot), arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveStep(moveX, moveY, bot->GetPositionZ());
}

// Infernals cannot be taunted
bool AnetheronInfernalTankTakePositionAction::Execute(Event& /*event*/)
{
    const float arrivalDist = 3.0f;
    float moveX, moveY;
    bool backwards;
    if (!GetStepToPosition(bot, GetInfernalTankPosition(bot), arrivalDist, GetInfernalTargetingBot(ai), moveX, moveY, backwards))
        return false;

    return MoveStep(moveX, moveY, bot->GetPositionZ());
}

// An Infernal burns everything within 10y, so everybody who doesn't tank it leaves
bool AnetheronGetOutOfImmolationAction::Execute(Event& /*event*/)
{
    Unit* infernal = GetNearestInfernal(ai);
    if (!infernal)
        return false;

    return FleePosition(WorldPosition(infernal), INFERNAL_DANGER_RADIUS);
}

// Melee stay on Anetheron. Ranged attack the Infernals when they are reasonably close.
bool AnetheronAssignDpsPriorityAction::Execute(Event& /*event*/)
{
    Unit* anetheron = GetBoss(ai, NPC_ANETHERON);
    if (!anetheron)
        return false;

    Unit* current = GetCurrentTarget(ai);
    if (IsMeleePlayer(bot))
        return current != anetheron && Attack(GetMaster(), anetheron);

    if (Unit* infernal = GetInfernalToAttack(ai, anetheron))
    {
        // Wait for the tank to pick up the Infernal
        Player* infernalTank = GetInfernalTank(bot);
        if (!infernalTank || infernal->GetVictim() == infernalTank)
            return current != infernal && Attack(GetMaster(), infernal);
    }

    return current != anetheron && Attack(GetMaster(), anetheron);
}

// Kaz'rogal

// The assist tanks stand with the main tank to split Malevolent Cleave
bool KazrogalAssistTanksMoveInFrontAction::Execute(Event& /*event*/)
{
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || mainTank == bot)
        return false;

    const float arrivalDist = 4.0f;
    float moveX, moveY;
    bool backwards;
    if (!GetStepToPosition(bot, WorldPosition(mainTank), arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveStep(moveX, moveY, bot->GetPositionZ());
}

bool KazrogalSpreadRangedInArcAction::Execute(Event& /*event*/)
{
    const std::vector<Player*> rangedMembers = GetRangedMembers(bot);
    const int botIndex = GetIndexOf(bot, rangedMembers);
    if (botIndex < 0)
        return false;

    Unit* kazrogal = GetBoss(ai, NPC_KAZROGAL);
    if (!kazrogal)
        return false;

    const size_t count = rangedMembers.size();
    const float arcRadius = GetKazrogalRangedArcRadius(kazrogal);
    const float arcSpan = GetKazrogalRangedArcSpan(arcRadius);
    const float arcStart = KAZROGAL_RANGED_ARC_CENTER - arcSpan / 2.0f;
    const float angle = (count == 1) ? KAZROGAL_RANGED_ARC_CENTER : (arcStart + arcSpan * float(botIndex) / float(count - 1));

    const float targetX = kazrogal->GetPositionX() + arcRadius * std::cos(angle);
    const float targetY = kazrogal->GetPositionY() + arcRadius * std::sin(angle);

    const float moveDist = 3.5f;
    float moveX, moveY, moveZ;
    if (!CanTakeStepTowards(bot, targetX, targetY, moveDist, moveX, moveY, moveZ))
        return false;

    return MoveStep(moveX, moveY, moveZ);
}

bool KazrogalMoveAwayFromGroupAction::Execute(Event& /*event*/)
{
    if (bot->GetPower(POWER_MANA) > MARK_REJOIN_MANA)
    {
        SET_AI_VALUE(bool, "kaz'rogal below mana threshold", false);
        return false;
    }

    Player* nearestPlayer = GetNearestPlayerInRadius(bot, MARK_ESCAPE_DISTANCE);
    if (!nearestPlayer)
        return false;

    const float step = MARK_ESCAPE_DISTANCE - ExactDist2d(bot, nearestPlayer);

    // Away from Kaz'rogal first when the nearest player is farther from the boss, then sideways
    Unit* kazrogal = GetBoss(ai, NPC_KAZROGAL);
    if (kazrogal && ExactDist2d(nearestPlayer, kazrogal) > ExactDist2d(bot, kazrogal))
        return MoveAway(kazrogal, step);

    return MoveAway(nearestPlayer, step);
}

bool KazrogalActivateAspectOfTheViperAction::Execute(Event& /*event*/)
{
    return TryCast(ai, "aspect of the viper", bot);
}

bool KazrogalCancelImmunityAction::Execute(Event& /*event*/)
{
    const uint32 spellId = GetSelfImmunitySpellId(ai);
    if (!spellId || !bot->HasAura(spellId))
        return false;

    bot->RemoveAurasDueToSpellByCancel(spellId);
    return true;
}

// Life Tap first, then Shadow Ward if there isn't enough health for it
bool KazrogalWarlockManageManaAction::Execute(Event& /*event*/)
{
    if (bot->GetPower(POWER_MANA) <= MARK_LIFE_TAP_MANA && TryCast(ai, "life tap", bot))
        return true;

    if (!HasMarkOfKazrogal(bot))
        return false;

    return TryCast(ai, "shadow ward", bot);
}

// Azgalor

bool AzgalorDisperseRangedAction::Execute(Event& /*event*/)
{
    Unit* azgalor = GetBoss(ai, NPC_AZGALOR);
    if (!azgalor)
        return false;

    // ~20 yards + both combat reaches, keeps the ranged out of Cleave
    const float safeDistFromBoss = 30.0f;
    if (ExactDist2d(bot, azgalor) < safeDistFromBoss && FleePosition(WorldPosition(azgalor), safeDistFromBoss))
        return true;

    // War Stomp of the Doomguard is 10 yards
    Unit* doomguard = FindUnit(ai, NPC_LESSER_DOOMGUARD);
    const float safeDistFromDoomguard = 10.0f;
    if (doomguard && ExactDist2d(bot, doomguard) < safeDistFromDoomguard)
        return FleePosition(WorldPosition(doomguard), safeDistFromDoomguard);

    // Don't spread while focusing the Doomguard, there's not much space
    if (doomguard && GetCurrentTarget(ai) == doomguard)
        return false;

    const float safeDistFromPlayer = 5.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    return nearestPlayer && FleePosition(WorldPosition(nearestPlayer), safeDistFromPlayer, 1000);
}

// Like Death and Decay, but there can be two Rains of Fire, and the escape must not take the bot
// into the frontal arc of Azgalor (Cleave)
bool AzgalorMeleeManeuverThroughFireAction::Execute(Event& /*event*/)
{
    Unit* azgalor = GetBoss(ai, NPC_AZGALOR);
    if (!azgalor)
        return false;

    const std::vector<WorldPosition> pools = GetRainOfFirePositions(ai);
    if (pools.empty())
        return false;

    const std::vector<BlockedArc> cleaveArc = { { azgalor->GetOrientation(), CLEAVE_DANGER_ARC / 2.0f } };
    auto cleaveSafe = [azgalor](float x, float y) { return IsSafeFromAzgalorCleave(azgalor, x, y); };

    float moveX, moveY, moveZ;
    if (!GetMeleeHazardManeuverStep(bot, azgalor, pools, RAIN_OF_FIRE_RADIUS, cleaveArc, moveX, moveY, moveZ, cleaveSafe))
        return false;

    return MoveStep(moveX, moveY, moveZ);
}

bool AzgalorRangedGetOutOfRainOfFireAction::Execute(Event& /*event*/)
{
    WorldPosition pool;
    if (!GetNearestRainOfFirePosition(ai, pool))
        return false;

    return FleePosition(pool, RAIN_OF_FIRE_RADIUS);
}

bool AzgalorMoveToDoomguardTankAction::Execute(Event& /*event*/)
{
    const float arrivalDist = 5.0f;
    float moveX, moveY;
    bool backwards;
    if (!GetStepToPosition(bot, ToPosition(AZGALOR_DOOMGUARD_POSITION), arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveStep(moveX, moveY, bot->GetPositionZ());
}

bool AzgalorTankPositionDoomguardAction::Execute(Event& /*event*/)
{
    const WorldPosition position = ToPosition(AZGALOR_DOOMGUARD_POSITION);
    const float arrivalDist = 3.0f;

    Unit* doomguard = FindUnit(ai, NPC_LESSER_DOOMGUARD);
    if (doomguard)
    {
        if (GetCurrentTarget(ai) != doomguard)
            return Attack(GetMaster(), doomguard);

        if (doomguard->GetVictim() != bot || !bot->CanReachWithMeleeAttack(doomguard))
            return false;
    }
    else if (ExactDist2d(bot, position.getX(), position.getY()) <= arrivalDist)
    {
        // At the position and no Doomguard yet: wait
        return true;
    }

    // Without a Doomguard the tank goes to the position in advance
    float moveX, moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, doomguard, moveX, moveY, backwards))
        return false;

    return MoveStep(moveX, moveY, bot->GetPositionZ());
}

// Only ranged dps within 70y attack the Doomguards, enough damage without running in front of Azgalor
bool AzgalorDetermineDpsPriorityAction::Execute(Event& /*event*/)
{
    Unit* azgalor = GetBoss(ai, NPC_AZGALOR);
    if (!azgalor)
        return false;

    Unit* current = GetCurrentTarget(ai);
    if (IsMeleePlayer(bot))
        return current != azgalor && Attack(GetMaster(), azgalor);

    Unit* target = azgalor;
    if (azgalor->GetHealthPercent() > BOSS_BURN_HEALTH_PCT)
    {
        const float doomguardEngageDist = 70.0f;
        Unit* doomguard = FindUnit(ai, NPC_LESSER_DOOMGUARD);
        if (doomguard && ExactDist2d(bot, doomguard) < doomguardEngageDist)
            target = doomguard;
    }

    return current != target && Attack(GetMaster(), target);
}

// Archimonde

bool ArchimondeSetTremorTotemAction::Execute(Event& /*event*/)
{
    if (AI_VALUE2(bool, "has totem", "tremor totem"))
        return false;

    return TryCast(ai, "tremor totem", bot);
}

// Air Burst knocks everyone around its target into the air. Losing the whole melee group at once
// must be avoided, so the bots get away from the tank.
bool ArchimondeKeepAirBurstAwayFromTankAction::Execute(Event& /*event*/)
{
    Unit* archimonde = GetBoss(ai, NPC_ARCHIMONDE);
    if (!archimonde)
        return false;

    Unit* activeTank = archimonde->GetVictim();
    Player* target = GetAirBurstTarget(archimonde);
    if (!activeTank || !target || (target != activeTank && target != bot))
        return false;

    const float distanceToActiveTank = ExactDist2d(bot, activeTank);
    if (distanceToActiveTank >= AIR_BURST_SAFE_DISTANCE)
        return false;

    return MoveAway(activeTank, AIR_BURST_SAFE_DISTANCE - distanceToActiveTank);
}

bool ArchimondeSpreadRangedAction::Execute(Event& /*event*/)
{
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, ARCHIMONDE_RANGED_SPREAD_DISTANCE);
    if (!nearestPlayer)
        return false;

    const uint32 minInterval = 3000;
    return FleePosition(WorldPosition(nearestPlayer), ARCHIMONDE_RANGED_SPREAD_DISTANCE, minInterval);
}

// The pools near the bot push it away (weighted by distance). When the push cancels out inside a
// pool, the bot tries a fan of directions for the best clearance. Without danger, the bot walks
// back into range of Archimonde along a path free of Doomfire.
bool ArchimondeAvoidDoomfireAction::Execute(Event& /*event*/)
{
    Unit* archimonde = GetBoss(ai, NPC_ARCHIMONDE);
    if (!archimonde)
        return false;

    const std::vector<WorldPosition> trail = GetDoomfirePositions(ai);
    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();

    bool hasNearest = false;
    float nearestDistance = std::numeric_limits<float>::max();
    float totalDx = 0.0f;
    float totalDy = 0.0f;

    for (const WorldPosition& patch : trail)
    {
        const float d = ExactDist2d(bot, patch.getX(), patch.getY());
        if (d >= DOOMFIRE_FIELD_RADIUS)
            continue;

        if (d < nearestDistance)
        {
            hasNearest = true;
            nearestDistance = d;
        }

        if (d > 0.0f)
        {
            const float weight = (DOOMFIRE_FIELD_RADIUS - d) / DOOMFIRE_FIELD_RADIUS;
            totalDx += (botX - patch.getX()) / d * weight;
            totalDy += (botY - patch.getY()) / d * weight;
        }
    }

    float norm = std::sqrt(totalDx * totalDx + totalDy * totalDy);
    float moveDist = (hasNearest && nearestDistance < DOOMFIRE_DANGER_RADIUS) ? DOOMFIRE_DANGER_RADIUS - nearestDistance : 0.0f;

    const float minFieldStrength = 0.05f;
    if (hasNearest && nearestDistance < DOOMFIRE_BURN_RADIUS && norm < minFieldStrength)
    {
        const uint8 fanSteps = 12;
        const float escapeStep = 10.0f;
        bool found = false;

        for (uint8 pass = 0; pass < 2 && !found; ++pass)
        {
            const bool validate = (pass == 0);
            float bestClearance = -1.0f;

            for (uint8 i = 0; i < fanSteps; ++i)
            {
                const float angle = 2.0f * M_PI_F * i / fanSteps;
                const float testX = botX + std::cos(angle) * DOOMFIRE_DANGER_RADIUS;
                const float testY = botY + std::sin(angle) * DOOMFIRE_DANGER_RADIUS;

                float clearance = DOOMFIRE_FIELD_RADIUS;
                for (const WorldPosition& patch : trail)
                {
                    const float dx = patch.getX() - testX;
                    const float dy = patch.getY() - testY;
                    clearance = std::min(clearance, std::sqrt(dx * dx + dy * dy));
                }

                if (clearance <= bestClearance)
                    continue;

                float stepX = testX;
                float stepY = testY;
                float stepZ = bot->GetPositionZ();
                if (validate && !CanTakeStepTowards(bot, testX, testY, escapeStep, stepX, stepY, stepZ))
                    continue;

                bestClearance = clearance;
                totalDx = stepX - botX;
                totalDy = stepY - botY;
                found = true;
            }
        }

        norm = std::sqrt(totalDx * totalDx + totalDy * totalDy);
        moveDist = norm;
    }

    if (norm > 0.0f && moveDist >= 0.5f)
    {
        const float targetX = botX + (totalDx / norm) * moveDist;
        const float targetY = botY + (totalDy / norm) * moveDist;
        return MoveStep(targetX, targetY, bot->GetPositionZ());
    }

    const bool inPosition = IsRangedPlayer(bot) ?
        bot->IsWithinDistInMap(archimonde, ai->GetRange("spell")) :
        ExactDist2d(bot, archimonde) <= GetMeleeRange(bot, archimonde) - MELEE_RANGE_INSET;

    if (inPosition)
        return false;

    const float arrivalDist = 0.5f;
    float moveX, moveY;
    bool backwards;
    if (!GetStepToPosition(bot, WorldPosition(archimonde), arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    if (IsPositionNearDoomfire(ai, moveX, moveY, DOOMFIRE_DANGER_RADIUS))
        return false;

    return MoveStep(moveX, moveY, bot->GetPositionZ());
}
