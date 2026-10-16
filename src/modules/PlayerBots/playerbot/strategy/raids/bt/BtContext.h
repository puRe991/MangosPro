#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Black Temple (map 564) - strategy "boss black temple"
namespace ai
{
    class BtStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        BtStrategyContext()
        {
        }
    };

    class BtActionContext : public NamedObjectContext<Action>
    {
    public:
        BtActionContext()
        {
        }
    };

    class BtTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        BtTriggerContext()
        {
        }
    };

    class BtValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        BtValueContext()
        {
        }
    };
}
