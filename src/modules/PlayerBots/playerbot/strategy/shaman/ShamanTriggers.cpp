
#include "playerbot/playerbot.h"
#include "ShamanTriggers.h"
#include "ShamanActions.h"
#include "playerbot/strategy/generic/TbcRaidEncounters.h"

using namespace ai;

// Bloodlust / Heroism timing: never while "hold lust" is set, in raids follow the boss timing of the encounter table
static bool IsLustAllowed(PlayerbotAI* ai)
{
    if (ai->HasStrategy("hold lust", BotState::BOT_STATE_COMBAT))
        return false;

    // Raid wide Bloodlust / Heroism: one shaman is enough
    Player* bot = ai->GetBot();
    if (ai->HasAura("bloodlust", bot) || ai->HasAura("heroism", bot))
        return false;

    const tbcraid::Encounter* encounter = tbcraid::EncounterRegistry::Instance().FindNearby(ai);
    if (!encounter || encounter->lustBelowHealthPct >= 100.0f || encounter->entries.empty())
        return true;

    Unit* target = ai->GetUnit(ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Get());
    if (!target || target->GetEntry() != encounter->entries.front())
        return false;

    return target->GetHealthPercent() <= encounter->lustBelowHealthPct;
}

bool HeroismTrigger::IsActive()
{
    return IsLustAllowed(ai) && BoostTrigger::IsActive();
}

bool BloodlustTrigger::IsActive()
{
    return IsLustAllowed(ai) && BoostTrigger::IsActive();
}

std::list<std::string> ShamanWeaponTrigger::spells;

bool ShamanWeaponTrigger::IsActive()
{
    if (spells.empty())
    {
        spells.push_back("frostbrand weapon");
        spells.push_back("rockbiter weapon");
        spells.push_back("flametongue weapon");
        spells.push_back("earthliving weapon");
        spells.push_back("windfury weapon");
    }

    for (std::list<std::string>::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        uint32 spellId = AI_VALUE2(uint32, "spell id", *i);
        if (!spellId)
            continue;

        if (AI_VALUE2(Item*, "item for spell", spellId))
            return true;
    }

    return false;
}

bool ShockTrigger::IsActive()
{
#ifdef MANGOSBOT_ONE
    // Earth Shock has no lasting debuff; it fits between two Flame Shocks
    return SpellTrigger::IsActive() && !ai->HasAura("frost shock", GetTarget()) && !HasMaxDebuffs();
#else
    return SpellTrigger::IsActive() && !ai->HasAnyAuraOf(GetTarget(), "frost shock", "earth shock", "flame shock", NULL) && !HasMaxDebuffs();
#endif
}

bool ElementalMasteryTrigger::IsActive()
{
    return BoostTrigger::IsActive() && bot->HasSpell(16166) && bot->IsSpellReady(16166);
}

bool FireElementalTotemTrigger::IsActive()
{
    return BoostTrigger::IsActive() && bot->HasSpell(2894) && bot->IsSpellReady(2894) && !AI_VALUE2(bool, "has totem", "fire elemental totem");
}

bool FlameShockTrigger::IsActive()
{
    return SpellTrigger::IsActive() && !ai->HasAnyAuraOf(GetTarget(), "flame shock", NULL) && !HasMaxDebuffs();
}
