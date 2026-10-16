// Ported from mod-playerbots (src/Ai/Raid/Mag, GPL v2+) and adapted for this core.
#pragma once

#include "playerbot/strategy/Value.h"
#include "MagtheridonHelpers.h"

namespace ai
{
    class MagtheridonBurningAbyssalsValue : public ObjectGuidListCalculatedValue
    {
    public:
        MagtheridonBurningAbyssalsValue(PlayerbotAI* ai) : ObjectGuidListCalculatedValue(ai, "magtheridon burning abyssals", 1) {}

    protected:
        std::list<ObjectGuid> Calculate() override { return magtheridon::FindBurningAbyssalGuids(bot); }
    };

    // Living Hellfire Channelers (replaces the AzerothCore database guid lookups)
    class MagtheridonHellfireChannelersValue : public ObjectGuidListCalculatedValue
    {
    public:
        MagtheridonHellfireChannelersValue(PlayerbotAI* ai) : ObjectGuidListCalculatedValue(ai, "magtheridon hellfire channelers", 1) {}

    protected:
        std::list<ObjectGuid> Calculate() override { return magtheridon::FindChannelerGuids(bot); }
    };
}
