#pragma once

#include <vector>

class Player;
class Item;
struct ItemPrototype;

namespace ai
{
    // Gems for the gear of the bots
    namespace botgear
    {
        // Gem item ids for the sockets of the item (0 = no gem for that socket).
        // Uses the best gems for the spec of the bot (stat weights), takes the socket bonus only when
        // matching colors are worth more than the best gems of any color, meta gem by stat weight / role.
        std::vector<uint32> ChooseGems(Player* bot, ItemPrototype const* proto);

        // Creates the chosen gems and puts them into the empty sockets of the item. True if a gem was socketed.
        bool SocketItem(Player* bot, Item* item);

        // Tier tokens (T4-T6): set items a vendor trades for the token (from the vendor extended costs)
        const std::vector<uint32>& GetTierTokenRewards(uint32 tokenId);

        // Best set item of the token for the class and spec of the bot, 0 if the item is no token for the bot
        uint32 ChooseTierTokenReward(Player* bot, uint32 tokenId);

        // Trades a tier token in the bags for the best set item. True if the token was exchanged.
        bool ExchangeTierToken(Player* bot, uint32 tokenId);
    }
}
