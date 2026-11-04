#include "../../src/modules/PlayerBots/playerbot/strategy/raids/ssc/SscHydrossRules.h"
#include <cassert>
#include <iostream>

using namespace ai::ssc::hydross;

int main()
{
    // The boss leaves the field at 28 yards (20 + combat reach 3 + 5), as measured in the raid logs
    assert(SWITCH_DISTANCE == 28.0f);

    // Tank spots of SscHelpers.cpp
    const float frostX = -235.2f, frostY = -354.2f;
    const float natureX = -225.471f, natureY = -327.790f;
    assert(IsFrostSpotInside(frostX, frostY));
    assert(IsNatureSpotOutside(natureX, natureY));

    // The spots that failed in the raid logs: the boss trailing the tank stayed inside the field
    assert(!IsNatureSpotOutside(-229.4f, -338.3f));   // 30 yards
    assert(!IsNatureSpotOutside(-227.3f, -332.6f));   // 36 yards

    // Hand-off after the 25% mark (30 s), walk finished before the 50% mark (45 s)
    assert(!IsHandoffTime(HANDOFF_SECONDS - 1) && IsHandoffTime(HANDOFF_SECONDS));
    assert(HANDOFF_SECONDS > 2 * MARK_INTERVAL_SECONDS);
    assert(HANDOFF_SECONDS + 1 + HANDOFF_WALK_SECONDS < 3 * MARK_INTERVAL_SECONDS);

    // The walk between the spots fits into the walk time at run speed 7 yards per second
    const float dx = natureX - frostX, dy = natureY - frostY;
    assert(std::sqrt(dx * dx + dy * dy) / 7.0f < float(HANDOFF_WALK_SECONDS));

    std::cout << "PASS: Hydross switch distance, tank spots and hand-off timing\n";
}
