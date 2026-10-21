// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#pragma once

#include "playerbot/strategy/Strategy.h"

namespace ai
{
    // Gruul's Lair (map 565): High King Maulgar and Gruul the Dragonkiller
    class GruulStrategy : public Strategy
    {
    public:
        GruulStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "boss gruul"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "boss gruul"; }
        virtual std::string GetHelpDescription()
        {
            return "Gruul's Lair boss tactics: ogre council tanking, kill order, Misdirection and Banish\n"
                   "assignments for High King Maulgar; tank position, ranged spread, Cave In and Shatter\n"
                   "for Gruul the Dragonkiller.";
        }
#endif

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionMultipliers(std::list<Multiplier*>& multipliers) override;
    };
}
