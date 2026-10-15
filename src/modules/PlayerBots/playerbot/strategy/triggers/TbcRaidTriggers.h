#pragma once
#include "playerbot/strategy/Trigger.h"
#include <functional>

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

    // The bot holds a creature of the encounter that is loose or not its current target
    class TbcRaidTankAssignedTrigger : public Trigger
    {
    public:
        TbcRaidTankAssignedTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid tank assigned", 1) {}
        bool IsActive() override;
    };

    // The bot holds its creature but is not at its spot
    class TbcRaidTankPositionTrigger : public Trigger
    {
    public:
        TbcRaidTankPositionTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid tank position", 1) {}
        bool IsActive() override;
    };

    // A creature the bot holds has the aura the bot has to spellsteal (Krosh Firehand - Spell Shield)
    class TbcRaidStealAuraTrigger : public Trigger
    {
    public:
        TbcRaidStealAuraTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid steal aura", 1) {}
        bool IsActive() override;
    };

    // The bot is a fixed interrupter and the creature casts a spell to interrupt
    class TbcRaidInterruptTrigger : public Trigger
    {
    public:
        TbcRaidInterruptTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid interrupt", 1) {}
        bool IsActive() override;
    };

    // The bot sets the raid icons and a creature of the encounter is not marked yet
    class TbcRaidMarkTargetsTrigger : public Trigger
    {
    public:
        TbcRaidMarkTargetsTrigger(PlayerbotAI* ai) : Trigger(ai, "tbc raid mark targets", 2) {}
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

    // Generic "the helper says yes" trigger for the special mechanics
    class TbcRaidMechanicTrigger : public Trigger
    {
    public:
        TbcRaidMechanicTrigger(PlayerbotAI* ai, std::string name, std::function<bool(PlayerbotAI*)> check)
            : Trigger(ai, name, 1), check(check) {}
        bool IsActive() override { return bot->IsAlive() && check(ai); }

    private:
        std::function<bool(PlayerbotAI*)> check;
    };
}
