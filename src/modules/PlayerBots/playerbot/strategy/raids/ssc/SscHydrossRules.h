#pragma once

#include <cmath>
#include <cstdint>

namespace ai
{
    namespace ssc
    {
        // Hydross the Unstable timing and positions, kept free of game types for the offline tests
        namespace hydross
        {
            // Cleansing Field helper (npc 21934) and the field radius of spell 37935
            constexpr float FIELD_X = -239.715f;
            constexpr float FIELD_Y = -366.440f;
            constexpr float FIELD_RADIUS = 20.0f;
            // The area aura range adds the combined combat reach of helper (3) and boss (5): the boss centre
            // leaves the field at 28 yards, matching the switches measured in the raid logs
            constexpr float SWITCH_DISTANCE = FIELD_RADIUS + 3.0f + 5.0f;

            // The boss stops this far short of a tank who stopped walking (boss reach 5 + player 1.5 + 1.33)
            constexpr float BOSS_TRAIL = 8.0f;
            // The boss centre has to end up at least this far beyond the switch distance
            constexpr float SWITCH_MARGIN = 4.0f;

            // Marks are cast 15, 30, 45 ... seconds after a form change and stack the damage taken
            // (10%, 25%, 50%, 100%, 250%, 500%). The hand-off starts after the 25% mark and the walk (about
            // 5 seconds) is over before the 50% mark: bots have no resistance gear and the tanks died under the
            // 50% mark. The boss script now also retries a transform that collides with a mark cast.
            constexpr int64_t MARK_INTERVAL_SECONDS = 15;
            constexpr int64_t HANDOFF_SECONDS = 33;
            constexpr int64_t HANDOFF_WALK_SECONDS = 6;

            inline float DistanceFromField(float x, float y)
            {
                const float dx = x - FIELD_X;
                const float dy = y - FIELD_Y;
                return std::sqrt(dx * dx + dy * dy);
            }

            // A nature tank spot is good when the boss trailing him stays clearly outside the field
            inline bool IsNatureSpotOutside(float x, float y)
            {
                return DistanceFromField(x, y) - BOSS_TRAIL >= SWITCH_DISTANCE + SWITCH_MARGIN;
            }

            // A frost tank spot is good when the boss trailing him (from any side) is clearly inside
            inline bool IsFrostSpotInside(float x, float y)
            {
                return DistanceFromField(x, y) + BOSS_TRAIL <= SWITCH_DISTANCE - SWITCH_MARGIN;
            }

            inline bool IsHandoffTime(int64_t secondsInForm)
            {
                return secondsInForm >= HANDOFF_SECONDS;
            }
        }
    }
}
