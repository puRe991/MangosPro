#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Karazhan (map 532) - strategy "boss karazhan"
namespace ai
{
    class KarazhanStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        KarazhanStrategyContext()
        {
        }
    };

    class KarazhanActionContext : public NamedObjectContext<Action>
    {
    public:
        KarazhanActionContext()
        {
        }
    };

    class KarazhanTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        KarazhanTriggerContext()
        {
        }
    };

    class KarazhanValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        KarazhanValueContext()
        {
        }
    };
}
