// Ported from mod-playerbots (src/Ai/Raid/Mag, GPL v2+) and adapted for this core.
#pragma once

#include "playerbot/strategy/Strategy.h"

namespace ai
{
    // Magtheridon's Lair (map 544)
    class MagtheridonStrategy : public Strategy
    {
    public:
        MagtheridonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "boss magtheridon"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "boss magtheridon"; }
        virtual std::string GetHelpDescription()
        {
            return "Magtheridon's Lair boss tactics: channeler tanking and kill order, Misdirection,\n"
                   "Burning Abyssal Banish/Fear rotation, boss tank position, ranged spread, Manticron Cube\n"
                   "clicking on Blast Nova and Debris avoidance after the ceiling collapses.";
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
