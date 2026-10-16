// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/Hyjal/HyjalValueContext.h) and adapted for this core.
#pragma once

#include "playerbot/strategy/Value.h"
#include "HyjalHelpers.h"
#include "Util/Timer.h"

namespace ai
{
    // Value recalculated after a number of milliseconds (CalculatedValue works with whole seconds,
    // which is too slow for moving ground effects)
    template<class T>
    class HyjalTimedValue : public UntypedValue, public Value<T>
    {
    public:
        HyjalTimedValue(PlayerbotAI* ai, std::string name, uint32 intervalMs) : UntypedValue(ai, name), intervalMs(intervalMs) {}

        T Get() override
        {
            const uint32 now = WorldTimer::getMSTime();
            if (!calculated || WorldTimer::getMSTimeDiff(lastCalculation, now) >= intervalMs)
            {
                calculated = true;
                lastCalculation = now;
                value = Calculate();
            }

            return value;
        }

        T LazyGet() override { return calculated ? value : Get(); }
        void Set(T newValue) override { value = newValue; }
        void Reset() override { calculated = false; }

    protected:
        virtual T Calculate() = 0;

    private:
        uint32 intervalMs;
        uint32 lastCalculation = 0;
        bool calculated = false;
        T value = T();
    };

    // The Hyjal boss the group is fighting (empty when no encounter is in progress)
    class HyjalBossValue : public HyjalTimedValue<ObjectGuid>
    {
    public:
        HyjalBossValue(PlayerbotAI* ai) : HyjalTimedValue<ObjectGuid>(ai, "hyjal boss", 500) {}

    protected:
        ObjectGuid Calculate() override
        {
            Unit* boss = hyjal::FindEngagedBoss(ai);
            return boss ? boss->GetObjectGuid() : ObjectGuid();
        }
    };

    // Every living Towering Infernal, oldest first
    class HyjalInfernalsValue : public HyjalTimedValue<std::list<ObjectGuid>>
    {
    public:
        HyjalInfernalsValue(PlayerbotAI* ai) : HyjalTimedValue<std::list<ObjectGuid>>(ai, "hyjal infernals", hyjal::INFERNAL_CACHE_INTERVAL) {}

    protected:
        std::list<ObjectGuid> Calculate() override { return hyjal::FindInfernalGuids(bot); }
    };

    // Positions of the dynamic objects of a ground effect around the bot
    class HyjalHazardPositionsValue : public HyjalTimedValue<std::vector<WorldPosition>>
    {
    public:
        HyjalHazardPositionsValue(PlayerbotAI* ai, std::string name, uint32 spellId, float searchRadius)
            : HyjalTimedValue<std::vector<WorldPosition>>(ai, name, hyjal::HAZARD_CACHE_INTERVAL), spellId(spellId), searchRadius(searchRadius) {}

    protected:
        std::vector<WorldPosition> Calculate() override;

    private:
        uint32 spellId;
        float searchRadius;
    };

    // Latch, so the gap between MARK_DANGER_MANA and MARK_REJOIN_MANA does not have bots running
    // back and forth to / from the group
    class KazrogalBelowManaThresholdValue : public ManualSetValue<bool>
    {
    public:
        KazrogalBelowManaThresholdValue(PlayerbotAI* ai) : ManualSetValue<bool>(ai, false, "kaz'rogal below mana threshold") {}
    };

    // The initial ring spread of the ranged is done (movement is free afterwards)
    class HyjalRangedPositionReachedValue : public ManualSetValue<bool>
    {
    public:
        HyjalRangedPositionReachedValue(PlayerbotAI* ai, std::string name) : ManualSetValue<bool>(ai, false, name) {}
    };
}
