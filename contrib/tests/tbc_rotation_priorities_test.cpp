// Actual TBC strategy trigger lists against an offline queue fixture: the highest possible action wins.
// Run with contrib/tests/run_tests.pl.
#include <cassert>
#include <cstdarg>
#include <iostream>
#include <list>
#include <set>
#include <string>
//@extract Strategy.h | enum ActionPriority
;
struct NextAction {
    std::string name; float priority;
    NextAction(const char* n, float p) : name(n), priority(p) {}
    static NextAction** array(int first, ...) {
        auto result = new NextAction*[32]{};
        va_list args; va_start(args, first);
        int i = 0;
        while (auto action = va_arg(args, NextAction*)) result[i++] = action;
        va_end(args);
        return result;
    }
};
struct TriggerNode {
    std::string name; NextAction** actions;
    TriggerNode(const char* n, NextAction** a) : name(n), actions(a) {}
    ~TriggerNode() {
        for (int i=0; actions[i]; ++i) delete actions[i];
        delete[] actions;
    }
};
struct BaseStrategy { static void InitCombatTriggers(std::list<TriggerNode*>&) {} };
using DruidStrategy = BaseStrategy;
using WarriorStrategy = BaseStrategy;
using RogueStrategy = BaseStrategy;
struct RestorationDruidStrategy { void InitCombatTriggers(std::list<TriggerNode*>&); };
//@extract druid/RestorationDruidStrategy.cpp | void RestorationDruidStrategy::InitCombatTriggers | #ifdef MANGOSBOT_ONE // TBC
struct TankFeralDruidStrategy { void InitCombatTriggers(std::list<TriggerNode*>&); };
//@extract druid/TankFeralDruidStrategy.cpp | void TankFeralDruidStrategy::InitCombatTriggers | #ifdef MANGOSBOT_ONE // TBC
struct ArmsWarriorStrategy { void InitCombatTriggers(std::list<TriggerNode*>&); };
//@extract warrior/ArmsWarriorStrategy.cpp | void ArmsWarriorStrategy::InitCombatTriggers | #ifdef MANGOSBOT_ONE // TBC
struct CombatRogueStrategy { void InitCombatTriggers(std::list<TriggerNode*>&); };
//@extract rogue/CombatRogueStrategy.cpp | void CombatRogueStrategy::InitCombatTriggers | #ifdef MANGOSBOT_ONE // TBC
template<class Strategy>
std::string choose(std::set<std::string> active, std::set<std::string> possible) {
    std::list<TriggerNode*> triggers;
    Strategy strategy; strategy.InitCombatTriggers(triggers);
    std::string selected; float best = -1;
    for (auto node : triggers) {
        if (active.count(node->name))
            for (int i=0; node->actions[i]; ++i) {
                auto action = node->actions[i];
                if (possible.count(action->name) && action->priority > best) {
                    best = action->priority; selected = action->name;
                }
            }
        delete node;
    }
    return selected;
}
int main() {
    const std::set<std::string> critical{"party member critical health"};
    assert(choose<RestorationDruidStrategy>(critical,
        {"swiftmend on party", "regrowth on party", "healing touch on party"}) == "swiftmend on party");
    assert(choose<RestorationDruidStrategy>(critical,
        {"regrowth on party", "healing touch on party"}) == "regrowth on party");
    assert(choose<RestorationDruidStrategy>(critical,
        {"healing touch on party"}) == "healing touch on party");
    assert(choose<CombatRogueStrategy>({"slice and dice", "rupture", "eviscerate"},
        {"slice and dice", "rupture", "eviscerate"}) == "slice and dice");
    assert(choose<CombatRogueStrategy>({"rupture", "eviscerate"},
        {"rupture", "eviscerate"}) == "rupture");
    assert(choose<CombatRogueStrategy>({"eviscerate"}, {"eviscerate"}) == "eviscerate");
    assert(choose<TankFeralDruidStrategy>({"mangle (bear) ready", "lacerate", "maul ready"},
        {"mangle (bear)", "lacerate", "maul"}) == "mangle (bear)");
    assert(choose<TankFeralDruidStrategy>({"lacerate", "maul ready"},
        {"lacerate", "maul"}) == "lacerate");
    assert(choose<TankFeralDruidStrategy>({"maul ready"}, {"maul"}) == "maul");
    assert(choose<ArmsWarriorStrategy>({"slam", "mortal strike", "whirlwind", "heroic strike"},
        {"slam", "mortal strike", "whirlwind", "heroic strike"}) == "slam");
    assert(choose<ArmsWarriorStrategy>({"mortal strike", "whirlwind", "heroic strike"},
        {"mortal strike", "whirlwind", "heroic strike"}) == "mortal strike");
    std::cout << "PASS: 11 scenarios using actual TBC strategy bodies\n";
}
