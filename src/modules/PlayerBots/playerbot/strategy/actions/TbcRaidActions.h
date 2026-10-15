#pragma once
#include "MovementActions.h"
#include "AttackAction.h"
#include "ChangeStrategyAction.h"
#include "UseItemAction.h"
#include <functional>

namespace ai
{
    class TbcRaidEnableStrategyAction : public ChangeAllStrategyAction
    {
    public:
        TbcRaidEnableStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable tbc raid strategy", "+tbc raid") {}
    };

    class TbcRaidDisableStrategyAction : public ChangeAllStrategyAction
    {
    public:
        TbcRaidDisableStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable tbc raid strategy", "-tbc raid") {}
    };

    // Base class for the raid movements: searches a reachable point around the bot
    class TbcRaidMovementAction : public MovementAction
    {
    public:
        TbcRaidMovementAction(PlayerbotAI* ai, std::string name) : MovementAction(ai, name) {}
        bool isPossible() override { return MovementAction::isPossible() && ai->CanMove(); }
        bool ShouldReactionInterruptCast() const override { return true; }
        bool ShouldReactionInterruptMovement() const override { return true; }

    protected:
        // Score: lower is better, negative = point not acceptable
        bool MoveToBestPoint(float minRadius, float maxRadius, const std::function<float(const WorldPosition&)>& score);
        bool MoveToPoint(const WorldPosition& point);

        // Unit the bot should stay in range of (current target or boss)
        Unit* GetAnchor();
        float GetAnchorRange();
    };

    // Leave fire, void zones, whirlwinds, fixating adds, ...
    class TbcRaidLeaveHazardAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidLeaveHazardAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid leave hazard") {}
        bool Execute(Event& event) override;
    };

    // Spread out / run away from the raid with a debuff
    class TbcRaidSpreadAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidSpreadAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid spread") {}
        bool Execute(Event& event) override;
    };

    // Stack on a raid member (Electrical Storm)
    class TbcRaidGatherAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidGatherAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid gather") {}
        bool Execute(Event& event) override;
    };

    // Get out of the frontal cone of a boss
    class TbcRaidLeaveFrontAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidLeaveFrontAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid leave front") {}
        bool Execute(Event& event) override;
    };

    // Magtheridon: go to the assigned Manticron Cube and click it on Blast Nova
    class TbcRaidUseCubeAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidUseCubeAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid use cube") {}
        bool Execute(Event& event) override;
        bool isPossible() override { return true; }
    };

    // Naj'entus: walk to the Impaling Spine of an impaled player
    class TbcRaidMoveToSpineAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidMoveToSpineAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid move to spine") {}
        bool Execute(Event& event) override;
    };

    // Naj'entus: pick up the spine / throw it at the Tidal Shield
    class TbcRaidUseSpineAction : public UseAction
    {
    public:
        TbcRaidUseSpineAction(PlayerbotAI* ai) : UseAction(ai, "tbc raid use spine") {}
        bool Execute(Event& event) override;
        bool isUseful() override;
    };

    // Attack the add which has to die first
    class TbcRaidAttackPriorityTargetAction : public AttackAction
    {
    public:
        TbcRaidAttackPriorityTargetAction(PlayerbotAI* ai) : AttackAction(ai, "tbc raid attack priority target") {}
        bool Execute(Event& event) override;
        bool isUseful() override;
    };

    // Taunt the boss (tank swap)
    class TbcRaidTauntAction : public Action
    {
    public:
        TbcRaidTauntAction(PlayerbotAI* ai) : Action(ai, "tbc raid taunt") {}
        bool Execute(Event& event) override;
        bool isUseful() override;
    };

    // Off tanks stand on the main tank (Saber Lash)
    class TbcRaidStackOnTankAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidStackOnTankAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid stack on tank") {}
        bool Execute(Event& event) override;
    };

    // Come closer to the boss with debuff stacks (Bloodboil rotation)
    class TbcRaidApproachBossAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidApproachBossAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid approach boss") {}
        bool Execute(Event& event) override;
    };

    // Stand in front of the boss to soak (Meteor Slash)
    class TbcRaidSoakFrontAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidSoakFrontAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid soak front") {}
        bool Execute(Event& event) override;
    };

    // Hydross: drag the boss in or out of the purification circle
    class TbcRaidHydrossTankAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidHydrossTankAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid hydross tank") {}
        bool Execute(Event& event) override;
    };

    // Lady Vashj: loot the Tainted Core
    class TbcRaidLootCoreAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidLootCoreAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid loot core") {}
        bool Execute(Event& event) override;
    };

    // Lady Vashj: use the Tainted Core on a shield generator or throw it closer to one
    class TbcRaidUseCoreAction : public UseAction
    {
    public:
        TbcRaidUseCoreAction(PlayerbotAI* ai) : UseAction(ai, "tbc raid use core") {}
        bool Execute(Event& event) override;
        bool isUseful() override;
        bool isUsefulWhenStunned() override { return true; }
    };

    // The Lurker Below: jump into the water during Spout
    class TbcRaidJumpIntoWaterAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidJumpIntoWaterAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid jump into water") {}
        bool Execute(Event& event) override;
    };

    // Teron Gorefiend / Kil'jaeden: control the possessed Vengeful Spirit or Blue Dragon
    class TbcRaidControlPossessedAction : public Action
    {
    public:
        TbcRaidControlPossessedAction(PlayerbotAI* ai) : Action(ai, "tbc raid control possessed") {}
        bool Execute(Event& event) override;
        bool isUsefulWhenStunned() override { return true; }

    private:
        bool ControlSpirit(Creature* spirit);
        bool ControlBlueDragon(Creature* dragon);
        bool CastCharmSpell(Creature* charm, uint32 spellId, Unit* target);
    };

    // Kil'jaeden: use a Blue Dragon orb
    class TbcRaidUseBlueOrbAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidUseBlueOrbAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid use blue orb") {}
        bool Execute(Event& event) override;
    };

    // Kil'jaeden: stand below the Blue Dragon for Shield of the Blue
    class TbcRaidGatherUnderDragonAction : public TbcRaidMovementAction
    {
    public:
        TbcRaidGatherUnderDragonAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "tbc raid gather under dragon") {}
        bool Execute(Event& event) override;
    };
}
