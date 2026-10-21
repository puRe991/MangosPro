
#include "playerbot/playerbot.h"
#include "DpsTargetValue.h"
#include "LeastHpTargetValue.h"
#include "playerbot/strategy/raids/karazhan/KarazhanHelpers.h"
#include "playerbot/strategy/raids/magtheridon/MagtheridonHelpers.h"
#include "playerbot/strategy/raids/ssc/SscHelpers.h"

using namespace ai;


ObjectGuid DpsTargetValue::Calculate()
{
    if (bot->GetMapId() == 548)
        if (Unit* demon = ssc::GetMyInnerDemon(bot)) return demon->GetObjectGuid();
    if (bot->GetMapId() == 544 && bot->IsInCombat())
    {
        Unit* target = magtheridon::GetCombatTarget(ai);
        return target ? target->GetObjectGuid() : ObjectGuid();
    }
    if (Unit* priority = karazhan::GetIllhoofTarget(ai, false))
        return priority->GetObjectGuid();
    ObjectGuid rti = RtiTargetValue::Calculate();
    if (rti) return rti;

    FindLeastHpTargetStrategy strategy(ai);
    Unit* target = TargetValue::FindTarget(&strategy);
    return target ? target->GetObjectGuid() : ObjectGuid();
}

class FindMaxHpTargetStrategy : public FindTargetStrategy
{
public:
    FindMaxHpTargetStrategy(PlayerbotAI* ai) : FindTargetStrategy(ai)
    {
        maxHealth = 0;
    }

public:
    virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager) override
    {
        if (!result || result->GetHealth() < attacker->GetHealth())
            result = attacker;
    }

protected:
    float maxHealth;
};

ObjectGuid DpsAoeTargetValue::Calculate()
{
    ObjectGuid rti = RtiTargetValue::Calculate();
    if (rti) return rti;

    FindMaxHpTargetStrategy strategy(ai);
    Unit* target = TargetValue::FindTarget(&strategy);
    return target ? target->GetObjectGuid() : ObjectGuid();
}
