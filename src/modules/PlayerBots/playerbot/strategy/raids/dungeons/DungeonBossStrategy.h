#pragma once

#include "playerbot/strategy/generic/TbcRaidStrategy.h"
#include "playerbot/strategy/actions/TbcRaidActions.h"
#include "playerbot/strategy/Trigger.h"

namespace ai
{
    // One source of truth for map activation and context registration.
    const std::vector<std::pair<uint32, std::string>>& GetDungeonBossMaps();
    bool IsDungeonBossMap(uint32 mapId);
    const std::vector<std::pair<uint32, std::string>>& GetClassicRaidBossMaps();
    bool IsClassicRaidBossMap(uint32 mapId);
    Unit* GetClassicRaidPriorityTarget(PlayerbotAI* ai);

    class DungeonBossStrategy : public TbcRaidStrategy
    {
    public:
        DungeonBossStrategy(PlayerbotAI* ai, const std::string& name) : TbcRaidStrategy(ai), name(name) {}
        std::string getName() override { return name; }

    protected:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitReactionMultipliers(std::list<Multiplier*>& multipliers) override;
    private:
        std::string name;
    };

    class ClassicRaidBossStrategy : public DungeonBossStrategy
    {
    public:
        ClassicRaidBossStrategy(PlayerbotAI* ai, const std::string& name) : DungeonBossStrategy(ai, name) {}
    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class DungeonBossSpecialTrigger : public Trigger
    {
    public:
        DungeonBossSpecialTrigger(PlayerbotAI* ai) : Trigger(ai, "dungeon boss special", 1) {}
        bool IsActive() override;
    };

    class DungeonBossSpecialAction : public TbcRaidMovementAction
    {
    public:
        DungeonBossSpecialAction(PlayerbotAI* ai) : TbcRaidMovementAction(ai, "dungeon boss special") {}
        bool isPossible() override { return true; }
        bool ShouldReactionInterruptCast() const override;
        bool Execute(Event& event) override;
    };

    class DungeonBossMultiplier : public Multiplier
    {
    public:
        DungeonBossMultiplier(PlayerbotAI* ai) : Multiplier(ai, "dungeon boss mechanics") {}
        float GetValue(Action* action) override;
    };
}
