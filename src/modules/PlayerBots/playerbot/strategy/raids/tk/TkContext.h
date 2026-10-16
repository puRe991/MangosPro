#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "TkStrategy.h"
#include "TkActions.h"
#include "TkTriggers.h"
#include "TkValues.h"

// Tempest Keep: The Eye (map 550) - strategy "boss tempest keep"
namespace ai
{
    class TkStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        TkStrategyContext()
        {
            creators["boss tempest keep"] = [](PlayerbotAI* ai) { return new TkStrategy(ai); };
        }
    };

    class TkActionContext : public NamedObjectContext<Action>
    {
    public:
        TkActionContext()
        {
            creators["tempest keep reset encounter states"] = [](PlayerbotAI* ai) { return new TkResetEncounterStatesAction(ai); };
            creators["tempest keep clear stale falling flag"] = [](PlayerbotAI* ai) { return new TkClearStaleFallingFlagAction(ai); };
            creators["crimson hand centurion cast polymorph"] = [](PlayerbotAI* ai) { return new TkCrimsonHandCenturionCastPolymorphAction(ai); };
            creators["al'ar misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new TkAlarMisdirectBossToMainTankAction(ai); };
            creators["al'ar boss tanks move between platforms"] = [](PlayerbotAI* ai) { return new TkAlarBossTanksMoveBetweenPlatformsAction(ai); };
            creators["al'ar melee dps move between platforms"] = [](PlayerbotAI* ai) { return new TkAlarMeleeDpsMoveBetweenPlatformsAction(ai); };
            creators["al'ar ranged and ember tank move under platforms"] = [](PlayerbotAI* ai) { return new TkAlarRangedAndEmberTankMoveUnderPlatformsAction(ai); };
            creators["al'ar assist tanks pick up embers"] = [](PlayerbotAI* ai) { return new TkAlarAssistTanksPickUpEmbersAction(ai); };
            creators["al'ar assign non-tank target"] = [](PlayerbotAI* ai) { return new TkAlarAssignNonTankTargetAction(ai); };
            creators["al'ar jump from platform"] = [](PlayerbotAI* ai) { return new TkAlarJumpFromPlatformAction(ai); };
            creators["al'ar move away from rebirth"] = [](PlayerbotAI* ai) { return new TkAlarMoveAwayFromRebirthAction(ai); };
            creators["al'ar swap tanks on boss"] = [](PlayerbotAI* ai) { return new TkAlarSwapTanksOnBossAction(ai); };
            creators["al'ar avoid flame patches and dive bombs"] = [](PlayerbotAI* ai) { return new TkAlarAvoidFlamePatchesAndDiveBombsAction(ai); };
            creators["al'ar manage phase tracker"] = [](PlayerbotAI* ai) { return new TkAlarManagePhaseTrackerAction(ai); };
            creators["void reaver tanks position boss"] = [](PlayerbotAI* ai) { return new TkVoidReaverTanksPositionBossAction(ai); };
            creators["void reaver use aggro dump ability"] = [](PlayerbotAI* ai) { return new TkVoidReaverUseAggroDumpAbilityAction(ai); };
            creators["void reaver ranged back off and spread"] = [](PlayerbotAI* ai) { return new TkVoidReaverRangedBackOffAndSpreadAction(ai); };
            creators["void reaver avoid arcane orb"] = [](PlayerbotAI* ai) { return new TkVoidReaverAvoidArcaneOrbAction(ai); };
            creators["high astromancer solarian main tank pick up boss"] = [](PlayerbotAI* ai) { return new TkSolarianMainTankPickUpBossAction(ai); };
            creators["high astromancer solarian move away from group"] = [](PlayerbotAI* ai) { return new TkSolarianMoveAwayFromGroupAction(ai); };
            creators["high astromancer solarian target solarium priests"] = [](PlayerbotAI* ai) { return new TkSolarianTargetSolariumPriestsAction(ai); };
            creators["kael'thas sunstrider kite thaladred"] = [](PlayerbotAI* ai) { return new TkKaelthasKiteThaladredAction(ai); };
            creators["kael'thas sunstrider misdirect advisors to tanks"] = [](PlayerbotAI* ai) { return new TkKaelthasMisdirectAdvisorsToTanksAction(ai); };
            creators["kael'thas sunstrider melee tanks position advisors"] = [](PlayerbotAI* ai) { return new TkKaelthasMeleeTanksPositionAdvisorsAction(ai); };
            creators["kael'thas sunstrider warlock tank position capernian"] = [](PlayerbotAI* ai) { return new TkKaelthasWarlockTankPositionCapernianAction(ai); };
            creators["kael'thas sunstrider spread and move away from capernian"] = [](PlayerbotAI* ai) { return new TkKaelthasSpreadAndMoveAwayFromCapernianAction(ai); };
            creators["kael'thas sunstrider handle advisor roles in phase 3"] = [](PlayerbotAI* ai) { return new TkKaelthasHandleAdvisorRolesInPhase3Action(ai); };
            creators["kael'thas sunstrider assign advisor dps priority"] = [](PlayerbotAI* ai) { return new TkKaelthasAssignAdvisorDpsPriorityAction(ai); };
            creators["kael'thas sunstrider manage advisor dps timer"] = [](PlayerbotAI* ai) { return new TkKaelthasManageAdvisorDpsTimerAction(ai); };
            creators["kael'thas sunstrider assign legendary weapon dps priority"] = [](PlayerbotAI* ai) { return new TkKaelthasAssignLegendaryWeaponDpsPriorityAction(ai); };
            creators["kael'thas sunstrider move devastation away"] = [](PlayerbotAI* ai) { return new TkKaelthasMoveDevastationAwayAction(ai); };
            creators["kael'thas sunstrider loot legendary weapons"] = [](PlayerbotAI* ai) { return new TkKaelthasLootLegendaryWeaponsAction(ai); };
            creators["kael'thas sunstrider use legendary weapons"] = [](PlayerbotAI* ai) { return new TkKaelthasUseLegendaryWeaponsAction(ai); };
            creators["kael'thas sunstrider reequip gear"] = [](PlayerbotAI* ai) { return new TkKaelthasReequipGearAction(ai); };
            creators["kael'thas sunstrider tanks position boss"] = [](PlayerbotAI* ai) { return new TkKaelthasTanksPositionBossAction(ai); };
            creators["kael'thas sunstrider avoid flame strike"] = [](PlayerbotAI* ai) { return new TkKaelthasAvoidFlameStrikeAction(ai); };
            creators["kael'thas sunstrider assign final phase target"] = [](PlayerbotAI* ai) { return new TkKaelthasAssignFinalPhaseTargetAction(ai); };
            creators["kael'thas sunstrider break mind control"] = [](PlayerbotAI* ai) { return new TkKaelthasBreakMindControlAction(ai); };
            creators["kael'thas sunstrider spread out in midair"] = [](PlayerbotAI* ai) { return new TkKaelthasSpreadOutInMidairAction(ai); };
        }
    };

    class TkTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        TkTriggerContext()
        {
            creators["al'ar pulling boss"] = [](PlayerbotAI* ai) { return new TkAlarPullingBossTrigger(ai); };
            creators["al'ar flies between platforms"] = [](PlayerbotAI* ai) { return new TkAlarFliesBetweenPlatformsTrigger(ai); };
            creators["al'ar embers explode upon death"] = [](PlayerbotAI* ai) { return new TkAlarEmbersExplodeUponDeathTrigger(ai); };
            creators["al'ar should assign non-tank target"] = [](PlayerbotAI* ai) { return new TkAlarShouldAssignNonTankTargetTrigger(ai); };
            creators["al'ar incoming flame quills"] = [](PlayerbotAI* ai) { return new TkAlarIncomingFlameQuillsTrigger(ai); };
            creators["al'ar rising from the ashes"] = [](PlayerbotAI* ai) { return new TkAlarRisingFromTheAshesTrigger(ai); };
            creators["al'ar in phase 2"] = [](PlayerbotAI* ai) { return new TkAlarInPhase2Trigger(ai); };
            creators["al'ar should manage phase tracker"] = [](PlayerbotAI* ai) { return new TkAlarShouldManagePhaseTrackerTrigger(ai); };
            creators["void reaver should be tanked"] = [](PlayerbotAI* ai) { return new TkVoidReaverShouldBeTankedTrigger(ai); };
            creators["void reaver knock away pulls aggro to non-tanks"] = [](PlayerbotAI* ai) { return new TkVoidReaverKnockAwayPullsAggroToNonTanksTrigger(ai); };
            creators["void reaver ranged should stand back"] = [](PlayerbotAI* ai) { return new TkVoidReaverRangedShouldStandBackTrigger(ai); };
            creators["void reaver arcane orb is incoming"] = [](PlayerbotAI* ai) { return new TkVoidReaverArcaneOrbIsIncomingTrigger(ai); };
            creators["high astromancer solarian should be tanked"] = [](PlayerbotAI* ai) { return new TkSolarianShouldBeTankedTrigger(ai); };
            creators["high astromancer solarian wrath of the astromancer"] = [](PlayerbotAI* ai) { return new TkSolarianWrathOfTheAstromancerTrigger(ai); };
            creators["high astromancer solarian solarium priests spawned"] = [](PlayerbotAI* ai) { return new TkSolarianSolariumPriestsSpawnedTrigger(ai); };
            creators["kael'thas sunstrider chased by thaladred"] = [](PlayerbotAI* ai) { return new TkKaelthasChasedByThaladredTrigger(ai); };
            creators["kael'thas sunstrider pulling tankable advisors"] = [](PlayerbotAI* ai) { return new TkKaelthasPullingTankableAdvisorsTrigger(ai); };
            creators["kael'thas sunstrider sanguinar or telonicus should be tanked"] = [](PlayerbotAI* ai) { return new TkKaelthasSanguinarOrTelonicusShouldBeTankedTrigger(ai); };
            creators["kael'thas sunstrider capernian should be tanked by warlock"] = [](PlayerbotAI* ai) { return new TkKaelthasCapernianShouldBeTankedByWarlockTrigger(ai); };
            creators["kael'thas sunstrider should stand back from capernian"] = [](PlayerbotAI* ai) { return new TkKaelthasShouldStandBackFromCapernianTrigger(ai); };
            creators["kael'thas sunstrider should hold phase 3 positions"] = [](PlayerbotAI* ai) { return new TkKaelthasShouldHoldPhase3PositionsTrigger(ai); };
            creators["kael'thas sunstrider determining advisor kill order"] = [](PlayerbotAI* ai) { return new TkKaelthasDeterminingAdvisorKillOrderTrigger(ai); };
            creators["kael'thas sunstrider should manage advisor dps timer"] = [](PlayerbotAI* ai) { return new TkKaelthasShouldManageAdvisorDpsTimerTrigger(ai); };
            creators["kael'thas sunstrider legendary weapons are alive"] = [](PlayerbotAI* ai) { return new TkKaelthasLegendaryWeaponsAreAliveTrigger(ai); };
            creators["kael'thas sunstrider legendary axe casts whirlwind"] = [](PlayerbotAI* ai) { return new TkKaelthasLegendaryAxeCastsWhirlwindTrigger(ai); };
            creators["kael'thas sunstrider legendary weapons are dead"] = [](PlayerbotAI* ai) { return new TkKaelthasLegendaryWeaponsAreDeadTrigger(ai); };
            creators["kael'thas sunstrider legendary weapons are equipped"] = [](PlayerbotAI* ai) { return new TkKaelthasLegendaryWeaponsAreEquippedTrigger(ai); };
            creators["kael'thas sunstrider has entered the fight"] = [](PlayerbotAI* ai) { return new TkKaelthasHasEnteredTheFightTrigger(ai); };
            creators["kael'thas sunstrider should assign final phase target"] = [](PlayerbotAI* ai) { return new TkKaelthasShouldAssignFinalPhaseTargetTrigger(ai); };
            creators["kael'thas sunstrider raid member is mind controlled"] = [](PlayerbotAI* ai) { return new TkKaelthasRaidMemberIsMindControlledTrigger(ai); };
            creators["kael'thas sunstrider in gravity lapse phase"] = [](PlayerbotAI* ai) { return new TkKaelthasInGravityLapsePhaseTrigger(ai); };
            creators["tempest keep no encounter in progress"] = [](PlayerbotAI* ai) { return new TkNoEncounterInProgressTrigger(ai); };
            creators["tempest keep stuck falling"] = [](PlayerbotAI* ai) { return new TkStuckFallingTrigger(ai); };
            creators["crimson hand centurion casts arcane flurry"] = [](PlayerbotAI* ai) { return new TkCrimsonHandCenturionCastsArcaneFlurryTrigger(ai); };
            creators["kael'thas sunstrider legendary weapons were lost"] = [](PlayerbotAI* ai) { return new TkKaelthasLegendaryWeaponsWereLostTrigger(ai); };
        }
    };

    class TkValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        TkValueContext()
        {
            creators["tk flame patches"] = [](PlayerbotAI* ai) { return new TkFlamePatchesValue(ai); };
            creators["tk legendary weapons"] = [](PlayerbotAI* ai) { return new TkLegendaryWeaponsValue(ai); };
        }
    };
}
