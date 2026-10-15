#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Naxxramas (map 533) - strategy "boss naxxramas"
namespace ai
{
    class NaxxStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        NaxxStrategyContext()
        {
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
