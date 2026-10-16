// Ported from mod-playerbots (AzerothCore, GPL v2+), src/Ai/Raid/ZA/ZAActions.h, and adapted for this core.
#pragma once
#include "playerbot/strategy/Action.h"
#include "playerbot/strategy/actions/MovementActions.h"
#include "playerbot/strategy/actions/AttackAction.h"
#include "playerbot/strategy/actions/TbcRaidActions.h"
#include "ZaHelpers.h"
#include <string>

namespace ai
{
    // General

    class ZaResetEncounterStatesAction : public Action
    {
    public:
        ZaResetEncounterStatesAction(PlayerbotAI* ai) : Action(ai, "zul'aman reset encounter states") {}
        bool Execute(Event& event) override;
    };

    class ZaMisdirectBossToMainTankAction : public Action
    {
    public:
        ZaMisdirectBossToMainTankAction(PlayerbotAI* ai, std::string name, uint32 bossEntry) : Action(ai, name), bossEntry(bossEntry) {}
        bool Execute(Event& event) override;

    private:
        uint32 bossEntry;
    };

    class ZaTanksPositionBossAction : public AttackAction
    {
    public:
        ZaTanksPositionBossAction(PlayerbotAI* ai, std::string name, uint32 bossEntry, const WorldPosition& position)
            : AttackAction(ai, name), bossEntry(bossEntry), position(position) {}
        bool Execute(Event& event) override;

    private:
        uint32 bossEntry;
        WorldPosition position;
    };

    class ZaSpreadRangedAction : public TbcRaidMovementAction
    {
    public:
        ZaSpreadRangedAction(PlayerbotAI* ai, std::string name, float minDistance) : TbcRaidMovementAction(ai, name), minDistance(minDistance) {}
        bool Execute(Event& event) override;

    private:
        float minDistance;
    };

    class ZaRunAwayFromWhirlwindAction : public TbcRaidMovementAction
    {
    public:
        ZaRunAwayFromWhirlwindAction(PlayerbotAI* ai, std::string name, uint32 bossEntry) : TbcRaidMovementAction(ai, name), bossEntry(bossEntry) {}
        bool Execute(Event& event) override;

    private:
        uint32 bossEntry;
    };

    // Trash

    class ZaAmanishiMedicineManMarkWardAction : public Action
    {
    public:
        ZaAmanishiMedicineManMarkWardAction(PlayerbotAI* ai) : Action(ai, "amani'shi medicine man mark ward") {}
        bool Execute(Event& event) override;
    };

    // Akil'zon <Eagle Avatar>

    class ZaAkilzonMoveToEyeOfTheStormAction : public MovementAction
    {
    public:
        ZaAkilzonMoveToEyeOfTheStormAction(PlayerbotAI* ai) : MovementAction(ai, "akil'zon move to eye of the storm") {}
        bool Execute(Event& event) override;
    };

    class ZaAkilzonStartElectricalStormTimerAction : public Action
    {
    public:
        ZaAkilzonStartElectricalStormTimerAction(PlayerbotAI* ai) : Action(ai, "akil'zon start electrical storm timer") {}
        bool Execute(Event& event) override;
    };

    // Nalorakk <Bear Avatar>

    class ZaNalorakkTanksPositionBossAction : public AttackAction
    {
    public:
        ZaNalorakkTanksPositionBossAction(PlayerbotAI* ai) : AttackAction(ai, "nalorakk tanks position boss") {}
        bool Execute(Event& event) override;
    };

    // Jan'alai <Dragonhawk Avatar>

    class ZaJanalaiSpreadRangedInCircleAction : public MovementAction
    {
    public:
        ZaJanalaiSpreadRangedInCircleAction(PlayerbotAI* ai) : MovementAction(ai, "jan'alai spread ranged in circle") {}
        bool Execute(Event& event) override;
    };

    class ZaJanalaiAvoidFireBombsAction : public MovementAction
    {
    public:
        ZaJanalaiAvoidFireBombsAction(PlayerbotAI* ai) : MovementAction(ai, "jan'alai avoid fire bombs") {}
        bool Execute(Event& event) override;
    };

    class ZaJanalaiMarkAmanishiHatchersAction : public Action
    {
    public:
        ZaJanalaiMarkAmanishiHatchersAction(PlayerbotAI* ai) : Action(ai, "jan'alai mark amani'shi hatchers") {}
        bool Execute(Event& event) override;
    };

    // Halazzi <Lynx Avatar>

    class ZaHalazziFirstAssistTankAttackSpiritLynxAction : public AttackAction
    {
    public:
        ZaHalazziFirstAssistTankAttackSpiritLynxAction(PlayerbotAI* ai) : AttackAction(ai, "halazzi first assist tank attack spirit lynx") {}
        bool Execute(Event& event) override;
    };

    class ZaHalazziDpsAttackTotemAndBossAction : public AttackAction
    {
    public:
        ZaHalazziDpsAttackTotemAndBossAction(PlayerbotAI* ai) : AttackAction(ai, "halazzi dps attack totem and boss") {}
        bool Execute(Event& event) override;
    };

    // Hex Lord Malacrass

    class ZaHexLordMalacrassAssignDpsPriorityAction : public AttackAction
    {
    public:
        ZaHexLordMalacrassAssignDpsPriorityAction(PlayerbotAI* ai) : AttackAction(ai, "hex lord malacrass assign dps priority") {}
        bool Execute(Event& event) override;
    };

    class ZaHexLordMalacrassMoveAwayFromFreezingTrapAction : public TbcRaidMovementAction
    {
    public:
        ZaHexLordMalacrassMoveAwayFromFreezingTrapAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "hex lord malacrass move away from freezing trap") {}
        bool Execute(Event& event) override;
    };

    // Zul'jin

    class ZaZuljinMassDispelCreepingParalysisAction : public Action
    {
    public:
        ZaZuljinMassDispelCreepingParalysisAction(PlayerbotAI* ai) : Action(ai, "zul'jin mass dispel creeping paralysis") {}
        bool Execute(Event& event) override;
    };

    class ZaZuljinPositionRangedForCyclonesAction : public MovementAction
    {
    public:
        ZaZuljinPositionRangedForCyclonesAction(PlayerbotAI* ai) : MovementAction(ai, "zul'jin position ranged for cyclones") {}
        bool Execute(Event& event) override;
    };
}
