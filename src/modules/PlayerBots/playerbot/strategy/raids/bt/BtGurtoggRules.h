#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace ai
{
    namespace bt
    {
        // Gurtogg Bloodboil soaking rules, kept free of game types for the offline tests
        namespace gurtogg
        {
            // Bloodboil hits the 5 players furthest away every 11 seconds and stacks
            constexpr size_t BLOODBOIL_TARGETS = 5;

            struct SoakCandidate
            {
                uint32_t stacks = 0;        // Bloodboil stacks on the player
                int32_t remainingMs = 0;    // time left on the debuff, 0 without it
                uint64_t key = 0;           // fixed order (guid) so every bot picks the same soakers
            };

            // Fresh players soak first, then the debuff that runs out first. The order only changes when
            // a Bloodboil lands or a debuff runs out, so the soakers swap right after a hit instead of on
            // a timer that drifts against the 11 second cast.
            inline bool SoaksBefore(const SoakCandidate& a, const SoakCandidate& b)
            {
                if (a.stacks != b.stacks)
                    return a.stacks < b.stacks;
                if (a.remainingMs != b.remainingMs)
                    return a.remainingMs < b.remainingMs;
                return a.key < b.key;
            }

            // Indices of the candidates that stand at the far position
            inline std::vector<size_t> PickSoakers(const std::vector<SoakCandidate>& candidates, size_t count = BLOODBOIL_TARGETS)
            {
                std::vector<size_t> order(candidates.size());
                for (size_t i = 0; i < order.size(); ++i)
                    order[i] = i;

                std::sort(order.begin(), order.end(), [&](size_t a, size_t b) { return SoaksBefore(candidates[a], candidates[b]); });
                if (order.size() > count)
                    order.resize(count);

                return order;
            }
        }
    }
}
