// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/TK/TKValueContext.h) and adapted for this core.
#pragma once
#include "playerbot/strategy/Value.h"
#include "TkHelpers.h"

namespace ai
{
    // Alive Flame Patches of Al'ar around the bot
    class TkFlamePatchesValue : public CalculatedValue<std::list<ObjectGuid>>
    {
    public:
        TkFlamePatchesValue(PlayerbotAI* ai) : CalculatedValue<std::list<ObjectGuid>>(ai, "tk flame patches", 1) {}

    protected:
        std::list<ObjectGuid> Calculate() override { return tk::FindFlamePatchGuids(bot); }
    };

    // Kael'thas legendary weapons in the room (alive and dead, the dead ones are looted)
    class TkLegendaryWeaponsValue : public CalculatedValue<std::list<ObjectGuid>>
    {
    public:
        TkLegendaryWeaponsValue(PlayerbotAI* ai) : CalculatedValue<std::list<ObjectGuid>>(ai, "tk legendary weapons", 1) {}

    protected:
        std::list<ObjectGuid> Calculate() override { return tk::FindLegendaryWeaponGuids(bot); }
    };
}
