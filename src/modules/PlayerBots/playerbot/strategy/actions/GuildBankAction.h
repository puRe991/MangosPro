#pragma once
#include "GenericActions.h"

class Guild;

namespace ai
{
    class GuildBankAction : public ChatCommandAction
    {
    public:
        GuildBankAction(PlayerbotAI* ai) : ChatCommandAction(ai, "guild bank") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        bool Execute(std::string text, GameObject* bank, Player* requester);
        bool MoveFromCharToBank(Item* item, GameObject* bank, Player* requester);

    public:
        bool AutoDeposit(GameObject* bank);
        bool AutoWithdraw(GameObject* bank);

#ifndef MANGOSBOT_ZERO
        // Surplus mats, consumables and crafted items with the amount to hand over. Only items that fit in a tab the bot may deposit to.
        static std::vector<std::pair<Item*, uint32>> GetDepositItems(PlayerbotAI* ai);
        static bool ShouldWithdrawItem(PlayerbotAI* ai, Item* item);

    private:
        static int32 FindDepositTab(Guild* guild, Player* bot, ItemPrototype const* proto, uint32 count);
#endif
    };
}
