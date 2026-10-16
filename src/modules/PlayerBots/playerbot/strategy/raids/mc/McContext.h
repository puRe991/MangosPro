#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Molten Core (map 409) - strategy "boss molten core"
namespace ai
{
    class McStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        McStrategyContext()
        {
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
