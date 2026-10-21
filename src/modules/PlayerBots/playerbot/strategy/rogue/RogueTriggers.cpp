
#include "playerbot/playerbot.h"
#include "RogueTriggers.h"
#include "RogueActions.h"
#include "playerbot/strategy/generic/TbcRotationRules.h"

using namespace ai;

bool RuptureTrigger::IsActive()
{
    if (!NoDebuffAndComboPointsAvailableTrigger::IsActive())
        return false;
#ifdef MANGOSBOT_ONE
    Unit* target = GetTarget();
    return target && tbc::UseBleedFinisher(target->GetHealth(), bot->GetMaxHealth(), target->GetHealthPercent()) &&
        ai->CanCastSpell("rupture", target, 0);
#else
    return true;
#endif
}

// Remaining duration (ms) of our own deadly poison on the target, -1 if absent
static int32 GetOwnDeadlyPoisonDuration(Player* bot, Unit* target)
{
    if (!target)
        return -1;

    for (Aura* aura : target->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE))
    {
        if (!aura || !aura->GetHolder() || aura->GetHolder()->GetCasterGuid() != bot->GetObjectGuid())
            continue;

        const std::string name = aura->GetSpellProto()->SpellName[0];
        if (name.rfind("Deadly Poison", 0) == 0)
            return aura->GetHolder()->GetAuraDuration();
    }

    return -1;
}

bool EnvenomTrigger::IsActive()
{
    return ComboPointsAvailableTrigger::IsActive() &&
           GetOwnDeadlyPoisonDuration(bot, ai->GetUnit(AI_VALUE(ObjectGuid, "current target"))) >= 0;
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
