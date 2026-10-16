// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/Hyjal/HyjalStrategy) and adapted for this core.
#pragma once

#include "playerbot/strategy/Strategy.h"

namespace ai
{
    // Hyjal Summit (map 534)
    class HyjalBossStrategy : public Strategy
    {
    public:
        HyjalBossStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "boss hyjal"; }

    protected:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };
}
