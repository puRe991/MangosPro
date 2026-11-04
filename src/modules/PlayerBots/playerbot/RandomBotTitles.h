#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ai
{
    // Titles a TBC random bot can carry (CharTitles.dbc 2.4.3 ids), kept free of game types for the offline tests.
    // Every roll is derived from the bot guid, so running it again after a level up or a relog gives the same
    // titles and only adds the ones the new level unlocks.
    namespace titles
    {
        enum TitleId : uint32_t
        {
            TITLE_PRIVATE                   = 1,    // Alliance honor ranks 1-14 (Private .. Grand Marshal)
            HORDE_RANK_OFFSET               = 14,   // Horde ranks 15-28 (Scout .. High Warlord)
            TITLE_GLADIATOR                 = 42,
            TITLE_DUELIST                   = 43,
            TITLE_RIVAL                     = 44,
            TITLE_CHALLENGER                = 45,
            TITLE_SCARAB_LORD               = 46,
            TITLE_CONQUEROR                 = 47,
            TITLE_JUSTICAR                  = 48,
            TITLE_CHAMPION_OF_THE_NAARU     = 53,
            TITLE_MERCILESS_GLADIATOR       = 62,
            TITLE_OF_THE_SHATTERED_SUN      = 63,
            TITLE_HAND_OF_ADAL              = 64,
            TITLE_VENGEFUL_GLADIATOR        = 71,
        };

        inline uint32_t Hash(uint32_t guid, uint32_t salt)
        {
            uint32_t h = guid * 2654435761u ^ (salt + 0x9E3779B9u) * 2246822519u;
            h ^= h >> 15;
            h *= 2246822519u;
            h ^= h >> 13;
            h *= 3266489917u;
            h ^= h >> 16;
            return h;
        }

        // Per-mille roll in [0, 1000) for one bot and one decision
        inline uint32_t Roll(uint32_t guid, uint32_t salt) { return Hash(guid, salt) % 1000; }

        // Highest rank reached in the old honor system (1 Private .. 14 Grand Marshal), low ranks far more common
        inline uint32_t LegacyRank(uint32_t roll)
        {
            static const uint32_t perMille[14] = { 180, 150, 130, 110, 90, 80, 70, 60, 45, 35, 25, 15, 7, 3 };
            for (uint32_t rank = 0; rank < 14; ++rank)
            {
                if (roll < perMille[rank])
                    return rank + 1;
                roll -= perMille[rank];
            }
            return 1;
        }

        inline std::vector<uint32_t> RollTitles(uint32_t guid, uint32_t level, bool alliance)
        {
            std::vector<uint32_t> result;
            if (level < 60)
                return result;

            // Left over from 1.x: the old honor ranks, the AQ gong, exalted with all three battleground factions
            if (Roll(guid, 1) < 200)
                result.push_back(LegacyRank(Roll(guid, 2)) + (alliance ? 0 : HORDE_RANK_OFFSET));
            if (Roll(guid, 3) < 3)
                result.push_back(TITLE_SCARAB_LORD);
            if (Roll(guid, 4) < 60)
                result.push_back(alliance ? TITLE_JUSTICAR : TITLE_CONQUEROR);

            if (level < 70)
                return result;

            // Arena ladder: at most one of the season titles, rarer the higher it is
            const uint32_t arena = Roll(guid, 5);
            if (arena < 8)
                result.push_back(TITLE_GLADIATOR);
            else if (arena < 33)
                result.push_back(TITLE_DUELIST);
            else if (arena < 83)
                result.push_back(TITLE_RIVAL);
            else if (arena < 183)
                result.push_back(TITLE_CHALLENGER);
            if (Roll(guid, 6) < 5)
                result.push_back(TITLE_MERCILESS_GLADIATOR);
            if (Roll(guid, 7) < 5)
                result.push_back(TITLE_VENGEFUL_GLADIATOR);

            // Tempest Keep attunement, the Black Temple attunement on top of it, exalted with the Shattered Sun
            if (Roll(guid, 8) < 80)
            {
                result.push_back(TITLE_CHAMPION_OF_THE_NAARU);
                if (Roll(guid, 9) < 400)
                    result.push_back(TITLE_HAND_OF_ADAL);
            }
            if (Roll(guid, 10) < 100)
                result.push_back(TITLE_OF_THE_SHATTERED_SUN);

            return result;
        }

        // Most bots with titles show one of them, picked from the titles they know
        inline bool ShowsTitle(uint32_t guid) { return Roll(guid, 11) < 750; }
        inline size_t ShownIndex(uint32_t guid, size_t count) { return count ? Hash(guid, 12) % count : 0; }
    }
}
