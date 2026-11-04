#include "../../src/modules/PlayerBots/playerbot/RandomBotTitles.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

using namespace ai::titles;

namespace
{
    // Ids present in the 2.4.3 CharTitles.dbc
    bool IsTbcTitle(uint32_t id)
    {
        return (id >= 1 && id <= 28) || (id >= 42 && id <= 48) || id == 53 || (id >= 62 && id <= 64) || id == 71;
    }

    bool Has(const std::vector<uint32_t>& titles, uint32_t id)
    {
        return std::find(titles.begin(), titles.end(), id) != titles.end();
    }
}

int main()
{
    // Rank table covers the whole per-mille range and never leaves 1..14
    uint32_t rankCount[15] = {};
    for (uint32_t roll = 0; roll < 1000; ++roll)
    {
        const uint32_t rank = LegacyRank(roll);
        assert(rank >= 1 && rank <= 14);
        ++rankCount[rank];
    }
    assert(rankCount[1] == 180 && rankCount[14] == 3);

    const uint32_t bots = 20000;
    uint32_t withTitle = 0, shown = 0, gladiators = 0, naaru = 0, adal = 0;
    for (uint32_t guid = 1; guid <= bots; ++guid)
    {
        const bool alliance = guid % 2;

        // Same bot, same titles: relogs and level ups do not reshuffle them
        const std::vector<uint32_t> titles = RollTitles(guid, 70, alliance);
        assert(titles == RollTitles(guid, 70, alliance));

        // Nothing below 60, and 60s only get what a level 70 bot of the same guid also has
        assert(RollTitles(guid, 59, alliance).empty());
        for (uint32_t id : RollTitles(guid, 65, alliance))
            assert(Has(titles, id));

        uint32_t arenaLadder = 0;
        for (uint32_t id : titles)
        {
            assert(IsTbcTitle(id));
            assert(std::count(titles.begin(), titles.end(), id) == 1);

            // Faction titles match the bot's side
            if (alliance)
            {
                assert(id <= 14 || id > 14 + HORDE_RANK_OFFSET);
                assert(id != TITLE_CONQUEROR);
            }
            else
            {
                assert(id > 14);
                assert(id != TITLE_JUSTICAR);
            }

            if (id >= TITLE_GLADIATOR && id <= TITLE_CHALLENGER)
                ++arenaLadder;
        }
        assert(arenaLadder <= 1);
        assert(!Has(titles, TITLE_HAND_OF_ADAL) || Has(titles, TITLE_CHAMPION_OF_THE_NAARU));

        if (!titles.empty())
        {
            ++withTitle;
            if (ShowsTitle(guid))
                ++shown;
            assert(ShownIndex(guid, titles.size()) < titles.size());
        }
        gladiators += Has(titles, TITLE_GLADIATOR);
        naaru += Has(titles, TITLE_CHAMPION_OF_THE_NAARU);
        adal += Has(titles, TITLE_HAND_OF_ADAL);
    }

    // Roughly half of the level 70s know a title, most of those wear one, Gladiator stays rare
    const double titled = double(withTitle) / bots;
    std::cout << "level 70 with a title: " << titled * 100.0 << "%, shown: " << double(shown) / bots * 100.0
              << "%, gladiators: " << gladiators << ", naaru: " << naaru << ", adal: " << adal << "\n";
    assert(titled > 0.40 && titled < 0.60);
    assert(double(shown) / withTitle > 0.70 && double(shown) / withTitle < 0.80);
    assert(gladiators > bots * 4 / 1000 && gladiators < bots * 12 / 1000);
    assert(adal > naaru / 4 && adal < naaru / 2 + naaru / 10);

    std::cout << "random_bot_titles_test passed\n";
    return 0;
}
