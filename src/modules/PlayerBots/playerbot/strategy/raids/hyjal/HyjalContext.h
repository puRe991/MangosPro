#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Hyjal Summit (map 534) - strategy "boss hyjal"
namespace ai
{
    class HyjalStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        HyjalStrategyContext()
        {
        }
    };

    class HyjalActionContext : public NamedObjectContext<Action>
    {
    public:
        HyjalActionContext()
        {
        }
    };

    class HyjalTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        HyjalTriggerContext()
        {
        }
    };

    class HyjalValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        HyjalValueContext()
        {
        }
    };
}
