#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Tempest Keep: The Eye (map 550) - strategy "boss tempest keep"
namespace ai
{
    class TkStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        TkStrategyContext()
        {
        }
    };

    class TkActionContext : public NamedObjectContext<Action>
    {
    public:
        TkActionContext()
        {
        }
    };

    class TkTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        TkTriggerContext()
        {
        }
    };

    class TkValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        TkValueContext()
        {
        }
    };
}
