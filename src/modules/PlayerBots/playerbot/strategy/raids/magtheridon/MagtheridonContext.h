#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Magtheridon's Lair (map 544) - strategy "boss magtheridon"
namespace ai
{
    class MagtheridonStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        MagtheridonStrategyContext()
        {
        }
    };

    class MagtheridonActionContext : public NamedObjectContext<Action>
    {
    public:
        MagtheridonActionContext()
        {
        }
    };

    class MagtheridonTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        MagtheridonTriggerContext()
        {
        }
    };

    class MagtheridonValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        MagtheridonValueContext()
        {
        }
    };
}
