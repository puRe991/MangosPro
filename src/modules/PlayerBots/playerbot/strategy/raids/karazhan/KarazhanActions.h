// Ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara/KaraActions) and adapted for this core.
#pragma once
#include "playerbot/strategy/Action.h"
#include "playerbot/strategy/actions/MovementActions.h"
#include "playerbot/strategy/actions/AttackAction.h"
#include "KarazhanHelpers.h"
#include <ctime>
#include <vector>

namespace ai
{
    // Movement helpers shared by the Karazhan movement and attack actions
    template <class Base>
    class KarazhanMoveHelper : public Base
    {
    public:
        KarazhanMoveHelper(PlayerbotAI* ai, std::string name) : Base(ai, name) {}

    protected:
        bool MoveToPoint(float x, float y, float z)
        {
            return this->MoveTo(this->bot->GetMapId(), x, y, z, false, this->IsReaction(), false, true);
        }

        bool MoveToPoint(const karazhan::KaraPosition& position)
        {
            return MoveToPoint(position.x, position.y, position.z);
        }

        // Move in the direction away from (fromX, fromY) until the bot is 'distance' away from it
        bool MoveAwayFromPoint(float fromX, float fromY, float distance);
        // Move to a point on the line between the target and the bot, 'distance' away from the target
        bool MoveToDistanceFrom(Unit* target, float distance);
        // Move away from the raid members around the bot (MoveFromGroup of mod-playerbots)
        bool MoveAwayFromGroup(float distance);
        // Tanks drag the boss to a position in short steps
        bool StepToPosition(const karazhan::KaraPosition& position, float arrivalDist, Unit* boss);
    };

    using KarazhanMovementAction = KarazhanMoveHelper<MovementAction>;
    using KarazhanAttackAction = KarazhanMoveHelper<AttackAction>;

    // General

    class KarazhanResetEncounterStatesAction : public Action
    {
    public:
        KarazhanResetEncounterStatesAction(PlayerbotAI* ai) : Action(ai, "karazhan reset encounter states") {}
        bool Execute(Event& event) override;
    };

    class KarazhanSetTremorTotemAction : public Action
    {
    public:
        KarazhanSetTremorTotemAction(PlayerbotAI* ai) : Action(ai, "karazhan set tremor totem") {}
        bool Execute(Event& event) override;
    };

    // Trash

    class KarazhanManaWarpStunAction : public Action
    {
    public:
        KarazhanManaWarpStunAction(PlayerbotAI* ai) : Action(ai, "mana warp stun creature before warp breach") {}
        bool Execute(Event& event) override;
    };

    // Attumen the Huntsman

    class KarazhanAttumenHandlePhaseOneAction : public KarazhanAttackAction
    {
    public:
        KarazhanAttumenHandlePhaseOneAction(PlayerbotAI* ai) : KarazhanAttackAction(ai, "attumen the huntsman handle phase one") {}
        bool Execute(Event& event) override;

    private:
        bool AssistTankMoveAttumenFromGroup(Player* requester, Unit* midnight, Unit* attumen);
    };

    class KarazhanAttumenHandlePhaseTwoAction : public KarazhanAttackAction
    {
    public:
        KarazhanAttumenHandlePhaseTwoAction(PlayerbotAI* ai) : KarazhanAttackAction(ai, "attumen the huntsman handle phase two") {}
        bool Execute(Event& event) override;

    private:
        bool CurrentTankPositionAttumen(Unit* attumen);
        bool StackBehindAttumen(Unit* attumen);
    };

    class KarazhanAttumenSetDpsTimerAction : public Action
    {
    public:
        KarazhanAttumenSetDpsTimerAction(PlayerbotAI* ai) : Action(ai, "attumen the huntsman set dps timer") {}
        bool Execute(Event& event) override;
    };

    // Moroes

    class KarazhanMoroesMarkTargetAction : public Action
    {
    public:
        KarazhanMoroesMarkTargetAction(PlayerbotAI* ai) : Action(ai, "moroes mark target") {}
        bool Execute(Event& event) override;
    };

    // Maiden of Virtue

    class KarazhanMaidenTankPositionBossAction : public KarazhanAttackAction
    {
    public:
        KarazhanMaidenTankPositionBossAction(PlayerbotAI* ai) : KarazhanAttackAction(ai, "maiden of virtue tank position boss") {}
        bool Execute(Event& event) override;

    private:
        bool MoveBossToStunnedHealer(Player* healer);
    };

    class KarazhanMaidenPositionRangedAction : public KarazhanMovementAction
    {
    public:
        KarazhanMaidenPositionRangedAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "maiden of virtue position ranged between pillars") {}
        bool Execute(Event& event) override;
    };

    class KarazhanMaidenSetGroundingTotemAction : public Action
    {
    public:
        KarazhanMaidenSetGroundingTotemAction(PlayerbotAI* ai) : Action(ai, "maiden of virtue set grounding totem") {}
        bool Execute(Event& event) override;
    };

    // The Big Bad Wolf

    class KarazhanBigBadWolfPositionBossAction : public KarazhanAttackAction
    {
    public:
        KarazhanBigBadWolfPositionBossAction(PlayerbotAI* ai) : KarazhanAttackAction(ai, "big bad wolf position boss") {}
        bool Execute(Event& event) override;
    };

    class KarazhanBigBadWolfRunAwayAction : public KarazhanMovementAction
    {
    public:
        KarazhanBigBadWolfRunAwayAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "big bad wolf little red riding hood run away") {}
        bool Execute(Event& event) override;
        bool isUseful() override { return true; }   // run even with "stay"
        bool ResetRunIndex()
        {
            if (!runIndex)
                return false;

            runIndex = 0;
            return true;
        }

    private:
        uint8 runIndex = 0;
    };

    // Romulo and Julianne

    class KarazhanRomuloAndJulianneMarkTargetAction : public Action
    {
    public:
        KarazhanRomuloAndJulianneMarkTargetAction(PlayerbotAI* ai) : Action(ai, "romulo and julianne mark target") {}
        bool Execute(Event& event) override;
    };

    // The Wizard of Oz

    class KarazhanWizardOfOzMarkTargetAction : public Action
    {
    public:
        KarazhanWizardOfOzMarkTargetAction(PlayerbotAI* ai) : Action(ai, "wizard of oz mark target") {}
        bool Execute(Event& event) override;
    };

    class KarazhanWizardOfOzScorchStrawmanAction : public Action
    {
    public:
        KarazhanWizardOfOzScorchStrawmanAction(PlayerbotAI* ai) : Action(ai, "wizard of oz scorch strawman") {}
        bool Execute(Event& event) override;
    };

    // The Curator

    class KarazhanCuratorMarkAstralFlareAction : public Action
    {
    public:
        KarazhanCuratorMarkAstralFlareAction(PlayerbotAI* ai) : Action(ai, "the curator mark astral flare") {}
        bool Execute(Event& event) override;
    };

    class KarazhanCuratorPositionBossAction : public KarazhanAttackAction
    {
    public:
        KarazhanCuratorPositionBossAction(PlayerbotAI* ai) : KarazhanAttackAction(ai, "the curator position boss") {}
        bool Execute(Event& event) override;
    };

    class KarazhanCuratorSpreadRangedAction : public KarazhanMovementAction
    {
    public:
        KarazhanCuratorSpreadRangedAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "the curator spread ranged") {}
        bool Execute(Event& event) override;
    };

    // Terestian Illhoof

    class KarazhanIllhoofMarkTargetAction : public Action
    {
    public:
        KarazhanIllhoofMarkTargetAction(PlayerbotAI* ai) : Action(ai, "terestian illhoof mark target") {}
        bool Execute(Event& event) override;
    };

    // Shade of Aran

    class KarazhanAranRunAwayFromArcaneExplosionAction : public KarazhanMovementAction
    {
    public:
        KarazhanAranRunAwayFromArcaneExplosionAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "shade of aran run away from arcane explosion") {}
        bool Execute(Event& event) override;
        bool isUseful() override { return true; }   // run even with "stay"
    };

    class KarazhanAranStopMovingDuringFlameWreathAction : public KarazhanMovementAction
    {
    public:
        KarazhanAranStopMovingDuringFlameWreathAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "shade of aran stop moving during flame wreath") {}
        bool Execute(Event& event) override;
        bool isUseful() override { return true; }
        bool isPossible() override { return true; }
    };

    class KarazhanAranMarkConjuredElementalAction : public Action
    {
    public:
        KarazhanAranMarkConjuredElementalAction(PlayerbotAI* ai) : Action(ai, "shade of aran mark conjured elemental") {}
        bool Execute(Event& event) override;
    };

    class KarazhanAranRangedMaintainDistanceAction : public KarazhanMovementAction
    {
    public:
        KarazhanAranRangedMaintainDistanceAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "shade of aran ranged maintain distance") {}
        bool Execute(Event& event) override;
    };

    // Netherspite

    // The red beam dance (5 seconds in, 5 seconds out)
    class KarazhanNetherspiteBlockRedBeamAction : public KarazhanMovementAction
    {
    public:
        KarazhanNetherspiteBlockRedBeamAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "netherspite block red beam") {}
        bool Execute(Event& event) override;
        bool isUseful() override { return true; }
        bool ResetRedBeamState()
        {
            if (!redBeamTimerWasSet && !wasBlockingRedBeam)
                return false;

            redBeamMoveTimer = 0;
            lastBeamMoveSideways = false;
            wasBlockingRedBeam = false;
            redBeamTimerWasSet = false;
            return true;
        }

    private:
        time_t redBeamMoveTimer = 0;
        bool lastBeamMoveSideways = false;
        bool wasBlockingRedBeam = false;
        bool redBeamTimerWasSet = false;
    };

    // Mana using dps bots block the blue beam (swap at 25 debuff stacks)
    class KarazhanNetherspiteBlockBlueBeamAction : public KarazhanMovementAction
    {
    public:
        KarazhanNetherspiteBlockBlueBeamAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "netherspite block blue beam") {}
        bool Execute(Event& event) override;
        bool isUseful() override { return true; }
        bool ResetBlueBeamState()
        {
            if (!wasBlockingBlueBeam)
                return false;

            wasBlockingBlueBeam = false;
            return true;
        }

    private:
        bool wasBlockingBlueBeam = false;
    };

    // Healer bots block the green beam (swap at 25 debuff stacks)
    // or one non mana dps bot blocks the green beam for an entire phase
    class KarazhanNetherspiteBlockGreenBeamAction : public KarazhanMovementAction
    {
    public:
        KarazhanNetherspiteBlockGreenBeamAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "netherspite block green beam") {}
        bool Execute(Event& event) override;
        bool isUseful() override { return true; }
        bool ResetGreenBeamState()
        {
            if (!wasBlockingGreenBeam)
                return false;

            wasBlockingGreenBeam = false;
            return true;
        }

    private:
        bool wasBlockingGreenBeam = false;
    };

    class KarazhanNetherspiteAvoidBeamAndVoidZoneAction : public KarazhanMovementAction
    {
    public:
        KarazhanNetherspiteAvoidBeamAndVoidZoneAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "netherspite avoid beam and void zone") {}
        bool Execute(Event& event) override;
        bool isUseful() override { return true; } // hazards override stay

    private:
        struct BeamAvoid
        {
            float minDist;
            float maxDist;
            float dirX;
            float dirY;
        };
        static bool IsAwayFromBeams(float x, float y, float bossX, float bossY, const std::vector<BeamAvoid>& beams);
    };

    class KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction : public KarazhanMovementAction
    {
    public:
        KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "netherspite banish phase avoid void zone") {}
        bool Execute(Event& event) override;
        bool isUseful() override { return true; }
    };

    class KarazhanNetherspiteManageTimersAndTrackersAction : public Action
    {
    public:
        KarazhanNetherspiteManageTimersAndTrackersAction(PlayerbotAI* ai) : Action(ai, "netherspite manage timers and trackers") {}
        bool Execute(Event& event) override;
    };

    // Prince Malchezaar

    class KarazhanMalchezaarEnfeebledAvoidHazardAction : public KarazhanMovementAction
    {
    public:
        KarazhanMalchezaarEnfeebledAvoidHazardAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "prince malchezaar enfeebled bot avoid hazard") {}
        bool Execute(Event& event) override;
        bool isUseful() override { return true; }
    };

    class KarazhanMalchezaarNonTankAvoidInfernalAction : public KarazhanMovementAction
    {
    public:
        KarazhanMalchezaarNonTankAvoidInfernalAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "prince malchezaar non-tank avoid infernal") {}
        bool Execute(Event& event) override;
        bool isUseful() override { return true; }
    };

    class KarazhanMalchezaarTanksPositionBossAction : public KarazhanAttackAction
    {
    public:
        KarazhanMalchezaarTanksPositionBossAction(PlayerbotAI* ai) : KarazhanAttackAction(ai, "prince malchezaar tanks position boss") {}
        bool Execute(Event& event) override;
    };

    // Nightbane

    class KarazhanNightbaneTanksPositionBossAction : public KarazhanAttackAction
    {
    public:
        KarazhanNightbaneTanksPositionBossAction(PlayerbotAI* ai) : KarazhanAttackAction(ai, "nightbane ground phase tanks position boss") {}
        bool Execute(Event& event) override;
    };

    // Ranged bots stack on one "ranged leader" (an assistant of the group if there is one),
    // the ranged leader leads the ranged group out of Charred Earth
    class KarazhanNightbaneCoordinateRangedAction : public KarazhanMovementAction
    {
    public:
        KarazhanNightbaneCoordinateRangedAction(PlayerbotAI* ai) : KarazhanMovementAction(ai, "nightbane ground phase coordinate ranged movement") {}
        bool Execute(Event& event) override;

    private:
        bool MoveRangedLeaderToSafeSpot();
        bool StackOnRangedLeader(Player* rangedLeader);
    };

    class KarazhanNightbaneControlPetAggressionAction : public Action
    {
    public:
        KarazhanNightbaneControlPetAggressionAction(PlayerbotAI* ai) : Action(ai, "nightbane control pet aggression") {}
        bool Execute(Event& event) override;

    private:
        bool setPassive = false;
    };

    // 1. Stack at the "flight stack position" near Nightbane so he doesn't use Fireball Barrage
    // 2. Once Rain of Bones hits, the whole party moves to a new stack position
    // This lasts for the first 35 seconds of the flight phase, after which Nightbane begins landing
    class KarazhanNightbaneFlightPhaseStackAction : public KarazhanAttackAction
    {
    public:
        KarazhanNightbaneFlightPhaseStackAction(PlayerbotAI* ai) : KarazhanAttackAction(ai, "nightbane flight phase stack and move") {}
        bool Execute(Event& event) override;
        bool ResetRainOfBonesHit()
        {
            if (!rainOfBonesHit)
                return false;

            rainOfBonesHit = false;
            return true;
        }

    private:
        bool rainOfBonesHit = false;
    };

    // Failsafe in case bots fall through the world or off the terrace
    class KarazhanNightbaneTeleportBackAction : public Action
    {
    public:
        KarazhanNightbaneTeleportBackAction(PlayerbotAI* ai) : Action(ai, "nightbane teleport back to terrace") {}
        bool Execute(Event& event) override;
    };

    class KarazhanNightbaneManageTimersAndTrackersAction : public Action
    {
    public:
        KarazhanNightbaneManageTimersAndTrackersAction(PlayerbotAI* ai) : Action(ai, "nightbane manage timers and trackers") {}
        bool Execute(Event& event) override;
    };
}
