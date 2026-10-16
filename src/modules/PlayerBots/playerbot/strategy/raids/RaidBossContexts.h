#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

namespace ai
{
    class Strategy;
    class Action;
    class Trigger;
    class UntypedValue;

    // Registers the strategies, actions, triggers and values of all boss strategies in strategy/raids
    void AddRaidBossContexts(NamedObjectContextList<Strategy>& strategies, NamedObjectContextList<Action>& actions,
                             NamedObjectContextList<Trigger>& triggers, NamedObjectContextList<UntypedValue>& values);
}
