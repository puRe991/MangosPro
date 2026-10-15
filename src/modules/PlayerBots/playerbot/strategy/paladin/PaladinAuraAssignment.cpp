
#include "playerbot/playerbot.h"
#include "PaladinAuraAssignment.h"
#include "playerbot/strategy/generic/TbcRaidEncounters.h"

using namespace ai;

namespace
{
    const std::vector<std::string> allAuras = {
        "devotion aura", "retribution aura", "concentration aura", "sanctity aura",
        "shadow resistance aura", "fire resistance aura", "frost resistance aura", "crusader aura"
    };

    bool KnowsAura(Player* player, const std::string& aura)
    {
        for (uint32 spellId : ChatHelper::SpellIds(aura))
        {
            if (player->HasSpell(spellId))
                return true;
        }

        return false;
    }

    // Aura preference of a sub group, depending on the roles in it
    std::vector<std::string> GroupPreference(PlayerbotAI* ai, const std::vector<Player*>& members)
    {
        uint32 tanks = 0, melee = 0, casters = 0;
        for (Player* member : members)
        {
            const uint8 role = tbcraid::GetRole(ai, member);
            if (role == tbcraid::ROLE_TANK)
                tanks++;
            else if (role == tbcraid::ROLE_MELEE)
                melee++;
            else
                casters++;
        }

        if (tanks)
            return { "devotion aura", "retribution aura", "concentration aura", "sanctity aura" };

        if (melee >= casters)
            return { "sanctity aura", "retribution aura", "devotion aura", "concentration aura" };

        return { "concentration aura", "devotion aura", "retribution aura", "sanctity aura" };
    }
}

const std::vector<std::pair<std::string, std::string>>& paladinaura::ManualAuraStrategies()
{
    static const std::vector<std::pair<std::string, std::string>> strategies = {
        { "aura devotion", "devotion aura" },
        { "aura retribution", "retribution aura" },
        { "aura concentration", "concentration aura" },
        { "aura sanctity", "sanctity aura" },
        { "aura shadow", "shadow resistance aura" },
        { "aura fire", "fire resistance aura" },
        { "aura frost", "frost resistance aura" },
        { "aura crusader", "crusader aura" },
    };

    return strategies;
}

std::string paladinaura::GetManualAuraStrategy(PlayerbotAI* ai)
{
    for (auto& strategy : ManualAuraStrategies())
    {
        if (ai->HasStrategy(strategy.first, BotState::BOT_STATE_NON_COMBAT))
            return strategy.first;
    }

    return "";
}

std::string paladinaura::GetActiveAura(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    for (const std::string& aura : allAuras)
    {
        if (ai->HasMyAura(aura, bot))
            return aura;
    }

    return "";
}

std::string paladinaura::GetAssignedAura(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    Group* group = bot->GetGroup();
    if (!group || !GetManualAuraStrategy(ai).empty())
        return "";

    const bool autoMode = ai->HasStrategy("auras auto", BotState::BOT_STATE_NON_COMBAT);

    std::vector<std::string> preference;
    const tbcraid::Encounter* encounter = tbcraid::EncounterRegistry::Instance().FindNearby(ai);
    if (encounter && !encounter->resistanceAura.empty())
        preference.push_back(encounter->resistanceAura);

    // Members of the sub group, bot paladins sorted by guid so every bot computes the same assignment
    std::vector<Player*> members;
    std::vector<Player*> paladins;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || !group->SameSubGroup(bot, member))
            continue;

        members.push_back(member);
        if (member->getClass() == CLASS_PALADIN && member->GetPlayerbotAI() && !member->GetPlayerbotAI()->IsRealPlayer())
            paladins.push_back(member);
    }

    std::sort(paladins.begin(), paladins.end(), [](Player* a, Player* b) { return a->GetObjectGuid() < b->GetObjectGuid(); });

    if (autoMode)
    {
        for (const std::string& aura : GroupPreference(ai, members))
            if (std::find(preference.begin(), preference.end(), aura) == preference.end())
                preference.push_back(aura);
    }

    if (preference.empty())
        return "";

    // Hand out the auras in order, each paladin gets the first one it knows that is still free
    std::vector<std::string> taken;
    for (Player* paladin : paladins)
    {
        std::string assigned;
        for (const std::string& aura : preference)
        {
            if (std::find(taken.begin(), taken.end(), aura) != taken.end() || !KnowsAura(paladin, aura))
                continue;

            assigned = aura;
            break;
        }

        if (paladin == bot)
            return assigned;

        if (!assigned.empty())
            taken.push_back(assigned);
    }

    return "";
}
