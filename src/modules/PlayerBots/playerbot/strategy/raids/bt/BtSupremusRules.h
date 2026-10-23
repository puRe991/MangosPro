#pragma once

#include <cstdint>

namespace ai
{
    namespace bt
    {
        // Supremus phase timing, kept free of game types for the offline tests
        namespace supremus
        {
            // The boss switches between the tank phase and the Gaze phase every 60 seconds, each switch resets threat
            constexpr int64_t PHASE_SECONDS = 60;
            constexpr int64_t CYCLE_SECONDS = 2 * PHASE_SECONDS;

            // Misdirection window after the pull or the return to the tank phase
            constexpr int64_t MISDIRECT_WINDOW_SECONDS = 10;

            // 1.5 seconds the boss stands still after the switch plus the time the tanks need for threat
            constexpr int64_t DPS_WAIT_SECONDS = 5;

            // Supremus has a combat reach of 24 yards, his melee reaches about 29.5 yards (centre to centre) at a
            // running target. The Gaze target keeps a buffer beyond that: the boss closes in at about 5 yards per
            // second, so the buffer covers two seconds of reaction time.
            constexpr float KITE_BUFFER = 10.0f;
            // Every kite move gains this much extra room so the bot does not stop right at the border
            constexpr float KITE_STEP = 6.0f;

            inline bool NeedsToKite(float distance, float meleeReach)
            {
                return distance < meleeReach + KITE_BUFFER;
            }

            inline float KiteFleeDistance(float meleeReach)
            {
                return meleeReach + KITE_BUFFER + KITE_STEP;
            }

            inline bool IsTankPhase(int64_t elapsed)
            {
                return elapsed >= 0 && elapsed % CYCLE_SECONDS < PHASE_SECONDS;
            }

            // Threat only matters when the tank phase starts. Misdirection has a 2 minute cooldown, so spending it on
            // the switch to the Gaze phase (random fixate, threat is ignored) would leave it unavailable for the next
            // return to the tank phase.
            inline bool IsMisdirectWindow(int64_t elapsed)
            {
                return elapsed >= 0 && elapsed % CYCLE_SECONDS < MISDIRECT_WINDOW_SECONDS;
            }

            // Damage dealers hold back after the threat reset at the start of every tank phase except the pull
            // (the pull is covered by the tanks engaging first and the delayed dps cooldowns)
            inline bool IsDpsWaitAfterThreatReset(int64_t elapsed)
            {
                return elapsed >= CYCLE_SECONDS && elapsed % CYCLE_SECONDS < DPS_WAIT_SECONDS;
            }
        }
    }
}
