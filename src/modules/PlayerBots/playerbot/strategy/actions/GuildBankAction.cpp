
#include "playerbot/playerbot.h"
#include "GuildBankAction.h"

#include "playerbot/strategy/values/ItemCountValue.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "Guilds/Guild.h"
#include "Guilds/GuildMgr.h"
#include "playerbot/strategy/values/GuildValues.h"

using namespace ai;

bool GuildBankAction::Execute(Event& event)
{
#ifndef MANGOSBOT_ZERO
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    if (text.empty())
        return false;

    if (!bot->GetGuildId() || (requester && requester->GetGuildId() != bot->GetGuildId()))
    {
        ai->TellPlayer(requester, "I'm not in your guild!");
            return false;
    }

    std::list<ObjectGuid> gos = *ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest game objects no los");
    for (std::list<ObjectGuid>::iterator i = gos.begin(); i != gos.end(); ++i)
    {
        GameObject* go = ai->GetGameObject(*i);
        if (!go || !bot->GetGameObjectIfCanInteractWith(go->GetObjectGuid(), GAMEOBJECT_TYPE_GUILD_BANK))
            continue;

        return Execute(text, go, requester);
    }

    ai->TellPlayer(requester, BOT_TEXT("error_gbank_found"));
    return false;
#else
    return false;
#endif
}

bool GuildBankAction::Execute(std::string text, GameObject* bank, Player* requester)
{
    bool result = true;

    IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS);

    std::list<Item*> found = ai->InventoryParseItems(text, mask);
    if (found.empty())
        return false;

    for (std::list<Item*>::iterator i = found.begin(); i != found.end(); i++)
    {
        Item* item = *i;
        if (item)
            result &= MoveFromCharToBank(item, bank, requester);
    }

    return result;
}

bool GuildBankAction::MoveFromCharToBank(Item* item, GameObject* bank, Player* requester)
{
#ifndef MANGOSBOT_ZERO
    uint32 playerSlot = item->GetSlot();
    uint32 playerBag = item->GetBagSlot();
    std::ostringstream out;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    //guild->SwapItems(bot, 0, playerSlot, 0, INVENTORY_SLOT_BAG_0, 0);

    // check source pos rights (item moved to bank)
    if (!guild->IsMemberHaveRights(bot->GetGUIDLow(), 0, GUILD_BANK_RIGHT_DEPOSIT_ITEM))
        out << BOT_TEXT("error_cant_put") << chat->formatItem(item) << BOT_TEXT("error_gbank_rights");
    else
    {
        out << chat->formatItem(item) << BOT_TEXT("gbank_put");
        guild->MoveFromCharToBank(bot, playerBag, playerSlot, 0, 255, 0);
    }

    ai->TellPlayer(requester, out);
    return true;
#else
    return false;
#endif
}

#ifndef MANGOSBOT_ZERO
int32 GuildBankAction::FindDepositTab(Guild* guild, Player* bot, ItemPrototype const* proto, uint32 count)
{
    GuildAccess* guildAccess = static_cast<GuildAccess*>(guild);
    uint32 maxStack = std::max<uint32>(proto->GetMaxStackSize(), 1);

    for (uint8 tabId = 0; tabId < guild->GetPurchasedTabs(); ++tabId)
    {
        if (!guild->IsMemberHaveRights(bot->GetGUIDLow(), tabId, GUILD_BANK_RIGHT_DEPOSIT_ITEM))
            continue;

        uint32 room = 0;
        for (uint8 slotId = 0; slotId < GUILD_BANK_MAX_SLOTS && room < count; ++slotId)
        {
            Item* bankItem = guildAccess->GetGuildItem(tabId, slotId);
            if (!bankItem)
                room += maxStack;
            else if (bankItem->GetEntry() == proto->ItemId && bankItem->GetCount() < maxStack)
                room += maxStack - bankItem->GetCount();
        }

        if (room >= count)
            return tabId;
    }

    return -1;
}

std::vector<std::pair<Item*, uint32>> GuildBankAction::GetDepositItems(PlayerbotAI* ai)
{
    std::vector<std::pair<Item*, uint32>> result;

    Player* bot = ai->GetBot();
    AiObjectContext* context = ai->GetAiObjectContext();

    if (!bot->GetGuildId())
        return result;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild || guild->GetPurchasedTabs() == 0)
        return result;

    GuildAccess* guildAccess = static_cast<GuildAccess*>(guild);

    // Items the bot has no use for itself (it would otherwise sell them to a vendor or on the AH).
    std::map<uint32, std::vector<Item*>> surplus;
    for (uint8 usageType : {(uint8)ItemUsage::ITEM_USAGE_VENDOR, (uint8)ItemUsage::ITEM_USAGE_AH})
    {
        std::list<Item*> items = AI_VALUE2(std::list<Item*>, "inventory items", "usage " + std::to_string(usageType));
        for (auto item : items)
        {
            if (!item || !item->CanBeTraded())
                continue;

            ItemPrototype const* proto = item->GetProto();
            if (!proto || proto->Quality < ITEM_QUALITY_NORMAL || proto->Bonding == BIND_WHEN_PICKED_UP || proto->Bonding == BIND_QUEST_ITEM)
                continue;

            bool isMaterial = proto->Class == ITEM_CLASS_TRADE_GOODS || proto->Class == ITEM_CLASS_REAGENT || proto->Class == ITEM_CLASS_GEM;
            bool isConsumable = proto->Class == ITEM_CLASS_CONSUMABLE;
            bool isCrafted = !item->GetGuidValue(ITEM_FIELD_CREATOR).IsEmpty()
                && (proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON || proto->Class == ITEM_CLASS_CONTAINER)
                && proto->Quality >= ITEM_QUALITY_UNCOMMON;

            if (!isMaterial && !isConsumable && !isCrafted)
                continue;

            surplus[proto->ItemId].push_back(item);
        }
    }

    for (auto& [itemId, items] : surplus)
    {
        ItemPrototype const* proto = items.front()->GetProto();

        // Flasks, elixirs and potions the bot can use itself: keep a stack (at least 5) for raids and hand over the rest.
        uint32 keep = 0;
        if (proto->Class == ITEM_CLASS_CONSUMABLE && proto->RequiredLevel <= bot->GetLevel() && bot->CanUseItem(proto) == EQUIP_ERR_OK)
            keep = std::max<uint32>(proto->GetMaxStackSize(), 5);

        // Gear does not stack: one copy of each crafted piece in the bank is enough.
        if (proto->GetMaxStackSize() <= 1 && proto->Class != ITEM_CLASS_CONSUMABLE)
        {
            bool inBank = false;
            for (uint8 tabId = 0; tabId < guild->GetPurchasedTabs() && !inBank; ++tabId)
                for (uint8 slotId = 0; slotId < GUILD_BANK_MAX_SLOTS && !inBank; ++slotId)
                    if (Item* bankItem = guildAccess->GetGuildItem(tabId, slotId))
                        inBank = bankItem->GetEntry() == itemId;

            if (inBank)
                continue;

            items.resize(1);
        }

        for (auto item : items)
        {
            uint32 count = item->GetCount();
            if (keep >= count)
            {
                keep -= count;
                continue;
            }

            count -= keep;
            keep = 0;

            if (FindDepositTab(guild, bot, proto, count) < 0)
                break;

            result.push_back(std::make_pair(item, count));
        }
    }

    return result;
}

bool GuildBankAction::ShouldWithdrawItem(PlayerbotAI* ai, Item* item)
{
    Player* bot = ai->GetBot();
    AiObjectContext* context = ai->GetAiObjectContext();

    ItemPrototype const* proto = item->GetProto();
    if (!proto)
        return false;

    ItemQualifier qualifier(item);

    // Equip upgrade.
    if (proto->InventoryType != INVTYPE_NON_EQUIP && bot->CanUseItem(proto) == EQUIP_ERR_OK)
    {
        if (ItemUsageValue::QueryItemUsageForEquip(qualifier, bot) == ItemUsage::ITEM_USAGE_EQUIP)
            return true;
    }

    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", qualifier.GetQualifier());

    // Needed for crafting.
    if ((proto->Class == ITEM_CLASS_TRADE_GOODS || proto->Class == ITEM_CLASS_REAGENT || proto->Class == ITEM_CLASS_GEM) && usage == ItemUsage::ITEM_USAGE_SKILL)
        return true;

    // Food, potions, bandages or spell reagents the bot has run out of.
    if ((proto->Class == ITEM_CLASS_CONSUMABLE || proto->Class == ITEM_CLASS_REAGENT) && usage == ItemUsage::ITEM_USAGE_USE)
        return true;

    return false;
}
#endif

bool GuildBankAction::AutoDeposit(GameObject* bank)
{
#ifndef MANGOSBOT_ZERO
    if (!bot->GetGuildId())
        return false;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild || guild->GetPurchasedTabs() == 0)
        return false;

    bool deposited = false;

    for (auto& [item, count] : GetDepositItems(ai))
    {
        ItemPrototype const* proto = item->GetProto();
        int32 tabId = FindDepositTab(guild, bot, proto, count);
        if (tabId < 0)
            continue;

        uint8 playerBag = item->GetBagSlot();
        uint8 playerSlot = item->GetSlot();
        uint32 before = bot->GetItemCount(proto->ItemId);

        guild->MoveFromCharToBank(bot, playerBag, playerSlot, tabId, NULL_SLOT, count < item->GetCount() ? count : 0);

        if (bot->GetItemCount(proto->ItemId) < before)
            deposited = true;
    }

    return deposited;
#else
    return false;
#endif
}

bool GuildBankAction::AutoWithdraw(GameObject* bank)
{
#ifndef MANGOSBOT_ZERO
    if (!bot->GetGuildId())
        return false;

    if (AI_VALUE(uint8, "bag space") > 80)
        return false;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild || guild->GetPurchasedTabs() == 0)
        return false;

    bool withdrew = false;
    std::set<uint32> taken;
    GuildAccess* guildAccess = static_cast<GuildAccess*>(guild);

    for (uint8 tabId = 0; tabId < guild->GetPurchasedTabs(); ++tabId)
    {
        if (!guild->IsMemberHaveRights(bot->GetGUIDLow(), tabId, GUILD_BANK_RIGHT_VIEW_TAB))
            continue;

        for (uint8 slotId = 0; slotId < GUILD_BANK_MAX_SLOTS; ++slotId)
        {
            Item* item = guildAccess->GetGuildItem(tabId, slotId);
            if (!item || taken.count(item->GetEntry()) || !ShouldWithdrawItem(ai, item))
                continue;

            uint32 entry = item->GetEntry();
            uint32 before = bot->GetItemCount(entry);

            // One stack per item and visit: item usage is cached and would still ask for more.
            taken.insert(entry);
            guild->MoveFromBankToChar(bot, tabId, slotId, INVENTORY_SLOT_BAG_0, NULL_SLOT, 0);

            if (bot->GetItemCount(entry) > before)
                withdrew = true;

            if (AI_VALUE(uint8, "bag space") > 80)
                return withdrew;
        }
    }

    return withdrew;
#else
    return false;
#endif
}
