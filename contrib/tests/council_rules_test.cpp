#include "../../src/modules/PlayerBots/playerbot/strategy/raids/bt/BtCouncilRules.h"
#include <cassert>
#include <iostream>
#include <vector>

using namespace ai::bt::council;

int main()
{
    // The only hunter of the raid misdirects Gathios onto the main tank, Zerevor comes last
    assert(MisdirectTarget(0) == Member::Gathios);
    assert(MisdirectTarget(1) == Member::Malande);
    assert(MisdirectTarget(2) == Member::Veras);
    assert(MisdirectTarget(3) == Member::Zerevor);
    assert(MisdirectTarget(-1) == Member::None);
    assert(MisdirectTarget(4) == Member::None);

    // Casters start on Gathios, never on Veras while his tank does not hold him (threat wipe after Deadly Poison)
    std::vector<Member> order = DpsOrder(false, false, false);
    assert((order == std::vector<Member>{ Member::Gathios, Member::Malande }));
    order = DpsOrder(false, false, true);
    assert((order == std::vector<Member>{ Member::Gathios, Member::Malande, Member::Veras }));

    // The Malande group stays at Malande for the interrupts, unless she stands in Zerevor's Arcane Explosion
    order = DpsOrder(true, false, false);
    assert((order == std::vector<Member>{ Member::Malande, Member::Gathios }));
    order = DpsOrder(true, true, false);
    assert((order == std::vector<Member>{ Member::Gathios }));
    order = DpsOrder(false, true, true);
    assert((order == std::vector<Member>{ Member::Gathios, Member::Veras }));

    // Arcane Explosion: everybody leaves the 10 yards once the council is pulled
    assert(!MustLeaveZerevor(5.0f, false, false, -1));      // before the pull nobody runs
    assert(MustLeaveZerevor(5.0f, true, false, 0));
    assert(MustLeaveZerevor(ARCANE_EXPLOSION_RADIUS, true, false, 0));
    assert(!MustLeaveZerevor(ZEREVOR_TOO_CLOSE, true, false, 30));
    // Melee tanks pick up their member on the dais first, then step out
    assert(!MustLeaveZerevor(5.0f, true, true, -1));
    assert(!MustLeaveZerevor(5.0f, true, true, PULL_WINDOW_SECONDS - 1));
    assert(MustLeaveZerevor(5.0f, true, true, PULL_WINDOW_SECONDS));
    // The flee point is outside the trigger distance, so the bot does not run again right away
    assert(ZEREVOR_SAFE_DISTANCE > ZEREVOR_TOO_CLOSE);

    // Blessings: Protection blocks physical interrupts, Spell Warding the magical ones
    assert(InterruptLands(true, false, false));
    assert(!InterruptLands(true, true, false));
    assert(InterruptLands(true, false, true));
    assert(InterruptLands(false, true, false));
    assert(!InterruptLands(false, false, true));

    // Main tank: drag Gathios to the corner, then stand and build threat; danger moves him on
    assert(MainTankMove(30.0f, false) == TankMove::StepToCorner);
    assert(MainTankMove(GATHIOS_DRAG_DISTANCE, false) == TankMove::Stay);
    assert(MainTankMove(5.0f, false) == TankMove::Stay);
    assert(MainTankMove(1.0f, false) == TankMove::Stay);
    assert(MainTankMove(5.0f, true) == TankMove::StepToCorner);
    assert(MainTankMove(GATHIOS_CORNER_REACHED, true) == TankMove::NextCorner);

    // AoE threat only while no other council member stands next to the tank
    assert(AoeThreatAllowed(false));
    assert(!AoeThreatAllowed(true));
    assert(AOE_THREAT_RANGE > 10.0f);

    std::cout << "PASS: Illidari Council misdirects, dps order, Arcane Explosion range, interrupts and tank threat\n";
}
