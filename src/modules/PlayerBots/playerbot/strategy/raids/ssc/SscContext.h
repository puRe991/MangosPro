#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Serpentshrine Cavern (map 548) - strategy "boss ssc"
namespace ai
{
    class SscStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        SscStrategyContext()
        {
        }
    };

    class SscActionContext : public NamedObjectContext<Action>
    {
    public:
        SscActionContext()
        {
        }
    };

    class SscTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        SscTriggerContext()
        {
        }
    };

    class SscValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        SscValueContext()
        {
        }
    };
}
