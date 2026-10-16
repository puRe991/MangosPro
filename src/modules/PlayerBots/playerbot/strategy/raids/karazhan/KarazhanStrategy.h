// Ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara/KaraStrategy) and adapted for this core.
#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    // Karazhan (map 532) boss tactics, added automatically on the map (see RaidBossStrategies.cpp)
    class KarazhanBossStrategy : public Strategy
    {
    public:
        KarazhanBossStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "boss karazhan"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitReactionMultipliers(std::list<Multiplier*>& multipliers) override;
    };
}
