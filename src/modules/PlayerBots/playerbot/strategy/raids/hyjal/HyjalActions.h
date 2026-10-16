// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/Hyjal/HyjalActions) and adapted for this core.
#pragma once

#include "playerbot/strategy/actions/MovementActions.h"
#include "playerbot/strategy/actions/AttackAction.h"
#include "HyjalHelpers.h"

namespace ai
{
    // Movement helpers shared by the movement and attack actions of this raid
    template<class Base>
    class HyjalMoveHelper : public Base
    {
    public:
        HyjalMoveHelper(PlayerbotAI* ai, std::string name) : Base(ai, name) {}

    protected:
        // Step to a point (the height is taken from the map when possible)
        bool MoveStep(float x, float y, float z);
        // Leave the circle of the given radius around a position (FleePosition of mod-playerbots).
        // minIntervalMs throttles repeated flees of the same action.
        bool FleePosition(const WorldPosition& from, float radius, uint32 minIntervalMs = 0);
        // Move the given distance away from a unit
        bool MoveAway(Unit* from, float distance);

    private:
        uint32 lastFleeTime = 0;
    };

    typedef HyjalMoveHelper<MovementAction> HyjalMovementAction;
    typedef HyjalMoveHelper<AttackAction> HyjalAttackAction;

    // General

    class HyjalResetEncounterStatesAction : public Action
    {
    public:
        HyjalResetEncounterStatesAction(PlayerbotAI* ai) : Action(ai, "hyjal reset encounter states") {}
        bool Execute(Event& event) override;
    };

    class HyjalMisdirectBossToMainTankAction : public Action
    {
    public:
        HyjalMisdirectBossToMainTankAction(PlayerbotAI* ai, std::string name, uint32 bossEntry) : Action(ai, name), bossEntry(bossEntry) {}
        bool Execute(Event& event) override;

    private:
        uint32 bossEntry;
    };

    // For all five bosses. bailBelowHealthPct is the health of the tank below which it stops walking the boss.
    class HyjalMainTankPositionBossAction : public HyjalAttackAction
    {
    public:
        HyjalMainTankPositionBossAction(PlayerbotAI* ai, std::string name, uint32 bossEntry, const hyjal::Point& position, float bailBelowHealthPct = 0.0f)
            : HyjalAttackAction(ai, name), bossEntry(bossEntry), position(position), bailBelowHealthPct(bailBelowHealthPct) {}
        bool Execute(Event& event) override;

    private:
        uint32 bossEntry;
        hyjal::Point position;
        float bailBelowHealthPct;
    };

    // Ice Block / Divine Shield / Cloak of Shadows against Mark of Kaz'rogal and Doomfire
    class HyjalRemoveDangerousDotAction : public Action
    {
    public:
        HyjalRemoveDangerousDotAction(PlayerbotAI* ai) : Action(ai, "hyjal remove dangerous dot") {}
        bool Execute(Event& event) override;
    };

    // Rage Winterchill

    class RageWinterchillRangedGetOutOfDeathAndDecayAction : public HyjalMovementAction
    {
    public:
        RageWinterchillRangedGetOutOfDeathAndDecayAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "rage winterchill ranged get out of death and decay") {}
        bool Execute(Event& event) override;
    };

    class RageWinterchillSpreadRangedInCircleAction : public HyjalMovementAction
    {
    public:
        RageWinterchillSpreadRangedInCircleAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "rage winterchill spread ranged in circle") {}
        bool Execute(Event& event) override;
    };

    class RageWinterchillMeleeManeuverThroughDeathAndDecayAction : public HyjalAttackAction
    {
    public:
        RageWinterchillMeleeManeuverThroughDeathAndDecayAction(PlayerbotAI* ai) : HyjalAttackAction(ai, "rage winterchill melee maneuver through death and decay") {}
        bool Execute(Event& event) override;
    };

    // Anetheron

    class AnetheronMisdirectBossAndInfernalsToTanksAction : public Action
    {
    public:
        AnetheronMisdirectBossAndInfernalsToTanksAction(PlayerbotAI* ai) : Action(ai, "anetheron misdirect boss and infernals to tanks") {}
        bool Execute(Event& event) override;
    };

    class AnetheronSpreadRangedInCircleAction : public HyjalMovementAction
    {
    public:
        AnetheronSpreadRangedInCircleAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "anetheron spread ranged in circle") {}
        bool Execute(Event& event) override;
    };

    class AnetheronMoveAwayFromInfernoTargetAction : public HyjalMovementAction
    {
    public:
        AnetheronMoveAwayFromInfernoTargetAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "anetheron move away from inferno target") {}
        bool Execute(Event& event) override;
    };

    class AnetheronBringInfernalToInfernalTankAction : public HyjalMovementAction
    {
    public:
        AnetheronBringInfernalToInfernalTankAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "anetheron bring infernal to infernal tank") {}
        bool Execute(Event& event) override;
    };

    class AnetheronInfernalTankTakePositionAction : public HyjalMovementAction
    {
    public:
        AnetheronInfernalTankTakePositionAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "anetheron infernal tank take position") {}
        bool Execute(Event& event) override;
    };

    class AnetheronGetOutOfImmolationAction : public HyjalMovementAction
    {
    public:
        AnetheronGetOutOfImmolationAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "anetheron get out of immolation") {}
        bool Execute(Event& event) override;
    };

    class AnetheronAssignDpsPriorityAction : public HyjalAttackAction
    {
    public:
        AnetheronAssignDpsPriorityAction(PlayerbotAI* ai) : HyjalAttackAction(ai, "anetheron assign dps priority") {}
        bool Execute(Event& event) override;
    };

    // Kaz'rogal

    class KazrogalAssistTanksMoveInFrontAction : public HyjalAttackAction
    {
    public:
        KazrogalAssistTanksMoveInFrontAction(PlayerbotAI* ai) : HyjalAttackAction(ai, "kaz'rogal assist tanks move in front") {}
        bool Execute(Event& event) override;
    };

    class KazrogalSpreadRangedInArcAction : public HyjalMovementAction
    {
    public:
        KazrogalSpreadRangedInArcAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "kaz'rogal spread ranged in arc") {}
        bool Execute(Event& event) override;
    };

    class KazrogalMoveAwayFromGroupAction : public HyjalMovementAction
    {
    public:
        KazrogalMoveAwayFromGroupAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "kaz'rogal move away from group") {}
        bool Execute(Event& event) override;
    };

    class KazrogalActivateAspectOfTheViperAction : public Action
    {
    public:
        KazrogalActivateAspectOfTheViperAction(PlayerbotAI* ai) : Action(ai, "kaz'rogal activate aspect of the viper") {}
        bool Execute(Event& event) override;
    };

    class KazrogalCancelImmunityAction : public Action
    {
    public:
        KazrogalCancelImmunityAction(PlayerbotAI* ai) : Action(ai, "kaz'rogal cancel immunity") {}
        bool Execute(Event& event) override;
    };

    class KazrogalWarlockManageManaAction : public Action
    {
    public:
        KazrogalWarlockManageManaAction(PlayerbotAI* ai) : Action(ai, "kaz'rogal warlock manage mana") {}
        bool Execute(Event& event) override;
    };

    // Azgalor

    class AzgalorDisperseRangedAction : public HyjalMovementAction
    {
    public:
        AzgalorDisperseRangedAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "azgalor disperse ranged") {}
        bool Execute(Event& event) override;
    };

    class AzgalorMeleeManeuverThroughFireAction : public HyjalMovementAction
    {
    public:
        AzgalorMeleeManeuverThroughFireAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "azgalor melee maneuver through fire") {}
        bool Execute(Event& event) override;
    };

    class AzgalorRangedGetOutOfRainOfFireAction : public HyjalMovementAction
    {
    public:
        AzgalorRangedGetOutOfRainOfFireAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "azgalor ranged get out of rain of fire") {}
        bool Execute(Event& event) override;
    };

    class AzgalorMoveToDoomguardTankAction : public HyjalMovementAction
    {
    public:
        AzgalorMoveToDoomguardTankAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "azgalor move to doomguard tank") {}
        bool Execute(Event& event) override;
    };

    // The Doomguard tank is the first assist tank, or the second one when the first is Doomed
    class AzgalorTankPositionDoomguardAction : public HyjalAttackAction
    {
    public:
        AzgalorTankPositionDoomguardAction(PlayerbotAI* ai) : HyjalAttackAction(ai, "azgalor tank position doomguard") {}
        bool Execute(Event& event) override;
    };

    class AzgalorDetermineDpsPriorityAction : public HyjalAttackAction
    {
    public:
        AzgalorDetermineDpsPriorityAction(PlayerbotAI* ai) : HyjalAttackAction(ai, "azgalor determine dps priority") {}
        bool Execute(Event& event) override;
    };

    // Archimonde

    class ArchimondeSetTremorTotemAction : public Action
    {
    public:
        ArchimondeSetTremorTotemAction(PlayerbotAI* ai) : Action(ai, "archimonde set tremor totem") {}
        bool Execute(Event& event) override;
    };

    class ArchimondeKeepAirBurstAwayFromTankAction : public HyjalMovementAction
    {
    public:
        ArchimondeKeepAirBurstAwayFromTankAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "archimonde keep air burst away from tank") {}
        bool Execute(Event& event) override;
    };

    class ArchimondeSpreadRangedAction : public HyjalMovementAction
    {
    public:
        ArchimondeSpreadRangedAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "archimonde spread ranged") {}
        bool Execute(Event& event) override;
    };

    class ArchimondeAvoidDoomfireAction : public HyjalMovementAction
    {
    public:
        ArchimondeAvoidDoomfireAction(PlayerbotAI* ai) : HyjalMovementAction(ai, "archimonde avoid doomfire") {}
        bool Execute(Event& event) override;
    };
}
