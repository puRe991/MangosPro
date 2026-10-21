#include "../../src/modules/PlayerBots/playerbot/strategy/generic/TbcRotationRules.h"
#include <cassert>
#include <iostream>

int main()
{
    using namespace ai::tbc;

    // Raid boss vs. dying target: do not spend finishers on a late bleed.
    assert(UseBleedFinisher(500000, 10000, 80));
    assert(!UseBleedFinisher(9000, 10000, 90));
    assert(!UseBleedFinisher(500000, 10000, 20));
    assert(!UseBleedFinisher(0, 10000, 0));
    assert(!UseBleedFinisher(10000, 10000, 100));

    // A slow two-hander just swung; a nearly ready or stale swing must not
    // admit Slam, even if the spell itself is available.
    assert(SlamWindow(3500, 3500, 500));
    assert(SlamWindow(2900, 3500, 1000));
    assert(!SlamWindow(2899, 3500, 500));
    assert(!SlamWindow(100, 3500, 500));
    assert(!SlamWindow(3500, 3500, 1500));
    assert(!SlamWindow(3501, 3500, 500));
    assert(!SlamWindow(0, 0, 500));
    // Hasted weapon: use its effective interval, not its item delay.
    assert(SlamWindow(1700, 1800, 500));
    assert(!SlamWindow(1700, 3500, 500));

    assert(!PreferWindfury(0, 5));
    assert(!PreferWindfury(1, 4));
    assert(PreferWindfury(4, 1));
    assert(PreferWindfury(2, 2));
    assert(!PreferWindfury(0, 0));
    std::cout << "PASS: TBC finisher, Slam timing and air-totem boundaries\n";
}
