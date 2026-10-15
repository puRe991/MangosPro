
#include "playerbot/playerbot.h"
#include "LootRollAction.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "playerbot/strategy/values/LootValues.h"
#include "playerbot/RandomItemMgr.h"
#include "playerbot/RandomPlayerbotMgr.h"
#include "playerbot/BotGearHelper.h"

using namespace ai;

// Upgrade of the item over the gear the player wears in that slot, in percent of the stat weight.
// -1 when the item is no equip upgrade for the player (or the player is no bot).
static float GetUpgradePct(Player* player, ItemQualifier& itemQualifier)
{
    PlayerbotAI* playerAi = player->GetPlayerbotAI();
    if (!playerAi || playerAi->IsRealPlayer())
        return -1.0f;

    // Tier token: compare the set item the player would trade it for
    if (const uint32 tokenReward = botgear::ChooseTierTokenReward(player, itemQualifier.GetId()))
    {
        ItemQualifier rewardQualifier(tokenReward);
        if (playerAi->GetAiObjectContext()->GetValue<ItemUsage>("item usage", itemQualifier.GetQualifier())->Get() != ItemUsage::ITEM_USAGE_EQUIP)
            return -1.0f;

        uint16 dest;
        if (RandomPlayerbotMgr::CanEquipUnseenItem(player, NULL_SLOT, dest, tokenReward) != EQUIP_ERR_OK)
            return -1.0f;

        Item* oldItem = player->GetItemByPos(dest);
        const uint32 oldWeight = oldItem ? sRandomItemMgr.ItemStatWeight(player, oldItem) : 0;
        if (!oldWeight)
            return 100.0f;

        return (float(sRandomItemMgr.ItemStatWeight(player, rewardQualifier)) - float(oldWeight)) * 100.0f / float(oldWeight);
    }

    ItemUsage usage = playerAi->GetAiObjectContext()->GetValue<ItemUsage>("item usage", itemQualifier.GetQualifier())->Get();
    if (usage != ItemUsage::ITEM_USAGE_EQUIP)
        return -1.0f;

    uint16 dest;
    if (RandomPlayerbotMgr::CanEquipUnseenItem(player, NULL_SLOT, dest, itemQualifier.GetId()) != EQUIP_ERR_OK)
        return -1.0f;

    Item* oldItem = player->GetItemByPos(dest);
    const uint32 oldWeight = oldItem ? sRandomItemMgr.ItemStatWeight(player, oldItem) : 0;
    if (!oldWeight)
        return 100.0f;

    const uint32 newWeight = sRandomItemMgr.ItemStatWeight(player, itemQualifier);
    return (float(newWeight) - float(oldWeight)) * 100.0f / float(oldWeight);
}

bool LootStartRollAction::Execute(Event& event)
{
    WorldPacket p(event.getPacket()); //WorldPacket packet for CMSG_LOOT_ROLL, (8+4+1)
    ObjectGuid creatureGuid;
    uint32 itemSlot;
    uint32 itemId;
    uint32 randomSuffix;
    int32 randomPropertyId;
#ifdef MANGOSBOT_TWO
    uint32 mapId;
    uint32 count;
#endif 
    uint32 timeout;

    p.rpos(0); //reset packet pointer
    p >> creatureGuid; //creature guid what we're looting
#ifdef MANGOSBOT_TWO
    p >> mapId; /// 3.3.3 mapid
#endif 
    p >> itemSlot; // the itemEntryId for the item that shall be rolled for
    p >> itemId; // the itemEntryId for the item that shall be rolled for
    p >> randomSuffix; // randomSuffix
    p >> randomPropertyId; // item random property ID
#ifdef MANGOSBOT_TWO
    p >> count; // items in stack
#endif 
    p >> timeout;  // the countdown time to choose "need" or "greed"

    LootRollMap lootRolls = AI_VALUE(LootRollMap, "active rolls");

    if (lootRolls.find(creatureGuid) != lootRolls.end())
        return false;

    Loot* loot = sLootMgr.GetLoot(bot, creatureGuid);
    if (!loot)
        return false;

    for(uint8 i=0;i< MAX_NR_LOOT_ITEMS;i++)
        if(loot->GetRollForSlot(i))
            lootRolls.insert({ creatureGuid, i });
        
    ActiveRolls::CleanUp(bot,lootRolls);

    SET_AI_VALUE(LootRollMap, "active rolls", lootRolls);

    return false;
}

bool RollAction::Execute(Event& event)
{      
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();

    if (text.empty())
    {
        ai->TellPlayerNoFacing(requester, "Please give a roll type or item. See " + ChatHelper::formatValue("help", "action:roll", "roll help") + " for more information.");
        return false;
    }

    ItemIds ids = ChatHelper::parseItems(text);

    std::string type = "auto";
    if (ids.empty())
        type = text;
    else
        type = text.substr(0, text.find(" "));

    if (type == "emote")
    {
        std::vector<std::string> args = ChatHelper::splitString(text, " ");

        if (args.size() == 2)
            args = { args[0], "1", args[1] };
        if (args.size() == 1)
            args = { args[0], "1", "100" };

        for (char& d : args[1]) //Check if itemId contains only numbers
            if (!isdigit(d))
                return false;

        for (char& d : args[2]) //Check if itemId contains only numbers
            if (!isdigit(d))
                return false;

        WorldPacket data(MSG_RANDOM_ROLL);
        data << stoi(args[1]);
        data << stoi(args[2]);
        bot->GetSession()->HandleRandomRollOpcode(data);

        return true;
    }

    bool rollFeedback = AI_VALUE2(bool, "manual bool", "roll feedback");

    if (type == "feedback")
    {
        rollFeedback = !rollFeedback;

        if (!rollFeedback)
            ai->TellPlayerNoFacing(requester, "Roll feedback disabled.");
        else
            ai->TellPlayerNoFacing(requester, "Roll feedback enalbed.");

        SET_AI_VALUE2(bool, "manual bool", "roll feedback", rollFeedback);

        return true;
    }

    if (!bot->GetGroup())
        return false;

    if (AI_VALUE(LootRollMap, "active rolls").empty())
        return false;

    if (AI_VALUE(uint8, "bag space") >= 100)
        return false;

    if (type != "need" && type != "greed" && type != "pass" && type != "auto")
    {
        ai->TellPlayerNoFacing(requester, "Please give a correct roll type. need, greed, pass or auto. See " + ChatHelper::formatValue("help", "action:roll", "roll help") + " for more information.");
        return false;
    }

    RollVote vote = ROLL_NOT_VALID;

    if (type.find("need") == 0)
        vote = ROLL_NEED;
    else if (type.find("greed") == 0)
        vote = ROLL_GREED;
    else if (type.find("pass") == 0)
        vote = ROLL_PASS;

    uint32 rolledItems = 0;

    LootRollMap lootRolls = AI_VALUE(LootRollMap, "active rolls");

    for (auto roll : lootRolls)
    {
        ItemQualifier itemQualifier = GetRollItem(roll.first, roll.second);

        if (!itemQualifier.GetId())
            continue;

        if (!ids.empty() && ids.find(itemQualifier.GetId()) == ids.end())
            continue;

        RollVote doVote = vote;
        if (doVote == ROLL_NOT_VALID) //Auto
            doVote = CalculateRollVote(itemQualifier);

        rolledItems += RollOnItemInSlot(doVote, roll.first, roll.second);     
    }

    return rolledItems;
}

ItemQualifier RollAction::GetRollItem(ObjectGuid lootGuid, uint32 slot)
{
    Loot* loot = sLootMgr.GetLoot(bot, lootGuid);
    if (!loot)
        return ItemQualifier();

    LootItem* item = loot->GetLootItemInSlot(slot);

    if (!item)
        return ItemQualifier();

    return ItemQualifier(item);
}

RollVote RollAction::CalculateRollVote(ItemQualifier& itemQualifier)
{
    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", itemQualifier.GetQualifier());

    RollVote needVote = ROLL_PASS;
    switch (usage)
    {
    case ItemUsage::ITEM_USAGE_EQUIP:
    case ItemUsage::ITEM_USAGE_GUILD_TASK:
    case ItemUsage::ITEM_USAGE_FORCE_NEED:
        needVote = ROLL_NEED;
        break;
    case ItemUsage::ITEM_USAGE_SKILL:
    case ItemUsage::ITEM_USAGE_USE:
    case ItemUsage::ITEM_USAGE_AH:
    case ItemUsage::ITEM_USAGE_BROKEN_AH:
    case ItemUsage::ITEM_USAGE_VENDOR:
    case ItemUsage::ITEM_USAGE_FORCE_GREED:
        needVote = ROLL_GREED;
        break;
    case ItemUsage::ITEM_USAGE_DISENCHANT:
#ifndef MANGOSBOT_TWO
        needVote = ROLL_GREED;
#else
        needVote = ROLL_DISENCHANT;
#endif
        break;
    }

    // special case for bad equip
    if (usage == ItemUsage::ITEM_USAGE_BAD_EQUIP)
    {
        bool shouldEquipBadItems = sPlayerbotAIConfig.rollBadItemsWithPlayer || !ai->HasRealPlayerMaster();
        if (shouldEquipBadItems)
            needVote = ROLL_NEED;
        else
            needVote = ROLL_GREED;
    }

    // Need only for real upgrades, and with a real player in the group only the bot that gains the most
    std::string needReason;
    if (needVote == ROLL_NEED && usage == ItemUsage::ITEM_USAGE_EQUIP)
    {
        const float upgradePct = GetUpgradePct(bot, itemQualifier);
        if (upgradePct >= 0.0f && upgradePct < sPlayerbotAIConfig.needRollMinUpgradePct)
        {
            needVote = ROLL_GREED;
            needReason = " (upgrade too small)";
        }
        else if (upgradePct >= 0.0f && sPlayerbotAIConfig.needRollBestUpgradeOnly && ai->HasRealPlayerMaster() && bot->GetGroup())
        {
            for (GroupReference* ref = bot->GetGroup()->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->getSource();
                if (!member || member == bot || !member->IsInWorld() || !member->IsAtGroupRewardDistance(bot))
                    continue;

                const float memberPct = GetUpgradePct(member, itemQualifier);
                if (memberPct > upgradePct || (memberPct == upgradePct && member->GetObjectGuid() < bot->GetObjectGuid()))
                {
                    needVote = ROLL_GREED;
                    needReason = std::string(" (") + member->GetName() + " needs it more)";
                    break;
                }
            }
        }
    }

    bool canLoot = StoreLootAction::IsLootAllowed(itemQualifier, bot->GetPlayerbotAI());

    if (AI_VALUE2(bool, "manual bool", "roll feedback"))
    {
        std::string reason = "because it can not be looted.";
        std::string vote = "Passing";
        if(canLoot)
            reason = ItemUsageValue::ReasonForNeed(usage, itemQualifier, 1, bot) + needReason;

        if (needVote == ROLL_GREED)
            vote = "Rolling greed";
        else if (needVote == ROLL_NEED)
            vote = "Rolling need";
        else if (needVote == ROLL_DISENCHANT)
            vote = "Rolling disenchant";

         ai->TellPlayerNoFacing(ai->GetMaster(), vote + " on " + ChatHelper::formatItem(itemQualifier) + " " + reason);
    }

    return canLoot ? needVote : ROLL_PASS;
}

bool RollAction::RollOnItemInSlot(RollVote vote, ObjectGuid lootGuid, uint32 slot)
{
    Loot* loot = sLootMgr.GetLoot(bot, lootGuid);
    if (!loot)
        return false;

    LootItem* item = loot->GetLootItemInSlot(slot);
    ItemPrototype const* proto = sItemStorage.LookupEntry<ItemPrototype>(item->itemId);
    if (!proto)
        return false;

    GroupLootRoll* lootRoll = loot->GetRollForSlot(slot);
    if (!lootRoll)
        return false;

    bool didRoll = lootRoll->PlayerVote(bot, vote);

    if (didRoll)
    {
        LootRollMap lootRolls = AI_VALUE(LootRollMap, "active rolls");

        ActiveRolls::CleanUp(bot, lootRolls, lootGuid, slot);

        SET_AI_VALUE(LootRollMap, "active rolls", lootRolls);
    }

    return didRoll;
}

bool LootRollAction::Execute(Event& event)
{
    Player* bot = QueryItemUsageAction::ai->GetBot();

    WorldPacket p(event.getPacket()); //WorldPacket packet for CMSG_LOOT_ROLL, (8+4+1)
    ObjectGuid guid;
    uint32 slot;
    uint8 rollType;
    p.rpos(0); //reset packet pointer
    p >> guid; //guid of the item rolled
    p >> slot; //number of players invited to roll
    p >> rollType; //need,greed or pass on roll

    ItemQualifier itemQualifier = GetRollItem(guid, slot);

    if (!itemQualifier.GetId())
        return false;

    RollVote vote = CalculateRollVote(itemQualifier);

    return RollOnItemInSlot(vote, guid, slot);
}

bool AutoLootRollAction::Execute(Event& event)
{
    LootRollMap lootRolls = AI_VALUE(LootRollMap, "active rolls");

    auto currentRoll = lootRolls.begin();

    currentRoll = std::next(currentRoll, urand(0, lootRolls.size() - 1));

    ItemQualifier itemQualifier = GetRollItem(currentRoll->first, currentRoll->second);

    if (!itemQualifier.GetId())
        return false;

    RollVote vote = CalculateRollVote(itemQualifier);

    return RollOnItemInSlot(vote, currentRoll->first, currentRoll->second);
}

bool AutoLootRollAction::isPossible()
{
    return bot->GetGroup() && !AI_VALUE(LootRollMap, "active rolls").empty() && AI_VALUE(uint8, "bag space") < 100;
}