// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/BT/BTMultipliers) and adapted for this core.
#include "playerbot/playerbot.h"
#include "BtMultipliers.h"
#include "BtActions.h"
#include "BtHelpers.h"
#include "BtSupremusRules.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/actions/ChooseTargetActions.h"
#include "playerbot/strategy/actions/GenericSpellActions.h"
#include "playerbot/strategy/actions/ReachTargetActions.h"

using namespace ai;
using namespace ai::bt;

namespace
{
    bool IsRangedDps(Player* bot) { return raidboss::IsDps(bot) && raidboss::IsRangedPlayer(bot); }

    bool IsMovement(Action* action) { return dynamic_cast<MovementAction*>(action) != nullptr; }

    // Formation moves and flee/follow are replaced by the encounter positions of the port
    bool IsUncontrolledMove(Action* action)
    {
        return IsFormationMoveAction(action) || IsFleeOrFollowAction(action);
    }

    bool IsCouncilTank(Player* bot)
    {
        return raidboss::IsMainTank(bot) || raidboss::IsAssistTank(bot, 0) || raidboss::IsAssistTank(bot, 1) ||
               GetZerevorMageTank(bot) == bot;
    }

    bool IsDpsWaitBlocked(Action* action, BtState timer, uint32 instanceId, int64 waitSeconds)
    {
        const int64 elapsed = GetTimerElapsed(timer, instanceId);
        return (elapsed < 0 || elapsed < waitSeconds) && IsAttackOrSpellAction(action);
    }
}

// General

// Illidan is handled separately
float BlackTempleDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (!raidboss::IsDpsCooldownAction(bot, action))
        return 1.0f;

    for (uint32 entry : { NPC_GATHIOS, NPC_MOTHER_SHAHRAZ, NPC_ESSENCE_OF_SUFFERING, NPC_GURTOGG_BLOODBOIL,
                          NPC_TERON_GOREFIEND, NPC_SUPREMUS, NPC_NAJENTUS })
    {
        if (Unit* boss = FindBoss(ai, entry))
            return boss->GetHealthPercent() > raidboss::BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
    }

    return 1.0f;
}

// High Warlord Naj'entus

float HighWarlordNajentusDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!action || !FindBoss(ai, NPC_NAJENTUS))
        return 1.0f;

    if (IsFormationMoveAction(action))
        return 0.0f;

    // Melee keep their flank group, "set behind" would pull them back into one pile
    if (action->getName() == "set behind" && raidboss::IsMeleePlayer(bot) && !PlayerbotAI::IsTank(bot))
        return 0.0f;

    return 1.0f;
}

// Supremus

// The Gaze target only kites: other movement is blocked, and while the boss is close nothing else may delay the
// next kite step (a cast would let him walk into his 29 yard melee reach)
float SupremusFocusOnAvoidanceInPhase2Multiplier::GetValue(Action* action)
{
    if (!action || dynamic_cast<SupremusKiteBossAction*>(action) || dynamic_cast<SupremusMoveAwayFromVolcanosAction*>(action))
        return 1.0f;

    Unit* supremus = FindBoss(ai, NPC_SUPREMUS);
    if (!supremus || supremus->GetVictim() != bot || !supremus->HasAura(SPELL_SNARE_SELF))
        return 1.0f;

    if (IsMovement(action))
        return 0.0f;

    const bool close = bt::supremus::NeedsToKite(raidboss::ExactDist2d(bot, supremus), GetSupremusMeleeReach(bot, supremus));
    return close ? 0.0f : 1.0f;
}

// Threat is reset when the tank phase starts again: only tanks and the misdirecting hunters act for a few seconds
float SupremusWaitForTanksAfterThreatResetMultiplier::GetValue(Action* action)
{
    if (!IsAttackOrSpellAction(action) || PlayerbotAI::IsTank(bot) || dynamic_cast<SupremusMisdirectBossToMainTankAction*>(action))
        return 1.0f;

    Unit* supremus = FindBoss(ai, NPC_SUPREMUS);
    if (!supremus)
        return 1.0f;

    const int64 elapsed = GetTimerElapsed(BtState::SupremusPhaseTimer, supremus->GetInstanceId());
    return bt::supremus::IsDpsWaitAfterThreatReset(elapsed) ? 0.0f : 1.0f;
}

float SupremusHitboxIsBuggedMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_ROGUE || !action || action->getName() != "killing spree")
        return 1.0f;

    return FindBoss(ai, NPC_SUPREMUS) ? 0.0f : 1.0f;
}

// Shade of Akama

float ShadeOfAkamaMeleeFocusChannelersMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "tbc raid attack priority target" || !raidboss::IsDps(bot) || !raidboss::IsMeleePlayer(bot))
        return 1.0f;

    Unit* channeler = FindNearest(bot, NPC_ASHTONGUE_CHANNELER, 30.0f);
    return channeler && !channeler->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE) ? 0.0f : 1.0f;
}

// Teron Gorefiend

float TeronGorefiendControlMovementMultiplier::GetValue(Action* action)
{
    if (!IsUncontrolledMove(action) && !(raidboss::IsRangedPlayer(bot) && IsReachAction(action)))
        return 1.0f;

    return FindBoss(ai, NPC_TERON_GOREFIEND) ? 0.0f : 1.0f;
}

float TeronGorefiendMarkedBotOnlyMoveToDieMultiplier::GetValue(Action* action)
{
    SpellAuraHolder* holder = bot->GetSpellAuraHolder(SPELL_SHADOW_OF_DEATH);
    if (!holder || holder->GetAuraDuration() >= 15000)
        return 1.0f;

    return dynamic_cast<TeronGorefiendMoveToCornerToDieAction*>(action) ? 1.0f : 0.0f;
}

float TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_SPIRITUAL_VENGEANCE))
        return 1.0f;

    return dynamic_cast<TeronGorefiendControlAndDestroyShadowyConstructsAction*>(action) ? 1.0f : 0.0f;
}

// The Shadowy Constructs only take damage from the spirits: don't waste time or AoE on them
float TeronGorefiendDisableAttackingConstructsMultiplier::GetValue(Action* action)
{
    const bool tankAssist = dynamic_cast<TankAssistAction*>(action) && bot->GetVictim();
    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    const bool aoeSpell = IsRangedDps(bot) && spellAction && !dynamic_cast<CastHealingSpellAction*>(action) &&
                          spellAction->getThreatType() == ActionThreatType::ACTION_THREAT_AOE;

    if (!tankAssist && !aoeSpell)
        return 1.0f;

    return FindBoss(ai, NPC_TERON_GOREFIEND) ? 0.0f : 1.0f;
}

// Gurtogg Bloodboil

float GurtoggBloodboilControlMovementMultiplier::GetValue(Action* action)
{
    const bool felRageMove = bot->HasAura(SPELL_PLAYER_FEL_RAGE) && IsMovement(action) && !dynamic_cast<AttackAction*>(action);
    if (!IsUncontrolledMove(action) && !felRageMove)
        return 1.0f;

    return FindBoss(ai, NPC_GURTOGG_BLOODBOIL) ? 0.0f : 1.0f;
}

// Reliquary of Souls

// Aura of Suffering: no healing (Power Word: Shield is an absorb, not a heal, and stays allowed)
float ReliquaryOfSoulsDontWasteHealingMultiplier::GetValue(Action* action)
{
    if (!action || action->getName().find("power word: shield") != std::string::npos)
        return 1.0f;

    if (!dynamic_cast<CastHealingSpellAction*>(action) && action->getName() != "tree of life")
        return 1.0f;

    return FindBoss(ai, NPC_ESSENCE_OF_SUFFERING) ? 0.0f : 1.0f;
}

// Mother Shahraz

float MotherShahrazControlMovementMultiplier::GetValue(Action* action)
{
    if (!IsUncontrolledMove(action))
        return 1.0f;

    return FindBoss(ai, NPC_MOTHER_SHAHRAZ) ? 0.0f : 1.0f;
}

float MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_FATAL_ATTRACTION) || dynamic_cast<MotherShahrazRunAwayToBreakFatalAttractionAction*>(action))
        return 1.0f;

    return FindBoss(ai, NPC_MOTHER_SHAHRAZ) ? 0.0f : 1.0f;
}

// Illidari Council

// Each tank has its own council member, the default tank actions would make them swap
float IllidariCouncilDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (ai->GetState() == BotState::BOT_STATE_NON_COMBAT || !PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) && !raidboss::IsTauntAction(bot, action) && !raidboss::IsAoeThreatAction(bot, action))
        return 1.0f;

    return FindBoss(ai, NPC_GATHIOS) ? 0.0f : 1.0f;
}

float IllidariCouncilControlMovementMultiplier::GetValue(Action* action)
{
    if (!IsMovement(action) && !IsUncontrolledMove(action))
        return 1.0f;

    if (!FindBoss(ai, NPC_ZEREVOR))
        return 1.0f;

    if (IsUncontrolledMove(action))
        return 0.0f;

    // The healer of the mage tank only moves between its two positions
    if (GetZerevorHealer(bot) == bot && PlayerbotAI::IsHeal(bot) && !dynamic_cast<IllidariCouncilPositionMageTankHealerAction*>(action))
        return 0.0f;

    // The tanks keep their council member in place
    if (IsCouncilTank(bot) && action->getName() == "tbc raid leave hazard")
        return 0.0f;

    return 1.0f;
}

float IllidariCouncilControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER || !action || action->getName() != "misdirection on party tank")
        return 1.0f;

    return FindBoss(ai, NPC_ZEREVOR) ? 0.0f : 1.0f;
}

float IllidariCouncilDisableIceBlockMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_MAGE || !action || action->getName() != "ice block")
        return 1.0f;

    return FindBoss(ai, NPC_ZEREVOR) && GetZerevorMageTank(bot) == bot ? 0.0f : 1.0f;
}

// Hunters don't use Arcane Shot on Zerevor (kept from mod-playerbots)
float IllidariCouncilDisableArcaneShotOnZerevorMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "arcane shot")
        return 1.0f;

    Unit* zerevor = FindBoss(ai, NPC_ZEREVOR);
    if (!zerevor)
        return 1.0f;

    return AI_VALUE(ObjectGuid, "current target") == zerevor->GetObjectGuid() ? 0.0f : 1.0f;
}

float IllidariCouncilWaitForDpsMultiplier::GetValue(Action* action)
{
    if (IsCouncilTank(bot) || dynamic_cast<IllidariCouncilMisdirectBossesToTanksAction*>(action))
        return 1.0f;

    Unit* gathios = FindBoss(ai, NPC_GATHIOS);
    if (!gathios)
        return 1.0f;

    const int64 elapsed = GetTimerElapsed(BtState::CouncilDpsTimer, gathios->GetInstanceId());
    if (elapsed < 0 || elapsed >= 5)
        return 1.0f;

    return IsAttackOrSpellAction(action) ? 0.0f : 1.0f;
}

// Illidan Stormrage <The Betrayer>

// Cooldowns from 95% to 62% are saved for later, Bloodlust / Heroism for the end
float IllidanStormrageDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (!raidboss::IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan || illidan->GetHealthPercent() <= 62.0f)
        return 1.0f;

    if (bot->getClass() == CLASS_SHAMAN && (action->getName() == "heroism" || action->getName() == "bloodlust"))
        return 0.0f;

    return illidan->GetHealthPercent() > 95.0f ? 1.0f : 0.0f;
}

float IllidanStormrageControlTankActionsMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsTank(bot) || !action)
        return 1.0f;

    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan || IsIllidanDefeated(illidan) || GetIllidanPhase(illidan) != 2)
        return 1.0f;

    if (raidboss::IsMainTank(bot))
    {
        if (IsMovement(action) && !dynamic_cast<IllidanStormragePositionAboveGrateAction*>(action))
            return 0.0f;

        if (dynamic_cast<CastMeleeSpellAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;
    }
    else if (raidboss::IsAssistTank(bot, 0) || raidboss::IsAssistTank(bot, 1))
    {
        if (IsMovement(action) && !dynamic_cast<IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction*>(action))
            return 0.0f;

        if (dynamic_cast<CastHealingSpellAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float IllidanStormrageDisableDefaultTargetingMultiplier::GetValue(Action* action)
{
    if (!action || ai->GetState() == BotState::BOT_STATE_NON_COMBAT)
        return 1.0f;

    const bool tankAssist = dynamic_cast<TankAssistAction*>(action) != nullptr;
    const bool dpsAssist = dynamic_cast<DpsAssistAction*>(action) != nullptr || action->getName() == "dps aoe";
    const bool priorityTarget = action->getName() == "tbc raid attack priority target";
    const bool attackerDebuff = dynamic_cast<CastMeleeDebuffSpellOnAttackerAction*>(action) ||
                                dynamic_cast<CastRangedDebuffSpellOnAttackerAction*>(action);
    if (!tankAssist && !dpsAssist && !priorityTarget && !attackerDebuff)
        return 1.0f;

    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan || IsIllidanDefeated(illidan))
        return 1.0f;

    // Target selection is done by "illidan stormrage dps prioritize adds"
    if (tankAssist || priorityTarget)
        return 0.0f;

    const int phase = GetIllidanPhase(illidan);
    if (dpsAssist && (phase == 4 || IsRangedDps(bot)))
        return 0.0f;

    // No debuffs (Serpent Sting, Corruption, ...) on the short lived Shadow Demons and Shadowfiends
    if (attackerDebuff)
    {
        const ObjectGuid target = AI_VALUE(ObjectGuid, "current target");
        Unit* shadowDemon = FindNearest(bot, NPC_SHADOW_DEMON, 40.0f);
        Unit* shadowfiend = FindNearest(bot, NPC_PARASITIC_SHADOWFIEND, 40.0f);
        if ((shadowDemon && target == shadowDemon->GetObjectGuid()) || (shadowfiend && target == shadowfiend->GetObjectGuid()))
            return 0.0f;
    }

    return 1.0f;
}

float IllidanStormrageControlNonTankMovementMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot) || !action)
        return 1.0f;

    const bool uncontrolled = IsUncontrolledMove(action);
    const bool phase2Move = action->getName() == "set behind" || action->getName() == "killing spree" || IsReachAction(action);
    if (!uncontrolled && !phase2Move)
        return 1.0f;

    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan || IsIllidanDefeated(illidan))
        return 1.0f;

    if (uncontrolled)
        return 0.0f;

    const int phase = GetIllidanPhase(illidan);
    if (phase == 2)
        return 0.0f;

    if (phase == 4 && PlayerbotAI::IsHeal(bot) && dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float IllidanStormrageUseEarthbindTotemMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN || !action)
        return 1.0f;

    const std::string& name = action->getName();
    if (name != "strength of earth totem" && name != "stoneskin totem" && name != "stoneclaw totem" && name != "tremor totem")
        return 1.0f;

    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    return illidan && GetIllidanPhase(illidan) != 2 ? 0.0f : 1.0f;
}

// Give the tanks time to build threat after every phase change
float IllidanStormrageWaitForDpsMultiplier::GetValue(Action* action)
{
    if (!IsAttackOrSpellAction(action) || dynamic_cast<IllidanStormrageMisdirectToTankAction*>(action))
        return 1.0f;

    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan)
        return 1.0f;

    const uint32 instanceId = illidan->GetInstanceId();
    const int phase = GetIllidanPhase(illidan);

    if ((phase == 1 || phase == 3 || phase == 5) && !raidboss::IsMainTank(bot) &&
        IsDpsWaitBlocked(action, BtState::IllidanBossDpsTimer, instanceId, 3))
    {
        return 0.0f;
    }

    if (phase == 4 && GetIllidanWarlockTank(bot) != bot && IsDpsWaitBlocked(action, BtState::IllidanBossDpsTimer, instanceId, 8))
        return 0.0f;

    if (!raidboss::IsAssistTank(bot, 0) && !raidboss::IsAssistTank(bot, 1) && FindBoss(ai, NPC_FLAME_OF_AZZINOTH) &&
        IsDpsWaitBlocked(action, BtState::IllidanFlameDpsTimer, instanceId, 6))
    {
        return 0.0f;
    }

    return 1.0f;
}
