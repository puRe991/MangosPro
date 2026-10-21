
#include "playerbot/playerbot.h"
#include "InvalidTargetValue.h"
#include "playerbot/strategy/raids/ssc/SscHelpers.h"
#include "PossibleTargetsValue.h"
#include "playerbot/strategy/raids/RaidBossStrategies.h"
#include "PossibleAttackTargetsValue.h"
#include "EnemyPlayerValue.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/raids/karazhan/KarazhanHelpers.h"

using namespace ai;

namespace
{
    void LogInvalidRaidTarget(PlayerbotAI* ai, Unit* target, const char* reason)
    {
        Player* bot = ai->GetBot();
        if (bot->GetMapId() != 532 && bot->GetMapId() != 544 && bot->GetMapId() != 548) return;
        auto* debug = dynamic_cast<RaidBossDebugValue*>(ai->GetAiObjectContext()->GetUntypedValue("raid boss debug"));
        if (!debug || !debug->Get()) return;
        time_t& last = debug->lastLog[std::string("target ") + reason];
        if (time(nullptr) - last < 2) return;
        last = time(nullptr);
        sLog.outString("[RaidTarget] bot=%s reason=%s target=%u entry=%u selected=%u alive=%u combat=%u",
            bot->GetName(), reason, target->GetObjectGuid().GetCounter(), target->GetEntry(),
            bot->GetSelectionGuid().GetCounter(), target->IsAlive(), bot->IsInCombat());
    }
}

bool InvalidTargetValue::Calculate()
{
    Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, qualifier));
    if (!target || !target->IsInWorld() || target->GetMapId() != bot->GetMapId())
    {
        return true;
    }

    Unit* duelTarget = ai->GetUnit(AI_VALUE(ObjectGuid, "duel target"));
    if (duelTarget && duelTarget == target)
    {
        return false;
    }

    // Passive encounter objectives do not maintain an ordinary threat list.
    // An auxiliary spell can also change selection without changing the
    // assigned objective; that must not start another target-reset loop.
    if (karazhan::IsLiveDemonChains(ai, target))
        return false;

    if (bot->GetMapId() == 548 && ssc::GetMyInnerDemon(bot) == target &&
        PossibleTargetsValue::IsValid(target, bot)) return false;

    if (qualifier == "current target")
    {
        if (target->GetObjectGuid() != bot->GetSelectionGuid())
        {
            LogInvalidRaidTarget(ai, target, "selection_changed");
            return true;
        }
    }

    const bool validTarget = PossibleAttackTargetsValue::IsValid(target, bot);
    if (!validTarget)
    {
        std::list<ObjectGuid> attackers = AI_VALUE(std::list<ObjectGuid>, "possible attack targets");
        if (std::find(attackers.begin(), attackers.end(), target->GetObjectGuid()) != attackers.end())
        {
            return false;
        }
    }

    if (!validTarget) LogInvalidRaidTarget(ai, target, "not_attackable");
    return !validTarget;
}
