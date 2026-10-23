/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* ScriptData
SDName: npc_gear_service
SD%Complete: 100
SDComment: MangosPro custom NPCs: npc_gear_enchanter puts TBC enchants on the equipped gear of the player,
           npc_gear_gemcutter fills the sockets with the best gems for the class and talent spec of the player.
SDCategory: NPCs
EndScriptData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include <algorithm>
#include <iterator>

#ifdef ENABLE_PLAYERBOTS
#include "playerbot/playerbot.h"
#include "playerbot/AiFactory.h"
#include "playerbot/BotGearHelper.h"
#endif

namespace
{
    enum
    {
        SENDER_SLOT_MENU        = GOSSIP_SENDER_MAIN + 1,   // action: equipment slot
        SENDER_ENCHANT_BASE     = 100,                      // + equipment slot, action: enchant spell
        ACTION_SLOTS            = GOSSIP_ACTION_INFO_DEF + 1,
        ACTION_RECOMMENDED      = GOSSIP_ACTION_INFO_DEF + 2,
        ACTION_REMOVE_ENCHANT   = GOSSIP_ACTION_INFO_DEF + 3,
        ACTION_GEMS_EMPTY       = GOSSIP_ACTION_INFO_DEF + 10,
        ACTION_GEMS_ALL         = GOSSIP_ACTION_INFO_DEF + 11,
        ACTION_CLOSE            = GOSSIP_ACTION_INFO_DEF + 20,
    };

    // TBC permanent enchants: enchanting formulas, leg armor / spellthread, head glyphs, shoulder
    // inscriptions and scopes. Each item only offers the ones its class / slot can take.
    const uint32 TBC_ENCHANTS[] =
    {
        // Head
        35452, 35447, 35445, 35443, 35453, 35444, 37891, 37889,
        // Shoulder
        35417, 35439, 35406, 35437, 35404, 35435, 35402, 35433,
        // Back
        34004, 25084, 34003, 27961, 47051, 34005, 27962,
        // Chest
        27960, 27957, 33990, 33992, 46594,
        // Wrist
        27899, 34002, 27917, 27911, 27914, 27906, 34001, 27913, 27905,
        // Hands
        33995, 25080, 33996, 33997, 33999, 33994, 33993, 25072,
        // Legs
        35490, 35495, 31372, 31370, 31371, 31369,
        // Feet
        34007, 34008, 27951, 27950, 27948, 27954,
        // Finger
        27920, 27924, 27926, 27927,
        // Weapons
        27984, 42974, 46578, 27982, 27981, 27975, 34010, 27968, 28004, 28003, 27972, 27967, 42620,
        27971, 27977,
        // Shield
        34009, 27945, 27946, 44383, 27947, 27944,
        // Ranged
        30260, 30252,
    };

    const char* SlotName(uint8 slot)
    {
        switch (slot)
        {
            case EQUIPMENT_SLOT_HEAD:       return "Kopf";
            case EQUIPMENT_SLOT_SHOULDERS:  return "Schultern";
            case EQUIPMENT_SLOT_BACK:       return "Umhang";
            case EQUIPMENT_SLOT_CHEST:      return "Brust";
            case EQUIPMENT_SLOT_WRISTS:     return "Handgelenke";
            case EQUIPMENT_SLOT_HANDS:      return "Haende";
            case EQUIPMENT_SLOT_LEGS:       return "Beine";
            case EQUIPMENT_SLOT_FEET:       return "Fuesse";
            case EQUIPMENT_SLOT_FINGER1:    return "Ring 1";
            case EQUIPMENT_SLOT_FINGER2:    return "Ring 2";
            case EQUIPMENT_SLOT_MAINHAND:   return "Waffenhand";
            case EQUIPMENT_SLOT_OFFHAND:    return "Schildhand";
            case EQUIPMENT_SLOT_RANGED:     return "Distanzwaffe";
            default:                        return nullptr;
        }
    }

    std::string SpellName(Player* player, SpellEntry const* spellInfo)
    {
        const LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();
        const char* name = spellInfo->SpellName[locale];
        if (!name || !*name)
            name = spellInfo->SpellName[0];
        return name ? name : "";
    }

    // Same rules as the enchanting spell: item class, then the subclass and inventory type masks
    // (0 means no restriction, weapon / shield / scope enchants have no inventory type mask)
    bool IsEnchantable(Item* item, SpellEntry const* spellInfo)
    {
        if (!spellInfo || spellInfo->Effect[EFFECT_INDEX_0] != SPELL_EFFECT_ENCHANT_ITEM || !spellInfo->EffectMiscValue[EFFECT_INDEX_0])
            return false;

        ItemPrototype const* proto = item->GetProto();
        if (spellInfo->EquippedItemClass >= 0 && int32(proto->Class) != spellInfo->EquippedItemClass)
            return false;

        if (spellInfo->EquippedItemSubClassMask && !((1 << proto->SubClass) & spellInfo->EquippedItemSubClassMask))
            return false;

        if (spellInfo->EquippedItemInventoryTypeMask && !((1 << proto->InventoryType) & spellInfo->EquippedItemInventoryTypeMask))
            return false;

        return true;
    }

    bool ApplyEnchant(Player* player, Item* item, SpellEntry const* spellInfo)
    {
        if (!IsEnchantable(item, spellInfo))
            return false;

        const uint32 enchantId = spellInfo->EffectMiscValue[EFFECT_INDEX_0];
        player->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, false);
        item->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchantId, 0, 0, player->GetObjectGuid());
        player->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, true);
        return true;
    }

    // The enchants the playerbots use for this class and talent spec (table ai_playerbot_enchants)
    uint32 ApplyRecommendedEnchants(Player* player)
    {
#ifdef ENABLE_PLAYERBOTS
        const uint32 spec = player->getClass() * 10 + AiFactory::GetPlayerSpecTab(player);
        auto result = WorldDatabase.PQuery("SELECT spellid FROM ai_playerbot_enchants WHERE class = %u AND spec = %u", player->getClass(), spec);
        if (!result)
            return 0;

        uint32 count = 0;
        do
        {
            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(result->Fetch()[0].GetUInt32());
            if (!spellInfo)
                continue;

            for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
            {
                Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                if (item && item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT) != uint32(spellInfo->EffectMiscValue[EFFECT_INDEX_0]) &&
                    ApplyEnchant(player, item, spellInfo))
                    ++count;
            }
        }
        while (result->NextRow());

        return count;
#else
        return 0;
#endif
    }

    void SendEnchanterMain(Player* player, Creature* creature)
    {
        player->PrepareGossipMenu(creature, 0);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "Ausruestung selbst verzaubern (Slot waehlen)", GOSSIP_SENDER_MAIN, ACTION_SLOTS);
#ifdef ENABLE_PLAYERBOTS
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "Empfohlene Verzauberungen fuer meine Skillung", GOSSIP_SENDER_MAIN, ACTION_RECOMMENDED);
#endif
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Auf Wiedersehen", GOSSIP_SENDER_MAIN, ACTION_CLOSE);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetObjectGuid());
    }

    void SendSlotMenu(Player* player, Creature* creature)
    {
        player->PrepareGossipMenu(creature, 0);
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            const char* slotName = SlotName(slot);
            Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (!slotName || !item)
                continue;

            std::string text = std::string(slotName) + ": " + item->GetProto()->Name1;
            if (uint32 enchantId = item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT))
                if (SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchantId))
                    if (enchant->description[0])
                        text += std::string(" [") + enchant->description[0] + "]";

            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, text.c_str(), SENDER_SLOT_MENU, slot);
        }
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Zurueck", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetObjectGuid());
    }

    void SendEnchantMenu(Player* player, Creature* creature, uint8 slot)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item)
        {
            SendSlotMenu(player, creature);
            return;
        }

        player->PrepareGossipMenu(creature, 0);
        for (uint32 spellId : TBC_ENCHANTS)
        {
            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
            if (!IsEnchantable(item, spellInfo))
                continue;

            const bool active = item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT) == uint32(spellInfo->EffectMiscValue[EFFECT_INDEX_0]);
            const std::string text = (active ? "* " : "") + SpellName(player, spellInfo);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, text.c_str(), SENDER_ENCHANT_BASE + slot, spellId);
        }

        if (item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT, "Verzauberung entfernen", SENDER_ENCHANT_BASE + slot, ACTION_REMOVE_ENCHANT);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Zurueck", GOSSIP_SENDER_MAIN, ACTION_SLOTS);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetObjectGuid());
    }
}

bool GossipHello_npc_gear_enchanter(Player* player, Creature* creature)
{
    SendEnchanterMain(player, creature);
    return true;
}

bool GossipSelect_npc_gear_enchanter(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (sender == GOSSIP_SENDER_MAIN)
    {
        switch (action)
        {
            case ACTION_SLOTS:
                SendSlotMenu(player, creature);
                return true;
            case ACTION_RECOMMENDED:
            {
                const uint32 count = ApplyRecommendedEnchants(player);
                creature->MonsterWhisper(count ? "Deine Ausruestung traegt jetzt die empfohlenen Verzauberungen." :
                    "Es gab nichts zu verbessern.", player);
                SendEnchanterMain(player, creature);
                return true;
            }
            case ACTION_CLOSE:
                player->CLOSE_GOSSIP_MENU();
                return true;
            default:
                SendEnchanterMain(player, creature);
                return true;
        }
    }

    if (sender == SENDER_SLOT_MENU)
    {
        if (action < EQUIPMENT_SLOT_END)
            SendEnchantMenu(player, creature, uint8(action));
        else
            SendSlotMenu(player, creature);
        return true;
    }

    if (sender >= SENDER_ENCHANT_BASE && sender < SENDER_ENCHANT_BASE + EQUIPMENT_SLOT_END)
    {
        const uint8 slot = uint8(sender - SENDER_ENCHANT_BASE);
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (item && action == ACTION_REMOVE_ENCHANT)
        {
            player->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, false);
            item->ClearEnchantment(PERM_ENCHANTMENT_SLOT);
        }
        else if (item && std::find(std::begin(TBC_ENCHANTS), std::end(TBC_ENCHANTS), action) != std::end(TBC_ENCHANTS))
            ApplyEnchant(player, item, sSpellTemplate.LookupEntry<SpellEntry>(action));

        SendEnchantMenu(player, creature, slot);
        return true;
    }

    player->CLOSE_GOSSIP_MENU();
    return true;
}

namespace
{
    void SendGemcutterMain(Player* player, Creature* creature)
    {
        player->PrepareGossipMenu(creature, 0);
#ifdef ENABLE_PLAYERBOTS
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "Leere Sockel mit den besten Edelsteinen fuellen", GOSSIP_SENDER_MAIN, ACTION_GEMS_EMPTY);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "Alle Sockel neu mit den besten Edelsteinen besetzen", GOSSIP_SENDER_MAIN, ACTION_GEMS_ALL);
#endif
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Auf Wiedersehen", GOSSIP_SENDER_MAIN, ACTION_CLOSE);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetObjectGuid());
    }

    // Sockets the equipped gear. The gems follow the class and talent spec of the player (stat weights
    // of the playerbots): matching colors only when the socket bonus is worth it, meta gem by role.
    uint32 SocketEquippedGear(Player* player, bool replaceGems)
    {
        uint32 count = 0;
#ifdef ENABLE_PLAYERBOTS
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (!item)
                continue;

            bool hasSockets = false;
            for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
                hasSockets |= item->GetProto()->Socket[i].Color != 0;
            if (!hasSockets)
                continue;

            if (replaceGems)
            {
                // The old gems are destroyed: socketing in TBC destroys a replaced gem as well
                const std::vector<uint32> gems = ai::botgear::ChooseGems(player, item->GetProto());
                bool changed = false;
                for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
                {
                    const EnchantmentSlot enchantSlot = EnchantmentSlot(SOCK_ENCHANTMENT_SLOT + i);
                    const uint32 current = item->GetEnchantmentId(enchantSlot);
                    ItemPrototype const* gemProto = gems[i] ? sObjectMgr.GetItemPrototype(gems[i]) : nullptr;
                    GemPropertiesEntry const* gemProperty = gemProto ? sGemPropertiesStore.LookupEntry(gemProto->GemProperties) : nullptr;
                    if (!current || (gemProperty && gemProperty->spellitemenchantement == current))
                        continue;

                    player->ApplyEnchantment(item, enchantSlot, false);
                    item->ClearEnchantment(enchantSlot);
                    changed = true;
                }

                if (changed)
                {
                    // the socket bonus is reapplied by the socketing below
                    player->ApplyEnchantment(item, BONUS_ENCHANTMENT_SLOT, false);
                    item->ClearEnchantment(BONUS_ENCHANTMENT_SLOT);
                }
            }

            if (ai::botgear::SocketItem(player, item))
                ++count;
        }
#endif
        return count;
    }
}

bool GossipHello_npc_gear_gemcutter(Player* player, Creature* creature)
{
    SendGemcutterMain(player, creature);
    return true;
}

bool GossipSelect_npc_gear_gemcutter(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
{
    switch (action)
    {
        case ACTION_GEMS_EMPTY:
        case ACTION_GEMS_ALL:
        {
            const uint32 count = SocketEquippedGear(player, action == ACTION_GEMS_ALL);
            creature->MonsterWhisper(count ? "Deine Ausruestung ist jetzt gesockelt." :
                "Nichts zu sockeln (oder kein Platz in den Taschen fuer die Edelsteine).", player);
            SendGemcutterMain(player, creature);
            break;
        }
        default:
            player->CLOSE_GOSSIP_MENU();
            break;
    }

    return true;
}

void AddSC_npc_gear_service()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_gear_enchanter";
    pNewScript->pGossipHello = &GossipHello_npc_gear_enchanter;
    pNewScript->pGossipSelect = &GossipSelect_npc_gear_enchanter;
    pNewScript->RegisterSelf(false);

    pNewScript = new Script;
    pNewScript->Name = "npc_gear_gemcutter";
    pNewScript->pGossipHello = &GossipHello_npc_gear_gemcutter;
    pNewScript->pGossipSelect = &GossipSelect_npc_gear_gemcutter;
    pNewScript->RegisterSelf(false);
}
