// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#pragma once

#include "playerbot/strategy/Value.h"
#include "GruulHelpers.h"

namespace ai
{
    // Olm summons a Wild Fel Stalker every 25-35s; the grid search is cached anyway.
    class HighKingMaulgarWildFelStalkersValue : public ObjectGuidListCalculatedValue
    {
    public:
        HighKingMaulgarWildFelStalkersValue(PlayerbotAI* ai) : ObjectGuidListCalculatedValue(ai, "high king maulgar wild fel stalkers", 1) {}

    protected:
        std::list<ObjectGuid> Calculate() override { return gruul::FindNearbyWildFelStalkerGuids(bot); }
    };

    // Both ogre caster tanks are chosen by iterating the raid.
    class HighKingMaulgarKroshMageTankValue : public ObjectGuidCalculatedValue
    {
    public:
        HighKingMaulgarKroshMageTankValue(PlayerbotAI* ai) : ObjectGuidCalculatedValue(ai, "high king maulgar krosh mage tank", 1) {}

    protected:
        ObjectGuid Calculate() override { return gruul::FindKroshMageTankGuid(bot); }
    };

    class HighKingMaulgarKigglerMoonkinTankValue : public ObjectGuidCalculatedValue
    {
    public:
        HighKingMaulgarKigglerMoonkinTankValue(PlayerbotAI* ai) : ObjectGuidCalculatedValue(ai, "high king maulgar kiggler moonkin tank", 1) {}

    protected:
        ObjectGuid Calculate() override { return gruul::FindKigglerMoonkinTankGuid(bot); }
    };
}
