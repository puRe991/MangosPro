// Ported from mod-playerbots (src/Ai/Raid/Mag, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "MagtheridonMultipliers.h"
#include "MagtheridonActions.h"
#include "MagtheridonHelpers.h"
#include "playerbot/strategy/actions/GenericSpellActions.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;
using namespace ai::magtheridon;

namespace
{
    // mod-playerbots CastReachTargetSpellAction: spells which move the bot to its target
    bool IsGapCloserAction(Action* action)
    {
        const std::string& name = action->getName();
        return name == "charge" || name == "intercept" || name == "feral charge - bear" ||
               name == "feral charge - cat" || name == "shadowstep" || name == "intervene";
    }

    bool IsAnyMovementAction(Action* action)
    {
        return raidboss::IsMovementAction(action) || IsGapCloserAction(action);
    }
}

float MagtheridonEncounterMultiplier::GetValue(Action* action)
{
    if (!action || !raidboss::IsEncounterInProgress(bot, MAG_MAP_ID))
        return 1.0f;

    return GetValueInEncounter(action);
}

// Cube clickers stay by their cube once Blast Nova is coming
float MagtheridonControlCrowdControlMultiplier::GetValueInEncounter(Action* action)
{
    // Generic CC picks the same marked target for several warlocks and can even
    // remove an existing Banish. The encounter action owns the Abyssal rotation.
    if (bot->getClass() == CLASS_WARLOCK && (action->getName() == "banish" || action->getName() == "fear") &&
        !GetBurningAbyssals(ai).empty())
        return 0.0f;
    return 1.0f;
}

float MagtheridonUseManticronCubeMultiplier::GetValueInEncounter(Action* action)
{
    if (dynamic_cast<AttackAction*>(action) || dynamic_cast<MagtheridonUseManticronCubeAction*>(action))
        return 1.0f;

    const std::string& name = action->getName();
    if (!IsAnyMovementAction(action) && name != "blink" && name != "disengage")
        return 1.0f;

    if (!IsCubeClicker(bot) || !IsMagtheridonActive(FindMagtheridon(ai)))
        return 1.0f;

    return IsBlastNovaSoon(bot) ? 0.0f : 1.0f;
}

// Give the main tank some seconds of threat after Magtheridon is released
float MagtheridonHoldDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<AttackAction*>(action) && !dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* magtheridon = FindMagtheridon(ai);
    if (!IsMagtheridonActive(magtheridon) || raidboss::IsMainTank(bot))
        return 1.0f;

    uint32 time = 0;
    if (!GetDpsWaitTime(magtheridon->GetInstanceId(), time))
        return 0.0f;

    return WorldTimer::getMSTimeDiff(time, WorldTimer::getMSTime()) <= MAG_DPS_HOLD_MS ? 0.0f : 1.0f;
}

float MagtheridonControlTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    // "flee" is the action of the "avoid aoe" strategy
    const bool isAvoidAoe = action->getName() == "flee";
    const bool isGapCloser = IsGapCloserAction(action);
    if (!isAvoidAoe && !isGapCloser && !raidboss::IsTauntAction(bot, action) && action->getName() != "tank assist")
        return 1.0f;

    Unit* magtheridon = FindMagtheridon(ai);
    if (!magtheridon)
        return 1.0f;

    if (isAvoidAoe && magtheridon->GetVictim() != bot)
        return 1.0f;

    // Blocks the main tank from charging the assist tanks' channelers while moving to the waiting
    // position.
    if (isGapCloser && raidboss::IsMainTank(bot))
        return IsMagtheridonActive(magtheridon) ? 1.0f : 0.0f;

    return 0.0f;
}

float MagtheridonAvoidDebrisDangerMultiplier::GetValueInEncounter(Action* action)
{
    if (dynamic_cast<AttackAction*>(action) || dynamic_cast<MagtheridonUseManticronCubeAction*>(action) ||
        dynamic_cast<MagtheridonMoveOutOfDebrisAction*>(action))
    {
        return 1.0f;
    }

    if (!IsAnyMovementAction(action) || !IsCeilingCollapsed(bot))
        return 1.0f;

    if (!IsMagtheridonActive(FindMagtheridon(ai)))
        return 1.0f;

    const float debrisSuppressionZone = 15.0f;
    return IsPositionInActiveDebris(bot, bot->GetPositionX(), bot->GetPositionY(), debrisSuppressionZone) ? 0.0f : 1.0f;
}
