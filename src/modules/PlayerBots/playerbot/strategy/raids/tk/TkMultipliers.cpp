// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/TK/TKMultipliers) and adapted for this core.
#include "playerbot/playerbot.h"
#include "TkMultipliers.h"
#include "TkActions.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/actions/ChooseTargetActions.h"
#include "playerbot/strategy/actions/ReachTargetActions.h"
#include "playerbot/strategy/actions/FollowActions.h"
#include "playerbot/strategy/actions/GenericSpellActions.h"

using namespace ai;
using namespace ai::tk;

namespace
{
    bool NameIs(Action* action, std::initializer_list<const char*> names)
    {
        const std::string& name = action->getName();
        for (const char* n : names)
        {
            if (name == n)
                return true;
        }

        return false;
    }

    // Spread / flee movements of the generic strategies (CombatFormationMoveAction / FleeAction)
    bool IsDisperseOrFlee(Action* action)
    {
        return dynamic_cast<FleeAction*>(action) || NameIs(action, { "runaway", "tbc raid spread" });
    }

    // Automatic target selection (TankAssistAction / DpsAssistAction)
    bool IsDpsAssist(Action* action)
    {
        return dynamic_cast<DpsAssistAction*>(action) ||
            NameIs(action, { "dps aoe", "attack least hp target", "tbc raid attack priority target" });
    }

    bool IsTankAssist(Action* action)
    {
        return dynamic_cast<TankAssistAction*>(action) != nullptr;
    }

    bool IsMovementOrReachSpell(Action* action)
    {
        return dynamic_cast<MovementAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action);
    }
}

float TkEncounterMultiplier::GetValue(Action* action)
{
    if (!action || !tk::IsTkEncounterInProgress(bot))
        return 1.0f;

    return GetValueInEncounter(action);
}

// Al'ar <Phoenix God>

float TkAlarSuppressGapClosersMultiplier::GetValueInEncounter(Action* action)
{
    if (ai->GetState() == BotState::BOT_STATE_NON_COMBAT)
        return 1.0f;

    const bool isBlockedMovement = dynamic_cast<ReachTargetAction*>(action) || NameIs(action, { "killing spree", "disengage", "blink" });
    if (!isBlockedMovement && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return 1.0f;

    if (isBlockedMovement)
        return 0.0f;

    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    // Block Charge, etc. for non-tanks when not at a platform.
    const int8 currentLocationIndex = GetAlarCurrentLocationIndex(alar);
    return currentLocationIndex < PLATFORM_0_IDX || currentLocationIndex > PLATFORM_3_IDX ? 0.0f : 1.0f;
}

float TkAlarControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    const bool isDisperseOrFlee = IsDisperseOrFlee(action);
    if (!isDisperseOrFlee && !dynamic_cast<FollowAction*>(action))
        return 1.0f;

    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar)
        return 1.0f;

    if (isDisperseOrFlee)
        return 0.0f;

    if (!IsAlarInPhase2(alar->GetInstanceId()))
        return 1.0f;

    // Enable following only in the non-combat engine in Phase 2.
    return ai->GetState() == BotState::BOT_STATE_COMBAT ? 0.0f : 1.0f;
}

float TkAlarDisableAutomaticTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (ai->GetState() == BotState::BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsTankAssist(action) && !IsDpsAssist(action))
        return 1.0f;

    return raidboss::FindUnit(ai, NPC_ALAR) ? 0.0f : 1.0f;
}

float TkAlarStayAwayFromRebirthMultiplier::GetValueInEncounter(Action* action)
{
    if (raidboss::IsRangedPlayer(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    // Don't block Flame Quills avoidance in case of bad timing for the transition.
    if (dynamic_cast<TkAlarMoveAwayFromRebirthAction*>(action) || dynamic_cast<TkAlarJumpFromPlatformAction*>(action))
        return 1.0f;

    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return 1.0f;

    if (IsAlarPassive(alar))
        return 0.0f;

    constexpr float phase1AlmostEndedHpThreshold = 5.0f;
    return alar->GetHealthPercent() <= phase1AlmostEndedHpThreshold ? 0.0f : 1.0f;
}

float TkAlarControlTauntingMultiplier::GetValueInEncounter(Action* action)
{
    if (ai->GetState() == BotState::BOT_STATE_NON_COMBAT || !raidboss::IsTauntAction(bot, action))
        return 1.0f;

    const bool isFirstAlarTank = IsFirstAlarTank(bot);
    if (!isFirstAlarTank && !IsSecondAlarTank(bot))
        return 1.0f;

    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar)
        return 1.0f;

    if (bot->HasAura(SPELL_MELT_ARMOR) && GetCurrentTarget(ai) == alar)
        return 0.0f;

    if (IsAlarInPhase2(alar->GetInstanceId()))
        return 1.0f;

    const int8 platformIndex = GetAlarPlatformIndex(alar);
    if (isFirstAlarTank)
        return platformIndex != PLATFORM_0_IDX && platformIndex != PLATFORM_2_IDX ? 0.0f : 1.0f;

    return platformIndex != PLATFORM_1_IDX && platformIndex != PLATFORM_3_IDX ? 0.0f : 1.0f;
}

// Void Reaver

float TkVoidReaverMaintainPositionsMultiplier::GetValueInEncounter(Action* action)
{
    if (ai->GetState() == BotState::BOT_STATE_NON_COMBAT || !IsDisperseOrFlee(action))
        return 1.0f;

    return raidboss::FindUnit(ai, NPC_VOID_REAVER) ? 0.0f : 1.0f;
}

// High Astromancer Solarian

float TkSolarianWrathStayAwayMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsMovementOrReachSpell(action) || dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<TkSolarianMoveAwayFromGroupAction*>(action))
        return 1.0f;

    Unit* astromancer = raidboss::FindUnit(ai, NPC_SOLARIAN);
    if (!astromancer || astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM))
        return 1.0f;

    return HasWrathOfTheAstromancer(bot) ? 0.0f : 1.0f;
}

float TkSolarianDisableMeleeTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (ai->GetState() == BotState::BOT_STATE_NON_COMBAT || !raidboss::IsMeleePlayer(bot))
        return 1.0f;

    if (!IsTankAssist(action) && !IsDpsAssist(action))
        return 1.0f;

    Unit* astromancer = raidboss::FindUnit(ai, NPC_SOLARIAN);
    if (!astromancer)
        return 1.0f;

    if (raidboss::IsMainTank(bot))
        return IsSolarianTankable(astromancer) ? 0.0f : 1.0f;

    return raidboss::FindUnit(ai, NPC_SOLARIUM_PRIEST) ? 0.0f : 1.0f;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

float TkKaelthasWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action) || NameIs(action, { "fear ward" }))
        return 1.0f;

    Unit* kaelthas = FindKaelthas(ai);
    if (!kaelthas || GetKaelthasTkPhase(ai, kaelthas) != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    constexpr uint32 dpsWaitMs = 10 * IN_MILLISECONDS;
    if (IsAdvisorDpsWaitOver(kaelthas->GetInstanceId(), dpsWaitMs))
        return 1.0f;

    // Only the applicable tank may attack during the first 10 seconds of an advisor in phase 1.
    if (IsAdvisorActive(raidboss::FindUnit(ai, NPC_SANGUINAR)))
        return raidboss::IsMainTank(bot) ? 1.0f : 0.0f;

    if (IsAdvisorActive(raidboss::FindUnit(ai, NPC_CAPERNIAN)))
        return IsCapernianTank(bot) ? 1.0f : 0.0f;

    if (IsAdvisorActive(raidboss::FindUnit(ai, NPC_TELONICUS)))
        return raidboss::IsAssistTank(bot, 0) ? 1.0f : 0.0f;

    return 1.0f;
}

float TkKaelthasKiteThaladredMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsMovementOrReachSpell(action) || dynamic_cast<TkKaelthasKiteThaladredAction*>(action))
        return 1.0f;

    Unit* thaladred = raidboss::FindUnit(ai, NPC_THALADRED);
    if (!thaladred || thaladred->GetVictim() != bot)
        return 1.0f;

    const uint32 phase = GetKaelthasTkPhase(ai, FindKaelthas(ai));
    if (phase == PHASE_NONE || (PlayerbotAI::IsTank(bot) && phase == PHASE_ALL_ADVISORS))
        return 1.0f;

    return 0.0f;
}

float TkKaelthasControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (ai->GetState() == BotState::BOT_STATE_NON_COMBAT || bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!NameIs(action, { "misdirection on party tank" }))
        return 1.0f;

    const uint32 phase = GetKaelthasTkPhase(ai, FindKaelthas(ai));
    return phase != PHASE_NONE && phase != PHASE_FINAL ? 0.0f : 1.0f;
}

float TkKaelthasKeepDistanceFromCapernianMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsMovementOrReachSpell(action) || dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<TkKaelthasSpreadAndMoveAwayFromCapernianAction*>(action))
        return 1.0f;

    if (GetKaelthasTkPhase(ai, FindKaelthas(ai)) != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    return IsAdvisorActive(raidboss::FindUnit(ai, NPC_CAPERNIAN)) ? 0.0f : 1.0f;
}

// Try to keep main tank from grabbing aggro on any weapon other than the axe.
float TkKaelthasManageWeaponTankingMultiplier::GetValueInEncounter(Action* action)
{
    if (ai->GetState() == BotState::BOT_STATE_NON_COMBAT || !raidboss::IsMainTank(bot))
        return 1.0f;

    if (!raidboss::IsTauntAction(bot, action) && !raidboss::IsAoeThreatAction(bot, action))
        return 1.0f;

    return GetKaelthasTkPhase(ai, FindKaelthas(ai)) == PHASE_WEAPONS ? 0.0f : 1.0f;
}

float TkKaelthasSuppressEquipUpgradeMultiplier::GetValueInEncounter(Action* action)
{
    if (!NameIs(action, { "equip upgrades", "equip upgrade" }))
        return 1.0f;

    return FindKaelthas(ai) ? 0.0f : 1.0f;
}

float TkKaelthasManageAutomaticTargetingMultiplier::GetValueInEncounter(Action* action)
{
    const bool isDpsAssist = IsDpsAssist(action);
    if (!isDpsAssist && !IsTankAssist(action))
        return 1.0f;

    Unit* kaelthas = FindKaelthas(ai);
    const uint32 phase = GetKaelthasTkPhase(ai, kaelthas);
    if (phase == PHASE_NONE)
        return 1.0f;

    // Kael drops every attacker when he becomes uninteractible for his power-up scene.
    if (ai->GetState() == BotState::BOT_STATE_NON_COMBAT &&
        !(phase == PHASE_FINAL && kaelthas->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE)))
        return 1.0f;

    if (isDpsAssist)
        return 0.0f;

    // All tanks are blocked during advisor and final phases.
    if (phase == PHASE_SINGLE_ADVISOR || phase == PHASE_ALL_ADVISORS || phase == PHASE_FINAL)
        return 0.0f;

    // Only main tank is blocked during weapons/transition phases.
    return raidboss::IsMainTank(bot) ? 0.0f : 1.0f;
}

float TkKaelthasDisableDisperseMultiplier::GetValueInEncounter(Action* action)
{
    if (ai->GetState() == BotState::BOT_STATE_NON_COMBAT || !IsDisperseOrFlee(action))
        return 1.0f;

    return FindKaelthas(ai) ? 0.0f : 1.0f;
}

float TkKaelthasPrepareForPhase3Multiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<TkKaelthasHandleAdvisorRolesInPhase3Action*>(action))
        return 1.0f;

    if (GetKaelthasTkPhase(ai, FindKaelthas(ai)) != PHASE_ALL_ADVISORS)
        return 1.0f;

    Unit* sanguinar = raidboss::FindUnit(ai, NPC_SANGUINAR);
    if (IsFirstAssistHealer(bot))
    {
        if (dynamic_cast<TkKaelthasKiteThaladredAction*>(action))
            return 1.0f;

        return sanguinar ? 0.0f : 1.0f;
    }

    if (!IsAdvisorReviving(sanguinar))
        return 1.0f;

    return raidboss::IsMainTank(bot) || raidboss::IsAssistTank(bot, 0) || IsCapernianTank(bot) ? 0.0f : 1.0f;
}

// Bloodlust/Heroism and other major cooldowns should be saved until Phase 3.
float TkKaelthasDelayCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (ai->GetState() == BotState::BOT_STATE_NON_COMBAT || !raidboss::IsDpsCooldownAction(bot, action))
        return 1.0f;

    const uint32 phase = GetKaelthasTkPhase(ai, FindKaelthas(ai));
    if (phase == PHASE_NONE)
        return 1.0f;

    const bool isLustAction = bot->getClass() == CLASS_SHAMAN && NameIs(action, { "bloodlust", "heroism" });
    if (isLustAction && phase == PHASE_WEAPONS)
        return 0.0f;

    return phase == PHASE_SINGLE_ADVISOR || phase == PHASE_TRANSITION ? 0.0f : 1.0f;
}

float TkKaelthasStaySpreadDuringGravityLapseMultiplier::GetValueInEncounter(Action* action)
{
    if (!bot->HasAura(SPELL_GRAVITY_LAPSE_AURA) || !IsMovementOrReachSpell(action))
        return 1.0f;

    if (raidboss::IsRangedPlayer(bot) && dynamic_cast<AttackAction*>(action))
        return 1.0f;

    return dynamic_cast<TkKaelthasSpreadOutInMidairAction*>(action) ? 1.0f : 0.0f;
}
