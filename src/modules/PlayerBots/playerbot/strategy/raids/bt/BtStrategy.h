// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/BT/BTStrategy) and adapted for this core.
#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    // Black Temple (map 564) boss tactics
    class BtStrategy : public Strategy
    {
    public:
        BtStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "boss black temple"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "boss black temple"; }
        virtual std::string GetHelpDescription() { return "Boss tactics of Black Temple (ported from mod-playerbots)."; }
#endif

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };
}
