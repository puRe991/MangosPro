#pragma once

#include <cstdint>

namespace ai
{
    namespace bt
    {
        // High Warlord Naj'entus positioning and Tidal Shield rules, kept free of game types for the offline tests
        namespace najentus
        {
            constexpr float PI = 3.14159265f;

            // Needle Spine splashes 6 yards around each target, ranged slots keep more than that between them
            constexpr float INNER_RING = 20.0f;
            constexpr float OUTER_RING = 30.0f;
            constexpr float MIN_SPACING = 8.0f;

            // Slot of a ranged player on one or two rings around the boss. A second ring is only used once the
            // inner one gets too crowded; both rings then get the same spacing.
            inline void RangedSlot(int index, int count, float& radius, float& angle)
            {
                const int innerCapacity = int(2.0f * PI * INNER_RING / MIN_SPACING);
                int inner = count;
                if (count > innerCapacity)
                    inner = int(float(count) * INNER_RING / (INNER_RING + OUTER_RING) + 0.5f);

                if (index < inner)
                {
                    radius = INNER_RING;
                    angle = 2.0f * PI * float(index) / float(inner);
                    return;
                }

                // Outer ring is shifted by half a slot so the rings never line up
                const int outer = count - inner;
                radius = OUTER_RING;
                angle = 2.0f * PI * (float(index - inner) + 0.5f) / float(outer);
            }

            // Melee split into two groups on both rear flanks (80 degrees off the boss's back), out of the frontal
            // parry arc and far enough apart that one Needle Spine splash cannot hit both
            constexpr float MELEE_FLANK = 80.0f * PI / 180.0f;

            inline float MeleeGroupAngle(int index, float bossOrientation)
            {
                const float back = bossOrientation + PI;
                return index % 2 == 0 ? back + MELEE_FLANK : back - MELEE_FLANK;
            }

            // Both groups sit this far from the boss center, at least as far apart as the splash radius
            inline float MeleeRadius(float bossCombatReach)
            {
                const float radius = bossCombatReach + 1.0f;
                return radius < 3.5f ? 3.5f : radius;
            }

            // Tidal Burst hits the whole raid for 8500 frost: break the shield once the raid is topped up, but do
            // not leave it up much longer since the boss heals 1% per second under it
            constexpr float TOPPED_UP_PCT = 85.0f;
            constexpr int64_t MAX_SHIELD_SECONDS = 10;

            inline bool ThrowSpineNow(float raidHealthPct, int64_t shieldSeconds)
            {
                return raidHealthPct >= TOPPED_UP_PCT || shieldSeconds >= MAX_SHIELD_SECONDS;
            }
        }
    }
}
