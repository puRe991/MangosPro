#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Ruins of Ahn'Qiraj (map 509) - strategy "boss aq20"
namespace ai
{
    class Aq20StrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        Aq20StrategyContext()
        {
        }
    };

    class Aq20ActionContext : public NamedObjectContext<Action>
    {
    public:
        Aq20ActionContext()
        {
        }
    };

    class Aq20TriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        Aq20TriggerContext()
        {
        }
    };

    class Aq20ValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        Aq20ValueContext()
        {
        }
    };
}
