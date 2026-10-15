
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

    Unit* target = ai->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (!target || target->GetEntry() != encounter->entries.front())
        return false;

    return target->GetHealthPercent() <= encounter->lustBelowHealthPct;
}

bool HeroismTrigger::IsActive()
{
    return BoostTrigger::IsActive() && IsLustAllowed(ai);
}

bool BloodlustTrigger::IsActive()
{
    return BoostTrigger::IsActive() && IsLustAllowed(ai);
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
        uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
        if (!spellId)
            continue;

        if (AI_VALUE2(Item*, "item for spell", spellId))
            return true;
    }

    return false;
}

bool ShockTrigger::IsActive()
{
    return SpellTrigger::IsActive() && !ai->HasAnyAuraOf(GetTarget(), "frost shock", "earth shock", "flame shock", NULL) && !HasMaxDebuffs();
}

bool FlameShockTrigger::IsActive()
{
    return SpellTrigger::IsActive() && !ai->HasAnyAuraOf(GetTarget(), "flame shock", NULL) && !HasMaxDebuffs();
}
