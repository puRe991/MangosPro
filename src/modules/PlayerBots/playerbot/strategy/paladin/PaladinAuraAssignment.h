#pragma once

#include <string>
#include <vector>

class PlayerbotAI;

namespace ai
{
    // Coordinates the auras of the paladins in a raid group (sub group):
    //  - "auras auto": every bot paladin of a sub group takes a different aura that fits the group
    //    (tanks: devotion, melee: sanctity / retribution, casters: concentration)
    //  - on bosses of the TBC raid encounter table one paladin per sub group switches to the
    //    resistance aura of the encounter, unless the paladin has a manual aura strategy
    namespace paladinaura
    {
        // All manual aura strategies ("aura devotion", ...) and the aura spell they keep up
        const std::vector<std::pair<std::string, std::string>>& ManualAuraStrategies();

        // Manual aura strategy the bot uses, empty if none
        std::string GetManualAuraStrategy(PlayerbotAI* ai);

        // Aura this bot should keep up, empty if nothing is assigned
        std::string GetAssignedAura(PlayerbotAI* ai);

        // Own aura the bot currently keeps up, empty if none
        std::string GetActiveAura(PlayerbotAI* ai);
    }
}
