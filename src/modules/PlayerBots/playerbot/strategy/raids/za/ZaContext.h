#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "ZaStrategy.h"
#include "ZaActions.h"
#include "ZaTriggers.h"
#include "ZaValues.h"

// Zul'Aman (map 568) - strategy "boss zul'aman"
namespace ai
{
    class ZaStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        ZaStrategyContext()
        {
            creators["boss zul'aman"] = [](PlayerbotAI* ai) { return new ZaStrategy(ai); };
        }
    };

    class ZaActionContext : public NamedObjectContext<Action>
    {
    public:
        ZaActionContext()
        {
            creators["zul'aman reset encounter states"] = [](PlayerbotAI* ai) { return new ZaResetEncounterStatesAction(ai); };
            creators["amani'shi medicine man mark ward"] = [](PlayerbotAI* ai) { return new ZaAmanishiMedicineManMarkWardAction(ai); };
            creators["akil'zon misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new ZaMisdirectBossToMainTankAction(ai, "akil'zon misdirect boss to main tank", za::NPC_AKILZON); };
            creators["akil'zon tanks position boss"] = [](PlayerbotAI* ai) { return new ZaTanksPositionBossAction(ai, "akil'zon tanks position boss", za::NPC_AKILZON, za::AKILZON_TANK_POSITION); };
            creators["akil'zon spread ranged"] = [](PlayerbotAI* ai) { return new ZaSpreadRangedAction(ai, "akil'zon spread ranged", 13.0f); };
            creators["akil'zon move to eye of the storm"] = [](PlayerbotAI* ai) { return new ZaAkilzonMoveToEyeOfTheStormAction(ai); };
            creators["akil'zon start electrical storm timer"] = [](PlayerbotAI* ai) { return new ZaAkilzonStartElectricalStormTimerAction(ai); };
            creators["nalorakk misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new ZaMisdirectBossToMainTankAction(ai, "nalorakk misdirect boss to main tank", za::NPC_NALORAKK); };
            creators["nalorakk tanks position boss"] = [](PlayerbotAI* ai) { return new ZaNalorakkTanksPositionBossAction(ai); };
            creators["nalorakk spread ranged"] = [](PlayerbotAI* ai) { return new ZaSpreadRangedAction(ai, "nalorakk spread ranged", 11.0f); };
            creators["jan'alai misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new ZaMisdirectBossToMainTankAction(ai, "jan'alai misdirect boss to main tank", za::NPC_JANALAI); };
            creators["jan'alai tanks position boss"] = [](PlayerbotAI* ai) { return new ZaTanksPositionBossAction(ai, "jan'alai tanks position boss", za::NPC_JANALAI, za::JANALAI_TANK_POSITION); };
            creators["jan'alai spread ranged in circle"] = [](PlayerbotAI* ai) { return new ZaJanalaiSpreadRangedInCircleAction(ai); };
            creators["jan'alai avoid fire bombs"] = [](PlayerbotAI* ai) { return new ZaJanalaiAvoidFireBombsAction(ai); };
            creators["jan'alai mark amani'shi hatchers"] = [](PlayerbotAI* ai) { return new ZaJanalaiMarkAmanishiHatchersAction(ai); };
            creators["halazzi misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new ZaMisdirectBossToMainTankAction(ai, "halazzi misdirect boss to main tank", za::NPC_HALAZZI); };
            creators["halazzi main tank position boss"] = [](PlayerbotAI* ai) { return new ZaTanksPositionBossAction(ai, "halazzi main tank position boss", za::NPC_HALAZZI, za::HALAZZI_TANK_POSITION); };
            creators["halazzi first assist tank attack spirit lynx"] = [](PlayerbotAI* ai) { return new ZaHalazziFirstAssistTankAttackSpiritLynxAction(ai); };
            creators["halazzi dps attack totem and boss"] = [](PlayerbotAI* ai) { return new ZaHalazziDpsAttackTotemAndBossAction(ai); };
            creators["hex lord malacrass misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new ZaMisdirectBossToMainTankAction(ai, "hex lord malacrass misdirect boss to main tank", za::NPC_HEX_LORD_MALACRASS); };
            creators["hex lord malacrass assign dps priority"] = [](PlayerbotAI* ai) { return new ZaHexLordMalacrassAssignDpsPriorityAction(ai); };
            creators["hex lord malacrass run away from whirlwind"] = [](PlayerbotAI* ai) { return new ZaRunAwayFromWhirlwindAction(ai, "hex lord malacrass run away from whirlwind", za::NPC_HEX_LORD_MALACRASS); };
            creators["hex lord malacrass move away from freezing trap"] = [](PlayerbotAI* ai) { return new ZaHexLordMalacrassMoveAwayFromFreezingTrapAction(ai); };
            creators["zul'jin misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new ZaMisdirectBossToMainTankAction(ai, "zul'jin misdirect boss to main tank", za::NPC_ZULJIN); };
            creators["zul'jin tanks position boss"] = [](PlayerbotAI* ai) { return new ZaTanksPositionBossAction(ai, "zul'jin tanks position boss", za::NPC_ZULJIN, za::ZULJIN_TANK_POSITION); };
            creators["zul'jin run away from whirlwind"] = [](PlayerbotAI* ai) { return new ZaRunAwayFromWhirlwindAction(ai, "zul'jin run away from whirlwind", za::NPC_ZULJIN); };
            creators["zul'jin mass dispel creeping paralysis"] = [](PlayerbotAI* ai) { return new ZaZuljinMassDispelCreepingParalysisAction(ai); };
            creators["zul'jin position ranged for cyclones"] = [](PlayerbotAI* ai) { return new ZaZuljinPositionRangedForCyclonesAction(ai); };
            creators["zul'jin spread ranged"] = [](PlayerbotAI* ai) { return new ZaSpreadRangedAction(ai, "zul'jin spread ranged", 6.0f); };
        }
    };

    class ZaTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        ZaTriggerContext()
        {
            creators["zul'aman no encounter in progress"] = [](PlayerbotAI* ai) { return new ZaNoEncounterInProgressTrigger(ai); };
            creators["amani'shi medicine man summoned ward"] = [](PlayerbotAI* ai) { return new ZaAmanishiMedicineManSummonedWardTrigger(ai); };
            creators["akil'zon pulling boss"] = [](PlayerbotAI* ai) { return new ZaPullingBossTrigger(ai, "akil'zon pulling boss", za::NPC_AKILZON); };
            creators["akil'zon should be tanked"] = [](PlayerbotAI* ai) { return new ZaAkilzonShouldBeTankedTrigger(ai); };
            creators["akil'zon spread for static disruption"] = [](PlayerbotAI* ai) { return new ZaAkilzonSpreadForStaticDisruptionTrigger(ai); };
            creators["akil'zon electrical storm incoming"] = [](PlayerbotAI* ai) { return new ZaAkilzonElectricalStormIncomingTrigger(ai); };
            creators["akil'zon should track electrical storm"] = [](PlayerbotAI* ai) { return new ZaAkilzonShouldTrackElectricalStormTrigger(ai); };
            creators["nalorakk pulling boss"] = [](PlayerbotAI* ai) { return new ZaPullingBossTrigger(ai, "nalorakk pulling boss", za::NPC_NALORAKK); };
            creators["nalorakk both forms should be tanked"] = [](PlayerbotAI* ai) { return new ZaNalorakkBothFormsShouldBeTankedTrigger(ai); };
            creators["nalorakk spread for surge"] = [](PlayerbotAI* ai) { return new ZaNalorakkSpreadForSurgeTrigger(ai); };
            creators["jan'alai pulling boss"] = [](PlayerbotAI* ai) { return new ZaPullingBossTrigger(ai, "jan'alai pulling boss", za::NPC_JANALAI); };
            creators["jan'alai should be tanked"] = [](PlayerbotAI* ai) { return new ZaJanalaiShouldBeTankedTrigger(ai); };
            creators["jan'alai spread for flame breath"] = [](PlayerbotAI* ai) { return new ZaJanalaiSpreadForFlameBreathTrigger(ai); };
            creators["jan'alai is fire bombing"] = [](PlayerbotAI* ai) { return new ZaJanalaiIsFireBombingTrigger(ai); };
            creators["jan'alai amani'shi hatchers spawned"] = [](PlayerbotAI* ai) { return new ZaJanalaiAmanishiHatchersSpawnedTrigger(ai); };
            creators["halazzi pulling boss"] = [](PlayerbotAI* ai) { return new ZaPullingBossTrigger(ai, "halazzi pulling boss", za::NPC_HALAZZI); };
            creators["halazzi should be tanked"] = [](PlayerbotAI* ai) { return new ZaHalazziShouldBeTankedTrigger(ai); };
            creators["halazzi spirit lynx has appeared"] = [](PlayerbotAI* ai) { return new ZaHalazziSpiritLynxHasAppearedTrigger(ai); };
            creators["halazzi should focus dps"] = [](PlayerbotAI* ai) { return new ZaHalazziShouldFocusDpsTrigger(ai); };
            creators["hex lord malacrass pulling boss"] = [](PlayerbotAI* ai) { return new ZaPullingBossTrigger(ai, "hex lord malacrass pulling boss", za::NPC_HEX_LORD_MALACRASS); };
            creators["hex lord malacrass should prioritize adds"] = [](PlayerbotAI* ai) { return new ZaHexLordMalacrassShouldPrioritizeAddsTrigger(ai); };
            creators["hex lord malacrass channeling whirlwind"] = [](PlayerbotAI* ai) { return new ZaHexLordMalacrassChannelingWhirlwindTrigger(ai); };
            creators["hex lord malacrass freezing trap placed"] = [](PlayerbotAI* ai) { return new ZaHexLordMalacrassFreezingTrapPlacedTrigger(ai); };
            creators["zul'jin pulling boss"] = [](PlayerbotAI* ai) { return new ZaPullingBossTrigger(ai, "zul'jin pulling boss", za::NPC_ZULJIN); };
            creators["zul'jin should be tanked"] = [](PlayerbotAI* ai) { return new ZaZuljinShouldBeTankedTrigger(ai); };
            creators["zul'jin channeling whirlwind in troll form"] = [](PlayerbotAI* ai) { return new ZaZuljinChannelingWhirlwindInTrollFormTrigger(ai); };
            creators["zul'jin creeping paralysis in bear form"] = [](PlayerbotAI* ai) { return new ZaZuljinCreepingParalysisInBearFormTrigger(ai); };
            creators["zul'jin summoning cyclones in eagle form"] = [](PlayerbotAI* ai) { return new ZaZuljinSummoningCyclonesInEagleFormTrigger(ai); };
            creators["zul'jin spread for dragonhawk aoe"] = [](PlayerbotAI* ai) { return new ZaZuljinSpreadForDragonhawkAoeTrigger(ai); };
        }
    };

    class ZaValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        ZaValueContext()
        {
            creators["jan'alai fire bombs"] = [](PlayerbotAI* ai) { return new ZaJanalaiFireBombsValue(ai); };
            creators["hex lord malacrass freezing trap"] = [](PlayerbotAI* ai) { return new ZaHexLordMalacrassFreezingTrapValue(ai); };
            creators["akil'zon electrical storm timer"] = [](PlayerbotAI* ai) { return new ZaAkilzonStormTimerValue(ai); };
            creators["akil'zon last electrical storm"] = [](PlayerbotAI* ai) { return new ZaAkilzonLastStormValue(ai); };
        }
    };
}
