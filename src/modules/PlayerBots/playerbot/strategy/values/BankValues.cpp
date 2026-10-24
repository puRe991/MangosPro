
#include "playerbot/playerbot.h"
#include "BankValues.h"
#include "ItemCountValue.h"
#include "Guilds/Guild.h"
#include "Guilds/GuildMgr.h"
#include "playerbot/strategy/values/GuildValues.h"
#include "playerbot/strategy/actions/GuildBankAction.h"

using namespace ai;

bool ShouldBankDepositValue::Calculate()
{
    if (AI_VALUE(uint8, "bank space") > 80)
        return false;

    // Check if we have items worth banking
    uint32 toBankCount = AI_VALUE2(uint32, "item count", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_BANK));

    if (toBankCount == 0)
        return false;

    return true;
}

bool ShouldBankWithdrawValue::Calculate()
{
    if (AI_VALUE(uint8, "bag space") > 80)
        return false;

    uint32 inBankCount = AI_VALUE2(uint32, "bank item count", "all");
    uint32 stayInBankCount = AI_VALUE2(uint32, "bank item count", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_BANK));

    if (inBankCount == stayInBankCount)
        return false;    

    return true;
}

#ifndef MANGOSBOT_ZERO
bool ShouldGuildBankDepositValue::Calculate()
{
    return !GuildBankAction::GetDepositItems(ai).empty();
}

bool ShouldGuildBankWithdrawValue::Calculate()
{
    if (!bot->GetGuildId())
        return false;

    if (AI_VALUE(uint8, "bag space") > 80)
        return false;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild || guild->GetPurchasedTabs() == 0)
        return false;

    GuildAccess* guildAccess = static_cast<GuildAccess*>(guild);

    for (uint8 tabId = 0; tabId < guild->GetPurchasedTabs(); ++tabId)
    {
        if (!guild->IsMemberHaveRights(bot->GetGUIDLow(), tabId, GUILD_BANK_RIGHT_VIEW_TAB))
            continue;

        for (uint8 slotId = 0; slotId < GUILD_BANK_MAX_SLOTS; ++slotId)
        {
            Item* item = guildAccess->GetGuildItem(tabId, slotId);
            if (item && GuildBankAction::ShouldWithdrawItem(ai, item))
                return true;
        }
    }

    return false;
}
#endif
