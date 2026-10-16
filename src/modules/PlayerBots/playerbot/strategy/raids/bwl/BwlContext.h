#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Blackwing Lair (map 469) - strategy "boss blackwing lair"
namespace ai
{
    class BwlStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        BwlStrategyContext()
        {
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
