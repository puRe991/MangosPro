#pragma once
#include "NamedObjectContext.h"
#include "generic/TbcRaidStrategy.h"
#include "triggers/TbcRaidTriggers.h"
#include "actions/TbcRaidActions.h"

namespace ai
{
    // Objects of the Burning Crusade raid tactics ("tbc raid" strategy)
    class TbcRaidStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        TbcRaidStrategyContext()
        {
            creators["tbc raid"] = [](PlayerbotAI* ai) { return new TbcRaidStrategy(ai); };
        }
    };

    class TbcRaidTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        TbcRaidTriggerContext()
        {
            creators["enter tbc raid"] = [](PlayerbotAI* ai) { return new TbcRaidEnterTrigger(ai); };
            creators["leave tbc raid"] = [](PlayerbotAI* ai) { return new TbcRaidLeaveTrigger(ai); };
            creators["tbc raid in hazard"] = [](PlayerbotAI* ai) { return new TbcRaidInHazardTrigger(ai); };
            creators["tbc raid debuff run"] = [](PlayerbotAI* ai) { return new TbcRaidDebuffRunTrigger(ai); };
            creators["tbc raid spread"] = [](PlayerbotAI* ai) { return new TbcRaidSpreadTrigger(ai); };
            creators["tbc raid gather"] = [](PlayerbotAI* ai) { return new TbcRaidGatherTrigger(ai); };
            creators["tbc raid in front of boss"] = [](PlayerbotAI* ai) { return new TbcRaidInFrontOfBossTrigger(ai); };
            creators["tbc raid priority target"] = [](PlayerbotAI* ai) { return new TbcRaidPriorityTargetTrigger(ai); };
            creators["tbc raid taunt"] = [](PlayerbotAI* ai) { return new TbcRaidTauntTrigger(ai); };
            creators["tbc raid use cube"] = [](PlayerbotAI* ai) { return new TbcRaidUseCubeTrigger(ai); };
            creators["tbc raid use spine"] = [](PlayerbotAI* ai) { return new TbcRaidUseSpineTrigger(ai); };
        }
    };

    class TbcRaidActionContext : public NamedObjectContext<Action>
    {
    public:
        TbcRaidActionContext()
        {
            creators["enable tbc raid strategy"] = [](PlayerbotAI* ai) { return new TbcRaidEnableStrategyAction(ai); };
            creators["disable tbc raid strategy"] = [](PlayerbotAI* ai) { return new TbcRaidDisableStrategyAction(ai); };
            creators["tbc raid leave hazard"] = [](PlayerbotAI* ai) { return new TbcRaidLeaveHazardAction(ai); };
            creators["tbc raid spread"] = [](PlayerbotAI* ai) { return new TbcRaidSpreadAction(ai); };
            creators["tbc raid gather"] = [](PlayerbotAI* ai) { return new TbcRaidGatherAction(ai); };
            creators["tbc raid leave front"] = [](PlayerbotAI* ai) { return new TbcRaidLeaveFrontAction(ai); };
            creators["tbc raid attack priority target"] = [](PlayerbotAI* ai) { return new TbcRaidAttackPriorityTargetAction(ai); };
            creators["tbc raid taunt"] = [](PlayerbotAI* ai) { return new TbcRaidTauntAction(ai); };
            creators["tbc raid use cube"] = [](PlayerbotAI* ai) { return new TbcRaidUseCubeAction(ai); };
            creators["tbc raid move to spine"] = [](PlayerbotAI* ai) { return new TbcRaidMoveToSpineAction(ai); };
            creators["tbc raid use spine"] = [](PlayerbotAI* ai) { return new TbcRaidUseSpineAction(ai); };
        }
    };
}
