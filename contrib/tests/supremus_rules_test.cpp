#include "../../src/modules/PlayerBots/playerbot/strategy/raids/bt/BtSupremusRules.h"
#include <cassert>
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

    std::cout << "PASS: Supremus phase timing, misdirection windows, dps wait and Gaze kite distance\n";
}
