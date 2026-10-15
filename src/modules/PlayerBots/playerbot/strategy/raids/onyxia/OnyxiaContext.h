#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Onyxia's Lair (map 249) - strategy "boss onyxia"
namespace ai
{
    class OnyxiaStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        OnyxiaStrategyContext()
        {
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
