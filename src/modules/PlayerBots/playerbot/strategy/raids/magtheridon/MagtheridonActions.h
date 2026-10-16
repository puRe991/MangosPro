// Ported from mod-playerbots (src/Ai/Raid/Mag, GPL v2+) and adapted for this core.
#pragma once

#include "playerbot/strategy/actions/MovementActions.h"
#include "playerbot/strategy/actions/AttackAction.h"
#include "MagtheridonHelpers.h"

namespace ai
{
    class MagtheridonResetEncounterStatesAction : public Action
    {
    public:
        MagtheridonResetEncounterStatesAction(PlayerbotAI* ai) : Action(ai, "magtheridon reset encounter states") {}
        bool Execute(Event& event) override;
    };

    class MagtheridonMainTankAttackFirstThreeChannelersAction : public AttackAction
    {
    public:
        MagtheridonMainTankAttackFirstThreeChannelersAction(PlayerbotAI* ai) : AttackAction(ai, "magtheridon main tank attack first three channelers") {}
        bool Execute(Event& event) override;
    };

    class MagtheridonAssistTanksAttackLastTwoChannelersAction : public AttackAction
    {
    public:
        MagtheridonAssistTanksAttackLastTwoChannelersAction(PlayerbotAI* ai) : AttackAction(ai, "magtheridon assist tanks attack last two channelers") {}
        bool Execute(Event& event) override;
    };

    class MagtheridonMisdirectHellfireChannelersToMainTankAction : public Action
    {
    public:
        MagtheridonMisdirectHellfireChannelersToMainTankAction(PlayerbotAI* ai) : Action(ai, "magtheridon misdirect hellfire channelers to main tank") {}
        bool Execute(Event& event) override;
    };

    class MagtheridonAssignDpsPriorityAction : public AttackAction
    {
    public:
        MagtheridonAssignDpsPriorityAction(PlayerbotAI* ai) : AttackAction(ai, "magtheridon assign dps priority") {}
        bool Execute(Event& event) override;
    };

    class MagtheridonWarlockCcBurningAbyssalAction : public Action
    {
    public:
        MagtheridonWarlockCcBurningAbyssalAction(PlayerbotAI* ai) : Action(ai, "magtheridon warlock cc burning abyssal") {}
        bool Execute(Event& event) override;
    };

    class MagtheridonMainTankPositionBossAction : public AttackAction
    {
    public:
        MagtheridonMainTankPositionBossAction(PlayerbotAI* ai) : AttackAction(ai, "magtheridon main tank position boss") {}
        bool Execute(Event& event) override;
    };

    class MagtheridonSpreadRangedAction : public MovementAction
    {
    public:
        MagtheridonSpreadRangedAction(PlayerbotAI* ai) : MovementAction(ai, "magtheridon spread ranged") {}
        bool Execute(Event& event) override;
    };

    class MagtheridonMoveOutOfDebrisAction : public MovementAction
    {
    public:
        MagtheridonMoveOutOfDebrisAction(PlayerbotAI* ai) : MovementAction(ai, "magtheridon move out of debris") {}
        bool Execute(Event& event) override;

    private:
        bool FindSafePosition(float& outX, float& outY);
    };

    class MagtheridonUseManticronCubeAction : public MovementAction
    {
    public:
        MagtheridonUseManticronCubeAction(PlayerbotAI* ai) : MovementAction(ai, "magtheridon use manticron cube") {}
        bool Execute(Event& event) override;
        bool isPossible() override { return true; }

    private:
        bool HandleCubeRelease(Unit* magtheridon);
        bool HandleCubeInteraction(GameObject* cube);
        bool HandleWaitingPhase(const magtheridon::CubeInfo& cubeInfo);
        bool FindSafePositionNearCube(const magtheridon::CubeInfo& cubeInfo, float preferredDistance, float& outX, float& outY);

        // Staggered release so the clickers don't all let go on the same tick
        uint32 releaseTime = 0;
    };

    class MagtheridonUpdateTimersAndAssignmentsAction : public Action
    {
    public:
        MagtheridonUpdateTimersAndAssignmentsAction(PlayerbotAI* ai) : Action(ai, "magtheridon update timers and assignments") {}
        bool Execute(Event& event) override;
    };
}
