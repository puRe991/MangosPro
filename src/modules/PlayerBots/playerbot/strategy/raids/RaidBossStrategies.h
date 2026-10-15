#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "playerbot/strategy/Trigger.h"
#include "playerbot/strategy/Action.h"
#include <string>

namespace ai
{
    // Every raid / dungeon with a dedicated boss strategy (strategy/raids/<raid>) has one strategy
    // which is added automatically while the bot is on that map (through the "dungeon" strategy)
    // and removed again when it leaves.
    class RaidBossMapStrategies
    {
    public:
        // Strategy name of the map, empty if the map has no boss strategy
        static std::string GetStrategy(uint32 mapId);
        static const std::vector<std::pair<uint32, std::string>>& GetAll();
    };

    // The boss strategy of the current map is missing, or one of another map is still active
    class RaidBossMapStrategyTrigger : public Trigger
    {
    public:
        RaidBossMapStrategyTrigger(PlayerbotAI* ai) : Trigger(ai, "raid boss map strategy", 5) {}
        bool IsActive() override;
    };

    class UpdateRaidBossMapStrategyAction : public Action
    {
    public:
        UpdateRaidBossMapStrategyAction(PlayerbotAI* ai) : Action(ai, "update raid boss map strategy") {}
        bool Execute(Event& event) override;
        bool isUsefulWhenStunned() override { return true; }
    };

    class RaidBossMapStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        RaidBossMapStrategyContext() {}
    };

    class RaidBossMapActionContext : public NamedObjectContext<Action>
    {
    public:
        RaidBossMapActionContext()
        {
            creators["update raid boss map strategy"] = [](PlayerbotAI* ai) { return new UpdateRaidBossMapStrategyAction(ai); };
        }
    };

    class RaidBossMapTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        RaidBossMapTriggerContext()
        {
            creators["raid boss map strategy"] = [](PlayerbotAI* ai) { return new RaidBossMapStrategyTrigger(ai); };
        }
    };
}
