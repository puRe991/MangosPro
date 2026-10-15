#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Gruul's Lair (map 565) - strategy "boss gruul"
namespace ai
{
    class GruulStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        GruulStrategyContext()
        {
        }
    };

    class GruulActionContext : public NamedObjectContext<Action>
    {
    public:
        GruulActionContext()
        {
        }
    };

    class GruulTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        GruulTriggerContext()
        {
        }
    };

    class GruulValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        GruulValueContext()
        {
        }
    };
}
