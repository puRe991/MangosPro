// Tank drag for Maulgar and Gruul: no chase oscillation with alternating melee range, spot is held,
// aggro recovery and taunts stay available. Run with contrib/tests/run_tests.pl.
#include <string>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <iostream>
struct Unit { float x=143,y=193; Unit* victim=nullptr; Unit* GetVictim(){return victim;} float GetPositionX(){return x;} float GetPositionY(){return y;} };
struct Player:Unit { bool tank=true,main=true,melee=true; float GetPositionZ(){return 0;} bool CanReachWithMeleeAttack(Unit*){return melee;} };
struct PlayerbotAI { Player* bot; static bool IsTank(Player* p){return p->tank;} };
struct Action { std::string name; const std::string& getName(){return name;} };
struct WorldPosition { float x,y; float getX()const{return x;} float getY()const{return y;} };
struct Event{};
Unit *boss=nullptr,*current=nullptr; WorldPosition GRUUL_TANK_POSITION{241.238f,365.025f}; int GRUUL_MAP_ID=565;
bool IsMaulgarTank(Player* p){return p->main;}
Unit* FindMaulgar(PlayerbotAI*){return boss;} Unit* FindGruul(PlayerbotAI*){return boss;}
bool IsReachTargetAction(Action* a){return a->name=="reach melee" || a->name=="reach spell";}
bool IsGapCloserAction(Action* a){return a->name=="charge";}
Unit* GetCurrentTarget(PlayerbotAI*){return current;}
void LogTankPosition(PlayerbotAI*,Unit*,const WorldPosition&,const char*){}
namespace raidboss { float ExactDist2d(Player* p,float x,float y){return std::hypot(p->x-x,p->y-y);} bool GetStepToPosition(Player*,const WorldPosition&,float,Unit*,float&,float&,bool&); }
struct HighKingMaulgarControlTankActionsMultiplier { Player* bot; PlayerbotAI* ai; float GetValueInEncounter(Action*); };
struct GruulTheDragonkillerControlTankMovementMultiplier { Player* bot; PlayerbotAI* ai; float GetValueInEncounter(Action*); };
struct GruulTheDragonkillerTanksPositionBossAction { Player* bot; PlayerbotAI* ai; bool Execute(Event&); void* GetMaster(){return nullptr;} bool Attack(void*,Unit* t){current=t;return true;} bool MoveTo(int,float x,float y,float,bool,bool,bool,bool){bot->x=x;bot->y=y;return true;} };
//@extract raids/RaidBossHelpers.cpp | bool raidboss::GetStepToPosition
//@extract raids/gruul/GruulMultipliers.cpp | float HighKingMaulgarControlTankActionsMultiplier::GetValueInEncounter
//@extract raids/gruul/GruulMultipliers.cpp | float GruulTheDragonkillerControlTankMovementMultiplier::GetValueInEncounter
//@extract raids/gruul/GruulActions.cpp | bool GruulTheDragonkillerTanksPositionBossAction::Execute
int main(){
 Player tank; Player other; PlayerbotAI ai{&tank}; Unit encounter; boss=&encounter; current=boss; boss->victim=&tank;
 HighKingMaulgarControlTankActionsMultiplier maulgar{&tank,&ai}; GruulTheDragonkillerControlTankMovementMultiplier gruul{&tank,&ai};
 for(std::string name:{"reach melee","reach spell","charge","set behind","follow"}) {Action a{name};assert(maulgar.GetValueInEncounter(&a)==0);assert(gruul.GetValueInEncounter(&a)==0);}
 Action taunt{"taunt"};assert(maulgar.GetValueInEncounter(&taunt)==1);assert(gruul.GetValueInEncounter(&taunt)==1);
 boss->victim=&other; Action reach{"reach melee"}; assert(maulgar.GetValueInEncounter(&reach)==1);assert(gruul.GetValueInEncounter(&reach)==1);
 GruulTheDragonkillerTanksPositionBossAction move{&tank,&ai};Event e;float before=tank.x;assert(!move.Execute(e));assert(tank.x==before);
 boss->victim=&tank; tank.x=300;tank.y=400;
 for(int i=0;i<80;++i){tank.melee=(i%2)==0; move.Execute(e);if(gruul.GetValueInEncounter(&reach)>0){tank.x=boss->x;tank.y=boss->y;}}
 assert(raidboss::ExactDist2d(&tank,GRUUL_TANK_POSITION.x,GRUUL_TANK_POSITION.y)<=3);
 assert(!move.Execute(e));
 boss=nullptr;assert(gruul.GetValueInEncounter(&reach)==1);assert(maulgar.GetValueInEncounter(&reach)==1);
 std::cout<<"PASS: MT drag survives alternating melee range; no chase oscillation; holds spot; off-tank and aggro recovery remain available; taunt preserved.\n";
}
