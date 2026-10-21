// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/TK/TKStrategy) and adapted for this core.
#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    // Tempest Keep: The Eye (map 550)
    class TkStrategy : public Strategy
    {
    public:
        TkStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "boss tempest keep"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "boss tempest keep"; }
        virtual std::string GetHelpDescription()
        {
            return "Boss tactics of Tempest Keep: The Eye (Al'ar, Void Reaver, High Astromancer Solarian, Kael'thas Sunstrider).";
        }
#endif

    private:
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitNonCombatMultipliers(std::list<Multiplier*>& multipliers) override;
        static void AddMultipliers(PlayerbotAI* ai, std::list<Multiplier*>& multipliers);
    };
}
