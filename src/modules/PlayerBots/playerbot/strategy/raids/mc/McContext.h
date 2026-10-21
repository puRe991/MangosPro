#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "playerbot/strategy/raids/dungeons/DungeonBossStrategy.h"

// Molten Core (map 409) - strategy "boss molten core"
namespace ai
{
    class McStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        McStrategyContext()
        {
            creators["boss molten core"] = [](PlayerbotAI* ai) { return new ClassicRaidBossStrategy(ai, "boss molten core"); };
        }
    };

    class McActionContext : public NamedObjectContext<Action>
    {
    public:
        McActionContext()
        {
        }
    };

    class McTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        McTriggerContext()
        {
        }
    };

    class McValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        McValueContext()
        {
        }
    };
}
