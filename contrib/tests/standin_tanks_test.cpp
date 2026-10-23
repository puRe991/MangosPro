// Assist tank slots without a real tank go to melee dps stand-ins in order (Maulgar with two tanks:
// Olm keeps the real off tank, Blindeye gets the first stand-in). Run with contrib/tests/run_tests.pl.
#include <cassert>
#include <iostream>
#include <vector>
using uint8 = unsigned char;
struct Player { const char* name; };
std::vector<Player*> tanks, standIns;
namespace raidboss
{
    std::vector<Player*> GetGroupTanks(Player*) { return tanks; }
    std::vector<Player*> GetGroupStandInTanks(Player*) { return standIns; }
    Player* GetGroupAssistTank(Player*, uint8 index) { return size_t(index) + 1 < tanks.size() ? tanks[index + 1] : nullptr; }
    Player* GetGroupAssistTankOrStandIn(Player* bot, uint8 index);
    bool IsAssistTankOrStandIn(Player* bot, uint8 index);
}
//@extract raids/RaidBossHelpers.cpp | Player* raidboss::GetGroupAssistTankOrStandIn
//@extract raids/RaidBossHelpers.cpp | bool raidboss::IsAssistTankOrStandIn

int main()
{
    using namespace raidboss;
    Player mainTank{"Sailan"}, offTank{"Lengocow"}, warrior{"Warrior"}, paladin{"Paladin"};
    standIns = { &warrior, &paladin };

    // Four real tanks: no stand-in is used
    Player t3{"T3"}, t4{"T4"};
    tanks = { &mainTank, &offTank, &t3, &t4 };
    assert(GetGroupAssistTankOrStandIn(&warrior, 0) == &offTank);
    assert(GetGroupAssistTankOrStandIn(&warrior, 1) == &t3);
    assert(!IsAssistTankOrStandIn(&warrior, 1));

    // Two tanks (usual raid): Olm = real off tank, Blindeye = first stand-in, a third slot = second stand-in
    tanks = { &mainTank, &offTank };
    assert(GetGroupAssistTankOrStandIn(&warrior, 0) == &offTank);
    assert(IsAssistTankOrStandIn(&warrior, 1));
    assert(IsAssistTankOrStandIn(&paladin, 2));
    assert(GetGroupAssistTankOrStandIn(&warrior, 3) == nullptr);

    // One tank: both ogres get stand-ins
    tanks = { &mainTank };
    assert(IsAssistTankOrStandIn(&warrior, 0));
    assert(IsAssistTankOrStandIn(&paladin, 1));

    // No tank and no stand-in: nothing is assigned
    tanks.clear();
    standIns.clear();
    assert(GetGroupAssistTankOrStandIn(&warrior, 0) == nullptr);

    std::cout << "PASS: assist tank slots fall back to stand-ins in order\n";
}
