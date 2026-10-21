#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "playerbot/strategy/raids/dungeons/DungeonBossStrategy.h"

// Blackwing Lair (map 469) - strategy "boss blackwing lair"
namespace ai
{
    class BwlStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        BwlStrategyContext()
        {
            creators["boss blackwing lair"] = [](PlayerbotAI* ai) { return new ClassicRaidBossStrategy(ai, "boss blackwing lair"); };
        }
    };

    class BwlActionContext : public NamedObjectContext<Action>
    {
    public:
        BwlActionContext()
        {
        }
    };

    class BwlTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        BwlTriggerContext()
        {
        }
    };

    class BwlValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        BwlValueContext()
        {
        }
    };
}
