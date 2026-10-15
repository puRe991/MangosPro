
#include "playerbot/playerbot.h"
#include "BotGearHelper.h"
#include "RandomItemMgr.h"
#include "playerbot/strategy/values/ItemUsageValue.h"

using namespace ai;

#ifndef MANGOSBOT_ZERO
namespace
{
    struct GemInfo
    {
        uint32 itemId;
        uint32 enchantId;
        uint32 color;
        uint32 requiredSkill;
        uint32 requiredSkillRank;
        uint32 requiredLevel;
    };

    // All gems a bot could use, built once
    const std::vector<GemInfo>& GetGems()
    {
        static std::vector<GemInfo> gems;
        static bool built = false;
        if (built)
            return gems;

        built = true;
        for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
        {
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
            if (!proto || proto->Class != ITEM_CLASS_GEM || proto->SubClass == ITEM_SUBCLASS_GEM_SIMPLE)
                continue;

            // no test / unobtainable gems, no unique gems (they can only be used once)
            if (proto->Quality > ITEM_QUALITY_EPIC || (proto->Flags & ITEM_FLAG_UNIQUE_EQUIPPABLE))
                continue;

            const std::string name = proto->Name1;
            if (name.find("Test") != std::string::npos || name.find("Deprecated") != std::string::npos || name.find("QA") == 0)
                continue;

            GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(proto->GemProperties);
            if (!gemProperty || !gemProperty->spellitemenchantement || !gemProperty->color)
                continue;

            gems.push_back({ proto->ItemId, gemProperty->spellitemenchantement, gemProperty->color,
                             proto->RequiredSkill, proto->RequiredSkillRank, proto->RequiredLevel });
        }

        return gems;
    }

    bool CanUseGem(Player* bot, const GemInfo& gem)
    {
        if (gem.requiredLevel > bot->GetLevel())
            return false;

        if (gem.requiredSkill && bot->GetSkillValue(gem.requiredSkill) < gem.requiredSkillRank)
            return false;

        return true;
    }

    // Meta gem when no meta gem has stats the spec values: by role
    uint32 FallbackMetaGem(Player* bot)
    {
        if (PlayerbotAI::IsTank(bot, false))
            return 25896;   // Powerful Earthstorm Diamond (+18 stamina)

        const uint8 playerClass = bot->getClass();
        const bool caster = playerClass == CLASS_PRIEST || playerClass == CLASS_MAGE || playerClass == CLASS_WARLOCK ||
            ((playerClass == CLASS_SHAMAN || playerClass == CLASS_DRUID) && bot->GetPlayerbotAI() && bot->GetPlayerbotAI()->IsRanged(bot));
        if (PlayerbotAI::IsHeal(bot, false) || caster)
            return 25901;   // Insightful Earthstorm Diamond (+12 intellect)

        return 32409;       // Relentless Earthstorm Diamond (+12 agility, crit damage)
    }
}
#endif

std::vector<uint32> botgear::ChooseGems(Player* bot, ItemPrototype const* proto)
{
    std::vector<uint32> chosen(MAX_GEM_SOCKETS, 0);

#ifndef MANGOSBOT_ZERO
    if (!bot || !proto)
        return chosen;

    bool hasSockets = false;
    for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
        if (proto->Socket[i].Color)
            hasSockets = true;

    if (!hasSockets)
        return chosen;

    const uint8 playerClass = bot->getClass();
    const uint8 spec = sRandomItemMgr.GetPlayerSpecId(bot);

    // Best gem of every color (meta, red, yellow, blue) and the best non meta gem of any color
    uint32 bestMeta = 0, bestMetaWeight = 0;
    uint32 bestAny = 0, bestAnyWeight = 0;
    std::map<uint32, std::pair<uint32, uint32>> bestByColor;   // socket color -> (gem, weight)

    for (const GemInfo& gem : GetGems())
    {
        if (!CanUseGem(bot, gem))
            continue;

        const uint32 weight = sRandomItemMgr.CalculateGemWeight(playerClass, spec, gem.enchantId);

        if (gem.color & SOCKET_COLOR_META)
        {
            if (weight > bestMetaWeight)
            {
                bestMeta = gem.itemId;
                bestMetaWeight = weight;
            }
            continue;
        }

        if (!weight)
            continue;

        if (weight > bestAnyWeight)
        {
            bestAny = gem.itemId;
            bestAnyWeight = weight;
        }

        for (uint32 color : { (uint32)SOCKET_COLOR_RED, (uint32)SOCKET_COLOR_YELLOW, (uint32)SOCKET_COLOR_BLUE })
        {
            if ((gem.color & color) && weight > bestByColor[color].second)
                bestByColor[color] = { gem.itemId, weight };
        }
    }

    if (!bestMeta)
    {
        const uint32 fallback = FallbackMetaGem(bot);
        if (sObjectMgr.GetItemPrototype(fallback))
            bestMeta = fallback;
    }

    // Matching colors + socket bonus against the best gems of any color
    uint32 matchingWeight = proto->socketBonus ? sRandomItemMgr.CalculateEnchantWeight(playerClass, spec, proto->socketBonus) : 0;
    uint32 anyWeight = 0;
    bool canMatch = true;
    for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
    {
        const uint32 color = proto->Socket[i].Color;
        if (!color || color == SOCKET_COLOR_META)
            continue;

        anyWeight += bestAnyWeight;
        if (bestByColor[color].first)
            matchingWeight += bestByColor[color].second;
        else
            canMatch = false;
    }

    const bool useMatching = canMatch && matchingWeight >= anyWeight;

    for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
    {
        const uint32 color = proto->Socket[i].Color;
        if (!color)
            continue;

        if (color == SOCKET_COLOR_META)
            chosen[i] = bestMeta;
        else
            chosen[i] = useMatching ? bestByColor[color].first : bestAny;
    }
#endif

    return chosen;
}

bool botgear::SocketItem(Player* bot, Item* item)
{
#ifndef MANGOSBOT_ZERO
    if (!bot || !item)
        return false;

    std::vector<uint32> gems = ChooseGems(bot, item->GetProto());

    ObjectGuid gemGuids[MAX_GEM_SOCKETS];
    bool created = false;
    for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
    {
        // keep gems that are already in the socket
        if (!gems[i] || item->GetEnchantmentId(EnchantmentSlot(SOCK_ENCHANTMENT_SLOT + i)))
            continue;

        ItemPosCountVec dest;
        if (bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, gems[i], 1) != EQUIP_ERR_OK)
            continue;

        if (Item* gem = bot->StoreNewItem(dest, gems[i], true))
        {
            gemGuids[i] = gem->GetObjectGuid();
            created = true;
        }
    }

    if (!created)
        return false;

    WorldPacket socket(CMSG_SOCKET_GEMS);
    socket << item->GetObjectGuid();
    for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
        socket << gemGuids[i];

    bot->GetSession()->HandleSocketOpcode(socket);
    return true;
#else
    return false;
#endif
}

const std::vector<uint32>& botgear::GetTierTokenRewards(uint32 tokenId)
{
    static std::unordered_map<uint32, std::vector<uint32>> rewards;
    static bool loaded = false;
    static const std::vector<uint32> none;

    if (!loaded)
    {
        loaded = true;
        auto result = WorldDatabase.Query(
            "SELECT DISTINCT item, ExtendedCost FROM npc_vendor WHERE ExtendedCost > 0 "
            "UNION SELECT DISTINCT item, ExtendedCost FROM npc_vendor_template WHERE ExtendedCost > 0");
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                const uint32 itemId = fields[0].GetUInt32();
                ItemExtendedCostEntry const* cost = sItemExtendedCostStore.LookupEntry(fields[1].GetUInt32());
                ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
                if (!cost || !proto || !proto->ItemSet)
                    continue;

                // set items that cost exactly one tier token (class 15, epic)
                for (int i = 0; i < MAX_EXTENDED_COST_ITEMS; ++i)
                {
                    ItemPrototype const* token = cost->reqitem[i] ? sObjectMgr.GetItemPrototype(cost->reqitem[i]) : nullptr;
                    if (!token || token->Class != ITEM_CLASS_MISC || token->Quality != ITEM_QUALITY_EPIC || cost->reqitemcount[i] != 1)
                        continue;

                    std::vector<uint32>& list = rewards[token->ItemId];
                    if (std::find(list.begin(), list.end(), itemId) == list.end())
                        list.push_back(itemId);
                }
            } while (result->NextRow());
        }

        sLog.outString("Playerbots: %u tier tokens can be exchanged for set items", (uint32)rewards.size());
    }

    auto it = rewards.find(tokenId);
    return it == rewards.end() ? none : it->second;
}

uint32 botgear::ChooseTierTokenReward(Player* bot, uint32 tokenId)
{
    const std::vector<uint32>& rewards = GetTierTokenRewards(tokenId);
    if (rewards.empty() || !bot)
        return 0;

    uint32 best = 0, bestWeight = 0;
    for (uint32 itemId : rewards)
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (!proto || !(proto->AllowableClass & bot->getClassMask()) || bot->CanUseItem(proto) != EQUIP_ERR_OK)
            continue;

        ItemQualifier qualifier(itemId);
        const uint32 weight = sRandomItemMgr.ItemStatWeight(bot, qualifier);
        if (!best || weight > bestWeight)
        {
            best = itemId;
            bestWeight = weight;
        }
    }

    return best;
}

bool botgear::ExchangeTierToken(Player* bot, uint32 tokenId)
{
    const uint32 reward = ChooseTierTokenReward(bot, tokenId);
    if (!reward || !bot->HasItemCount(tokenId, 1))
        return false;

    // the token frees its bag slot first
    bot->DestroyItemCount(tokenId, 1, true);

    ItemPosCountVec dest;
    if (bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, reward, 1) != EQUIP_ERR_OK)
    {
        // no space: give the token back
        ItemPosCountVec tokenDest;
        if (bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, tokenDest, tokenId, 1) == EQUIP_ERR_OK)
            bot->StoreNewItem(tokenDest, tokenId, true);
        return false;
    }

    bot->StoreNewItem(dest, reward, true);
    return true;
}