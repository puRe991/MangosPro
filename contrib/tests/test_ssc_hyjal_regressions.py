"""Compile and exercise the production SSC state/handoff and lust trigger functions.

The small world fixture supplies clock, raid roles and GUID lookup; the functions
under test are extracted unchanged from the production sources.
Run from the repository root with Python in a Visual C++ developer environment.
"""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
STRATEGY = ROOT / 'src/modules/PlayerBots/playerbot/strategy'


def function(file, signature):
    source = (STRATEGY / file).read_text()
    start = source.index(signature)
    end = source.index('{', start) + 1
    depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end] + '\n'


fixture = r'''
#include <cassert>
#include <ctime>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <type_traits>
#include <iostream>
using uint32 = unsigned;
struct ObjectGuid { unsigned n=0; bool IsEmpty() const {return !n;} };
constexpr uint32 SSC_MAP_ID=548, NPC_HYDROSS=21216, NPC_LURKER_BELOW=21217,
 NPC_LEOTHERAS=21215, NPC_KARATHRESS=21214, NPC_MOROGRIM=21213, NPC_LADY_VASHJ=21212,
 SPELL_CORRUPTION=37961;
time_t clockNow=100;
time_t fakeTime(void*) {return clockNow;}
#define time fakeTime
struct Unit {
 uint32 entry=NPC_HYDROSS; bool combat=true, nature=false; Unit* victim=nullptr; float hp=100;
 bool IsInCombat(){return combat;} bool HasAura(uint32){return nature;}
 Unit* GetVictim(){return victim;} uint32 GetEntry(){return entry;}
 float GetHealthPercent(){return hp;}
};
struct Group;
struct Player:Unit {
 uint32 map=548; bool world=true, marked=false, attacking=false; Group* group=nullptr;
 uint32 GetMapId(){return map;} bool IsInWorld(){return world;} void* GetMap(){return reinterpret_cast<void*>(1);}
 Group* GetGroup(){return group;} bool CanReachWithMeleeAttack(Unit*){return true;}
};
struct GroupReference {
 Player* player=nullptr; GroupReference* following=nullptr;
 Player* getSource(){return player;} GroupReference* next(){return following;}
};
struct Group {GroupReference* first=nullptr; GroupReference* GetFirstMember(){return first;}};
struct Context {
 struct Value {ObjectGuid guid; ObjectGuid Get(){return guid;}} value;
 template<class T> Value* GetValue(const char*) {
  static_assert(std::is_same<T,ObjectGuid>::value, "current target stores a GUID, never Unit*");
  return &value;
 }
};
enum class BotState {BOT_STATE_COMBAT};
struct PlayerbotAI {
 Player* bot; Context context; Unit* resolved=nullptr; bool hold=false, lust=false;
 Player* GetBot(){return bot;} Context* GetAiObjectContext(){return &context;}
 Unit* GetUnit(ObjectGuid guid){return guid.n ? resolved : nullptr;}
 bool HasStrategy(const char*,BotState){return hold;} bool HasAura(const char*,Player*){return lust;}
};
Player* frostTank=nullptr; Player* natureTank=nullptr;
Unit* hydross=nullptr;
namespace raidboss {
 Player* GetGroupMainTank(Player*){return frostTank;}
 Player* GetGroupAssistTank(Player*,int){return natureTank;}
 bool MarkTargetWithSquare(Player*,Unit*){return false;}
 bool MarkTargetWithTriangle(Player*,Unit*){return false;}
 void SetRtiTarget(PlayerbotAI*,const char*){}
}
Unit* FindSscUnit(PlayerbotAI*,uint32 entry){return hydross && entry==NPC_HYDROSS ? hydross : nullptr;}
bool HasMarkOfHydrossAt100Percent(Player* p){return p->marked;}
bool HasMarkOfCorruptionAt100Percent(Player* p){return p->marked;}
struct SscInstanceState {
 time_t hydrossFrostDpsWait=0,hydrossNatureDpsWait=0,hydrossChangeToFrostPhase=0,hydrossChangeToNaturePhase=0;
 time_t lurkerSpoutEnd=0,leotherasHumanFormDpsWait=0,leotherasDemonFormDpsWait=0,leotherasFinalPhaseDpsWait=0;
 time_t karathressDpsWait=0,lastCoreThrow=0; ObjectGuid vashjGenerator;
 std::map<int,int> lurkerRangedPositions,tidewalkerTankStep,tidewalkerRangedStep,hasReachedVashjRangedPosition,intendedLineup,lastCoreInInventoryTime;
} state;
SscInstanceState GetState(Player*){return state;}
void WithState(Player*,const std::function<void(SscInstanceState&)>& fn){fn(state);}
bool StopAttacking(PlayerbotAI* ai){bool before=ai->bot->attacking; ai->bot->attacking=false; return before;}
struct Event{};
struct WorldPosition {int id;};
const WorldPosition HYDROSS_FROST_TANK_POSITION{1},HYDROSS_NATURE_TANK_POSITION{2};
struct TankAction {
 PlayerbotAI* ai; Player* bot; int moved=0; bool current=true;
 bool IsCurrentTarget(Unit*){return current;} bool AttackUnit(Unit*){current=true;return true;}
 bool MoveToPosition(const WorldPosition& p){moved=p.id;return true;}
 bool StepToPosition(const WorldPosition& p,float,float){return MoveToPosition(p);}
};
struct SscHydrossPositionFrostTankAction:TankAction{bool Execute(Event&);};
struct SscHydrossPositionNatureTankAction:TankAction{bool Execute(Event&);};
namespace tbcraid {
 struct Encounter {float lustBelowHealthPct=50;std::vector<unsigned> entries{21212};} encounter;
 struct EncounterRegistry {
  static EncounterRegistry& Instance(){static EncounterRegistry r;return r;}
  const Encounter* FindNearby(PlayerbotAI*){return &encounter;}
 };
}
'''

for signature in ['bool NeedsStateReset', 'bool UpdateHydrossState', 'bool ShouldPauseHydrossDps']:
    fixture += function('raids/ssc/SscHelpers.cpp', signature)
for signature in ['bool SscHydrossPositionFrostTankAction::Execute', 'bool SscHydrossPositionNatureTankAction::Execute']:
    fixture += function('raids/ssc/SscActions.cpp', signature)
fixture += function('shaman/ShamanTriggers.cpp', 'static bool IsLustAllowed')
fixture += r'''
int main() {
 Player frost,nature,dps; frostTank=&frost; natureTank=&nature;
 Unit boss; hydross=&boss; boss.victim=&frost;
 PlayerbotAI ai{&dps}; Event event;
 assert(UpdateHydrossState(&ai)); assert(ShouldPauseHydrossDps(&ai));
 clockNow=106; assert(!ShouldPauseHydrossDps(&ai));
 // A tracker without marks still observes the active tank's mark.
 clockNow=160; frost.marked=true; assert(UpdateHydrossState(&ai));
 assert(state.hydrossChangeToNaturePhase==160); assert(ShouldPauseHydrossDps(&ai));
 // Losing the mark cannot reverse a transition already requested.
 frost.marked=false; clockNow=162; UpdateHydrossState(&ai);
 assert(state.hydrossChangeToNaturePhase==160);
 boss.nature=true; UpdateHydrossState(&ai);
 assert(state.hydrossChangeToNaturePhase==0 && state.hydrossNatureDpsWait==162);
 clockNow=168; assert(ShouldPauseHydrossDps(&ai)); // wrong tank still owns boss
 boss.victim=&nature; assert(!ShouldPauseHydrossDps(&ai));
 clockNow=222; nature.marked=true; UpdateHydrossState(&ai);
 assert(state.hydrossChangeToFrostPhase==222 && ShouldPauseHydrossDps(&ai));
 boss.nature=false; UpdateHydrossState(&ai);
 assert(state.hydrossChangeToFrostPhase==0 && state.hydrossFrostDpsWait==222);
 // Old 100% marks must not immediately request a reverse transition.
 frost.marked=true; clockNow=224; UpdateHydrossState(&ai);
 assert(state.hydrossChangeToNaturePhase==0);
 // Outgoing tank stays on the current side until the incoming tank owns threat.
 PlayerbotAI frostAI{&frost},natureAI{&nature};
 SscHydrossPositionFrostTankAction fa;fa.ai=&frostAI;fa.bot=&frost;
 boss.nature=true;boss.victim=&frost;frost.attacking=true;
 assert(fa.Execute(event));assert(!frost.attacking && fa.moved==0);
 assert(!fa.Execute(event));assert(fa.moved==0);
 boss.victim=&dps;assert(!fa.Execute(event));assert(fa.moved==0);
 boss.victim=&nature;assert(fa.Execute(event));assert(fa.moved==1);
 SscHydrossPositionNatureTankAction na;na.ai=&natureAI;na.bot=&nature;
 boss.nature=false;boss.victim=&nature;nature.attacking=true;
 assert(na.Execute(event));assert(!nature.attacking && na.moved==0);
 assert(!na.Execute(event));boss.victim=&frost;assert(na.Execute(event));assert(na.moved==2);
 // Idle cleanup is silent when empty, and a nearby reset boss does not prevent it.
 state={};dps.combat=false;boss.combat=false;assert(!NeedsStateReset(&ai));
 state.intendedLineup[1]=1;state.lastCoreInInventoryTime[1]=1;state.lastCoreThrow=42;
 assert(NeedsStateReset(&ai));
 GroupReference member{&frost};Group group{&member};dps.group=&group;
 frost.combat=true;assert(!NeedsStateReset(&ai));frost.combat=false;
 boss.combat=true;assert(!NeedsStateReset(&ai));boss.combat=false;assert(NeedsStateReset(&ai));
 // Vashj pull: empty/stale targets are safe; threshold and hold-lust still work.
 Unit vashj;vashj.entry=21212;ai.resolved=&vashj;
 assert(!IsLustAllowed(&ai));ai.context.value.guid.n=1;
 assert(!IsLustAllowed(&ai));vashj.hp=50;assert(IsLustAllowed(&ai));
 ai.resolved=nullptr;assert(!IsLustAllowed(&ai));ai.resolved=&vashj;
 ai.hold=true;assert(!IsLustAllowed(&ai));ai.hold=false;
 ai.lust=true;assert(!IsLustAllowed(&ai));
 std::cout << "SSC state, tank handoff and Vashj lust regression tests passed\n";
}
'''

build = ROOT / 'build'
source = build / 'ssc-hyjal-regressions.cpp'
exe = build / 'ssc-hyjal-regressions.exe'
source.write_text(fixture)
subprocess.run(['cl', '/nologo', '/std:c++17', '/EHsc', str(source),
                '/Fo' + str(build / 'ssc-hyjal-regressions.obj'), '/Fe' + str(exe)], check=True)
subprocess.run([str(exe)], check=True)
