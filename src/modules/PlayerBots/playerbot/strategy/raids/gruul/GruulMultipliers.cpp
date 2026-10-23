// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "GruulMultipliers.h"
#include "GruulActions.h"
#include "GruulHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;
using namespace ai::gruul;

namespace
{
    // mod-playerbots CastReachTargetSpellAction: spells which move the bot to its target
    bool IsGapCloserAction(Action* action)
    {
        const std::string& name = action->getName();
        return name == "charge" || name == "intercept" || name == "feral charge - bear" ||
               name == "feral charge - cat" || name == "shadowstep" || name == "intervene";
    }

    // Movement in the sense of mod-playerbots: MovementAction or CastReachTargetSpellAction
    bool IsAnyMovementAction(Action* action)
    {
        return raidboss::IsMovementAction(action) || IsGapCloserAction(action);
    }

    bool IsReachTargetAction(Action* action)
    {
        const std::string& name = action->getName();
        return name == "reach melee" || name == "reach spell";
    }

    Unit* GetCurrentTarget(PlayerbotAI* ai)
    {
        return ai->GetUnit(ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Get());
    }
}

// General

float HighKingMaulgarHoldPreparationMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;
    const std::string& name = action->getName();
    if (name != "follow" && name != "follow master" && name != "move random" && name != "move to loot")
        return 1.0f;
    float range = 0;
    Unit* target = GetPrepareTarget(ai, range);
    return target && IsPrepared(bot, target, range) ? 0.0f : 1.0f;
}

float GruulsLairEncounterMultiplier::GetValue(Action* action)
{
    if (!action || !IsGruulsLairEncounterInProgress(bot))
        return 1.0f;

    return GetValueInEncounter(action);
}

float GruulsLairDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (!raidboss::IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* gruul = FindGruul(ai);
    if (gruul && gruul->GetHealthPercent() > raidboss::BOSS_ENGAGED_HEALTH_PCT)
        return 0.0f;

    Unit* blindeye = FindBlindeye(ai);
    return blindeye && blindeye->GetHealthPercent() > BLINDEYE_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
}

// High King Maulgar <Lord of the Ogres>

float HighKingMaulgarControlTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* maulgar = FindMaulgar(ai);
    if (!maulgar)
        return 1.0f;

    const std::string& name = action->getName();
    if (name == "tank assist")
        return 0.0f;

    // Once Maulgar is following his tank, let the positioning action lead him to the
    // tank spot (and wait there). Chasing him between steps undoes the drag. Keep
    // normal reaching and gap closers available when another player holds aggro.
    if (IsMaulgarTank(bot) && maulgar->GetVictim() == bot &&
        (IsReachTargetAction(action) || IsGapCloserAction(action) || name == "set behind" || name == "follow"))
        return 0.0f;

    return 1.0f;
}

float HighKingMaulgarRestrictTauntingMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    const bool isAoeThreat = raidboss::IsAoeThreatAction(bot, action);
    if (!isAoeThreat && !raidboss::IsTauntAction(bot, action))
        return 1.0f;

    // The main tank stays on Maulgar the whole time so it can do whatever.
    if (raidboss::IsMainTank(bot))
        return 1.0f;

    // Blindeye and Olm are tanked next to each other by separate tanks; until Blindeye is dead,
    // don't use AoE threat abilities.
    if (isAoeThreat && FindBlindeye(ai))
        return 0.0f;

    // After Blindeye and Olm the assist tanks join the melee on Maulgar: don't taunt him off the
    // main tank. The main tank is the first living tank, so this lifts itself if it dies.
    Unit* currentTarget = GetCurrentTarget(ai);
    if (currentTarget && currentTarget == FindMaulgar(ai))
        return 0.0f;

    // Kiggler is the only ogre tanked by a non-traditional tank that the assist tanks are directed
    // to attack (if Maulgar dies first): don't taunt him off the moonkin.
    Unit* kiggler = FindKiggler(ai);
    if (!kiggler || !GetKigglerMoonkinTank(ai))
        return 1.0f;

    return GetCurrentTarget(ai) == kiggler ? 0.0f : 1.0f;
}

float HighKingMaulgarDisableDpsAssistMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsTank(bot) || action->getName() != "dps assist")
        return 1.0f;

    return FindMaulgar(ai) ? 0.0f : 1.0f;
}

float HighKingMaulgarAvoidWhirlwindMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsAnyMovementAction(action) || dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<HighKingMaulgarRunAwayFromWhirlwindAction*>(action))
    {
        return 1.0f;
    }

    Unit* maulgar = FindMaulgar(ai);
    if (!maulgar || !maulgar->HasAura(SPELL_WHIRLWIND) || raidboss::IsMainTank(bot))
        return 1.0f;

    return raidboss::ExactDist2d(bot, maulgar) < MAULGAR_WHIRLWIND_HOLD_DISTANCE ? 0.0f : 1.0f;
}

float HighKingMaulgarControlHunterActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    const std::string& name = action->getName();
    const bool isTankMisdirect = name == "misdirection on party tank";
    if (!isTankMisdirect && name != "arcane shot")
        return 1.0f;

    // Krosh/Kiggler will be the last to die before Maulgar.
    // When only Maulgar is left, the standard Misdirection strategy is fine.
    Unit* krosh = FindKrosh(ai);
    if (isTankMisdirect && (krosh || FindKiggler(ai)))
        return 0.0f;

    // Arcane Shot removes Spell Shield, which the mage tank needs to survive.
    return krosh && action->GetTarget() == krosh ? 0.0f : 1.0f;
}

float HighKingMaulgarControlMageTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_MAGE)
        return 1.0f;

    const std::string& name = action->getName();
    if (action->getThreatType() != ActionThreatType::ACTION_THREAT_AOE && name != "ice block" && name != "invisibility")
        return 1.0f;

    if (!FindKrosh(ai))
        return 1.0f;

    return GetKroshMageTank(ai) == bot ? 0.0f : 1.0f;
}

// Gruul the Dragonkiller

float GruulTheDragonkillerControlTankMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* gruul = FindGruul(ai);
    if (!gruul)
        return 1.0f;

    const std::string& name = action->getName();
    // Keep both tanks in place instead of fleeing automatically. Only the tank
    // holding aggro leads Gruul; the off-tank must still reach melee for Hurtful Strike.
    if (name == "flee")
        return 0.0f;
    if (gruul->GetVictim() == bot &&
        (IsReachTargetAction(action) || IsGapCloserAction(action) || name == "set behind" || name == "follow"))
        return 0.0f;
    return 1.0f;
}

float GruulTheDragonkillerStaySpreadForShatterMultiplier::GetValueInEncounter(Action* action)
{
    if (!HasGroundSlam(bot) || !IsAnyMovementAction(action))
        return 1.0f;

    return dynamic_cast<GruulTheDragonkillerShatterSpreadAction*>(action) ||
           dynamic_cast<GruulTheDragonkillerGetOutOfCaveInAction*>(action) ? 1.0f : 0.0f;
}

// When near a Cave In, ignore the ranged spread, as well as standard movement actions like reaching
// the target, with some exceptions for tanks (and full exception for the active tank on Gruul).
float GruulTheDragonkillerControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (PlayerbotAI::IsTank(bot) && (IsGapCloserAction(action) || IsReachTargetAction(action)))
        return 1.0f;

    if (!IsAnyMovementAction(action) || dynamic_cast<GruulTheDragonkillerGetOutOfCaveInAction*>(action))
        return 1.0f;

    // The shatter spread multiplier takes over during Ground Slam (and allows the Cave In escape).
    if (HasGroundSlam(bot))
        return 1.0f;

    Unit* gruul = FindGruul(ai);
    if (!gruul || gruul->GetVictim() == bot)
        return 1.0f;

    return IsNearCaveIn(bot, CAVE_IN_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// A movement with a snare of -100% would never finish and block all other movements meanwhile.
float GruulTheDragonkillerHoldWhileSnaredMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->GetSpeed(MOVE_RUN) > 0.0f || !raidboss::IsMovementAction(action))
        return 1.0f;

    return FindGruul(ai) ? 0.0f : 1.0f;
}
