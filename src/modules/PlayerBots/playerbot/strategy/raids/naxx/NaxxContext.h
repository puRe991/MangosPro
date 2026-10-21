#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "playerbot/strategy/raids/dungeons/DungeonBossStrategy.h"

// Naxxramas (map 533) - strategy "boss naxxramas"
namespace ai
{
    class NaxxStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        NaxxStrategyContext()
        {
            creators["boss naxxramas"] = [](PlayerbotAI* ai) { return new ClassicRaidBossStrategy(ai, "boss naxxramas"); };
        }
    };

    class NaxxActionContext : public NamedObjectContext<Action>
    {
    public:
        NaxxActionContext()
        {
        }
    };

    class NaxxTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        NaxxTriggerContext()
        {
        }
    };

    class NaxxValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        NaxxValueContext()
        {
        }
    };
}
