#pragma once

#include "Common.h"
#include <atomic>

class Unit;
class PlayerbotAI;
struct SpellEntry;

// Per-fight performance summary of human-led bot groups (logs/bot-dps-*.log).
// The core hooks are cheap no-ops while no tracked fight is running.
namespace botperf
{
    extern std::atomic<bool> activeFights;

    void Configure(bool enabled, uint32 minSeconds, bool instancesOnly, bool includeMaster);
    void Update(PlayerbotAI* ai);

    void RecordDamage(Unit* dealer, Unit* victim, uint32 damage, SpellEntry const* spell, bool periodic);
    void RecordHeal(Unit* healer, Unit* victim, uint32 amount, int32 gain, SpellEntry const* spell);
    void RecordMelee(Unit* attacker, uint32 outcome);
    void RecordSpellResult(Unit* caster, Unit* target, SpellEntry const* spell, uint32 missInfo);
    void RecordCast(Unit* caster, SpellEntry const* spell);

    inline void OnDamage(Unit* dealer, Unit* victim, uint32 damage, SpellEntry const* spell, bool periodic)
    {
        if (activeFights && dealer && dealer != victim) RecordDamage(dealer, victim, damage, spell, periodic);
    }
    inline void OnHeal(Unit* healer, Unit* victim, uint32 amount, int32 gain, SpellEntry const* spell)
    {
        if (activeFights && healer) RecordHeal(healer, victim, amount, gain, spell);
    }
    inline void OnMelee(Unit* attacker, uint32 outcome)
    {
        if (activeFights) RecordMelee(attacker, outcome);
    }
    inline void OnSpellResult(Unit* caster, Unit* target, SpellEntry const* spell, uint32 missInfo)
    {
        if (activeFights && caster) RecordSpellResult(caster, target, spell, missInfo);
    }
    inline void OnCast(Unit* caster, SpellEntry const* spell)
    {
        if (activeFights && caster) RecordCast(caster, spell);
    }
}
