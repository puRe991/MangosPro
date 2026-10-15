#pragma once
#include "GenericActions.h"

namespace ai
{
    // "lust"       : every shaman casts Bloodlust / Heroism now
    // "lust hold"  : keep Bloodlust / Heroism for the "lust" command
    // "lust auto"  : use Bloodlust / Heroism on its own again (boss timing of the raid table)
    // "lust ?"     : tell the current mode and the cooldown
    class LustAction : public ChatCommandAction
    {
    public:
        LustAction(PlayerbotAI* ai) : ChatCommandAction(ai, "lust") {}
        bool Execute(Event& event) override;
        bool isUsefulWhenStunned() override { return true; }
    };

    // "auras"        : every paladin tells its aura and the blessings it keeps up
    // "auras auto"   : the paladins of each sub group share out the auras
    // "auras off"    : back to the default aura of the spec
    class AurasAction : public ChatCommandAction
    {
    public:
        AurasAction(PlayerbotAI* ai) : ChatCommandAction(ai, "auras") {}
        bool Execute(Event& event) override;
        bool isUsefulWhenStunned() override { return true; }
    };
}
