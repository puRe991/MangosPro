// Karazhan values for the boss tactics ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara) and adapted for this core.
// mod-playerbots looks up every unit with the "find target" value; here one cached grid search per bot
// collects all creatures the Karazhan tactics care about.
#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    // Alive Karazhan boss / add creatures around the bot (recalculated at most once per second)
    class KarazhanNearbyCreaturesValue : public ObjectGuidListCalculatedValue
    {
    public:
        KarazhanNearbyCreaturesValue(PlayerbotAI* ai) : ObjectGuidListCalculatedValue(ai, "karazhan nearby creatures", 1) {}
        std::list<ObjectGuid> Calculate() override;
    };
}
