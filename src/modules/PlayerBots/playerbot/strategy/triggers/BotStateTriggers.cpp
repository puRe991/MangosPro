
#include "playerbot/playerbot.h"
#include "playerbot/ServerFacade.h"
#include "BotStateTriggers.h"

using namespace ai;

bool CombatStartTrigger::IsActive()
{
    if (!ai->IsStateActive(BotState::BOT_STATE_COMBAT) && !ai->IsStateActive(BotState::BOT_STATE_DEAD))
    {
        // Check if any member of the group (near this bot) is getting attacked
        return AI_VALUE(bool, "has attackers");
    }

    return false;
}

bool CombatEndTrigger::IsActive()
{
    // Check if the bot is currently in combat
    if (ai->IsStateActive(BotState::BOT_STATE_COMBAT))
    {
        // Check if any member of the group (near this bot) is getting attacked
        if (AI_VALUE(bool, "has attackers"))
        {
            noAttackersSinceMs = 0;
            return false;
        }

        // Bosses briefly drop every target (Lurker spout/submerge, phase changes). Leaving the
        // combat state there made bots follow, emote and summon pets mid fight, so a bot that is
        // still in combat inside an instance keeps its combat state for a grace period.
        const uint32 now = WorldTimer::getMSTime();
        if (!noAttackersSinceMs)
            noAttackersSinceMs = now;
        if (bot->IsInCombat() && bot->GetMap()->IsDungeon() && WorldTimer::getMSTimeDiff(noAttackersSinceMs, now) < 10000)
            return false;

        noAttackersSinceMs = 0;
        return true;
    }

    noAttackersSinceMs = 0;
    return false;
}

bool DeathTrigger::IsActive()
{
    return !ai->IsStateActive(BotState::BOT_STATE_DEAD) && !sServerFacade.IsAlive(bot);
}

bool ResurrectTrigger::IsActive()
{
    return ai->IsStateActive(BotState::BOT_STATE_DEAD) && sServerFacade.IsAlive(bot);
}