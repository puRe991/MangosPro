
#include "playerbot/playerbot.h"
#include "PriestTriggers.h"
#include "PriestActions.h"

using namespace ai;

bool InnerFireTrigger::IsActive()
{
    Unit* target = GetTarget();
    return SpellTrigger::IsActive() && !ai->HasAura(spell, target);
}

bool ShadowformTrigger::IsActive()
{
    return !ai->HasAura("shadowform", bot);
}

bool ShadowfiendTrigger::IsActive()
{
    return BoostTrigger::IsActive() && bot->IsSpellReady(34433);
}

bool InnerFocusTrigger::IsActive()
{
    if (!BoostTrigger::IsActive() || !bot->IsSpellReady(14751))
        return false;

#ifdef MANGOSBOT_ONE
    // Save the free crit for a big spell: Mind Blast in Shadowform,
    // otherwise a heal (Greater Heal / Circle of Healing) that is needed now
    if (ai->HasAura("shadowform", bot))
    {
        const uint32 mindBlast = AI_VALUE2(uint32, "spell id", "mind blast");
        return mindBlast && bot->IsSpellReady(mindBlast);
    }

    return ai->GetUnit(AI_VALUE(ObjectGuid, "party member to heal")) != nullptr;
#else
    return true;
#endif
}