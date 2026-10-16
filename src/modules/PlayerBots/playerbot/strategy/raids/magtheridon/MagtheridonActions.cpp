// Ported from mod-playerbots (src/Ai/Raid/Mag, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "MagtheridonActions.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include <cmath>
#include <limits>

using namespace ai;
using namespace ai::magtheridon;

namespace
{
    Unit* GetCurrentTarget(PlayerbotAI* ai)
    {
        return ai->GetUnit(ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Get());
    }
}

bool MagtheridonResetEncounterStatesAction::Execute(Event& /*event*/)
{
    bool reset = ResetEncounterState(bot->GetInstanceId());

    if (!bot->IsInCombat())
        reset |= raidboss::ClearTargetIcon(bot, raidboss::ICON_SKULL);

    return reset;
}

bool MagtheridonMainTankAttackFirstThreeChannelersAction::Execute(Event& /*event*/)
{
    Unit* channeler = GetChanneler(ai, CHANNELER_SOUTH);
    if (!channeler)
        channeler = GetChanneler(ai, CHANNELER_WEST);
    if (!channeler)
        channeler = GetChanneler(ai, CHANNELER_EAST);

    if (channeler)
        return GetCurrentTarget(ai) != channeler && Attack(GetMaster(), channeler);

    // After the first three channelers are dead, wait for Magtheridon to activate.
    const WorldPosition& position = WAITING_FOR_MAGTHERIDON_POSITION;
    if (raidboss::ExactDist2d(bot, position.getX(), position.getY()) <= 2.0f)
    {
        if (std::fabs(bot->GetOrientation() - position.getO()) > 0.1f)
            bot->SetFacingTo(position.getO());

        return true;
    }

    return MoveTo(MAG_MAP_ID, position.getX(), position.getY(), position.getZ(), false, false, false, true);
}

bool MagtheridonAssistTanksAttackLastTwoChannelersAction::Execute(Event& /*event*/)
{
    Unit* channeler = nullptr;
    WorldPosition position;
    if (raidboss::IsAssistTank(bot, 0))
    {
        channeler = GetChanneler(ai, CHANNELER_NORTHWEST);
        position = NW_CHANNELER_TANK_POSITION;
    }
    else
    {
        channeler = GetChanneler(ai, CHANNELER_NORTHEAST);
        position = NE_CHANNELER_TANK_POSITION;
    }

    if (!channeler)
        return false;

    if (GetCurrentTarget(ai) != channeler)
        return Attack(GetMaster(), channeler);

    if (channeler->GetVictim() != bot)
        return false;

    // Movement is intentionally forwards only, so no facing is passed.
    const float arrivalDist = 3.0f;
    float moveX, moveY;
    bool backwards;
    if (!raidboss::GetStepToPosition(bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(MAG_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, true);
}

bool MagtheridonMisdirectHellfireChannelersToMainTankAction::Execute(Event& /*event*/)
{
    const int hunterIndex = raidboss::GetIndexOf(bot, GetGroupBotsOfClass(bot, CLASS_HUNTER));

    Player* mainTank = raidboss::GetGroupMainTank(bot);
    if (!mainTank || !mainTank->IsAlive())
        return false;

    Unit* channeler = nullptr;
    if (hunterIndex == 0)
        channeler = GetChanneler(ai, CHANNELER_WEST);
    else if (hunterIndex == 1)
        channeler = GetChanneler(ai, CHANNELER_EAST);

    if (!channeler)
        return false;

    if (raidboss::TryCast(ai, "misdirection", mainTank))
        return true;

    if (!bot->HasAura(SPELL_MISDIRECTION))
        return false;

    return raidboss::TryCast(ai, "steady shot", channeler);
}

bool MagtheridonAssignDpsPriorityAction::Execute(Event& /*event*/)
{
    const ChannelerSlot order[] = { CHANNELER_SOUTH, CHANNELER_WEST, CHANNELER_EAST, CHANNELER_NORTHWEST, CHANNELER_NORTHEAST };

    Unit* channeler = nullptr;
    for (ChannelerSlot slot : order)
    {
        if ((channeler = GetChanneler(ai, slot)))
            break;
    }

    if (!channeler)
        return false;

    if (GetCurrentTarget(ai) != channeler)
        return Attack(GetMaster(), channeler);

    return raidboss::MarkTargetWithSkull(bot, channeler);
}

// Establish a Banish rotation for Burning Abyssals and a Fear rotation for any Burning Abyssals
// that exceed the number of warlocks in the raid.
bool MagtheridonWarlockCcBurningAbyssalAction::Execute(Event& /*event*/)
{
    const std::vector<Unit*> abyssals = GetBurningAbyssals(ai);
    const std::vector<Player*> warlocks = GetGroupBotsOfClass(bot, CLASS_WARLOCK);
    const int warlockIndex = raidboss::GetIndexOf(bot, warlocks);
    if (warlockIndex < 0)
        return false;

    if (warlockIndex < int(abyssals.size()))
    {
        Unit* assignedAbyssal = abyssals[warlockIndex];
        if (!ai->HasAura("banish", assignedAbyssal) && raidboss::TryCast(ai, "banish", assignedAbyssal))
            return true;
    }

    for (size_t i = warlocks.size() + warlockIndex; i < abyssals.size(); i += warlocks.size())
    {
        Unit* excessAbyssal = abyssals[i];
        if (!ai->HasAura("banish", excessAbyssal) && !ai->HasAura("fear", excessAbyssal) &&
            raidboss::TryCast(ai, "fear", excessAbyssal))
        {
            return true;
        }
    }

    return false;
}

bool MagtheridonMainTankPositionBossAction::Execute(Event& /*event*/)
{
    Unit* magtheridon = FindMagtheridon(ai);
    if (!magtheridon)
        return false;

    if (GetCurrentTarget(ai) != magtheridon)
        return Attack(GetMaster(), magtheridon);

    const float stopForHealHealthPct = 50.0f;
    if (magtheridon->GetVictim() != bot || !bot->CanReachWithMeleeAttack(magtheridon) ||
        bot->GetHealthPercent() < stopForHealHealthPct)
    {
        return false;
    }

    const float arrivalDist = 3.0f;
    float moveX, moveY;
    bool backwards;
    if (!raidboss::GetStepToPosition(bot, MAGTHERIDON_TANK_POSITION, arrivalDist, magtheridon, moveX, moveY, backwards))
        return false;

    return MoveTo(MAG_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, true);
}

bool MagtheridonSpreadRangedAction::Execute(Event& /*event*/)
{
    Unit* magtheridon = FindMagtheridon(ai);
    if (!magtheridon)
        return false;

    float x, y, z;

    // This 20y distance is sort of a cheap way to stay out of Magtheridon's Cleave.
    const float safeDistFromBoss = 20.0f;
    if (raidboss::ExactDist2d(bot, magtheridon) < safeDistFromBoss &&
        GetPointAwayFrom(bot, magtheridon->GetPositionX(), magtheridon->GetPositionY(), safeDistFromBoss, x, y, z))
    {
        return MoveTo(MAG_MAP_ID, x, y, z, false, false, false, true);
    }

    const float safeDistFromPlayer = 6.0f;
    Player* nearestPlayer = raidboss::GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    if (!nearestPlayer || !GetPointAwayFrom(bot, nearestPlayer->GetPositionX(), nearestPlayer->GetPositionY(), safeDistFromPlayer, x, y, z))
        return false;

    return MoveTo(MAG_MAP_ID, x, y, z, false, false, false, true);
}

bool MagtheridonUseManticronCubeAction::Execute(Event& /*event*/)
{
    Unit* magtheridon = FindMagtheridon(ai);
    if (!magtheridon)
        return false;

    CubeInfo cubeInfo;
    if (!GetAssignedCube(bot, cubeInfo))
        return false;

    GameObject* cube = ai->GetGameObject(cubeInfo.guid);
    if (!cube)
        return false;

    // Release the cube after Blast Nova is interrupted.
    if (HandleCubeRelease(magtheridon))
        return true;

    // While Blast Nova is casting (2s), always try to click the cube.
    if (IsBlastNovaCasting(magtheridon))
        return HandleCubeInteraction(cube);

    // Otherwise, if Blast Nova is coming soon, move to and wait near the cube.
    return HandleWaitingPhase(cubeInfo);
}

bool MagtheridonUseManticronCubeAction::HandleCubeRelease(Unit* magtheridon)
{
    if (!bot->HasAura(SPELL_SHADOW_GRASP) || IsBlastNovaCasting(magtheridon))
    {
        releaseTime = 0;
        return false;
    }

    // Stagger releasing the cube so the clickers do not do it on the same tick.
    const uint32 now = WorldTimer::getMSTime();
    if (!releaseTime)
    {
        releaseTime = now + urand(400, 1500);
        return true;
    }

    if (now < releaseTime)
        return true;

    releaseTime = 0;
    bot->InterruptSpell(CURRENT_CHANNELED_SPELL);
    return true;
}

bool MagtheridonUseManticronCubeAction::HandleCubeInteraction(GameObject* cube)
{
    if (cube->IsAtInteractDistance(bot))
    {
        // If already clicked, don't let go!
        if (bot->HasAura(SPELL_SHADOW_GRASP))
            return true;

        if (bot->HasAura(SPELL_MIND_EXHAUSTION))
            return false;

        if (bot->IsNonMeleeSpellCasted(true))
            bot->InterruptNonMeleeSpells(true);

        bot->StopMoving();
        bot->SetFacingToObject(cube);
        cube->Use(bot);
        return true;
    }

    // Run to the side of the cube facing the bot
    const float targetDist = std::max(1.0f, cube->GetInteractionDistance() - 0.5f);
    const float angle = cube->GetAngle(bot);
    const float destX = cube->GetPositionX() + std::cos(angle) * targetDist;
    const float destY = cube->GetPositionY() + std::sin(angle) * targetDist;
    float destZ = bot->GetMap()->GetHeight(destX, destY, cube->GetPositionZ() + 5.0f);
    if (destZ <= INVALID_HEIGHT)
        destZ = cube->GetPositionZ();

    if (bot->IsNonMeleeSpellCasted(true))
        bot->InterruptNonMeleeSpells(true);

    return MoveTo(MAG_MAP_ID, destX, destY, destZ, false, false, false, true);
}

bool MagtheridonUseManticronCubeAction::HandleWaitingPhase(const CubeInfo& cubeInfo)
{
    if (!IsBlastNovaSoon(bot))
        return false;

    // If a hazard appears at the waiting position, find a new waiting position at the same distance
    // from the cube.
    const float safeWaitDistance = 10.0f;
    const bool onRing = std::fabs(raidboss::ExactDist2d(bot, cubeInfo.x, cubeInfo.y) - safeWaitDistance) <= 1.0f;
    const bool inHazard = IsPositionInActiveDebris(bot, bot->GetPositionX(), bot->GetPositionY()) ||
                          IsPositionInActiveConflagration(bot, bot->GetPositionX(), bot->GetPositionY());
    if (onRing && !inHazard)
        return false;

    float x, y;
    if (!FindSafePositionNearCube(cubeInfo, safeWaitDistance, x, y))
        return false;

    if (bot->IsNonMeleeSpellCasted(true))
        bot->InterruptNonMeleeSpells(true);

    return MoveTo(MAG_MAP_ID, x, y, bot->GetPositionZ(), false, false, false, true);
}

bool MagtheridonUseManticronCubeAction::FindSafePositionNearCube(const CubeInfo& cubeInfo, float preferredDistance, float& outX, float& outY)
{
    const uint8 numAngles = 16;
    const float angleStep = 2.0f * M_PI_F / numAngles;

    const std::vector<WorldPosition> debris = FindDebrisPositions(bot);
    const std::vector<GameObject*> blazes = GetActiveConflagrations(bot);

    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;
    for (uint8 i = 0; i < numAngles; ++i)
    {
        const float angle = i * angleStep;
        const float x = cubeInfo.x + std::cos(angle) * preferredDistance;
        const float y = cubeInfo.y + std::sin(angle) * preferredDistance;

        if (IsPositionInDebris(debris, x, y) || IsPositionInConflagration(blazes, x, y))
            continue;

        const float moveDistance = raidboss::ExactDist2d(bot, x, y);
        if (moveDistance < minMoveDistance)
        {
            outX = x;
            outY = y;
            minMoveDistance = moveDistance;
            foundSafe = true;
        }
    }

    return foundSafe;
}

bool MagtheridonMoveOutOfDebrisAction::Execute(Event& /*event*/)
{
    float x, y;
    if (!FindSafePosition(x, y))
        return false;

    bot->InterruptNonMeleeSpells(true);
    return MoveTo(MAG_MAP_ID, x, y, bot->GetPositionZ(), false, false, false, true);
}

bool MagtheridonMoveOutOfDebrisAction::FindSafePosition(float& outX, float& outY)
{
    const float minSearchRadius = 2.0f;
    const float distanceStep = 1.0f;
    const uint8 numDistSteps = 18;
    const uint8 numAngles = 24;
    const float angleStep = 2.0f * M_PI_F / numAngles;

    const std::vector<WorldPosition> debris = FindDebrisPositions(bot);
    const std::vector<GameObject*> blazes = GetActiveConflagrations(bot);

    // The nearest safe point wins: the first distance ring with a safe point is enough
    for (uint8 i = 0; i <= numDistSteps; ++i)
    {
        const float distance = minSearchRadius + i * distanceStep;
        for (uint8 j = 0; j < numAngles; ++j)
        {
            const float angle = j * angleStep;
            const float x = bot->GetPositionX() + distance * std::cos(angle);
            const float y = bot->GetPositionY() + distance * std::sin(angle);

            if (IsPositionInDebris(debris, x, y) || IsPositionInConflagration(blazes, x, y))
                continue;

            outX = x;
            outY = y;
            return true;
        }
    }

    return false;
}

bool MagtheridonUpdateTimersAndAssignmentsAction::Execute(Event& /*event*/)
{
    Unit* magtheridon = FindMagtheridon(ai);
    if (!IsMagtheridonActive(magtheridon))
        return false;

    return UpdateEncounterState(bot, magtheridon);
}
