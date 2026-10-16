#include "playerbot/playerbot.h"
#include "RaidBossStrategies.h"

using namespace ai;

const std::vector<std::pair<uint32, std::string>>& RaidBossMapStrategies::GetAll()
{
    static const std::vector<std::pair<uint32, std::string>> strategies =
    {
        // Classic raids
        { 409, "boss molten core" },
        { 469, "boss blackwing lair" },
        { 249, "boss onyxia" },
        { 509, "boss aq20" },
        { 533, "boss naxxramas" },

        // Burning Crusade raids
        { 532, "boss karazhan" },
        { 565, "boss gruul" },
        { 544, "boss magtheridon" },
        { 548, "boss ssc" },
        { 550, "boss tempest keep" },
        { 534, "boss hyjal" },
        { 564, "boss black temple" },
        { 568, "boss zul'aman" },

        // Burning Crusade dungeons
        { 543, "boss ramparts" },
        { 546, "boss underbog" },
        { 554, "boss mechanar" },
        { 556, "boss sethekk halls" },
        { 558, "boss auchenai crypts" },
        { 585, "boss magisters terrace" },
    };

    return strategies;
}

std::string RaidBossMapStrategies::GetStrategy(uint32 mapId)
{
    for (const auto& entry : GetAll())
    {
        if (entry.first == mapId)
            return entry.second;
    }

    return "";
}

bool RaidBossMapStrategyTrigger::IsActive()
{
    if (!bot->IsInWorld() || bot->IsBeingTeleported())
        return false;

    const std::string wanted = RaidBossMapStrategies::GetStrategy(bot->GetMapId());
    for (const auto& entry : RaidBossMapStrategies::GetAll())
    {
        // Only strategies which exist (a raid can be listed before its strategy is ported)
        if (!ai->GetAiObjectContext()->GetStrategy(entry.second))
            continue;

        const bool active = ai->HasStrategy(entry.second, BotState::BOT_STATE_COMBAT);
        if (active != (entry.second == wanted))
            return true;
    }

    return false;
}

bool UpdateRaidBossMapStrategyAction::Execute(Event& event)
{
    const std::string wanted = RaidBossMapStrategies::GetStrategy(bot->GetMapId());

    bool changed = false;
    for (const auto& entry : RaidBossMapStrategies::GetAll())
    {
        if (!ai->GetAiObjectContext()->GetStrategy(entry.second))
            continue;

        const bool active = ai->HasStrategy(entry.second, BotState::BOT_STATE_COMBAT);
        if (entry.second == wanted && !active)
        {
            ai->ChangeStrategy("+" + entry.second, BotState::BOT_STATE_COMBAT);
            ai->ChangeStrategy("+" + entry.second, BotState::BOT_STATE_NON_COMBAT);
            changed = true;
        }
        else if (entry.second != wanted && active)
        {
            ai->ChangeStrategy("-" + entry.second, BotState::BOT_STATE_COMBAT);
            ai->ChangeStrategy("-" + entry.second, BotState::BOT_STATE_NON_COMBAT);
            changed = true;
        }
    }

    return changed;
}
