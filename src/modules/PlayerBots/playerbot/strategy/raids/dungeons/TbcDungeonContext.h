#pragma once
#include "playerbot/strategy/NamedObjectContext.h"

// Burning Crusade dungeons: Auchenai Crypts (558), The Mechanar (554), Magisters' Terrace (585),
// Hellfire Ramparts (543), Sethekk Halls (556), The Underbog (546)
namespace ai
{
    class TbcDungeonStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        TbcDungeonStrategyContext()
        {
        }
    };

    class TbcDungeonActionContext : public NamedObjectContext<Action>
    {
    public:
        TbcDungeonActionContext()
        {
        }
    };

    class TbcDungeonTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        TbcDungeonTriggerContext()
        {
        }
    };

    class TbcDungeonValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        TbcDungeonValueContext()
        {
        }
    };
}
