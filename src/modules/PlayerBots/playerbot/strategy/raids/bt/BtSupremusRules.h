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

            // Hateful Strike (27750-32250 physical, every 1.2 seconds) hits the player with the most health in the
            // boss' melee reach, his victim excluded. Only plate tanks survive it, so everybody else waits on a ring
            // outside the reach during the tank phase. The 2.4.3 raid logs had 15 deaths in the first 40 seconds
            // with the ranged at 20-30 yards from his centre.
            constexpr float HATEFUL_SAFE_BUFFER = 7.0f;
            // A slot counts as reached within this distance, small boss movements don't move the whole raid
            constexpr float SLOT_TOLERANCE = 4.0f;
            // Inside this distance beyond the reach a player is in immediate danger
            constexpr float DANGER_BUFFER = 2.0f;

            constexpr float PI = 3.14159265f;
            // Healers stand on the tank's side so he stays in heal range (40 yards), the ranged damage dealers on
            // the rest of the circle with a gap to the healers
            constexpr float HEALER_ARC = 100.0f * PI / 180.0f;
            constexpr float ARC_GAP = 20.0f * PI / 180.0f;

            // Melee damage dealers only stay in melee reach when two tanks besides the boss' victim soak the
            // strikes, otherwise the strikes chain through the melee one after the other
            constexpr int MIN_HATEFUL_SOAKERS = 2;

            inline float SafeRadius(float meleeReach)
            {
                return meleeReach + HATEFUL_SAFE_BUFFER;
            }

            inline bool IsInHatefulDanger(float distance, float meleeReach)
            {
                return distance < meleeReach + DANGER_BUFFER;
            }

            inline bool MeleeMayEngage(int soakersInReach)
            {
                return soakersInReach >= MIN_HATEFUL_SOAKERS;
            }

            // Angle of a healer slot, spread over the arc around the direction from the boss to the tank
            inline float HealerSlotAngle(int index, int count, float anchor)
            {
                if (count <= 0)
                    return anchor;
                return anchor - HEALER_ARC / 2.0f + HEALER_ARC * (float(index) + 0.5f) / float(count);
            }

            // Angle of a damage dealer slot, spread over the rest of the circle
            inline float DpsSlotAngle(int index, int count, float anchor)
            {
                if (count <= 0)
                    return anchor + PI;
                const float start = anchor + HEALER_ARC / 2.0f + ARC_GAP;
                const float span = 2.0f * PI - HEALER_ARC - 2.0f * ARC_GAP;
                return start + span * (float(index) + 0.5f) / float(count);
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
