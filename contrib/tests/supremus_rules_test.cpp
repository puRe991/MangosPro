#include "../../src/modules/PlayerBots/playerbot/strategy/raids/bt/BtSupremusRules.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace ai::bt::supremus;

int main()
{
    // Phases alternate every 60 seconds, starting with the tank phase
    assert(IsTankPhase(0) && IsTankPhase(59));
    assert(!IsTankPhase(60) && !IsTankPhase(119));
    assert(IsTankPhase(120) && !IsTankPhase(180));
    assert(!IsTankPhase(-1));

    // Misdirection on the pull and on every return to the tank phase, never on the switch to the Gaze phase
    assert(IsMisdirectWindow(0) && IsMisdirectWindow(9) && !IsMisdirectWindow(10));
    assert(!IsMisdirectWindow(60) && !IsMisdirectWindow(65));
    assert(IsMisdirectWindow(120) && IsMisdirectWindow(245));
    assert(!IsMisdirectWindow(180));
    assert(!IsMisdirectWindow(-1));

    // Every misdirect window is at least the 2 minute Misdirection cooldown after the previous one
    int64_t last = -1000;
    for (int64_t t = 0; t < 900; ++t)
    {
        if (IsMisdirectWindow(t) && !IsMisdirectWindow(t - 1))
        {
            assert(t - last >= 120);
            last = t;
        }
    }

    // Damage dealers hold back after the threat reset of each return to the tank phase, not on the pull
    assert(!IsDpsWaitAfterThreatReset(0) && !IsDpsWaitAfterThreatReset(2));
    assert(!IsDpsWaitAfterThreatReset(60) && !IsDpsWaitAfterThreatReset(62));
    assert(IsDpsWaitAfterThreatReset(120) && IsDpsWaitAfterThreatReset(124) && !IsDpsWaitAfterThreatReset(125));
    assert(IsDpsWaitAfterThreatReset(240) && !IsDpsWaitAfterThreatReset(300));

    // Gaze kiting: Supremus reaches 24 (combat reach) + 1.5 (player) + 1.33 + 2.67 (both running) = 29.5 yards
    const float reach = 24.0f + 1.5f + 1.33f + 8.0f / 3.0f;
    assert(NeedsToKite(25.0f, reach));                   // the former 25 yard border was inside his melee
    assert(NeedsToKite(reach + KITE_BUFFER - 0.1f, reach));
    assert(!NeedsToKite(reach + KITE_BUFFER, reach));
    assert(KiteFleeDistance(reach) > reach + KITE_BUFFER);
    // The buffer covers two seconds of the boss closing in (about 5 yards per second measured in the raid log)
    assert(KITE_BUFFER >= 2.0f * 5.0f);
    // Hunters can still shoot from the flee distance (35 yards range plus the boss' combat reach)
    assert(KiteFleeDistance(reach) < 35.0f + 24.0f);

    // Hateful Strike: the waiting ring lies outside the reach, players at 20-30 yards (raid log deaths) are in danger
    assert(SafeRadius(reach) > reach + DANGER_BUFFER);
    assert(IsInHatefulDanger(20.0f, reach) && IsInHatefulDanger(29.0f, reach));
    assert(!IsInHatefulDanger(SafeRadius(reach), reach));
    // Casters still reach the boss from the ring (30 yard spells plus 24 combat reach and 1.5 own reach)
    assert(SafeRadius(reach) < 30.0f + 24.0f + 1.5f);
    // Hunters are out of the 5-8 yard dead zone on the ring
    assert(SafeRadius(reach) - 24.0f - 1.5f > 8.0f);

    // Melee only engage with two soakers besides the victim (a 2-tank raid has one)
    assert(!MeleeMayEngage(0) && !MeleeMayEngage(1) && MeleeMayEngage(2));

    // Healers on the tank side: every healer slot is within 40 yards of a tank 27 yards out in the anchor direction
    const float anchor = 0.7f;
    const float radius = SafeRadius(reach);
    for (int count = 1; count <= 10; ++count)
    {
        for (int i = 0; i < count; ++i)
        {
            const float a = HealerSlotAngle(i, count, anchor);
            const float dx = radius * std::cos(a) - 27.0f * std::cos(anchor);
            const float dy = radius * std::sin(a) - 27.0f * std::sin(anchor);
            assert(std::sqrt(dx * dx + dy * dy) <= 40.0f);
        }
    }

    // Damage dealer slots never overlap the healer arc and keep 6+ yards between them for 16 players
    for (int i = 0; i < 16; ++i)
    {
        float diff = std::fmod(DpsSlotAngle(i, 16, anchor) - anchor + 4.0f * PI, 2.0f * PI);
        if (diff > PI)
            diff -= 2.0f * PI;
        assert(std::fabs(diff) > HEALER_ARC / 2.0f);
    }
    const float spacing = radius * (2.0f * PI - HEALER_ARC - 2.0f * ARC_GAP) / 16.0f;
    assert(spacing >= 6.0f);

    std::cout << "PASS: Supremus phase timing, misdirection windows, dps wait, Gaze kite distance and Hateful Strike ring\n";
}
