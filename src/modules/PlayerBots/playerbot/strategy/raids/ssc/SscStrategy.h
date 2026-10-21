// Ported from mod-playerbots (src/Ai/Raid/SSC/SSCStrategy.h, GPL v2+) and adapted for this core.
#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    // Serpentshrine Cavern (map 548)
    class SscStrategy : public Strategy
    {
    public:
        SscStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "boss ssc"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "boss ssc"; }
        virtual std::string GetHelpDescription()
        {
            return "Boss tactics of Serpentshrine Cavern: Hydross tank swaps, Lurker Spout and Geyser,\n"
                   "Leotheras warlock tank and Inner Demons, Karathress tank spots and kill order,\n"
                   "Morogrim phase 2 stacking and Lady Vashj phases with the Tainted Core chain.";
        }
#endif

    private:
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };
}
