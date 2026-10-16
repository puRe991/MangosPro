// Ported from mod-playerbots (AzerothCore, GPL v2+), src/Ai/Raid/ZA/ZAStrategy.h, and adapted for this core.
#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    // Zul'Aman (map 568) boss tactics
    class ZaStrategy : public Strategy
    {
    public:
        ZaStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "boss zul'aman"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "boss zul'aman"; }
        virtual std::string GetHelpDescription() { return "Boss tactics of Zul'Aman (ported from mod-playerbots)."; }
#endif

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };
}
