
#include "playerbot/playerbot.h"
#include "RaidBuffActions.h"
#include "playerbot/strategy/paladin/PaladinAuraAssignment.h"

using namespace ai;

namespace
{
    // Bloodlust (horde) or Heroism (alliance) the player knows, 0 if none
    uint32 GetLustSpell(Player* player, std::string& name)
    {
        for (const std::string& spell : { "bloodlust", "heroism" })
        {
            for (uint32 spellId : ChatHelper::SpellIds(spell))
            {
                if (player->HasSpell(spellId))
                {
                    name = spell;
                    return spellId;
                }
            }
        }

        return 0;
    }

    std::string FormatAura(const std::string& aura)
    {
        return aura.empty() ? "none" : aura;
    }
}

bool LustAction::Execute(Event& event)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    const std::string param = event.getParam();

    std::string spellName;
    const uint32 spellId = GetLustSpell(bot, spellName);
    if (!spellId)
        return false;

    if (param == "hold")
    {
        ai->ChangeStrategy("+hold lust", BotState::BOT_STATE_COMBAT);
        ai->TellPlayerNoFacing(requester, "Holding " + spellName + " until you say lust.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        return true;
    }

    if (param == "auto")
    {
        ai->ChangeStrategy("-hold lust", BotState::BOT_STATE_COMBAT);
        ai->TellPlayerNoFacing(requester, "Using " + spellName + " on my own again (boss timing).", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        return true;
    }

    if (param == "?")
    {
        std::ostringstream out;
        out << spellName << ": " << (ai->HasStrategy("hold lust", BotState::BOT_STATE_COMBAT) ? "hold" : "auto");
        out << ", " << (bot->IsSpellReady(spellId) ? "ready" : "on cooldown");
        ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        return true;
    }

    // Raid wide Bloodlust / Heroism: only the first ready shaman (lowest guid) casts, the others keep theirs
    if (ai->HasAura("bloodlust", bot) || ai->HasAura("heroism", bot))
        return false;

    if (!bot->IsSpellReady(spellId))
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->getSource();
            if (!member || member == bot || member->getClass() != CLASS_SHAMAN || !member->IsAlive())
                continue;

            if (!member->GetPlayerbotAI() || member->GetPlayerbotAI()->IsRealPlayer())
                continue;

            std::string memberSpell;
            const uint32 memberSpellId = GetLustSpell(member, memberSpell);
            if (memberSpellId && member->IsSpellReady(memberSpellId) && member->GetObjectGuid() < bot->GetObjectGuid())
                return false;
        }
    }

    if (!ai->CastSpell(spellName, bot, nullptr, false))
        return false;

    ai->TellPlayerNoFacing(requester, spellName + "!", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    return true;
}

bool AurasAction::Execute(Event& event)
{
    if (bot->getClass() != CLASS_PALADIN)
        return false;

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    const std::string param = event.getParam();

    if (param == "auto")
    {
        // Leave the manual aura strategies, the default aura of the spec stays as fallback
        ai->ChangeStrategy("+aura", BotState::BOT_STATE_NON_COMBAT);
        ai->ChangeStrategy("+aura", BotState::BOT_STATE_COMBAT);
        ai->ChangeStrategy("+auras auto", BotState::BOT_STATE_NON_COMBAT);
        ai->ChangeStrategy("+auras auto", BotState::BOT_STATE_COMBAT);
    }
    else if (param == "off")
    {
        ai->ChangeStrategy("-auras auto", BotState::BOT_STATE_NON_COMBAT);
        ai->ChangeStrategy("-auras auto", BotState::BOT_STATE_COMBAT);
        ai->ChangeStrategy("+aura", BotState::BOT_STATE_NON_COMBAT);
        ai->ChangeStrategy("+aura", BotState::BOT_STATE_COMBAT);
    }
    else if (!param.empty() && param != "?")
    {
        return false;
    }

    std::ostringstream out;
    out << "Aura: " << FormatAura(paladinaura::GetActiveAura(ai));

    const std::string assigned = paladinaura::GetAssignedAura(ai);
    const std::string manual = paladinaura::GetManualAuraStrategy(ai);
    if (!manual.empty())
        out << " (manual: " << manual << ")";
    else if (!assigned.empty())
        out << " (assigned: " << assigned << ")";
    else if (ai->HasStrategy("auras auto", BotState::BOT_STATE_NON_COMBAT))
        out << " (auto)";

    // Blessings this paladin keeps up on the group
    std::map<std::string, uint32> blessings;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->getSource();
            if (!member)
                continue;

            for (const std::string& blessing : { "blessing of kings", "blessing of might", "blessing of wisdom", "blessing of light", "blessing of salvation", "blessing of sanctuary" })
            {
                if (ai->HasMyAura(blessing, member) || ai->HasMyAura("greater " + blessing, member))
                    blessings[blessing]++;
            }
        }
    }

    if (!blessings.empty())
    {
        out << " | Blessings:";
        for (auto& blessing : blessings)
            out << " " << blessing.first << " x" << blessing.second;
    }

    ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    return true;
}
