#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "playerbot/strategy/raids/dungeons/DungeonBossStrategy.h"

// Onyxia's Lair (map 249) - strategy "boss onyxia"
namespace ai
{
    class OnyxiaStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        OnyxiaStrategyContext()
        {
            creators["boss onyxia"] = [](PlayerbotAI* ai) { return new ClassicRaidBossStrategy(ai, "boss onyxia"); };
        }
    };

    class OnyxiaActionContext : public NamedObjectContext<Action>
    {
    public:
        OnyxiaActionContext()
        {
        }
    };

    class OnyxiaTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        OnyxiaTriggerContext()
        {
        }
    };

    class OnyxiaValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        OnyxiaValueContext()
        {
        }
    };
}
