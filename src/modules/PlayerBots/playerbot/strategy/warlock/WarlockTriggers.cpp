
#include "playerbot/playerbot.h"
#include "WarlockTriggers.h"
#include "WarlockActions.h"
#include "playerbot/strategy/generic/TbcRaidEncounters.h"
#include "playerbot/strategy/generic/TbcRotationRules.h"

using namespace ai;

bool DemonArmorTrigger::IsActive() 
{
	Unit* target = GetTarget();
	return !ai->HasAura("demon skin", target) &&
		   !ai->HasAura("demon armor", target) &&
		   !ai->HasAura("fel armor", target);
}

bool SpellstoneTrigger::IsActive() 
{
    return BuffTrigger::IsActive() && AI_VALUE2(uint32, "item count", getName()) > 0;
}

bool InfernoTrigger::IsActive()
{
	return AI_VALUE(uint8, "possible attack targets count") > 1 && bot->HasSpell(1122) && bot->HasItemCount(5565, 1) && !urand(0, 2);
}

bool CorruptionTrigger::IsActive()
{
	Unit* target = GetTarget();
	return target && target->IsAlive() && !ai->HasAura("corruption", target, false, true) &&
		   !ai->HasAura("seed of corruption", target, false, true) && !HasMaxDebuffs();
}

bool SiphonLifeTrigger::IsActive()
{
	if (!OwnDebuffTrigger::IsActive())
		return false;

	return ai->HasAura("shadow vulnerability", GetTarget()) || !warlockcurse::GroupHasImprovedShadowBolt(ai);
}

namespace
{
	const std::vector<std::string> manualCurseStrategies = {
		"curse agony", "curse doom", "curse elements", "curse recklessness",
		"curse weakness", "curse tongues", "curse shadow"
	};

	bool HasManualCurse(PlayerbotAI* ai)
	{
		for (const std::string& strategy : manualCurseStrategies)
		{
			if (ai->HasStrategy(strategy, BotState::BOT_STATE_COMBAT))
				return true;
		}

		return false;
	}

	bool HasAnySpell(Player* player, std::initializer_list<uint32> spellIds)
	{
		for (uint32 spellId : spellIds)
		{
			if (player->HasSpell(spellId))
				return true;
		}

		return false;
	}

	bool HasMalediction(Player* player)
	{
		return HasAnySpell(player, { 32477, 32483, 32484 });
	}
}

bool warlockcurse::GroupHasImprovedShadowBolt(PlayerbotAI* ai)
{
	Player* bot = ai->GetBot();
	const std::initializer_list<uint32> improvedShadowBolt = { 17793, 17796, 17801, 17802, 17803 };
	Group* group = bot->GetGroup();
	if (!group)
		return HasAnySpell(bot, improvedShadowBolt);

	for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
	{
		Player* member = ref->getSource();
		if (member && member->getClass() == CLASS_WARLOCK && member->IsAlive() &&
			member->GetMapId() == bot->GetMapId() && HasAnySpell(member, improvedShadowBolt))
			return true;
	}

	return false;
}

std::string warlockcurse::GetAssignedCurse(PlayerbotAI* ai, Unit* target)
{
	Player* bot = ai->GetBot();
	Group* group = bot->GetGroup();
	if (!group || !target || HasManualCurse(ai))
		return "";

	// Bot warlocks sorted so every bot computes the same order: Malediction first, then guid
	std::vector<Player*> warlocks;
	unsigned melee = 0, members = 0;
	for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
	{
		Player* member = ref->getSource();
		if (!member || !member->IsInWorld() || member->GetMapId() != bot->GetMapId())
			continue;

		members++;
		if (tbcraid::GetRole(ai, member) == tbcraid::ROLE_MELEE)
			melee++;

		PlayerbotAI* memberAi = member->GetPlayerbotAI();
		if (member->getClass() == CLASS_WARLOCK && member->IsAlive() && memberAi &&
			!memberAi->IsRealPlayer() && !HasManualCurse(memberAi))
			warlocks.push_back(member);
	}

	std::sort(warlocks.begin(), warlocks.end(), [](Player* a, Player* b)
	{
		const bool aMalediction = HasMalediction(a), bMalediction = HasMalediction(b);
		if (aMalediction != bMalediction)
			return aMalediction;
		return a->GetObjectGuid() < b->GetObjectGuid();
	});

	const auto self = std::find(warlocks.begin(), warlocks.end(), bot);
	if (self == warlocks.end())
		return "";

	const bool doomFight = target->GetTypeId() == TYPEID_UNIT && tbc::CurseOfDoomFight(target->GetHealth(), members);
	return tbc::RaidCurse(unsigned(self - warlocks.begin()), melee, doomFight);
}

bool AssignedCurseTrigger::IsActive()
{
	Unit* target = GetTarget();
	if (!target || !target->IsAlive() || warlockcurse::GetAssignedCurse(ai, target) != curse)
		return false;

	// Elements and Recklessness are raid debuffs, anyone's counts; Doom and Agony are personal
	const bool raidDebuff = curse == "curse of the elements" || curse == "curse of recklessness";
	return !ai->HasAura(curse, target, false, !raidDebuff);
}

bool NoDemonTrigger::IsActive()
{
	if (ai->GetUnit(AI_VALUE(ObjectGuid, "pet target")) || AI_VALUE2(bool, "mounted", "self target"))
		return false;

	// A sacrificed demon must stay sacrificed, summoning would remove the buff
	return !ai->HasAura(18789, bot) && !ai->HasAura(18790, bot) && !ai->HasAura(18791, bot) &&
		   !ai->HasAura(18792, bot) && !ai->HasAura(35701, bot);
}

bool LifeTapTrigger::IsActive()
{
	const uint32 mana = AI_VALUE2(uint8, "mana", "self target");
	if (mana <= sPlayerbotAIConfig.lowMana)
	{
		const uint32 health = AI_VALUE2(uint8, "health", "self target");
		if (health >= sPlayerbotAIConfig.lowHealth)
		{
			return true;
		}
	}

	return false;
}

bool DrainSoulTrigger::IsActive()
{
	// If no item cheats enabled
    if (!ai->HasCheat(BotCheatMask::item))
    {
		// Check if it has less than 5 soul shards
        if (!bot->HasItemCount(6265, 5))
        {
			// Check if it has enough bag space
			if (AI_VALUE(uint8, "bag space") > 0)
			{
                // Check if target health is less than 15%
                const uint32 targetHealth = AI_VALUE2(uint8, "health", "current target");
                if (targetHealth <= 15)
                {
                    return true;
                }
			}
        }
	}

	return false;
}

bool CorruptionOnAttackerTrigger::IsActive()
{
    if (DebuffOnAttackerTrigger::IsActive())
    {
        return !ai->HasAura("seed of corruption", GetTarget(), false, true);
	}

	return false;
}

bool SeedOfCorruptionOnAttackerTrigger::IsActive()
{
    if (DebuffOnAttackerTrigger::IsActive())
    {
        return AI_VALUE(uint8, "possible attack targets count") >= 3;
    }

    return false;
}

bool NoCurseTrigger::IsActive()
{
	Unit* target = GetTarget();
	if (target)
	{
		return !ai->HasAura("curse of agony", target, false, true) &&
			   !ai->HasAura("curse of doom", target, false, true) &&
			   !ai->HasAura("curse of recklessness", target, false, true) &&
			   !ai->HasAura("curse of shadow", target, false, true) &&
			   !ai->HasAura("curse of the elements", target, false, true) &&
			   !ai->HasAura("curse of weakness", target, false, true) &&
			   !ai->HasAura("curse of tongues", target, false, true);
	}

	return false;
}

bool NoCurseOnAttackerTrigger::IsActive()
{
    std::list<ObjectGuid> attackers = AI_VALUE(std::list<ObjectGuid>, "possible attack targets");
    Unit* currentTarget = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* attacker = ai->GetUnit(*i);
        if (attacker && attacker != currentTarget)
        {
			if (!ai->HasAura("curse of agony", attacker, false, true) &&
				!ai->HasAura("curse of doom", attacker, false, true) &&
				!ai->HasAura("curse of recklessness", attacker, false, true) &&
				!ai->HasAura("curse of shadow", attacker, false, true) &&
				!ai->HasAura("curse of the elements", attacker, false, true) &&
				!ai->HasAura("curse of weakness", attacker, false, true) &&
				!ai->HasAura("curse of tongues", attacker, false, true))
			{
				return true;
			}
        }
    }

	return false;
}

bool FearPvpTrigger::IsActive()
{
    Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
	if (target && target->IsPlayer())
	{
		// Check if low health
        const uint8 health = AI_VALUE2(uint8, "health", "self target");
        if (health <= sPlayerbotAIConfig.lowHealth)
        {
			// Check if targeting bot
			if (target->GetSelectionGuid() == bot->GetObjectGuid())
			{
                // Check if the bot has feared anyone
                bool alreadyFeared = false;
                std::list<ObjectGuid> attackers = AI_VALUE(std::list<ObjectGuid>, "attackers");
                for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); ++i)
                {
                    Unit* attacker = ai->GetUnit(*i);
                    if (ai->HasAura("fear", attacker, false, true))
                    {
                        alreadyFeared = true;
                        break;
                    }
                }

                if (!alreadyFeared)
                {
                    const float distance = target->GetDistance(bot);
                    return distance <= 10.0f;
                }
			}
		}
	}

	return false;
}

bool ConflagrateTrigger::IsActive()
{
	Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
	if (target)
	{
		// Check if immolate in target
		Aura* aura = ai->GetAura("immolate", target, true);
		if (aura)
		{
			// Check if immolate is about to expire
            if (aura->GetAuraDuration() <=
#ifdef MANGOSBOT_ONE
                1500
#else
                7000
#endif
            )
			{
				return true;
			}
		}
	}

	return false;
}

bool DemonicSacrificeTrigger::IsActive()
{
	// A Felguard build keeps its Felguard instead of sacrificing it
	if (ai->HasStrategy("pet", BotState::BOT_STATE_COMBAT) && !bot->HasSpell(30146))
	{
		return ai->HasSpell(18788) &&
			   !ai->HasAura(18789, bot) && // Burning Wish (Imp)
			   !ai->HasAura(18790, bot) && // Fel Stamina (Voidwalker)
			   !ai->HasAura(18791, bot) && // Touch of Shadow (Succubus)
			   !ai->HasAura(18792, bot) && // Fel Energy (Felhunter)
			   !ai->HasAura(35701, bot);   // Touch of Shadow (Felguard)
	}

	return false;
}

bool SoulLinkTrigger::IsActive()
{
	return ai->HasSpell(19028) && !ai->HasAura(19028, bot) && ai->GetUnit(AI_VALUE(ObjectGuid, "pet target"));
}

bool NoSpecificPetTrigger::IsActive()
{
    Unit* pet = ai->GetUnit(AI_VALUE(ObjectGuid, "pet target"));
    if (pet)
    {
        return pet->GetEntry() != entry;
    }

    return true;
}

uint32 SoulstoneTrigger::GetItemId()
{
    uint32 itemId = 0;
    const uint32 level = bot->GetLevel();
    if (level >= 18 && level < 30)
    {
        itemId = 5232;
    }
    else if (level >= 30 && level < 40)
    {
        itemId = 16892;
    }
    else if (level >= 40 && level < 50)
    {
        itemId = 16893;
    }
    else if (level >= 50 && level < 60)
    {
        itemId = 16895;
    }
    else if (level >= 60 && level < 70)
    {
        itemId = 16896;
    }
    else if (level >= 70 && level < 76)
    {
        itemId = 22116;
    }
    else if (level >= 76)
    {
        itemId = 36895;
    }

    return itemId;
}
