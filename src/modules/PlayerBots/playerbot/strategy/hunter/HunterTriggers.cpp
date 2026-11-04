
#include "playerbot/playerbot.h"
#include "HunterTriggers.h"
#include "HunterActions.h"

using namespace ai;

bool HunterRecoverRangedDistanceTrigger::IsActive()
{
    return HunterNeedsRangedDistance(ai);
}

bool SteadyShotCanCastTrigger::IsActive()
{
    if (!SpellCanBeCastedTrigger::IsActive())
        return false;

#ifdef MANGOSBOT_ONE
    // Weave steady shot between auto shots: only start the cast if it finishes before the next auto shot
    if (!bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL))
        return true;

    const uint32 spellId = AI_VALUE2(uint32, "spell id", "steady shot");
    SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(spellId);
    if (!spellInfo)
        return true;

    // A small clip is accepted: with a hasted ~2 s bow the exact window was only 0.5 s wide and
    // the AI tick mostly missed it (5 Steady Shots in a 4 minute fight)
    const int32 castTime = GetSpellCastTime(spellInfo, bot);
    return int32(bot->getAttackTimer(RANGED_ATTACK)) >= castTime - 500;
#else
    return true;
#endif
}

bool HunterNoStingsActiveTrigger::IsActive()
{
	Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
    return target && AI_VALUE2(uint8, "health", "current target") > 40 &&
        !ai->HasAura("serpent sting", target) &&
        !ai->HasAura("scorpid sting", target) &&
        !ai->HasAura("viper sting", target);
}

bool HuntersPetDeadTrigger::IsActive()
{
    return AI_VALUE(bool, "pet dead") && !AI_VALUE2(bool, "mounted", "self target");
}

bool HuntersPetLowHealthTrigger::IsActive()
{
    Unit* pet = ai->GetUnit(AI_VALUE(ObjectGuid, "pet target"));
    return pet && AI_VALUE2(uint8, "health", "pet target") < 40 &&
        !AI_VALUE2(bool, "dead", "pet target") && !AI_VALUE2(bool, "mounted", "self target");
}

bool HunterPetNotHappy::IsActive()
{
    return !AI_VALUE(bool, "pet happy") && !AI_VALUE2(bool, "mounted", "self target");
}

bool ViperStingOnAttackerTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (target)
    {
        const bool noStings = !ai->HasAura("serpent sting", target) &&
                              !ai->HasAura("scorpid sting", target) &&
                              !ai->HasAura("viper sting", target);
        if (noStings)
        {
            if (target->GetPower(POWER_MANA) >= 10)
            {
                return DebuffOnAttackerTrigger::IsActive();
            }
        }
    }

    return false;
}

bool SerpentStingOnAttackerTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (target)
    {
        const bool noStings = !ai->HasAura("serpent sting", target) &&
                              !ai->HasAura("scorpid sting", target) &&
                              !ai->HasAura("viper sting", target);
        if (noStings)
        {
            if (target->GetPower(POWER_MANA) < 10 || ai->HasStrategy("sting serpent", BotState::BOT_STATE_COMBAT))
            {
                return DebuffOnAttackerTrigger::IsActive();
            }
        }
    }

    return false;
}
