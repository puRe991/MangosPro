#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Zul'Aman (map 568) - strategy "boss zul'aman"
namespace ai
{
    class ZaStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        ZaStrategyContext()
        {
        }
    };

    class ZaActionContext : public NamedObjectContext<Action>
    {
    public:
        ZaActionContext()
        {
        }
    };

    class ZaTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        ZaTriggerContext()
        {
        }
    };

    class ZaValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        ZaValueContext()
        {
        }
    };
}
