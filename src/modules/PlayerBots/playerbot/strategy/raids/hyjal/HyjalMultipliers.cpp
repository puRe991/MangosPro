// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/Hyjal/HyjalMultipliers) and adapted for this core.
// The actions are recognized by name (and by the Hyjal action classes), see the hyjal::Is...Action helpers.
#include "playerbot/playerbot.h"
#include "HyjalMultipliers.h"
#include "HyjalActions.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;
using namespace ai::hyjal;
using namespace ai::raidboss;

namespace
{
    bool IsNamed(Action* action, std::initializer_list<const char*> names)
    {
        for (const char* name : names)
        {
            if (action->getName() == name)
                return true;
        }

        return false;
    }
}

// The multipliers are only added to the combat engine. Avoidance that holds a bot out of attack
// range therefore never sends it to the non-combat engine during a boss fight.

float HyjalEncounterMultiplier::GetValue(Action* action)
{
    if (!action || !IsHyjalEncounterInProgress(bot))
        return 1.0f;

    return GetValueInEncounter(action);
}

float HyjalDelayDpsCooldownsMultiplier::GetValue(Action* action)
{
    if (!action || bot->GetMapId() != HYJAL_MAP_ID)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    // Bloodlust / Heroism would be blown on cooldown during the trash waves
    Unit* boss = GetAnyBoss(ai);
    if (!boss)
        return bot->getClass() == CLASS_SHAMAN && IsNamed(action, { "bloodlust", "heroism" }) ? 0.0f : 1.0f;

    // Only the first percent of the pull (threat): until 95% this held every cooldown for a minute or more
    return boss->GetHealthPercent() > 99.0f ? 0.0f : 1.0f;
}

// Spreading / leaving the boss front of the "tbc raid" table fights the boss positioning here
float HyjalDisableDisperseAndTankFaceMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsFormationMoveAction(action))
        return 1.0f;

    if (HasProtectionOfElune(bot))
        return 1.0f;

    return GetBoss(ai, NPC_ARCHIMONDE) || GetBoss(ai, NPC_KAZROGAL) || GetBoss(ai, NPC_RAGE_WINTERCHILL) ? 0.0f : 1.0f;
}

// Rage Winterchill

float RageWinterchillMeleeControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsMeleePlayer(bot))
        return 1.0f;

    const bool isAvoidAoe = IsAvoidAoeAction(action);
    if (!isAvoidAoe && !IsReachTargetAction(action) && !IsSetBehindAction(action))
        return 1.0f;

    Unit* winterchill = GetBoss(ai, NPC_RAGE_WINTERCHILL);
    if (!winterchill)
        return 1.0f;

    if (!IsNearDeathAndDecay(ai, DEATH_AND_DECAY_CONTROL_RADIUS))
        return 1.0f;

    if (isAvoidAoe)
        return 0.0f;

    if (winterchill->GetVictim() == bot)
        return 1.0f;

    return IsMainTank(bot) ? 1.0f : 0.0f;
}

float RageWinterchillRangedControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsRangedPlayer(bot))
        return 1.0f;

    if (!IsMovementAction(action))
        return 1.0f;

    if (dynamic_cast<RageWinterchillRangedGetOutOfDeathAndDecayAction*>(action))
        return 1.0f;

    if (IsAttackAction(action))
        return 1.0f;

    if (!GetBoss(ai, NPC_RAGE_WINTERCHILL))
        return 1.0f;

    if (IsAvoidAoeAction(action))
        return 0.0f;

    return IsNearDeathAndDecay(ai, DEATH_AND_DECAY_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// Anetheron

float AnetheronDisableAssistTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsAssistTargetingAction(action))
        return 1.0f;

    if (action->getName() == "tank assist" && IsInfernalTank(bot))
        return 1.0f;

    return GetBoss(ai, NPC_ANETHERON) ? 0.0f : 1.0f;
}

// Keep the other tanks from grabbing an Infernal with Consecration, Thunder Clap, ...
float AnetheronAvoidAccidentalInfernalAggroMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsAoeThreatAction(bot, action))
        return 1.0f;

    const float holdTankAoeRadius = 20.0f;
    Unit* infernal = GetNearestInfernal(ai);
    if (!infernal || ExactDist2d(bot, infernal) > holdTankAoeRadius)
        return 1.0f;

    return IsInfernalTank(bot) ? 1.0f : 0.0f;
}

float AnetheronInfernalTargetRunToPositionMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsMovementAction(action) && !IsReachTargetAction(action))
        return 1.0f;

    if (dynamic_cast<AnetheronBringInfernalToInfernalTankAction*>(action))
        return 1.0f;

    Unit* anetheron = GetBoss(ai, NPC_ANETHERON);
    if (!anetheron || anetheron->GetVictim() == bot)
        return 1.0f;

    if (IsInfernalTank(bot))
        return 1.0f;

    return GetInfernoTarget(anetheron) == bot || GetInfernalTargetingBot(ai) ? 0.0f : 1.0f;
}

float AnetheronControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    const bool isTankAvoidAoe = PlayerbotAI::IsTank(bot) && IsAvoidAoeAction(action);
    if (!isTankAvoidAoe && !IsFormationMoveAction(action))
        return 1.0f;

    return GetBoss(ai, NPC_ANETHERON) ? 0.0f : 1.0f;
}

float AnetheronControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!IsNamed(action, { "misdirection on party tank" }))
        return 1.0f;

    return GetBoss(ai, NPC_ANETHERON) ? 0.0f : 1.0f;
}

// Kaz'rogal

float KazrogalControlLowManaMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsKazrogalManaUser(bot) || bot->getClass() == CLASS_HUNTER)
        return 1.0f;

    if (!IsMovementAction(action) && !IsReachTargetAction(action))
        return 1.0f;

    if (IsAttackAction(action))
        return 1.0f;

    if (dynamic_cast<KazrogalMoveAwayFromGroupAction*>(action))
        return 1.0f;

    Unit* kazrogal = GetBoss(ai, NPC_KAZROGAL);
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return 1.0f;

    return AI_VALUE(bool, "kaz'rogal below mana threshold") ? 0.0f : 1.0f;
}

float KazrogalKeepAspectOfTheViperActiveMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!IsNamed(action, { "aspect of the hawk", "aspect of the wild", "aspect of the dragonhawk", "aspect of the cheetah",
                           "aspect of the pack", "aspect of the monkey", "aspect of the beast" }))
        return 1.0f;

    if (!GetBoss(ai, NPC_KAZROGAL))
        return 1.0f;

    return bot->GetPower(POWER_MANA) <= MARK_REJOIN_MANA ? 0.0f : 1.0f;
}

// Azgalor

float AzgalorDisableAutoTargetingAndPositioningMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsAssistTargetingAction(action) && !IsFormationMoveAction(action) && !IsAvoidAoeAction(action))
        return 1.0f;

    return GetBoss(ai, NPC_AZGALOR) ? 0.0f : 1.0f;
}

float AzgalorDoomedBotPrioritizePositioningMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDoomed(bot))
        return 1.0f;

    if (!IsMovementAction(action))
        return 1.0f;

    if (IsAttackAction(action))
        return 1.0f;

    return dynamic_cast<AzgalorMoveToDoomguardTankAction*>(action) ? 1.0f : 0.0f;
}

float AzgalorMeleeDpsControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsMeleePlayer(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!IsMovementAction(action) && !IsReachTargetAction(action))
        return 1.0f;

    if (IsDoomed(bot))
        return 1.0f;

    if (dynamic_cast<AzgalorMeleeManeuverThroughFireAction*>(action) || dynamic_cast<AzgalorDetermineDpsPriorityAction*>(action))
        return 1.0f;

    if (!GetBoss(ai, NPC_AZGALOR))
        return 1.0f;

    return IsNearRainOfFire(ai, RAIN_OF_FIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

float AzgalorRangedControlAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsRangedPlayer(bot))
        return 1.0f;

    if (!IsMovementAction(action))
        return 1.0f;

    if (IsDoomed(bot))
        return 1.0f;

    if (dynamic_cast<AzgalorRangedGetOutOfRainOfFireAction*>(action) || dynamic_cast<AzgalorDetermineDpsPriorityAction*>(action))
        return 1.0f;

    if (!GetBoss(ai, NPC_AZGALOR))
        return 1.0f;

    return IsNearRainOfFire(ai, RAIN_OF_FIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// Archimonde

float ArchimondeControlDoomfireAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsMovementAction(action) && !IsReachTargetAction(action))
        return 1.0f;

    if (dynamic_cast<ArchimondeAvoidDoomfireAction*>(action) || dynamic_cast<ArchimondeKeepAirBurstAwayFromTankAction*>(action))
        return 1.0f;

    if (!GetBoss(ai, NPC_ARCHIMONDE))
        return 1.0f;

    if (IsAvoidAoeAction(action))
        return 0.0f;

    if (HasProtectionOfElune(bot))
        return 1.0f;

    return IsNearDoomfire(ai, DOOMFIRE_CONTROL_RADIUS) ? 0.0f : 1.0f;
}

// The shaman keeps Tremor Totem against Fear instead of the other earth totems
float ArchimondeSetTremorTotemMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!IsNamed(action, { "strength of earth totem", "stoneskin totem", "stoneclaw totem", "earthbind totem" }))
        return 1.0f;

    Unit* archimonde = GetBoss(ai, NPC_ARCHIMONDE);
    if (!archimonde || !archimonde->IsInCombat())
        return 1.0f;

    return !HasProtectionOfElune(bot) ? 0.0f : 1.0f;
}
