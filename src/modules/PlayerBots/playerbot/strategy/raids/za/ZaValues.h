// Ported from mod-playerbots (AzerothCore, GPL v2+), src/Ai/Raid/ZA/ZAValueContext.h, and adapted for this core.
#pragma once
#include "playerbot/strategy/Value.h"
#include "ZaHelpers.h"

namespace ai
{
    // Fire Bomb npcs around the bot (cached, the bombs stay a few seconds)
    class ZaJanalaiFireBombsValue : public CalculatedValue<std::list<ObjectGuid>>
    {
    public:
        ZaJanalaiFireBombsValue(PlayerbotAI* ai) : CalculatedValue<std::list<ObjectGuid>>(ai, "jan'alai fire bombs", 1) {}

    protected:
        std::list<ObjectGuid> Calculate() override { return za::FindNearbyFireBombGuids(bot); }
    };

    // Freezing Trap placed by Hex Lord Malacrass near the bot
    class ZaHexLordMalacrassFreezingTrapValue : public ObjectGuidCalculatedValue
    {
    public:
        ZaHexLordMalacrassFreezingTrapValue(PlayerbotAI* ai) : ObjectGuidCalculatedValue(ai, "hex lord malacrass freezing trap", 1) {}

    protected:
        ObjectGuid Calculate() override { return za::FindNearbyFreezingTrapGuid(bot); }
    };

    // Akil'zon: time (ms) the bot saw the boss engaged, 0 = not engaged.
    // mod-playerbots keeps one timer per instance in a global map, here every bot keeps its own
    // (instances update in parallel threads).
    class ZaAkilzonStormTimerValue : public ManualSetValue<uint32>
    {
    public:
        ZaAkilzonStormTimerValue(PlayerbotAI* ai) : ManualSetValue<uint32>(ai, 0, "akil'zon electrical storm timer") {}
    };

    // Akil'zon: time (ms) the last Electrical Storm started, 0 = none yet
    class ZaAkilzonLastStormValue : public ManualSetValue<uint32>
    {
    public:
        ZaAkilzonLastStormValue(PlayerbotAI* ai) : ManualSetValue<uint32>(ai, 0, "akil'zon last electrical storm") {}
    };
}
