#include "../../src/modules/PlayerBots/playerbot/strategy/raids/bt/BtGurtoggRules.h"
#include <algorithm>
#include <cassert>
#include <iostream>

using namespace ai::bt::gurtogg;

namespace
{
    // Bloodboil lasts 24 seconds (spell tooltip), Gurtogg casts it every 11 seconds (boss_bloodboil.cpp)
    constexpr int32_t DEBUFF_MS = 24000;
    constexpr int32_t CAST_INTERVAL_MS = 11000;

    bool Contains(const std::vector<size_t>& list, size_t value)
    {
        return std::find(list.begin(), list.end(), value) != list.end();
    }

    // Plays a fight: every 11 seconds the current soakers get a stack (which refreshes the debuff),
    // debuffs run out after 24 seconds. Returns the highest stack any player reached and the spread
    // between the most and the least hit player.
    uint32_t Simulate(size_t players, int casts, int& spread)
    {
        std::vector<SoakCandidate> raid(players);
        std::vector<int> hits(players, 0);
        for (size_t i = 0; i < players; ++i)
            raid[i].key = 100 + i;

        uint32_t highest = 0;
        for (int cast = 0; cast < casts; ++cast)
        {
            for (size_t index : PickSoakers(raid))
            {
                raid[index].stacks = std::min<uint32_t>(raid[index].stacks + 1, 5);
                raid[index].remainingMs = DEBUFF_MS;
                highest = std::max(highest, raid[index].stacks);
                ++hits[index];
            }

            for (SoakCandidate& player : raid)
            {
                player.remainingMs -= CAST_INTERVAL_MS;
                if (player.remainingMs <= 0)
                {
                    player.remainingMs = 0;
                    player.stacks = 0;
                }
            }
        }

        spread = *std::max_element(hits.begin(), hits.end()) - *std::min_element(hits.begin(), hits.end());
        return highest;
    }
}

int main()
{
    // Without any debuff the fixed order decides, so all bots pick the same five
    std::vector<SoakCandidate> fresh(8);
    for (size_t i = 0; i < fresh.size(); ++i)
        fresh[i].key = 50 - i;
    std::vector<size_t> soakers = PickSoakers(fresh);
    assert(soakers.size() == BLOODBOIL_TARGETS);
    assert(Contains(soakers, 7) && Contains(soakers, 3) && !Contains(soakers, 2));

    // Right after a Bloodboil the hit players step back and the fresh ones take the next one
    std::vector<SoakCandidate> afterHit(10);
    for (size_t i = 0; i < afterHit.size(); ++i)
    {
        afterHit[i].key = i;
        if (i < 5)
        {
            afterHit[i].stacks = 1;
            afterHit[i].remainingMs = DEBUFF_MS;
        }
    }
    soakers = PickSoakers(afterHit);
    for (size_t i = 5; i < 10; ++i)
        assert(Contains(soakers, i));

    // With equal stacks the debuff that runs out first goes back in
    std::vector<SoakCandidate> allHit(6);
    for (size_t i = 0; i < allHit.size(); ++i)
    {
        allHit[i].key = i;
        allHit[i].stacks = 1;
        allHit[i].remainingMs = 20000 - int32_t(i) * 1000;
    }
    soakers = PickSoakers(allHit);
    assert(!Contains(soakers, 0) && Contains(soakers, 5));

    // Fewer players than targets: everybody soaks
    assert(PickSoakers(std::vector<SoakCandidate>(3)).size() == 3);

    // 15 ranged: three groups of five, every stack runs out before the next hit
    int spread = 0;
    assert(Simulate(15, 30, spread) == 1 && spread == 0);
    // 10 ranged: two groups alternate every 22 seconds, inside the 24 second debuff, so the stacks grow
    // with any rotation. The rule still shares the hits evenly.
    assert(Simulate(10, 30, spread) == 5 && spread == 0);
    // Uneven count: players whose debuff ran out go back in more often, which keeps the stacks low
    assert(Simulate(12, 30, spread) <= 2);

    std::cout << "PASS: Bloodboil soakers rotate on the debuff, fresh players first\n";
}
