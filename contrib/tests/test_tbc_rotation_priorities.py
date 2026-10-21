"""Compile the actual TBC strategy bodies against an offline queue fixture.

No world/login server or database is started. Run from the repository root
with MSVC's INCLUDE/LIB environment configured.
"""
from pathlib import Path
import os
import subprocess

root = Path("src/modules/PlayerBots/playerbot/strategy")
out = Path("build/tbc-rotation-priorities-test")
out.mkdir(exist_ok=True)

def body(text, signature):
    start = text.index(signature)
    pos = text.index("{", start) + 1
    depth = 1
    while depth:
        depth += (text[pos] == "{") - (text[pos] == "}")
        pos += 1
    return text[start:pos]

source = r"""
#include <cassert>
#include <cstdarg>
#include <iostream>
#include <list>
#include <set>
#include <string>
"""
source += body((root / "Strategy.h").read_text(), "enum ActionPriority") + ";\n"
source += r"""
struct NextAction {
    std::string name; float priority;
    NextAction(const char* n, float p) : name(n), priority(p) {}
    static NextAction** array(int, ...) {
        auto result = new NextAction*[32]{};
        va_list args; va_start(args, 0);
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
"""
for file, cls in [
    ("druid/RestorationDruidStrategy.cpp", "RestorationDruidStrategy"),
    ("druid/TankFeralDruidStrategy.cpp", "TankFeralDruidStrategy"),
    ("warrior/ArmsWarriorStrategy.cpp", "ArmsWarriorStrategy"),
    ("rogue/CombatRogueStrategy.cpp", "CombatRogueStrategy"),
]:
    source += f"struct {cls} {{ void InitCombatTriggers(std::list<TriggerNode*>&); }};\n"
    text = (root / file).read_text().split("#ifdef MANGOSBOT_ONE // TBC", 1)[1]
    source += body(text, f"void {cls}::InitCombatTriggers") + "\n"

source += r"""
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
"""
# A named last parameter is required for va_start on all supported compilers.
source = source.replace("array(int, ...)", "array(int first, ...)").replace("va_start(args, 0)", "va_start(args, first)")
cpp = out / "test.cpp"
cpp.write_text(source)
compiler = Path(os.environ.get("TBC_TEST_CL",
    "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe"))
exe = out / "test.exe"
subprocess.run([str(compiler), "/nologo", "/EHsc", "/std:c++17", str(cpp),
                "/Fo:" + str(out / "test.obj"), "/Fe:" + str(exe)], check=True)
subprocess.run([str(exe.resolve())], check=True)
