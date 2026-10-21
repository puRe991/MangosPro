#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "MagtheridonStrategy.h"
#include "MagtheridonTriggers.h"
#include "MagtheridonActions.h"
#include "MagtheridonValues.h"

// Magtheridon's Lair (map 544) - strategy "boss magtheridon"
// Ported from mod-playerbots (src/Ai/Raid/Mag, GPL v2+) and adapted for this core.
namespace ai
{
    class MagtheridonStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        MagtheridonStrategyContext()
        {
            creators["boss magtheridon"] = [](PlayerbotAI* ai) { return new MagtheridonStrategy(ai); };
        }
    };

    class MagtheridonActionContext : public NamedObjectContext<Action>
    {
    public:
        MagtheridonActionContext()
        {
            creators["magtheridon reset encounter states"] = [](PlayerbotAI* ai) { return new MagtheridonResetEncounterStatesAction(ai); };
            creators["magtheridon main tank attack first three channelers"] = [](PlayerbotAI* ai) { return new MagtheridonMainTankAttackFirstThreeChannelersAction(ai); };
            creators["magtheridon assist tanks attack last two channelers"] = [](PlayerbotAI* ai) { return new MagtheridonAssistTanksAttackLastTwoChannelersAction(ai); };
            creators["magtheridon misdirect hellfire channelers to main tank"] = [](PlayerbotAI* ai) { return new MagtheridonMisdirectHellfireChannelersToMainTankAction(ai); };
            creators["magtheridon assign dps priority"] = [](PlayerbotAI* ai) { return new MagtheridonAssignDpsPriorityAction(ai); };
            creators["magtheridon warlock cc burning abyssal"] = [](PlayerbotAI* ai) { return new MagtheridonWarlockCcBurningAbyssalAction(ai); };
            creators["magtheridon interrupt channeler"] = [](PlayerbotAI* ai) { return new MagtheridonInterruptChannelerAction(ai); };
            creators["magtheridon main tank position boss"] = [](PlayerbotAI* ai) { return new MagtheridonMainTankPositionBossAction(ai); };
            creators["magtheridon spread ranged"] = [](PlayerbotAI* ai) { return new MagtheridonSpreadRangedAction(ai); };
            creators["magtheridon move out of debris"] = [](PlayerbotAI* ai) { return new MagtheridonMoveOutOfDebrisAction(ai); };
            creators["magtheridon use manticron cube"] = [](PlayerbotAI* ai) { return new MagtheridonUseManticronCubeAction(ai); };
            creators["magtheridon update timers and assignments"] = [](PlayerbotAI* ai) { return new MagtheridonUpdateTimersAndAssignmentsAction(ai); };
        }
    };

    class MagtheridonTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        MagtheridonTriggerContext()
        {
            creators["magtheridon no encounter in progress"] = [](PlayerbotAI* ai) { return new MagtheridonNoEncounterInProgressTrigger(ai); };
            creators["magtheridon main tank should tank channelers"] = [](PlayerbotAI* ai) { return new MagtheridonMainTankShouldTankChannelersTrigger(ai); };
            creators["magtheridon assist tanks should tank channelers"] = [](PlayerbotAI* ai) { return new MagtheridonAssistTanksShouldTankChannelersTrigger(ai); };
            creators["magtheridon pulling west and east channelers"] = [](PlayerbotAI* ai) { return new MagtheridonPullingWestAndEastChannelersTrigger(ai); };
            creators["magtheridon determining kill order"] = [](PlayerbotAI* ai) { return new MagtheridonDeterminingKillOrderTrigger(ai); };
            creators["magtheridon burning abyssal spawned"] = [](PlayerbotAI* ai) { return new MagtheridonBurningAbyssalSpawnedTrigger(ai); };
            creators["magtheridon channeler needs interrupt"] = [](PlayerbotAI* ai) { return new MagtheridonChannelerNeedsInterruptTrigger(ai); };
            creators["magtheridon should be tanked"] = [](PlayerbotAI* ai) { return new MagtheridonShouldBeTankedTrigger(ai); };
            creators["magtheridon should spread ranged"] = [](PlayerbotAI* ai) { return new MagtheridonShouldSpreadRangedTrigger(ai); };
            creators["magtheridon standing in debris"] = [](PlayerbotAI* ai) { return new MagtheridonStandingInDebrisTrigger(ai); };
            creators["magtheridon incoming blast nova"] = [](PlayerbotAI* ai) { return new MagtheridonIncomingBlastNovaTrigger(ai); };
            creators["magtheridon should manage timers and assignments"] = [](PlayerbotAI* ai) { return new MagtheridonShouldManageTimersAndAssignmentsTrigger(ai); };
        }
    };

    class MagtheridonValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        MagtheridonValueContext()
        {
            creators["magtheridon burning abyssals"] = [](PlayerbotAI* ai) { return new MagtheridonBurningAbyssalsValue(ai); };
            creators["magtheridon hellfire channelers"] = [](PlayerbotAI* ai) { return new MagtheridonHellfireChannelersValue(ai); };
        }
    };
}
