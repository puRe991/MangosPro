// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#pragma once

#include "playerbot/strategy/actions/MovementActions.h"
#include "playerbot/strategy/actions/AttackAction.h"

namespace ai
{
    // General

    class GruulsLairResetEncounterStatesAction : public Action
    {
    public:
        GruulsLairResetEncounterStatesAction(PlayerbotAI* ai) : Action(ai, "gruul's lair reset encounter states") {}
        bool Execute(Event& event) override;
    };

    // High King Maulgar <Lord of the Ogres>

    class HighKingMaulgarMeleeTanksPositionBossesAction : public AttackAction
    {
    public:
        HighKingMaulgarMeleeTanksPositionBossesAction(PlayerbotAI* ai) : AttackAction(ai, "high king maulgar melee tanks position bosses") {}
        bool Execute(Event& event) override;
    };

    class HighKingMaulgarMageTankAttackKroshAction : public AttackAction
    {
    public:
        HighKingMaulgarMageTankAttackKroshAction(PlayerbotAI* ai) : AttackAction(ai, "high king maulgar mage tank attack krosh") {}
        bool Execute(Event& event) override;

    private:
        bool AttackAndCast(Unit* krosh);
        bool MoveToDesiredDistance(Unit* krosh);
    };

    class HighKingMaulgarMoonkinTankAttackKigglerAction : public AttackAction
    {
    public:
        HighKingMaulgarMoonkinTankAttackKigglerAction(PlayerbotAI* ai) : AttackAction(ai, "high king maulgar moonkin tank attack kiggler") {}
        bool Execute(Event& event) override;
    };

    class HighKingMaulgarAssignDpsPriorityAction : public AttackAction
    {
    public:
        HighKingMaulgarAssignDpsPriorityAction(PlayerbotAI* ai) : AttackAction(ai, "high king maulgar assign dps priority") {}
        bool Execute(Event& event) override;
    };

    class HighKingMaulgarRunAwayFromWhirlwindAction : public MovementAction
    {
    public:
        HighKingMaulgarRunAwayFromWhirlwindAction(PlayerbotAI* ai) : MovementAction(ai, "high king maulgar run away from whirlwind") {}
        bool Execute(Event& event) override;
    };

    class HighKingMaulgarBackAwayFromKroshAction : public MovementAction
    {
    public:
        HighKingMaulgarBackAwayFromKroshAction(PlayerbotAI* ai) : MovementAction(ai, "high king maulgar back away from krosh") {}
        bool Execute(Event& event) override;
    };

    class HighKingMaulgarBanishFelStalkerAction : public Action
    {
    public:
        HighKingMaulgarBanishFelStalkerAction(PlayerbotAI* ai) : Action(ai, "high king maulgar banish fel stalker") {}
        bool Execute(Event& event) override;
    };

    class HighKingMaulgarMisdirectOgresToTanksAction : public Action
    {
    public:
        HighKingMaulgarMisdirectOgresToTanksAction(PlayerbotAI* ai) : Action(ai, "high king maulgar misdirect ogres to tanks") {}
        bool Execute(Event& event) override;
    };

    // Gruul the Dragonkiller

    class GruulTheDragonkillerTanksPositionBossAction : public AttackAction
    {
    public:
        GruulTheDragonkillerTanksPositionBossAction(PlayerbotAI* ai) : AttackAction(ai, "gruul the dragonkiller tanks position boss") {}
        bool Execute(Event& event) override;
    };

    class GruulTheDragonkillerSpreadRangedAction : public MovementAction
    {
    public:
        GruulTheDragonkillerSpreadRangedAction(PlayerbotAI* ai) : MovementAction(ai, "gruul the dragonkiller spread ranged") {}
        bool Execute(Event& event) override;
        bool ResetInitialPosition()
        {
            if (!hasReachedInitialPosition && !hasInitialPosition)
                return false;

            hasReachedInitialPosition = false;
            hasInitialPosition = false;
            return true;
        }

    private:
        float initialX = 0.0f;
        float initialY = 0.0f;
        bool hasInitialPosition = false;
        bool hasReachedInitialPosition = false;
    };

    class GruulTheDragonkillerGetOutOfCaveInAction : public MovementAction
    {
    public:
        GruulTheDragonkillerGetOutOfCaveInAction(PlayerbotAI* ai) : MovementAction(ai, "gruul the dragonkiller get out of cave in") {}
        bool Execute(Event& event) override;
    };

    class GruulTheDragonkillerShatterSpreadAction : public MovementAction
    {
    public:
        GruulTheDragonkillerShatterSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "gruul the dragonkiller shatter spread") {}
        bool Execute(Event& event) override;
    };
}
