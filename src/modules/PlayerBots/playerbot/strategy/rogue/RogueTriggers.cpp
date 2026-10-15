
#include "playerbot/playerbot.h"
#include "RogueTriggers.h"
#include "RogueActions.h"

using namespace ai;

// Remaining duration (ms) of our own deadly poison on the target, -1 if absent
static int32 GetOwnDeadlyPoisonDuration(Player* bot, Unit* target)
{
    if (!target)
        return -1;

    for (Aura* aura : target->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE))
    {
        if (!aura || !aura->GetHolder() || aura->GetHolder()->GetCasterGuid() != bot->GetObjectGuid())
            continue;

        SpellEntry const* spellInfo = aura->GetSpellProto();
        if (!spellInfo || spellInfo->SpellFamilyName != SPELLFAMILY_ROGUE)
            continue;

        // some spells have no name in the spell data (boss spells), never build a string from a null pointer
        const char* name = spellInfo->SpellName[0];
        if (name && strncmp(name, "Deadly Poison", 13) == 0)
            return aura->GetHolder()->GetAuraDuration();
    }

    return -1;
}

bool EnvenomTrigger::IsActive()
{
    if (!ComboPointsAvailableTrigger::IsActive())
        return false;

    Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
    return target && target->IsInWorld() && GetOwnDeadlyPoisonDuration(bot, target) >= 0;
}

bool ShivTrigger::IsActive()
{
    if (!SpellCanBeCastedTrigger::IsActive())
        return false;

    const int32 duration = GetOwnDeadlyPoisonDuration(bot, GetTarget());
    return duration >= 0 && duration < 4 * IN_MILLISECONDS;
}

bool RiposteCastTrigger::IsActive()
{
	Unit* target = GetTarget();
	if (!target)
		return false;

	bool isMelee = true;
	if (target->IsPlayer())
	{
		isMelee = !ai->IsRanged((Player*)target);
	}

	return SpellCanBeCastedTrigger::IsActive() && isMelee;
}