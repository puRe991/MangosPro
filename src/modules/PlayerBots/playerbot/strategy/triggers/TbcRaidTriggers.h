#pragma once
#include "playerbot/strategy/Trigger.h"

namespace ai
{
    // Activates the "tbc raid" strategy inside the Burning Crusade raids
    class TbcRaidEnterTrigger : public Trigger
    {
    public:
        TbcRaidEnterTrigger(PlayerbotAI* ai) : Trigger(ai, "enter tbc raid", 5) {}
        bool IsActive() override;
    };

    class TbcRaidLeaveTrigger : public Trigger
    {
    public:
        TbcRaidLeaveTrigger(PlayerbotAI* ai) : Trigger(ai, "leave tbc raid", 5) {}
        bool IsActive() override;
    };

    // The bot stands in a dangerous zone (fire, void zone, whirlwind, fixating add, ...)
    class TbcRaidInHazardTrigger : public Trigger
    {
    public:
        TbcRaidInHazardTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid in hazard", 1) {}
        bool IsActive() override;
    };

    // The bot has a debuff which requires it to run away from the raid
    class TbcRaidDebuffRunTrigger : public Trigger
    {
    public:
        TbcRaidDebuffRunTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid debuff run", 1) {}
        bool IsActive() override;
    };

    // The bot stands too close to another raid member
    class TbcRaidSpreadTrigger : public Trigger
    {
    public:
        TbcRaidSpreadTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid spread", 1) {}
        bool IsActive() override;
    };

    // The bot must stack on a raid member
    class TbcRaidGatherTrigger : public Trigger
    {
    public:
        TbcRaidGatherTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid gather", 1) {}
        bool IsActive() override;
    };

    // The bot stands in front of a boss with a frontal attack
    class TbcRaidInFrontOfBossTrigger : public Trigger
    {
    public:
        TbcRaidInFrontOfBossTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid in front of boss", 1) {}
        bool IsActive() override;
    };

    // The bot is not attacking the add which has to die first
    class TbcRaidPriorityTargetTrigger : public Trigger
    {
    public:
        TbcRaidPriorityTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid priority target", 1) {}
        bool IsActive() override;
    };

    // The bot (tank) has to taunt the boss
    class TbcRaidTauntTrigger : public Trigger
    {
    public:
        TbcRaidTauntTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid taunt", 1) {}
        bool IsActive() override;
    };

    // Magtheridon: the bot has to go to / click its Manticron Cube
    class TbcRaidUseCubeTrigger : public Trigger
    {
    public:
        TbcRaidUseCubeTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid use cube", 1) {}
        bool IsActive() override;
    };

    // Naj'entus: the bot has to free an impaled player or throw a spine at the Tidal Shield
    class TbcRaidUseSpineTrigger : public Trigger
    {
    public:
        TbcRaidUseSpineTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid use spine", 1) {}
        bool IsActive() override;
    };
}
