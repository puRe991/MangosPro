#pragma once
#include "DungeonBossStrategy.h"

// Classic and TBC dungeons share the existing encounter engine.
namespace ai
{
    class TbcDungeonStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        TbcDungeonStrategyContext()
        {
            for (const auto& entry : GetDungeonBossMaps())
            {
                const std::string name = entry.second;
                creators[name] = [name](PlayerbotAI* ai) { return new DungeonBossStrategy(ai, name); };
            }
        }
    };

    class TbcDungeonActionContext : public NamedObjectContext<Action>
    {
    public:
        TbcDungeonActionContext()
        {
            creators["dungeon boss special"] = [](PlayerbotAI* ai) { return new DungeonBossSpecialAction(ai); };
        }
    };

    class TbcDungeonTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        TbcDungeonTriggerContext()
        {
            creators["dungeon boss special"] = [](PlayerbotAI* ai) { return new DungeonBossSpecialTrigger(ai); };
        }
    };

    class TbcDungeonValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        TbcDungeonValueContext()
        {
        }
    };
}
