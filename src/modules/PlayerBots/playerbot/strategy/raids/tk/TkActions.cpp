// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/TK/TKActions) and adapted for this core.
#include "playerbot/playerbot.h"
#include "TkActions.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/generic/TbcRaidStrategy.h"
#include "playerbot/AiFactory.h"
#include "Loot/LootMgr.h"
#include "Movement/MoveSpline.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace ai;
using namespace ai::tk;

// General

bool TkResetEncounterStatesAction::Execute(Event& event)
{
    return ResetEncounterStates(bot->GetInstanceId());
}

// This is needed in the event of a wipe during Kael's Gravity Lapse phase.
bool TkClearStaleFallingFlagAction::Execute(Event& event)
{
    // Clear the flag only once the bot is back on the floor, or it would be cleared mid-descent.
    const float floorZ = bot->GetMap()->GetHeight(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    if (floorZ <= INVALID_HEIGHT || bot->GetPositionZ() - floorZ > 1.0f)
        return false;

    bot->m_movementInfo.RemoveMovementFlag(MovementFlags(MOVEFLAG_FALLING | MOVEFLAG_FALLINGFAR));
    return true;
}

bool TkTankPositionAction::MoveToTankPosition(Unit* target, const TkPos& position, float tolerance, bool shouldAttack)
{
    if (shouldAttack && GetCurrentTarget(ai) != target)
        return Attack(GetMaster(), target);

    if (target->GetVictim() != bot || !bot->CanReachWithMeleeAttack(target))
        return false;

    float moveX, moveY;
    bool backwards;
    if (!raidboss::GetStepToPosition(bot, WorldPosition(TK_MAP_ID, position.x, position.y, position.z), tolerance, target, moveX, moveY, backwards))
        return false;

    return MoveToPosition(moveX, moveY, bot->GetPositionZ());
}

// Trash

bool TkCrimsonHandCenturionCastPolymorphAction::Execute(Event& event)
{
    Unit* target = GetCenturionCastingArcaneFlurry(ai);
    if (!target)
        return false;

    bot->InterruptNonMeleeSpells(false);
    return raidboss::TryCast(ai, "polymorph", target);
}

// Al'ar <Phoenix God>

namespace
{
    bool MisdirectTo(PlayerbotAI* ai, Player* tank, Unit* target)
    {
        if (!tank || !target || !tank->IsAlive())
            return false;

        if (raidboss::TryCast(ai, "misdirection", tank))
            return true;

        if (!ai->GetBot()->HasAura(tk::SPELL_MISDIRECTION))
            return false;

        return raidboss::TryCast(ai, "steady shot", target) || raidboss::TryCast(ai, "arcane shot", target);
    }
}

bool TkAlarMisdirectBossToMainTankAction::Execute(Event& event)
{
    return MisdirectTo(ai, raidboss::GetGroupMainTank(bot), raidboss::FindUnit(ai, NPC_ALAR));
}

bool TkAlarBossTanksMoveBetweenPlatformsAction::Execute(Event& event)
{
    const bool isFirstAlarTank = IsFirstAlarTank(bot);
    if (!isFirstAlarTank && !IsSecondAlarTank(bot))
        return false;

    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar)
        return false;

    if (GetCurrentTarget(ai) != alar)
        return Attack(GetMaster(), alar);

    // Which platform the tank goes to depends on Al'ar's platform.
    const int8 alarPlatformIndex = GetAlarPlatformIndex(alar);
    int8 tankPlatformIndex;
    if (isFirstAlarTank)
        tankPlatformIndex = (alarPlatformIndex == PLATFORM_0_IDX || alarPlatformIndex == PLATFORM_3_IDX) ? PLATFORM_0_IDX : PLATFORM_2_IDX;
    else
        tankPlatformIndex = (alarPlatformIndex == PLATFORM_0_IDX || alarPlatformIndex == PLATFORM_1_IDX) ? PLATFORM_1_IDX : PLATFORM_3_IDX;

    const TkPos& target = ALAR_TANK_PLATFORM_POSITIONS[tankPlatformIndex];
    if (Dist2d(bot, target) <= 2.0f)
        return alar->GetVictim() != bot && CastTaunt(ai, alar);

    return MoveToPosition(target);
}

bool TkAlarMeleeDpsMoveBetweenPlatformsAction::Execute(Event& event)
{
    if (!raidboss::IsMeleePlayer(bot) || IsFirstAlarTank(bot) || IsSecondAlarTank(bot) || IsPrimaryEmberTank(bot))
        return false;

    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar || alar->GetHealthPercent() <= 5.0f)
        return false;

    const int8 platformIndex = GetAlarPlatformIndex(alar);
    if (platformIndex == LOCATION_NONE)
        return false;

    const TkPos& target = ALAR_MELEE_DPS_PLATFORM_POSITIONS[platformIndex];
    return Dist2d(bot, target) > 2.0f && MoveToPosition(target);
}

bool TkAlarRangedAndEmberTankMoveUnderPlatformsAction::Execute(Event& event)
{
    const bool isRanged = raidboss::IsRangedPlayer(bot);
    const bool isEmberTank = IsPrimaryEmberTank(bot);
    if (!isRanged && !isEmberTank)
        return false;

    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar)
        return false;

    const int8 platformIndex = GetAlarPlatformIndex(alar);
    if (platformIndex == LOCATION_NONE)
        return false;

    float distFromTarget;
    if (isRanged)
        distFromTarget = 8.0f;
    else if (!raidboss::FindUnit(ai, NPC_EMBER_OF_ALAR))
        distFromTarget = 20.0f;
    else
        return false;

    return MoveInside(ALAR_GROUND_POSITIONS[platformIndex], distFromTarget);
}

bool TkAlarAssistTanksPickUpEmbersAction::Execute(Event& event)
{
    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar)
        return false;

    if (!IsAlarInPhase2(alar->GetInstanceId()))
        return HandlePhase1Embers(alar);

    return HandlePhase2Embers();
}

// Embers will be tanked by only the second assist tank in Phase 1.
bool TkAlarAssistTanksPickUpEmbersAction::HandlePhase1Embers(Unit* alar)
{
    if (!IsPrimaryEmberTank(bot))
        return false;

    Unit* ember = raidboss::FindUnit(ai, NPC_EMBER_OF_ALAR);
    if (!ember)
        return false;

    if (GetCurrentTarget(ai) != ember)
        return Attack(GetMaster(), ember);

    if (!bot->CanReachWithMeleeAttack(ember))
        return MoveToPosition(ember->GetPositionX(), ember->GetPositionY(), ember->GetPositionZ());

    if (ember->GetVictim() != bot)
        return false;

    const int8 platformIndex = GetAlarPlatformIndex(alar);
    if (platformIndex == LOCATION_NONE)
        return false;

    // Drag the Ember from under the platform towards the room center
    const TkPos& position = ALAR_GROUND_POSITIONS[platformIndex];
    const float dx = ALAR_POINT_MIDDLE.x - position.x;
    const float dy = ALAR_POINT_MIDDLE.y - position.y;
    const float distToCenter = std::sqrt(dx * dx + dy * dy);
    constexpr float moveDist = 26.0f;
    const float targetX = position.x + (dx / distToCenter) * moveDist;
    const float targetY = position.y + (dy / distToCenter) * moveDist;

    if (raidboss::ExactDist2d(bot, targetX, targetY) <= 1.0f)
        return false;

    return MoveToPosition(targetX, targetY, position.z);
}

// One Ember will be tanked by the second assist tank in Phase 2, and the other by
// the main tank or first assist tank (whichever is not tanking Al'ar; see helpers for more info).
bool TkAlarAssistTanksPickUpEmbersAction::HandlePhase2Embers()
{
    const std::pair<Unit*, Unit*> embers = GetTargetUnitPair(ai, NPC_EMBER_OF_ALAR);

    Unit* ember = nullptr;
    if (IsPrimaryEmberTank(bot))
        ember = embers.first;
    else if (GetSecondaryEmberTank(bot) == bot)
        ember = embers.second;

    if (!ember)
        return false;

    if (GetCurrentTarget(ai) != ember)
        return Attack(GetMaster(), ember);

    if (ember->GetVictim() != bot)
        return CastTaunt(ai, ember);

    // The Embers' explosion has a range of 15 yards (16.5y damage radius), but more space is
    // needed since this movement is tank-to-player, not Ember.
    constexpr float safeDistance = 20.0f;
    if (!GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return false;

    return MoveFromGroup(safeDistance, true);
}

bool TkAlarAssignNonTankTargetAction::Execute(Event& event)
{
    Unit* target = nullptr;
    if (raidboss::IsRangedPlayer(bot))
    {
        const std::pair<Unit*, Unit*> embers = GetTargetUnitPair(ai, NPC_EMBER_OF_ALAR);
        Unit* ember = embers.first ? embers.first : embers.second;
        if (ember)
        {
            target = ember;
            constexpr float safeDistance = 20.0f;
            const float currentDistance = raidboss::ExactDist2d(bot, ember);
            if (currentDistance < safeDistance)
            {
                bot->InterruptNonMeleeSpells(false);
                if (MoveAwayFrom(ember, safeDistance - currentDistance))
                    return true;
            }
        }
    }

    if (!target)
        target = raidboss::FindUnit(ai, NPC_ALAR);

    if (!target || GetCurrentTarget(ai) == target)
        return false;

    return Attack(GetMaster(), target);
}

// During Flame Quills
bool TkAlarJumpFromPlatformAction::Execute(Event& event)
{
    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
        return JumpTo(GetClosestGroundPosition(bot));

    if (IsFirstAlarTank(bot))
        return MoveToPosition(ALAR_SW_RAMP_BASE);

    if (IsSecondAlarTank(bot))
        return MoveToPosition(ALAR_SE_RAMP_BASE);

    if (PlayerbotAI::IsTank(bot))
        return MoveToPosition(ALAR_POINT_MIDDLE);

    if (raidboss::IsMeleePlayer(bot))
        return MoveToPosition(ALAR_ROOM_S_CENTER);

    // Ranged
    constexpr float distFromPos = 10.0f;
    return MoveInside(ALAR_POINT_MIDDLE, distFromPos);
}

bool TkAlarMoveAwayFromRebirthAction::Execute(Event& event)
{
    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar)
        return false;

    // Ranged/tanks wait until Al'ar actually "dies" to activate this P1->P2 transition action.
    if ((raidboss::IsRangedPlayer(bot) || PlayerbotAI::IsTank(bot)) && !IsAlarPassive(alar))
        return false;

    // On the other hand, melee dps jumps off at 5% HP because TBC hates them.
    if (bot->GetPositionZ() > ALAR_BALCONY_Z)
        return JumpTo(GetClosestGroundPosition(bot));

    // Al'ar is moved to the middle for Rebirth, so move away from the room center, not Al'ar.
    constexpr float safeDistance = 35.0f;
    const float currentDistance = Dist2d(bot, ALAR_ROOM_CENTER);
    if (currentDistance >= safeDistance)
        return false;

    return MoveAwayFrom(ALAR_ROOM_CENTER.x, ALAR_ROOM_CENTER.y, safeDistance - currentDistance);
}

bool TkAlarSwapTanksOnBossAction::Execute(Event& event)
{
    if (!IsFirstAlarTank(bot) && !IsSecondAlarTank(bot))
        return false;

    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar || GetSecondaryEmberTank(bot) == bot)
        return false;

    if (GetCurrentTarget(ai) != alar)
        return Attack(GetMaster(), alar);

    return alar->GetVictim() != bot && CastTaunt(ai, alar);
}

bool TkAlarAvoidFlamePatchesAndDiveBombsAction::Execute(Event& event)
{
    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar)
        return false;

    return AvoidFlamePatch() || HandleDiveBomb(alar);
}

bool TkAlarAvoidFlamePatchesAndDiveBombsAction::AvoidFlamePatch()
{
    const std::vector<Unit*> flamePatches = GetFlamePatches(ai);
    constexpr float hazardRadius = 8.0f;

    for (Unit* flamePatch : flamePatches)
    {
        if (raidboss::ExactDist2d(bot, flamePatch) < hazardRadius)
        {
            const WorldPosition safestPos = FindSafestNearbyPosition(flamePatches, hazardRadius);
            bot->InterruptNonMeleeSpells(false);
            return MoveToPosition(safestPos);
        }
    }

    return false;
}

// Al'ar is the one fight in TK with multiple ground AoEs active at once, so where to run has to be
// specifically weighed. This searches rings going outward from the bot, with the first candidate
// clear of every patch that is reachable without crossing a patch being the one selected (with a
// fallback to the nearest safe spot, regardless of whether the path is safe).
WorldPosition TkAlarAvoidFlamePatchesAndDiveBombsAction::FindSafestNearbyPosition(const std::vector<Unit*>& flamePatches, float hazardRadius)
{
    constexpr float searchStep = M_PI_F / 12.0f;
    constexpr float minDistance = 2.0f;
    constexpr float distanceStep = 1.0f;
    constexpr uint8 numAngles = 24;
    constexpr uint8 numDistSteps = 28;

    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();
    const float botZ = bot->GetPositionZ();
    WorldPosition fallback(bot->GetMapId(), botX, botY, botZ);
    bool haveFallback = false;

    for (uint8 i = 0; i <= numDistSteps; ++i)
    {
        const float distance = minDistance + i * distanceStep;
        for (uint8 j = 0; j < numAngles; ++j)
        {
            const float angle = j * searchStep;
            const float x = botX + distance * std::cos(angle);
            const float y = botY + distance * std::sin(angle);

            bool inPatch = false;
            for (Unit* flamePatch : flamePatches)
            {
                if (raidboss::ExactDist2d(flamePatch, x, y) < hazardRadius)
                {
                    inPatch = true;
                    break;
                }
            }

            if (inPatch)
                continue;

            float stepX, stepY, stepZ;
            if (!raidboss::CanTakeStepTowards(bot, x, y, distance, stepX, stepY, stepZ))
                continue;

            if (IsPathSafe(botX, botY, x, y, flamePatches, hazardRadius))
                return WorldPosition(bot->GetMapId(), stepX, stepY, stepZ);

            if (!haveFallback)
            {
                fallback = WorldPosition(bot->GetMapId(), stepX, stepY, stepZ);
                haveFallback = true;
            }
        }
    }

    return fallback;
}

bool TkAlarAvoidFlamePatchesAndDiveBombsAction::IsPathSafe(float startX, float startY, float endX, float endY, const std::vector<Unit*>& flamePatches, float hazardRadius)
{
    constexpr uint8 numChecks = 10;
    const float dx = endX - startX;
    const float dy = endY - startY;

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        const float ratio = float(i) / numChecks;
        for (Unit* flamePatch : flamePatches)
        {
            if (raidboss::ExactDist2d(flamePatch, startX + dx * ratio, startY + dy * ratio) < hazardRadius)
                return false;
        }
    }

    return true;
}

bool TkAlarAvoidFlamePatchesAndDiveBombsAction::HandleDiveBomb(Unit* alar)
{
    // After the Dive Bomb, before reappearance: fire ball form, uninteractible, or casting Rebirth
    if (alar->HasAura(SPELL_DIVE_BOMB_VISUAL) || alar->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE) ||
        TbcRaidHelper::IsCreatureCasting(alar, SPELL_REBIRTH_DIVE))
    {
        constexpr float safeDistance = 25.0f;
        const float currentDistance = raidboss::ExactDist2d(bot, alar);
        if (currentDistance >= safeDistance)
            return false;

        bot->InterruptNonMeleeSpells(false);
        return MoveAwayFrom(alar, safeDistance - currentDistance);
    }

    if (GetAlarCurrentLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX && GetAlarDestinationLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX)
        return false;

    // Spread during the Dive Bomb sequence
    constexpr float safeDistance = 15.0f;
    Player* nearestPlayer = raidboss::GetNearestPlayerInRadius(bot, safeDistance);
    if (!nearestPlayer)
        return false;

    return MoveAwayFrom(nearestPlayer, safeDistance - raidboss::ExactDist2d(bot, nearestPlayer));
}

bool TkAlarManagePhaseTrackerAction::Execute(Event& event)
{
    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    return alar && UpdateAlarPhaseTracker(alar);
}

// Void Reaver

// Pounding is part of why the spot is fixed, though tanking is needed in any case.
bool TkVoidReaverTanksPositionBossAction::Execute(Event& event)
{
    Unit* voidReaver = raidboss::FindUnit(ai, NPC_VOID_REAVER);
    if (!voidReaver)
        return false;

    constexpr float tolerance = 2.0f;
    return MoveToTankPosition(voidReaver, VOID_REAVER_TANK_POSITION, tolerance);
}

bool TkVoidReaverUseAggroDumpAbilityAction::Execute(Event& event)
{
    static const char* spells[] = { "divine shield", "divine protection", "fade", "feign death", "ice block", "soulshatter", "vanish" };
    for (const char* spell : spells)
    {
        if (raidboss::TryCast(ai, spell, bot))
            return true;
    }

    return false;
}

bool TkVoidReaverRangedBackOffAndSpreadAction::Execute(Event& event)
{
    Unit* voidReaver = raidboss::FindUnit(ai, NPC_VOID_REAVER);
    if (!voidReaver)
        return false;

    constexpr float minDistFromBoss = 38.5f;    // 22 yards between the bounding radii
    const float distance = raidboss::ExactDist2d(bot, voidReaver);
    if (distance < minDistFromBoss)
        return MoveAwayFrom(voidReaver, minDistFromBoss - distance);

    if (voidReaver->GetHealthPercent() > 90.0f)
        return false;

    // Maintain small spread after pull to discourage clumping from avoiding orbs.
    constexpr float minDistFromPlayer = 3.0f;
    Player* nearestPlayer = raidboss::GetNearestPlayerInRadius(bot, minDistFromPlayer);
    return nearestPlayer && MoveAwayFrom(nearestPlayer, minDistFromPlayer);
}

bool TkVoidReaverAvoidArcaneOrbAction::Execute(Event& event)
{
    Unit* voidReaver = raidboss::FindUnit(ai, NPC_VOID_REAVER);
    if (!voidReaver)
        return false;

    const std::vector<WorldPosition> activeOrbs = GetActiveArcaneOrbs(voidReaver);
    if (!IsNearArcaneOrb(bot, activeOrbs, ARCANE_ORB_SAFE_DISTANCE))
        return false;

    constexpr float searchStep = M_PI_F / 12.0f;
    constexpr float minSearchDist = 1.0f;
    constexpr float searchDistStep = 1.0f;
    constexpr float minDistFromBoss = 35.5f;
    constexpr float maxDistFromBoss = 43.5f;
    constexpr uint8 numAngles = 24;
    constexpr uint8 numDistSteps = 39;
    constexpr float safeDistanceSq = ARCANE_ORB_SAFE_DISTANCE * ARCANE_ORB_SAFE_DISTANCE;

    std::vector<WorldPosition> bestCandidates;
    float bestMoveDist = std::numeric_limits<float>::max();
    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();

    for (uint8 i = 0; i <= numDistSteps; ++i)
    {
        const float dist = minSearchDist + i * searchDistStep;
        if (dist > bestMoveDist)
            break;

        for (uint8 j = 0; j < numAngles; ++j)
        {
            const float angle = j * searchStep;
            const float x = botX + dist * std::cos(angle);
            const float y = botY + dist * std::sin(angle);

            const float distFromBoss = raidboss::ExactDist2d(voidReaver, x, y);
            if (distFromBoss < minDistFromBoss || distFromBoss > maxDistFromBoss)
                continue;

            bool safeFromOrbs = true;
            for (const WorldPosition& orb : activeOrbs)
            {
                const float dx = x - orb.getX();
                const float dy = y - orb.getY();
                if (dx * dx + dy * dy < safeDistanceSq)
                {
                    safeFromOrbs = false;
                    break;
                }
            }

            if (!safeFromOrbs)
                continue;

            float stepX, stepY, stepZ;
            if (!raidboss::CanTakeStepTowards(bot, x, y, dist, stepX, stepY, stepZ))
                continue;

            if (dist < bestMoveDist)
            {
                bestCandidates.clear();
                bestMoveDist = dist;
            }

            bestCandidates.push_back(WorldPosition(bot->GetMapId(), stepX, stepY, stepZ));
        }
    }

    bot->InterruptNonMeleeSpells(false);

    if (!bestCandidates.empty())
        return MoveToPosition(bestCandidates[urand(0, bestCandidates.size() - 1)]);

    const WorldPosition& orb = activeOrbs.front();
    return MoveAwayFrom(orb.getX(), orb.getY(), ARCANE_ORB_SAFE_DISTANCE - raidboss::ExactDist2d(bot, orb.getX(), orb.getY()));
}

// High Astromancer Solarian

bool TkSolarianMainTankPickUpBossAction::Execute(Event& event)
{
    Unit* astromancer = raidboss::FindUnit(ai, NPC_SOLARIAN);
    return astromancer && GetCurrentTarget(ai) != astromancer && Attack(GetMaster(), astromancer);
}

bool TkSolarianMoveAwayFromGroupAction::Execute(Event& event)
{
    constexpr float safeDistance = 15.0f;
    if (!raidboss::GetNearestPlayerInRadius(bot, safeDistance))
        return false;

    bot->InterruptNonMeleeSpells(false);
    return MoveFromGroup(safeDistance);
}

bool TkSolarianTargetSolariumPriestsAction::Execute(Event& event)
{
    const std::pair<Unit*, Unit*> priestsPair = GetTargetUnitPair(ai, NPC_SOLARIUM_PRIEST);
    if (!priestsPair.first)
        return false;

    if (raidboss::IsRangedPlayer(bot) && !raidboss::FindUnit(ai, NPC_SOLARIUM_AGENT))
        return GetCurrentTarget(ai) != priestsPair.first && Attack(GetMaster(), priestsPair.first);

    // Split melee into two groups, one on each Solarium Priest.
    Unit* targetPriest = AssignSolariumPriestsToMeleeBots(priestsPair, GetMeleeBots());
    return targetPriest && GetCurrentTarget(ai) != targetPriest && Attack(GetMaster(), targetPriest);
}

std::vector<Player*> TkSolarianTargetSolariumPriestsAction::GetMeleeBots()
{
    std::vector<Player*> meleeMembers;
    for (Player* member : raidboss::GetGroupMembers(bot, 500.0f, true))
    {
        if (member->GetPlayerbotAI() && raidboss::IsMeleePlayer(member) && !raidboss::IsMainTank(member))
            meleeMembers.push_back(member);
    }

    return meleeMembers;
}

Unit* TkSolarianTargetSolariumPriestsAction::AssignSolariumPriestsToMeleeBots(const std::pair<Unit*, Unit*>& priestsPair, const std::vector<Player*>& meleeMembers)
{
    const int botIndex = raidboss::GetIndexOf(bot, meleeMembers);
    if (!priestsPair.first || botIndex < 0)
        return nullptr;

    if (meleeMembers.size() == 1 || !priestsPair.second)
        return priestsPair.first;

    return size_t(botIndex) < meleeMembers.size() / 2 ? priestsPair.first : priestsPair.second;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool TkKaelthasKiteThaladredAction::Execute(Event& event)
{
    Unit* thaladred = raidboss::FindUnit(ai, NPC_THALADRED);
    if (!thaladred)
        return false;

    constexpr float safeDistance = 20.0f;
    const float currentDistance = raidboss::ExactDist2d(bot, thaladred);
    if (currentDistance >= safeDistance)
        return false;

    bot->InterruptNonMeleeSpells(false);
    return MoveAwayFrom(thaladred, safeDistance - currentDistance);
}

// Misdirect order: (1) Capernian, (2) Telonicus, (3) Capernian (again for good measure)
bool TkKaelthasMisdirectAdvisorsToTanksAction::Execute(Event& event)
{
    std::vector<Player*> hunters;
    for (Player* member : raidboss::GetGroupMembers(bot, 500.0f, true))
    {
        if (member->getClass() == CLASS_HUNTER && member->GetPlayerbotAI())
            hunters.push_back(member);

        if (hunters.size() >= 3)
            break;
    }

    const int hunterIndex = raidboss::GetIndexOf(bot, hunters);
    if (hunterIndex < 0)
        return false;

    Unit* advisor;
    Player* tank;
    if (hunterIndex == 1)
    {
        advisor = raidboss::FindUnit(ai, NPC_TELONICUS);
        tank = raidboss::GetGroupAssistTank(bot, 0);
    }
    else
    {
        advisor = raidboss::FindUnit(ai, NPC_CAPERNIAN);
        tank = GetCapernianTank(bot);
    }

    if (!IsAdvisorActive(advisor))
        return false;

    return MisdirectTo(ai, tank, advisor);
}

bool TkKaelthasMeleeTanksPositionAdvisorsAction::Execute(Event& event)
{
    const bool mainTank = raidboss::IsMainTank(bot);
    Unit* advisor = raidboss::FindUnit(ai, mainTank ? NPC_SANGUINAR : NPC_TELONICUS);
    if (!advisor)
        return false;

    constexpr float tolerance = 2.0f;
    return MoveToTankPosition(advisor, mainTank ? SANGUINAR_TANK_POSITION : TELONICUS_TANK_POSITION, tolerance);
}

bool TkKaelthasWarlockTankPositionCapernianAction::Execute(Event& event)
{
    Unit* capernian = raidboss::FindUnit(ai, NPC_CAPERNIAN);
    if (!capernian)
        return false;

    if (GetCurrentTarget(ai) != capernian && raidboss::TryCast(ai, "searing pain", capernian))
        return true;

    if (capernian->GetVictim() == bot)
    {
        constexpr float minDistance = 28.0f;
        const float currentDist = raidboss::ExactDist2d(bot, capernian) - capernian->GetCombatReach() - bot->GetCombatReach();
        if (currentDist < minDistance && MoveAwayFrom(capernian, minDistance - currentDist))
            return true;
    }

    return raidboss::TryCast(ai, "searing pain", capernian);
}

bool TkKaelthasSpreadAndMoveAwayFromCapernianAction::Execute(Event& event)
{
    Unit* capernian = raidboss::FindUnit(ai, NPC_CAPERNIAN);
    if (!capernian)
        return false;

    const uint32 phase = GetKaelthasTkPhase(ai, FindKaelthas(ai));
    if (phase == PHASE_NONE)
        return false;

    if (raidboss::IsRangedPlayer(bot) && capernian->GetVictim() != bot)
        return RangedBotsDisperse(phase, capernian);

    if (raidboss::IsMeleePlayer(bot) && phase == PHASE_SINGLE_ADVISOR)
        return MeleeStayBackFromCapernian(capernian);

    return false;
}

bool TkKaelthasSpreadAndMoveAwayFromCapernianAction::RangedBotsDisperse(uint32 phase, Unit* capernian)
{
    if (phase == PHASE_ALL_ADVISORS)
    {
        if (raidboss::FindUnit(ai, NPC_THALADRED))
            return false;

        constexpr float safeDistance = 6.0f;
        Player* nearestPlayer = raidboss::GetNearestPlayerInRadius(bot, safeDistance);
        return nearestPlayer && MoveAwayFrom(nearestPlayer, safeDistance);
    }

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (Player* member : raidboss::GetGroupMembers(bot, 500.0f, true))
    {
        if (!member->GetPlayerbotAI() || !raidboss::IsRangedPlayer(member))
            continue;

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    const bool isHeal = PlayerbotAI::IsHeal(bot);
    const std::vector<Player*>& ring = isHeal ? healers : rangedDps;
    const int botIndex = raidboss::GetIndexOf(bot, ring);
    if (botIndex < 0)
        return false;

    // The spread is a 90-degree arc for healers and a 120-degree arc for ranged DPS.
    // Capernian's CombatReach is 4.5y + standard player CombatReach (without Bloodlust) is 1.5y.
    const float arcSpan = isHeal ? M_PI_F / 2.0f : 2.0f * M_PI_F / 3.0f;
    const float radius = isHeal ? 42.0f : 34.0f;    // 36 and 28 yards of actual distance
    constexpr float arcCenter = 2.9f;
    const float arcStart = arcCenter - arcSpan / 2.0f;
    const size_t count = ring.size();
    const float angle = (count == 1) ? arcCenter : (arcStart + arcSpan * float(botIndex) / float(count - 1));

    const float targetX = capernian->GetPositionX() + radius * std::cos(angle);
    const float targetY = capernian->GetPositionY() + radius * std::sin(angle);
    if (raidboss::ExactDist2d(bot, targetX, targetY) <= 1.0f)
        return false;

    float targetZ = bot->GetMap()->GetHeight(targetX, targetY, bot->GetPositionZ() + 4.0f);
    if (targetZ <= INVALID_HEIGHT || std::fabs(targetZ - bot->GetPositionZ()) > 6.0f)
        return false;

    if (!bot->IsWithinLOS(targetX, targetY, targetZ + 2.0f))
        return false;

    bot->InterruptNonMeleeSpells(false);
    return MoveToPosition(targetX, targetY, targetZ);
}

bool TkKaelthasSpreadAndMoveAwayFromCapernianAction::MeleeStayBackFromCapernian(Unit* capernian)
{
    constexpr float safeDistance = 50.0f;   // There's no need for melee to be anywhere nearby.
    const float currentDistance = raidboss::ExactDist2d(bot, capernian);
    if (currentDistance >= safeDistance)
        return true;

    bot->InterruptNonMeleeSpells(false);
    return MoveAwayFrom(capernian, safeDistance - currentDistance);
}

bool TkKaelthasHandleAdvisorRolesInPhase3Action::Execute(Event& event)
{
    TkPos position;
    if (IsFirstAssistHealer(bot))
        position = ADVISOR_HEAL_POSITION;
    else if (raidboss::IsMainTank(bot))
        position = SANGUINAR_WAITING_POSITION;
    else if (raidboss::IsAssistTank(bot, 0))
        position = TELONICUS_WAITING_POSITION;
    else
        position = CAPERNIAN_WAITING_POSITION;

    return Dist2d(bot, position) > 2.0f && MoveToPosition(position);
}

bool TkKaelthasAssignAdvisorDpsPriorityAction::Execute(Event& event)
{
    const uint32 phase = GetKaelthasTkPhase(ai, FindKaelthas(ai));
    if (phase == PHASE_NONE)
        return false;

    const bool isPhase3 = phase == PHASE_ALL_ADVISORS;
    const bool isActiveCapernianTank = isPhase3 && IsCapernianTank(bot);
    Unit* target = nullptr;

    // Target priority 1: Thaladred, except Capernian tank during all advisors phase
    if (!isActiveCapernianTank)
    {
        Unit* thaladred = raidboss::FindUnit(ai, NPC_THALADRED);
        if (IsAdvisorActive(thaladred))
        {
            target = thaladred;
            if (isPhase3 && raidboss::MarkTargetWithSkull(bot, thaladred))
                return true;
        }
    }

    // Target priority 2: Capernian for ranged only (excluding debuff Hunter)
    if (!target && IsRangedDps(bot) && !IsSanguinarDebuffHunter(bot))
    {
        Unit* capernian = raidboss::FindUnit(ai, NPC_CAPERNIAN);
        if (IsAdvisorActive(capernian))
        {
            target = capernian;
            if (isPhase3 && raidboss::MarkTargetWithCross(bot, capernian))
                return true;
        }
    }

    // Target priority 3: Sanguinar (debuff Hunter and melee move here after Thaladred)
    if (!target)
    {
        Unit* sanguinar = raidboss::FindUnit(ai, NPC_SANGUINAR);
        if (IsAdvisorActive(sanguinar))
        {
            target = sanguinar;
            if (isPhase3 && raidboss::MarkTargetWithSkull(bot, sanguinar))
                return true;
        }
    }

    // Target priority 4: Telonicus
    Unit* telonicus = nullptr;
    if (!target)
    {
        telonicus = raidboss::FindUnit(ai, NPC_TELONICUS);
        if (IsAdvisorActive(telonicus))
        {
            target = telonicus;
            if (isPhase3 && raidboss::MarkTargetWithSkull(bot, telonicus))
                return true;
        }
    }

    if (!target)
        return false;

    if (GetCurrentTarget(ai) != target)
        return Attack(GetMaster(), target);

    if (target != telonicus || telonicus->GetVictim() == bot)
        return false;

    // Melee DPS need to stay at max-ish melee range behind Telonicus to avoid bombs.
    return MeleeDpsPositionOutsideBombRange(telonicus);
}

bool TkKaelthasAssignAdvisorDpsPriorityAction::MeleeDpsPositionOutsideBombRange(Unit* telonicus)
{
    if (!IsMeleeDps(bot))
        return false;

    const float desiredDist = bot->GetCombatReach() + telonicus->GetCombatReach() + 4.0f / 3.0f - 0.5f;
    const float behindAngle = telonicus->GetOrientation() + M_PI_F;
    const float targetX = telonicus->GetPositionX() + desiredDist * std::cos(behindAngle);
    const float targetY = telonicus->GetPositionY() + desiredDist * std::sin(behindAngle);

    if (raidboss::ExactDist2d(bot, targetX, targetY) <= 0.25f)
        return false;

    return MoveToPosition(targetX, targetY, telonicus->GetPositionZ());
}

bool TkKaelthasManageAdvisorDpsTimerAction::Execute(Event& event)
{
    Unit* kaelthas = FindKaelthas(ai);
    if (!kaelthas)
        return false;

    bool advisorAtFullHp = false;
    for (uint32 entry : { uint32(NPC_CAPERNIAN), uint32(NPC_TELONICUS), uint32(NPC_SANGUINAR) })
    {
        Unit* advisor = raidboss::FindUnit(ai, entry);
        if (IsAdvisorActive(advisor) && advisor->GetHealth() == advisor->GetMaxHealth())
        {
            advisorAtFullHp = true;
            break;
        }
    }

    return UpdateAdvisorDpsTimer(kaelthas->GetInstanceId(), advisorAtFullHp);
}

bool TkKaelthasAssignLegendaryWeaponDpsPriorityAction::Execute(Event& event)
{
    Unit* axe = GetLegendaryWeapon(ai, NPC_DEVASTATION);
    Unit* mace = GetLegendaryWeapon(ai, NPC_COSMIC_INFUSER);
    Unit* dagger = GetLegendaryWeapon(ai, NPC_INFINITY_BLADES);
    Unit* sword = GetLegendaryWeapon(ai, NPC_WARP_SLICER);

    const bool isTank = PlayerbotAI::IsTank(bot);
    const bool isRangedDps = IsRangedDps(bot);
    const bool isMeleeDps = IsMeleeDps(bot);

    // Priority 0: Everybody other than the main tank needs to stay away from the axe.
    // But for assist tanks, move away only after getting aggro on the mace, dagger, or sword.
    const bool didAvoidDevastation = axe && HandleDevastationAvoidance(axe, mace, dagger, sword, isTank, isMeleeDps);
    if (isTank)
        return didAvoidDevastation;

    // Melee dps obviously has to stand near a weapon to hit it, so any weapon too close to the axe
    // is skipped temporarily.
    constexpr float safeDistance = 12.0f;

    struct WeaponPriority
    {
        uint32 entry;
        // The axe is marked with a cross instead of skull as it is outside the full priority
        // chain (only ranged dps attacks it).
        bool markWithCross;
        bool rangedDpsOnly;
    };

    static const WeaponPriority weaponPriorities[] =
    {
        { NPC_STAFF_OF_DISINTEGRATION, false, false },
        { NPC_COSMIC_INFUSER,          false, false },
        { NPC_NETHERSTRAND_LONGBOW,    false, false },
        { NPC_DEVASTATION,             true,  true  },
        { NPC_INFINITY_BLADES,         false, false },
        { NPC_WARP_SLICER,             false, false },
        { NPC_PHASESHIFT_BULWARK,      false, false },
    };

    Unit* target = nullptr;
    bool markClaimed = false;
    bool skippedForAxe = false;
    for (const WeaponPriority& weapon : weaponPriorities)
    {
        if (weapon.rangedDpsOnly && !isRangedDps)
            continue;

        Unit* candidate = GetLegendaryWeapon(ai, weapon.entry);
        if (!candidate)
            continue;

        if (!markClaimed)
        {
            markClaimed = true;
            if (weapon.markWithCross ? raidboss::MarkTargetWithCross(bot, candidate) : raidboss::MarkTargetWithSkull(bot, candidate))
                return true;
        }

        if (isMeleeDps && axe && raidboss::ExactDist2d(candidate, axe) <= safeDistance)
        {
            skippedForAxe = true;
            continue;
        }

        target = candidate;
        break;
    }

    if (!target)
    {
        if (!skippedForAxe)
            return didAvoidDevastation;

        // If every weapon still standing is inside the axe's reach, then melee should hold and
        // attack nothing rather than end up in Whirlwind range.
        if (!GetCurrentTarget(ai))
            return didAvoidDevastation;

        ClearTarget(ai);
        return true;
    }

    return didAvoidDevastation || (GetCurrentTarget(ai) != target && Attack(GetMaster(), target));
}

bool TkKaelthasAssignLegendaryWeaponDpsPriorityAction::HandleDevastationAvoidance(Unit* axe, Unit* mace, Unit* dagger, Unit* sword, bool isTank, bool isMeleeDps)
{
    const bool hasAggroFromWeapon = (mace && mace->GetVictim() == bot) || (dagger && dagger->GetVictim() == bot) || (sword && sword->GetVictim() == bot);

    bool result = false;
    if (!isTank || hasAggroFromWeapon)
    {
        const float safeDistance = isTank ? 17.0f : 12.0f;
        const float currentDistance = raidboss::ExactDist2d(bot, axe);
        if (currentDistance < safeDistance)
            result = MoveAwayFrom(axe, safeDistance - currentDistance);
    }

    // Just in case melee ends up on the axe despite the priority list leaving them off
    if (isMeleeDps && GetCurrentTarget(ai) == axe)
        ClearTarget(ai);

    return result;
}

bool TkKaelthasMoveDevastationAwayAction::Execute(Event& event)
{
    Unit* axe = GetLegendaryWeapon(ai, NPC_DEVASTATION);
    if (!axe)
        return false;

    if (GetCurrentTarget(ai) != axe)
        return Attack(GetMaster(), axe);

    if (raidboss::MarkTargetWithCross(bot, axe))
        return true;

    if (axe->GetVictim() != bot || !bot->CanReachWithMeleeAttack(axe))
        return false;

    constexpr float safeDistance = 15.0f;
    if (!GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return false;

    return MoveFromGroup(safeDistance, true);
}

bool TkKaelthasLootLegendaryWeaponsAction::Execute(Event& event)
{
    static const std::pair<uint32, uint32> weapons[] =
    {
        { NPC_NETHERSTRAND_LONGBOW, ITEM_NETHERSTRAND_LONGBOW },
        { NPC_COSMIC_INFUSER, ITEM_COSMIC_INFUSER },
        { NPC_DEVASTATION, ITEM_DEVASTATION },
        { NPC_INFINITY_BLADES, ITEM_INFINITY_BLADE },
        { NPC_WARP_SLICER, ITEM_WARP_SLICER },
        { NPC_STAFF_OF_DISINTEGRATION, ITEM_STAFF_OF_DISINTEGRATION },
        { NPC_PHASESHIFT_BULWARK, ITEM_PHASESHIFT_BULWARK },
    };

    bool equipped = false;
    for (const auto& weapon : weapons)
    {
        if (!ShouldBotLootWeapon(weapon.first))
            continue;

        if (bot->HasItemCount(weapon.second, 1))
        {
            equipped |= EquipLegendaryWeapon(weapon.second);
            continue;
        }

        return LootWeapon(weapon.first, weapon.second) || equipped;
    }

    return equipped;
}

bool TkKaelthasLootLegendaryWeaponsAction::ShouldBotLootWeapon(uint32 weaponEntry)
{
    // Talent tabs: warrior 0 arms / 1 fury / 2 protection, rogue 1 combat, shaman 1 enhancement, druid 1 feral
    const int tab = AiFactory::GetPlayerSpecTab(bot);
    const uint8 cls = bot->getClass();
    const bool isArmsWarrior = cls == CLASS_WARRIOR && tab == 0;

    switch (weaponEntry)
    {
    case NPC_NETHERSTRAND_LONGBOW:      // Hunter
        return cls == CLASS_HUNTER;
    case NPC_COSMIC_INFUSER:            // Healer
        return PlayerbotAI::IsHeal(bot);
    // Fury Warriors are better off looting the dagger to break MC
    case NPC_DEVASTATION:               // Arms Warrior, Ret Paladin
        return isArmsWarrior || (cls == CLASS_PALADIN && raidboss::IsDps(bot));
    case NPC_INFINITY_BLADES:           // Rogue, Hunter, Fury/Prot Warrior, Enh Shaman
        return cls == CLASS_ROGUE || cls == CLASS_HUNTER || (cls == CLASS_WARRIOR && !isArmsWarrior) ||
            (cls == CLASS_SHAMAN && tab == 1);
    case NPC_WARP_SLICER:               // Prot Paladin, Combat Rogue
        return (cls == CLASS_PALADIN && PlayerbotAI::IsTank(bot)) || (cls == CLASS_ROGUE && tab == 1);
    case NPC_STAFF_OF_DISINTEGRATION:   // Dps caster, Feral Druid
        return (cls != CLASS_HUNTER && IsRangedDps(bot)) || (cls == CLASS_DRUID && tab == 1);
    case NPC_PHASESHIFT_BULWARK:        // Prot Paladin/Warrior
        return (cls == CLASS_WARRIOR || cls == CLASS_PALADIN) && PlayerbotAI::IsTank(bot);
    default:
        return false;
    }
}

bool TkKaelthasLootLegendaryWeaponsAction::LootWeapon(uint32 weaponEntry, uint32 itemId)
{
    ItemPosCountVec dest;
    if (bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1) != EQUIP_ERR_OK)
        return false;

    Creature* weapon = GetDeadLegendaryWeapon(ai, weaponEntry);
    if (!weapon)
        return false;

    if (!bot->IsWithinDistInMap(weapon, INTERACTION_DISTANCE - 1.0f))
    {
        const float targetDist = INTERACTION_DISTANCE - 2.0f;
        const float angle = weapon->GetAngle(bot);
        return MoveToPosition(weapon->GetPositionX() + std::cos(angle) * targetDist, weapon->GetPositionY() + std::sin(angle) * targetDist, bot->GetPositionZ());
    }

    if (bot->IsMoving())
        bot->StopMoving();

    const ObjectGuid corpseGuid = weapon->GetObjectGuid();
    WorldPacket lootPacket(CMSG_LOOT, 8);
    lootPacket << corpseGuid;
    bot->GetSession()->HandleLootOpcode(lootPacket);

    bool looted = false;
    if (Loot* loot = sLootMgr.GetLoot(bot, corpseGuid))
    {
        LootItemList items;
        loot->GetLootItemsListFor(bot, items);
        for (LootItem* item : items)
        {
            if (item && item->itemId == itemId)
            {
                looted = loot->SendItem(bot, item) == EQUIP_ERR_OK;
                break;
            }
        }
    }

    WorldPacket releasePacket(CMSG_LOOT_RELEASE, 8);
    releasePacket << corpseGuid;
    bot->GetSession()->HandleLootReleaseOpcode(releasePacket);
    return looted;
}

bool TkKaelthasLootLegendaryWeaponsAction::EquipLegendaryWeapon(uint32 itemId)
{
    Item* legendaryItem = nullptr;
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < INVENTORY_SLOT_ITEM_END && !legendaryItem; ++slot)
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (item && item->GetEntry() == itemId)
            legendaryItem = item;
    }

    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END && !legendaryItem; ++bag)
    {
        Item* bagItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (!bagItem || !bagItem->IsBag())
            continue;

        Bag* container = static_cast<Bag*>(bagItem);
        for (uint8 slot = 0; slot < container->GetBagSize(); ++slot)
        {
            Item* item = container->GetItemByPos(slot);
            if (item && item->GetEntry() == itemId)
            {
                legendaryItem = item;
                break;
            }
        }
    }

    if (!legendaryItem || !legendaryItem->GetProto())
        return false;

    const ItemPrototype* proto = legendaryItem->GetProto();
    if (proto->InventoryType == INVTYPE_NON_EQUIP)
        return false;

    uint8 dstSlot = EQUIPMENT_SLOT_MAINHAND;
    if (proto->InventoryType == INVTYPE_RANGED || proto->InventoryType == INVTYPE_RANGEDRIGHT)
        dstSlot = EQUIPMENT_SLOT_RANGED;
    else if (proto->InventoryType == INVTYPE_SHIELD || proto->InventoryType == INVTYPE_WEAPONOFFHAND)
        dstSlot = EQUIPMENT_SLOT_OFFHAND;

    // Infinity Blade prefers OH when MH already holds a legendary (Combat Rogues equip Warp
    // Slicer MH, Infinity Blade OH)
    if (dstSlot == EQUIPMENT_SLOT_MAINHAND && itemId == ITEM_INFINITY_BLADE && bot->CanDualWield())
    {
        Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        if (mhItem && IsLegendaryWeaponItem(mhItem->GetEntry()) && mhItem->GetEntry() != itemId)
            dstSlot = EQUIPMENT_SLOT_OFFHAND;
    }

    Item* alreadyEquipped = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, dstSlot);
    if (alreadyEquipped && alreadyEquipped->GetEntry() == itemId)
        return false;

    bot->InterruptNonMeleeSpells(false);

    // A 2h blocks the offhand, so it has to be stored in the backpack first.
    if (dstSlot == EQUIPMENT_SLOT_OFFHAND)
    {
        Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        if (mhItem && mhItem->GetProto()->InventoryType == INVTYPE_2HWEAPON)
        {
            for (uint8 bpSlot = INVENTORY_SLOT_ITEM_START; bpSlot < INVENTORY_SLOT_ITEM_END; ++bpSlot)
            {
                if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bpSlot))
                {
                    bot->SwapItem((INVENTORY_SLOT_BAG_0 << 8) | EQUIPMENT_SLOT_MAINHAND, (INVENTORY_SLOT_BAG_0 << 8) | bpSlot);
                    return true;
                }
            }

            return false;
        }
    }

    uint16 dest = 0;
    if (bot->CanEquipItem(dstSlot, dest, legendaryItem, true) != EQUIP_ERR_OK)
        return false;

    bot->SwapItem((legendaryItem->GetBagSlot() << 8) | legendaryItem->GetSlot(), (INVENTORY_SLOT_BAG_0 << 8) | dstSlot);

    // Swapping from a 2H to a 1H: fill the empty offhand from the bags
    if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
    {
        Item* mhItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        if (!mhItem || mhItem->GetProto()->InventoryType != INVTYPE_2HWEAPON)
        {
            if (Item* offhand = GetEquippableItemForSlot(bot, EQUIPMENT_SLOT_OFFHAND))
                bot->SwapItem((offhand->GetBagSlot() << 8) | offhand->GetSlot(), (INVENTORY_SLOT_BAG_0 << 8) | EQUIPMENT_SLOT_OFFHAND);
        }
    }

    return true;
}

bool TkKaelthasUseLegendaryWeaponsAction::Execute(Event& event)
{
    if (bot->getClass() == CLASS_HUNTER)
        return UseNetherstrandLongbow();

    if (bot->getClass() != CLASS_DRUID && PlayerbotAI::IsTank(bot))
        return UsePhaseshiftBulwark();

    return UseStaffOfDisintegration();
}

bool TkKaelthasUseLegendaryWeaponsAction::UsePhaseshiftBulwark()
{
    Unit* kaelthas = FindKaelthas(ai);
    if (!kaelthas || !kaelthas->HasAura(SPELL_SHOCK_BARRIER) || bot->HasAura(SPELL_ARCANE_BARRIER))
        return false;

    return UseEquippedItem(GetEquippedItemInSlot(bot, EQUIPMENT_SLOT_OFFHAND, ITEM_PHASESHIFT_BULWARK));
}

bool TkKaelthasUseLegendaryWeaponsAction::UseStaffOfDisintegration()
{
    if (bot->HasAura(SPELL_MENTAL_PROTECTION_FIELD))
        return false;

    return UseEquippedItem(GetEquippedItemInSlot(bot, EQUIPMENT_SLOT_MAINHAND, ITEM_STAFF_OF_DISINTEGRATION));
}

bool TkKaelthasUseLegendaryWeaponsAction::UseNetherstrandLongbow()
{
    // Make legendary arrows if none are left, and shoot them.
    if (bot->HasItemCount(ITEM_NETHER_SPIKES, 1))
    {
        if (bot->GetUInt32Value(PLAYER_AMMO_ID) != ITEM_NETHER_SPIKES)
        {
            bot->SetAmmo(ITEM_NETHER_SPIKES);
            return true;
        }

        return false;
    }

    return UseEquippedItem(GetEquippedItemInSlot(bot, EQUIPMENT_SLOT_RANGED, ITEM_NETHERSTRAND_LONGBOW));
}

bool TkKaelthasUseLegendaryWeaponsAction::UseEquippedItem(Item* item)
{
    if (!item || bot->CanUseItem(item) != EQUIP_ERR_OK || bot->IsNonMeleeSpellCasted(true))
        return false;

    const ItemPrototype* proto = item->GetProto();
    uint32 spellId = 0;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (proto->Spells[i].SpellId > 0 && proto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
        {
            spellId = proto->Spells[i].SpellId;
            break;
        }
    }

    if (!spellId || !bot->IsSpellReady(spellId, proto))
        return false;

    return bot->CastSpell(bot, spellId, TRIGGERED_NONE, item) == SPELL_CAST_OK;
}

// The legendary weapons are removed after the encounter: equip the old weapons again
bool TkKaelthasReequipGearAction::Execute(Event& event)
{
    const uint8 weaponSlots[] = { EQUIPMENT_SLOT_MAINHAND, EQUIPMENT_SLOT_OFFHAND, EQUIPMENT_SLOT_RANGED };
    for (uint8 slot : weaponSlots)
    {
        if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            continue;

        if (Item* item = GetEquippableItemForSlot(bot, slot))
        {
            bot->SwapItem((item->GetBagSlot() << 8) | item->GetSlot(), (INVENTORY_SLOT_BAG_0 << 8) | slot);
            return true;
        }
    }

    return false;
}

bool TkKaelthasTanksPositionBossAction::Execute(Event& event)
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* kaelthas = FindKaelthas(ai);
    if (!kaelthas || kaelthas->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE))
        return false;

    // Off-tanks are repositioned if Kael aggroes them, but only the main tank picks him up.
    constexpr float tolerance = 4.0f;
    return MoveToTankPosition(kaelthas, KAELTHAS_TANK_POSITION, tolerance, raidboss::IsMainTank(bot));
}

bool TkKaelthasAvoidFlameStrikeAction::Execute(Event& event)
{
    constexpr float hazardRadius = 10.0f;
    Creature* flameStrike = GetNearestFlameStrikeInRadius(bot, hazardRadius);
    if (!flameStrike)
        return false;

    bot->InterruptNonMeleeSpells(false);
    return MoveAwayFrom(flameStrike, hazardRadius - raidboss::ExactDist2d(bot, flameStrike) + 1.0f);
}

bool TkKaelthasAssignFinalPhaseTargetAction::Execute(Event& event)
{
    if (raidboss::IsAssistTank(bot, 0) || raidboss::IsAssistTank(bot, 1))
    {
        if (Unit* phoenix = GetAssignedPhoenix())
            return AssistTankPicksUpPhoenix(phoenix);

        Unit* kaelthas = FindKaelthas(ai);
        if (!kaelthas || kaelthas->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE))
            return false;

        return GetCurrentTarget(ai) != kaelthas && Attack(GetMaster(), kaelthas);
    }

    return NonTanksAssignTargetAndAvoidPhoenixes();
}

// If there is more than one Phoenix up, the assist tanks will each pick up one.
Unit* TkKaelthasAssignFinalPhaseTargetAction::GetAssignedPhoenix()
{
    std::vector<Unit*> phoenixes;
    for (Unit* phoenix : raidboss::FindUnits(bot, NPC_PHOENIX, 100.0f))
    {
        if (!phoenix->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE))
            phoenixes.push_back(phoenix);
    }

    if (phoenixes.empty())
        return nullptr;

    std::sort(phoenixes.begin(), phoenixes.end(), [](Unit* a, Unit* b) { return a->GetObjectGuid() < b->GetObjectGuid(); });

    if (!raidboss::IsAssistTank(bot, 0) && phoenixes.size() >= 2)
        return phoenixes[1];

    return phoenixes[0];
}

bool TkKaelthasAssignFinalPhaseTargetAction::AssistTankPicksUpPhoenix(Unit* phoenix)
{
    if (GetCurrentTarget(ai) != phoenix)
        return Attack(GetMaster(), phoenix);

    if (phoenix->GetVictim() != bot)
        return false;

    constexpr float safeDistance = 12.0f;
    if (!GetNearestNonTankPlayerInRadius(bot, safeDistance))
        return false;

    return MoveFromGroup(safeDistance, true);
}

// Priority: (1) Kael with Shock Barrier (to interrupt Pyroblast), (2) Eggs (ranged only), (3) Kael
// without Shock Barrier, and (4) Phoenixes but only for ranged during Kael's power-up scene.
bool TkKaelthasAssignFinalPhaseTargetAction::NonTanksAssignTargetAndAvoidPhoenixes()
{
    Unit* phoenix = raidboss::FindUnit(ai, NPC_PHOENIX);
    if (phoenix && phoenix->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE))
        phoenix = nullptr;

    if (phoenix)
    {
        constexpr float safeDistance = 15.0f;
        const float currentDistance = raidboss::ExactDist2d(bot, phoenix);

        // A Phoenix is survivable and a Flame Strike is not, so don't try to avoid Phoenixes when
        // too close to a Flame Strike
        constexpr float flameStrikeRadius = 15.0f;
        if (currentDistance < safeDistance && !GetNearestFlameStrikeInRadius(bot, flameStrikeRadius) &&
            MoveAwayFrom(phoenix, safeDistance - currentDistance))
        {
            return true;
        }
    }

    Unit* kaelthas = FindKaelthas(ai);
    const bool isKaelthasAttackable = kaelthas && !kaelthas->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
    const bool isRanged = raidboss::IsRangedPlayer(bot);

    Unit* target = nullptr;
    Creature* egg = isRanged ? GetPhoenixEgg(bot) : nullptr;
    if (isKaelthasAttackable && kaelthas->HasAura(SPELL_SHOCK_BARRIER))
        target = kaelthas;
    else if (egg)
        target = egg;
    else if (isKaelthasAttackable)
        target = kaelthas;
    else if (isRanged)
        target = phoenix;

    return target && GetCurrentTarget(ai) != target && Attack(GetMaster(), target);
}

bool TkKaelthasBreakMindControlAction::Execute(Event& event)
{
    Player* mcTarget = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    for (Player* member : raidboss::GetGroupMembers(bot, 100.0f, false))
    {
        if (!member->HasAura(SPELL_KAELTHAS_MIND_CONTROL))
            continue;

        const float dist = raidboss::ExactDist2d(bot, member);
        if (dist < closestDist)
        {
            closestDist = dist;
            mcTarget = member;
        }
    }

    if (!mcTarget)
        return false;

    if (!bot->CanReachWithMeleeAttack(mcTarget))
        return MoveToPosition(mcTarget->GetPositionX(), mcTarget->GetPositionY(), mcTarget->GetPositionZ());

    // Any melee hit with the Infinity Blade breaks the Mind Control
    const char* spell = nullptr;
    switch (bot->getClass())
    {
    case CLASS_HUNTER:  spell = "wing clip"; break;
    case CLASS_ROGUE:   spell = AiFactory::GetPlayerSpecTab(bot) == 1 ? "shiv" : "sinister strike"; break;
    case CLASS_SHAMAN:  spell = "stormstrike"; break;
    case CLASS_WARRIOR: spell = "hamstring"; break;
    default:            return false;
    }

    return raidboss::TryCast(ai, spell, mcTarget);
}

// Gravity Lapse lifts the players into the air with a fly aura. Bots don't answer the fly
// packets, so the flight flags are set here and the bots spread out in the air.
bool TkKaelthasSpreadOutInMidairAction::Execute(Event& event)
{
    if (!bot->HasAura(SPELL_GRAVITY_LAPSE_AURA))
        return DropToGround();

    return HoverAndSpread();
}

namespace
{
    const uint32 TK_FLY_FLAGS = MOVEFLAG_FLYING | MOVEFLAG_FLYING2 | MOVEFLAG_LEVITATING | MOVEFLAG_CAN_FLY;

    void SetBotFlying(Player* bot, bool apply)
    {
        WorldPacket data(apply ? SMSG_SPLINE_MOVE_SET_FLYING : SMSG_SPLINE_MOVE_UNSET_FLYING, 9);
        data << bot->GetPackGUID();
        bot->SendMessageToSet(data, true);

        if (apply)
            bot->m_movementInfo.AddMovementFlag(MovementFlags(TK_FLY_FLAGS));
        else
            bot->m_movementInfo.RemoveMovementFlag(MovementFlags(TK_FLY_FLAGS));
    }

    float Roll(uint32 value, uint32 salt)
    {
        uint32 hash = value + salt;
        hash ^= hash >> 16;
        hash *= 0x85EBCA6Bu;
        hash ^= hash >> 13;
        hash *= 0xC2B2AE35u;
        hash ^= hash >> 16;
        return (hash >> 8) / float(1 << 24);
    }
}

bool TkKaelthasSpreadOutInMidairAction::DropToGround()
{
    if (!bot->movespline->Finalized())
        return false;

    if (bot->IsFalling())
    {
        bot->m_movementInfo.RemoveMovementFlag(MovementFlags(MOVEFLAG_FALLING | MOVEFLAG_FALLINGFAR));
        return true;
    }

    const float floorZ = bot->GetMap()->GetHeight(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    if (floorZ <= INVALID_HEIGHT)
        return false;

    const bool flying = bot->m_movementInfo.HasMovementFlag(MovementFlags(TK_FLY_FLAGS));
    const float heightAboveFloor = bot->GetPositionZ() - floorZ;
    if (heightAboveFloor <= 1.0f && !flying)
        return false;

    if (flying)
        SetBotFlying(bot, false);

    if (heightAboveFloor <= 0.5f)
        return flying;

    bot->GetMotionMaster()->Clear();
    bot->GetMotionMaster()->MoveFall();
    return true;
}

bool TkKaelthasSpreadOutInMidairAction::HoverAndSpread()
{
    if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
        SetBotFlying(bot, true);

    SpellAuraHolder* lapse = bot->GetSpellAuraHolder(SPELL_GRAVITY_LAPSE_AURA);
    if (!lapse)
        return false;

    const uint32 seed = bot->GetObjectGuid().GetCounter();

    constexpr float minHoverHeight = 10.0f;
    constexpr float maxHoverHeight = 35.0f;
    const float desiredHeight = minHoverHeight + Roll(seed, 1u) * (maxHoverHeight - minHoverHeight);

    // Short random reaction time, so the bots don't all move at once
    constexpr int32 minReactionMs = 250;
    constexpr int32 maxReactionMs = 1600;
    const int32 reactionDelayMs = minReactionMs + int32(Roll(seed, 2u) * (maxReactionMs - minReactionMs));
    if (lapse->GetAuraMaxDuration() - lapse->GetAuraDuration() < reactionDelayMs)
        return false;

    constexpr float maxAimBias = 0.20f;
    const float aimBias = (Roll(seed, 3u) * 2.0f - 1.0f) * maxAimBias;

    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();
    const float botZ = bot->GetPositionZ();

    constexpr float safeDistance = 18.0f;
    float pushX = 0.0f, pushY = 0.0f, pushZ = 0.0f;
    float closestDist = std::numeric_limits<float>::max();
    for (Player* member : raidboss::GetGroupMembers(bot, 100.0f, false))
    {
        const float dx = botX - member->GetPositionX();
        const float dy = botY - member->GetPositionY();
        const float dz = botZ - member->GetPositionZ();
        const float distToMember = std::sqrt(dx * dx + dy * dy + dz * dz);
        closestDist = std::min(closestDist, distToMember);
        if (distToMember >= safeDistance)
            continue;

        const float violation = safeDistance - distToMember;
        if (distToMember < 0.1f)
        {
            const float bearing = bot->GetAngle(member) + M_PI_F;
            pushX += std::cos(bearing) * violation;
            pushY += std::sin(bearing) * violation;
            continue;
        }

        pushX += dx / distToMember * violation;
        pushY += dy / distToMember * violation;
        pushZ += dz / distToMember * violation;
    }

    const float floorZ = bot->GetMap()->GetHeight(botX, botY, botZ);
    if (floorZ <= INVALID_HEIGHT)
        return false;

    const float cosBias = std::cos(aimBias);
    const float sinBias = std::sin(aimBias);
    const float biasedPushX = pushX * cosBias - pushY * sinBias;
    pushY = pushX * sinBias + pushY * cosBias;
    pushX = biasedPushX;

    constexpr float heightPull = 0.35f;
    pushZ += (desiredHeight - (botZ - floorZ)) * heightPull;

    // Stay within 40 yards of Kael'thas
    Unit* kaelthas = FindKaelthas(ai);
    if (!kaelthas)
        return false;

    constexpr float containmentRadius = 40.0f;
    const float anchorDist = raidboss::ExactDist2d(bot, kaelthas);
    if (anchorDist > containmentRadius)
    {
        const float pull = anchorDist - containmentRadius;
        pushX += (kaelthas->GetPositionX() - botX) / anchorDist * pull;
        pushY += (kaelthas->GetPositionY() - botY) / anchorDist * pull;
    }

    constexpr float chainDangerDistance = 15.0f;
    constexpr float minMoveDistance = 4.0f;
    if (closestDist > chainDangerDistance && std::sqrt(pushX * pushX + pushY * pushY + pushZ * pushZ) < minMoveDistance)
        return false;

    constexpr float stepFraction = 0.4f;
    const float targetX = botX + pushX * stepFraction;
    const float targetY = botY + pushY * stepFraction;
    float targetZ = botZ + pushZ * stepFraction;

    const float targetFloorZ = bot->GetMap()->GetHeight(targetX, targetY, targetZ);
    if (targetFloorZ <= INVALID_HEIGHT)
        return false;

    targetZ = std::max(targetFloorZ + minHoverHeight, std::min(targetZ, targetFloorZ + maxHoverHeight));

    if (!bot->GetMap()->IsInLineOfSight(botX, botY, botZ + 1.0f, targetX, targetY, targetZ + 1.0f, true))
        return false;

    const float dx = targetX - botX, dy = targetY - botY, dz = targetZ - botZ;
    if (std::sqrt(dx * dx + dy * dy + dz * dz) <= 1.0f)
        return false;

    bot->GetMotionMaster()->Clear();
    bot->GetMotionMaster()->MovePoint(0, Position(targetX, targetY, targetZ, 0.f), FORCED_MOVEMENT_FLIGHT, 0.f, false);
    return true;
}
