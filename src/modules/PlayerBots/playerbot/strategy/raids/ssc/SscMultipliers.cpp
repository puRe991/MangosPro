// Ported from mod-playerbots (src/Ai/Raid/SSC/SSCMultipliers.cpp, GPL v2+) and adapted for this core.
// Action classes of AzerothCore without an equivalent class here are matched by action name.
#include "playerbot/playerbot.h"
#include "SscMultipliers.h"
#include "SscActions.h"
#include "SscHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/actions/ChooseTargetActions.h"
#include "playerbot/strategy/actions/FollowActions.h"
#include "playerbot/strategy/actions/GenericSpellActions.h"
#include "playerbot/strategy/actions/LootAction.h"
#include "playerbot/strategy/actions/ReachTargetActions.h"

using namespace ai;
using namespace ai::ssc;

namespace
{
    Unit* Engaged(PlayerbotAI* ai, uint32 entry)
    {
        Unit* unit = FindSscUnit(ai, entry);
        return unit && unit->IsInCombat() ? unit : nullptr;
    }

    // CombatFormationMoveAction of mod-playerbots: the spreading / distance keeping moves of this core
    bool IsFormationMove(Action* action)
    {
        return IsActionNamed(action, { "tbc raid spread", "move out of enemy contact", "move from group" });
    }

    bool IsFleeAction(Action* action)
    {
        return dynamic_cast<FleeAction*>(action) || IsActionNamed(action, { "flee", "runaway", "flee with pet" });
    }

    bool IsReachSpell(Action* action)
    {
        return dynamic_cast<CastReachTargetSpellAction*>(action) || IsActionNamed(action, { "reach spell" });
    }

    bool IsReachMelee(Action* action)
    {
        return dynamic_cast<ReachTargetAction*>(action) || IsActionNamed(action, { "reach melee", "reach target" });
    }

    bool IsDamageAction(Action* action)
    {
        return dynamic_cast<AttackAction*>(action) ||
               (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action));
    }

    bool IsLustAction(Action* action)
    {
        return IsActionNamed(action, { "bloodlust", "heroism" });
    }

    bool IsWaitExpired(time_t start, time_t seconds)
    {
        return start && time(nullptr) - start >= seconds;
    }
}

// Trash

float SscUnderbogColossusEscapeToxicPoolMultiplier::GetValue(Action* action)
{
    if (bot->HasAura(SPELL_TOXIC_POOL) && raidboss::IsMovementAction(action) &&
        !dynamic_cast<SscUnderbogColossusEscapeToxicPoolAction*>(action) && !IsTbcRaidAction(action))
        return 0.0f;

    return 1.0f;
}

// Hydross the Unstable <Duke of Currents>

float SscHydrossDisableTankActionsMultiplier::GetValue(Action* action)
{
    const bool frostTank = raidboss::IsMainTank(bot);
    const bool natureTank = raidboss::IsAssistTank(bot, 0);
    if (!frostTank && !natureTank)
        return 1.0f;

    Unit* hydross = Engaged(ai, NPC_HYDROSS);
    if (!hydross)
        return 1.0f;

    if (dynamic_cast<SscHydrossPositionFrostTankAction*>(action) || dynamic_cast<SscHydrossPositionNatureTankAction*>(action))
        return 1.0f;

    // The positioning / taunting of the "tbc raid" table would fight the fixed tank spots
    if (IsActionNamed(action, { "tank assist", "tbc raid hydross tank", "tbc raid taunt", "tank face", "set behind", "follow" }) || IsFormationMove(action))
        return 0.0f;

    const bool corrupted = hydross->HasAura(SPELL_CORRUPTION);
    if ((frostTank && !corrupted) || (natureTank && corrupted))
        return 1.0f;

    // The wrong tank for the current form waits at its spot
    if (IsReachSpell(action) || IsReachMelee(action) || IsDamageAction(action))
        return 0.0f;

    return 1.0f;
}

float SscHydrossWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* hydross = Engaged(ai, NPC_HYDROSS);
    if (!hydross)
        return 1.0f;

    // Additional tanks pick up the elementals
    if (PlayerbotAI::IsTank(bot) && !raidboss::IsMainTank(bot) && !raidboss::IsAssistTank(bot, 0) &&
        (FindSscUnit(ai, NPC_PURE_SPAWN_OF_HYDROSS) || FindSscUnit(ai, NPC_TAINTED_SPAWN_OF_HYDROSS)))
        return 1.0f;

    if (dynamic_cast<SscHydrossMisdirectBossToTankAction*>(action) ||
        dynamic_cast<SscHydrossPositionFrostTankAction*>(action) || dynamic_cast<SscHydrossPositionNatureTankAction*>(action) ||
        !IsDamageAction(action))
        return 1.0f;

    const bool nature = hydross->HasAura(SPELL_CORRUPTION);
    if ((!nature && raidboss::IsMainTank(bot)) || (nature && raidboss::IsAssistTank(bot, 0)))
        return 1.0f;

    return ShouldPauseHydrossDps(ai) ? 0.0f : 1.0f;
}
float SscHydrossControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() == CLASS_HUNTER && IsActionNamed(action, { "misdirection on party tank" }) && Engaged(ai, NPC_HYDROSS))
        return 0.0f;

    return 1.0f;
}

// The Lurker Below

float SscLurkerStayAwayFromSpoutMultiplier::GetValue(Action* action)
{
    if (!raidboss::IsMovementAction(action) && !IsReachSpell(action))
        return 1.0f;

    Unit* lurker = Engaged(ai, NPC_LURKER_BELOW);
    if (!lurker || (!IsLurkerSpouting(lurker) && GetState(bot).lurkerSpoutEnd <= time(nullptr)))
        return 1.0f;

    if (IsReachSpell(action))
        return 0.0f;

    // Jumping into the water ("tbc raid") and running behind the boss are the only moves
    if (!dynamic_cast<AttackAction*>(action) && !dynamic_cast<SscLurkerRunAroundBehindBossAction*>(action) && !IsTbcRaidAction(action))
        return 0.0f;

    return 1.0f;
}

float SscLurkerMaintainRangedSpreadMultiplier::GetValue(Action* action)
{
    if (!raidboss::IsRangedPlayer(bot) || !(IsFormationMove(action) || IsFleeAction(action)))
        return 1.0f;

    return Engaged(ai, NPC_LURKER_BELOW) ? 0.0f : 1.0f;
}

// Disable tank assist during Submerge only if there are 3 or more tanks in the raid
float SscLurkerDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsTank(bot) || !bot->GetVictim() || !IsActionNamed(action, { "tank assist" }))
        return 1.0f;

    if (!IsLurkerSubmerged(Engaged(ai, NPC_LURKER_BELOW)))
        return 1.0f;

    return raidboss::GetGroupTanks(bot).size() >= 3 ? 0.0f : 1.0f;
}

// Leotheras the Blind

float SscLeotherasAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot) || bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return 1.0f;

    if (!raidboss::IsMovementAction(action) && !IsReachSpell(action))
        return 1.0f;

    Unit* leotheras = FindSscUnit(ai, NPC_LEOTHERAS, 100.0f);
    if (!leotheras || (!leotheras->HasAura(SPELL_WHIRLWIND) && !leotheras->HasAura(SPELL_WHIRLWIND_CHANNEL)))
        return 1.0f;

    if (IsReachSpell(action))
        return 0.0f;

    if (!dynamic_cast<AttackAction*>(action) && !dynamic_cast<SscLeotherasRunAwayFromWhirlwindAction*>(action) && !IsTbcRaidAction(action))
        return 0.0f;

    return 1.0f;
}

float SscLeotherasDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsTank(bot) || bot->HasAura(SPELL_INSIDIOUS_WHISPER) || !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    // Inner demons still have to be killed
    if (dynamic_cast<SscLeotherasDestroyInnerDemonAction*>(action))
        return 1.0f;

    return GetLeotherasDemonFormTank(bot) && GetPhase2LeotherasDemon(ai) ? 0.0f : 1.0f;
}

float SscLeotherasMeleeDpsAvoidChaosBlastMultiplier::GetValue(Action* action)
{
    if (raidboss::IsRangedPlayer(bot) || PlayerbotAI::IsTank(bot) || bot->GetAuraCount(SPELL_CHAOS_BLAST) < 5)
        return 1.0f;

    if (!(dynamic_cast<AttackAction*>(action) || IsReachMelee(action) || IsFormationMove(action) || IsReachSpell(action)))
        return 1.0f;

    return GetPhase2LeotherasDemon(ai) ? 0.0f : 1.0f;
}

float SscLeotherasFocusOnInnerDemonMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return 1.0f;

    Unit* demon = GetMyInnerDemon(bot);
    if (!demon || dynamic_cast<SscLeotherasDestroyInnerDemonAction*>(action)) return 1.0f;
    if (dynamic_cast<AttackAction*>(action) && action->GetTarget() != demon) return 0.0f;

    if (dynamic_cast<TankAssistAction*>(action) || dynamic_cast<DpsAssistAction*>(action) ||
        dynamic_cast<CastHealingSpellAction*>(action) || dynamic_cast<CastCureSpellAction*>(action) ||
        dynamic_cast<CurePartyMemberAction*>(action) || dynamic_cast<CastBuffSpellAction*>(action) ||
        dynamic_cast<ResurrectPartyMemberAction*>(action) || dynamic_cast<PartyMemberActionNameSupport*>(action) ||
        IsActionNamed(action, { "bear form", "dire bear form", "tree form", "tree of life", "tbc raid attack priority target" }))
        return 0.0f;

    return 1.0f;
}

float SscLeotherasWaitForDpsMultiplier::GetValue(Action* action)
{
    if (bot->HasAura(SPELL_INSIDIOUS_WHISPER) || !IsDamageAction(action) ||
        dynamic_cast<SscLeotherasDestroyInnerDemonAction*>(action) ||
        dynamic_cast<SscLeotherasAttackMindControlledPlayerAction*>(action))
        return 1.0f;

    if (!FindSscUnit(ai, NPC_LEOTHERAS, 100.0f))
        return 1.0f;

    const SscInstanceState state = GetState(bot);
    Unit* leotherasHuman = GetLeotherasHuman(ai);
    Unit* phase2Demon = GetPhase2LeotherasDemon(ai);
    Unit* phase3Demon = GetPhase3LeotherasDemon(ai);
    Player* demonFormTank = GetLeotherasDemonFormTank(bot);

    if (leotherasHuman && !leotherasHuman->HasAura(SPELL_LEOTHERAS_BANISHED) && !phase3Demon)
    {
        if (!PlayerbotAI::IsTank(bot) && !IsWaitExpired(state.leotherasHumanFormDpsWait, 5))
            return 0.0f;
    }

    // Whirlwind wiped the threat: let the tank pick the human form up again
    // (the Warlock keeps working on the Shadow in the final phase)
    if (leotherasHuman && IsLeotherasWhirlwindAggroReset(bot) && !PlayerbotAI::IsTank(bot) &&
        !(phase3Demon && demonFormTank == bot))
        return 0.0f;

    if (phase2Demon)
    {
        if (demonFormTank == bot || (!demonFormTank && PlayerbotAI::IsTank(bot)))
            return 1.0f;

        if (!IsWaitExpired(state.leotherasDemonFormDpsWait, 12))
            return 0.0f;
    }

    if (phase3Demon)
    {
        if (demonFormTank == bot || PlayerbotAI::IsTank(bot))
            return 1.0f;

        if (!IsWaitExpired(state.leotherasFinalPhaseDpsWait, 8))
            return 0.0f;
    }

    return 1.0f;
}

// Bloodlust/Heroism and DPS cooldowns are saved for the final phase below 15%, where every ability
// is active at once. Bots with an Inner Demon may still use theirs to kill it in time.
float SscLeotherasSaveCooldownsForFinalPhaseMultiplier::GetValue(Action* action)
{
    const bool lust = bot->getClass() == CLASS_SHAMAN && IsLustAction(action);
    if (!lust && !raidboss::IsDpsCooldownAction(bot, action))
        return 1.0f;

    if (!lust && bot->HasAura(SPELL_INSIDIOUS_WHISPER))
        return 1.0f;

    Unit* leotheras = FindSscUnit(ai, NPC_LEOTHERAS, 100.0f);
    if (!leotheras || !leotheras->IsAlive())
        return 1.0f;

    return GetPhase3LeotherasDemon(ai) ? 1.0f : 0.0f;
}

// Fathom-Lord Karathress

float SscKarathressDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsTank(bot) || !Engaged(ai, NPC_KARATHRESS))
        return 1.0f;

    if (bot->GetVictim() && IsActionNamed(action, { "tank assist" }))
        return 0.0f;

    if (IsFormationMove(action) || IsActionNamed(action, { "avoid aoe", "tbc raid taunt", "tbc raid attack priority target" }))
        return 0.0f;

    // A main tank holding unassigned council members (fewer than four tanks) needs taunts and AoE threat
    if (raidboss::IsMainTank(bot) && GetKarathressMainTankUnits(ai).size() > 1)
        return 1.0f;

    if (raidboss::IsTauntAction(bot, action) || raidboss::IsAoeThreatAction(bot, action))
        return 0.0f;

    return 1.0f;
}

float SscKarathressDisableAoeMultiplier::GetValue(Action* action)
{
    if (!raidboss::IsDps(bot))
        return 1.0f;

    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction || spellAction->getThreatType() != ActionThreatType::ACTION_THREAT_AOE)
        return 1.0f;

    return Engaged(ai, NPC_KARATHRESS) ? 0.0f : 1.0f;
}

float SscKarathressControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() == CLASS_HUNTER && IsActionNamed(action, { "misdirection on party tank" }) && Engaged(ai, NPC_KARATHRESS))
        return 0.0f;

    return 1.0f;
}

float SscKarathressWaitForDpsMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* totem = FindSscUnit(ai, NPC_SPITFIRE_TOTEM, 100.0f);
    if (totem && raidboss::IsDps(bot))
    {
        if (dynamic_cast<SscKarathressAssignDpsPriorityAction*>(action))
            return 1.0f;
        if (raidboss::IsAttackTargetAction(action) || IsActionNamed(action, { "tbc raid attack priority target" }))
            return 0.0f;
        // Allow damage to the totem even during the initial council threat wait.
        if (AI_VALUE(ObjectGuid, "current target") == totem->GetObjectGuid())
            return 1.0f;
        if (IsDamageAction(action))
            return 0.0f;
    }

    // The table kill order of "tbc raid" contradicts the one of this strategy
    if (IsActionNamed(action, { "tbc raid attack priority target" }) && Engaged(ai, NPC_KARATHRESS))
        return 0.0f;

    if (!IsDamageAction(action) || !Engaged(ai, NPC_KARATHRESS))
        return 1.0f;

    return IsWaitExpired(GetState(bot).karathressDpsWait, 12) ? 1.0f : 0.0f;
}

float SscKarathressCaribdisTankHealerMaintainPositionMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsHeal(bot) || !(IsFleeAction(action) || dynamic_cast<FollowAction*>(action) || IsFormationMove(action)))
        return 1.0f;

    return GetHealerOfIndex(bot, 0) == bot && Engaged(ai, NPC_CARIBDIS) ? 0.0f : 1.0f;
}

// Morogrim Tidewalker

// Use Bloodlust/Heroism after the first Murloc spawn
float SscMorogrimDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN || !IsLustAction(action) || !Engaged(ai, NPC_MOROGRIM))
        return 1.0f;

    return FindSscUnit(ai, NPC_TIDEWALKER_LURKER) ? 1.0f : 0.0f;
}

float SscMorogrimDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!raidboss::IsMainTank(bot) || !(IsFormationMove(action) || IsActionNamed(action, { "tbc raid leave front" })))
        return 1.0f;

    return Engaged(ai, NPC_MOROGRIM) ? 0.0f : 1.0f;
}

float SscMorogrimMaintainPhase2StackingMultiplier::GetValue(Action* action)
{
    if (!raidboss::IsRangedPlayer(bot) || !(IsFormationMove(action) || IsFleeAction(action)))
        return 1.0f;

    Unit* tidewalker = Engaged(ai, NPC_MOROGRIM);
    return tidewalker && tidewalker->GetHealthPercent() <= 25.0f ? 0.0f : 1.0f;
}

// Lady Vashj <Coilfang Matron>

// Wait until phase 3 to use Bloodlust/Heroism, no other major cooldowns in phase 1
float SscVashjDelayCooldownsMultiplier::GetValue(Action* action)
{
    if (!raidboss::IsDpsCooldownAction(bot, action) || !Engaged(ai, NPC_LADY_VASHJ))
        return 1.0f;

    // Only Bloodlust / Heroism is saved for phase 3. Holding every dps cooldown through phase 1
    // left Vashj at 88% after four minutes; 2-3 minute cooldowns are ready again for phase 3.
    if (bot->getClass() == CLASS_SHAMAN && IsLustAction(action))
        return IsLadyVashjInPhase3(ai) ? 1.0f : 0.0f;

    return 1.0f;
}

float SscVashjMainTankGroupShamanUseGroundingTotemMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN ||
        !IsActionNamed(action, { "windfury totem", "wrath of air totem", "nature resistance totem", "grace of air totem", "tranquil air totem" }))
        return 1.0f;

    return Engaged(ai, NPC_LADY_VASHJ) && IsMainTankInSameSubgroup(bot) ? 0.0f : 1.0f;
}

float SscVashjMaintainPhase1RangedSpreadMultiplier::GetValue(Action* action)
{
    if (!raidboss::IsRangedPlayer(bot) || !(IsFormationMove(action) || IsFleeAction(action)))
        return 1.0f;

    return Engaged(ai, NPC_LADY_VASHJ) && IsLadyVashjInPhase1(ai) ? 0.0f : 1.0f;
}

float SscVashjStaticChargeStayAwayFromGroupMultiplier::GetValue(Action* action)
{
    // The main tank holds the boss in place even with Static Charge (the "tbc raid" debuff run would drag her)
    if (raidboss::IsMainTank(bot))
        return bot->HasAura(SPELL_STATIC_CHARGE) && IsActionNamed(action, { "tbc raid spread" }) && Engaged(ai, NPC_LADY_VASHJ) ? 0.0f : 1.0f;

    if (!bot->HasAura(SPELL_STATIC_CHARGE))
        return 1.0f;

    if (!(IsActionNamed(action, { "move out of enemy contact", "move from group" }) || IsReachMelee(action) ||
          dynamic_cast<FollowAction*>(action) || IsReachSpell(action)))
        return 1.0f;

    return Engaged(ai, NPC_LADY_VASHJ) ? 0.0f : 1.0f;
}

// Bots don't loot the core with the normal looting logic (nor with the "tbc raid" core handling)
float SscVashjDoNotLootTheTaintedCoreMultiplier::GetValue(Action* action)
{
    if (!(dynamic_cast<LootAction*>(action) || IsActionNamed(action, { "loot", "move to loot", "add all loot", "open loot",
          "tbc raid loot core", "tbc raid use core" })))
        return 1.0f;

    return FindSscUnit(ai, NPC_LADY_VASHJ) ? 0.0f : 1.0f;
}

float SscVashjCorePassersPrioritizePositioningMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<SscVashjPassTheTaintedCoreAction*>(action) || dynamic_cast<SscVashjDestroyTaintedCoreAction*>(action))
        return 1.0f;

    if (!Engaged(ai, NPC_LADY_VASHJ) || !IsLadyVashjInPhase2(ai))
        return 1.0f;

    // A bot holding the core only handles the core
    if (HasTaintedCore(bot))
        return 0.0f;

    const std::array<Player*, 5> handlers = GetCoreHandlers(bot);
    if (std::find(handlers.begin(), handlers.end(), bot) == handlers.end() || !raidboss::IsMovementAction(action))
        return 1.0f;

    // First and second passers keep their spots once the looter reached the elemental
    Unit* tainted = FindSscUnit(ai, NPC_TAINTED_ELEMENTAL);
    if (tainted && handlers[0] && raidboss::ExactDist2d(handlers[0], tainted) < 5.0f && (bot == handlers[1] || bot == handlers[2]))
        return 0.0f;

    // The looter itself has to get to the elemental
    if (bot == handlers[0] && dynamic_cast<SscVashjLootTaintedCoreAction*>(action))
        return 1.0f;

    // If any prior handler (including self) recently had the core, block other movement
    return AnyRecentCoreInInventory(bot) ? 0.0f : 1.0f;
}

// Phases 2 and 3 use their own movement and targeting, the standard target selection is disabled
float SscVashjDisableAutomaticTargetingAndMovementMultiplier::GetValue(Action* action)
{
    Unit* vashj = Engaged(ai, NPC_LADY_VASHJ);
    if (!vashj)
        return 1.0f;

    if (IsActionNamed(action, { "avoid aoe" }))
        return 0.0f;

    if (IsLadyVashjInPhase1(ai))
        return 1.0f;

    if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
        IsActionNamed(action, { "tbc raid attack priority target" }))
        return 0.0f;

    Unit* enchanted = FindSscUnit(ai, NPC_ENCHANTED_ELEMENTAL);
    const bool attackingEnchanted = enchanted && AI_VALUE(ObjectGuid, "current target") == enchanted->GetObjectGuid();
    const bool debuffOnAttacker = dynamic_cast<CastMeleeDebuffSpellOnAttackerAction*>(action) || dynamic_cast<CastRangedDebuffSpellOnAttackerAction*>(action);

    if (IsLadyVashjInPhase2(ai))
    {
        if (IsFleeAction(action))
            return 0.0f;

        if (raidboss::ExactDist2d(bot, vashj) < 60.0f && dynamic_cast<FollowAction*>(action))
            return 0.0f;

        if (!PlayerbotAI::IsHeal(bot) && dynamic_cast<CastHealingSpellAction*>(action))
            return 0.0f;

        if (attackingEnchanted && debuffOnAttacker)
            return 0.0f;

        return 1.0f;
    }

    // Phase 3
    if (enchanted || FindSscUnit(ai, NPC_COILFANG_STRIDER) || FindSscUnit(ai, NPC_COILFANG_ELITE))
    {
        if (dynamic_cast<FollowAction*>(action) || IsFleeAction(action))
            return 0.0f;

        if (attackingEnchanted && debuffOnAttacker)
            return 0.0f;
    }
    else if (IsFormationMove(action))
        return 0.0f;

    return 1.0f;
}
