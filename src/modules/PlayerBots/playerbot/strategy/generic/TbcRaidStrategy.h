#pragma once
#include "playerbot/strategy/Strategy.h"
#include "playerbot/strategy/Multiplier.h"
#include "playerbot/WorldPosition.h"
#include "TbcRaidEncounters.h"

namespace ai
{
    namespace tbcraid
    {
        // A zone the bot has to leave (creature, object or ground effect)
        struct HazardZone
        {
            WorldPosition position;
            float radius;
            ObjectGuid guid;        // creature / object guid, empty for ground effects
        };

        // Per bot state, cached for a short time to keep the raid triggers cheap
        struct RaidState
        {
            uint32 encounterCheckTime = 0;
            const Encounter* encounter = nullptr;
            ObjectGuid bossGuid;

            uint32 hazardCheckTime = 0;
            std::vector<HazardZone> hazards;

            uint32 priorityCheckTime = 0;
            ObjectGuid priorityTarget;
        };
    }

    class TbcRaidStrategy : public Strategy
    {
    public:
        TbcRaidStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "tbc raid"; }
        static TbcRaidStrategy* Get(PlayerbotAI* ai);

        tbcraid::RaidState& GetState() { return state; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "tbc raid"; }
        virtual std::string GetHelpDescription()
        {
            return "This strategy makes the bot follow the raid tactics of the Burning Crusade raid bosses:\n"
                   "avoid dangerous zones, spread out, run away with debuffs, kill adds in the right order,\n"
                   "swap tanks and handle special mechanics like the Manticron Cubes.";
        }
#endif

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
        void InitReactionMultipliers(std::list<Multiplier*>& multipliers) override;

    private:
        tbcraid::RaidState state;
    };

    // Shared logic used by the tbc raid triggers, actions and multipliers
    class TbcRaidHelper
    {
    public:
        // Active encounter of the bot (nullptr if not fighting a known boss)
        static const tbcraid::Encounter* GetEncounter(PlayerbotAI* ai, Unit** boss = nullptr);

        static uint8 GetRole(PlayerbotAI* ai);
        static bool IsMainTank(PlayerbotAI* ai, Unit* boss);

        // Zones the bot has to leave right now
        static const std::vector<tbcraid::HazardZone>& GetHazards(PlayerbotAI* ai);
        static bool IsInHazard(const WorldPosition& point, const std::vector<tbcraid::HazardZone>& hazards, float margin = 0.0f);

        // Minimal distance the bot must keep to the other raid members (0 = no spreading needed)
        static float GetRequiredSpread(PlayerbotAI* ai, bool* isDebuffRun = nullptr);

        // Raid member the bot has to stack on
        static Player* GetGatherTarget(PlayerbotAI* ai, float& distance);

        // Add / boss which must be attacked by the bot right now
        static Unit* GetPriorityTarget(PlayerbotAI* ai);

        // Boss which must be taunted by the bot right now
        static Unit* GetTauntTarget(PlayerbotAI* ai);
        static bool ShouldHoldTaunt(PlayerbotAI* ai);

        // Boss in front of which the bot is standing (cleave, breath, ...)
        static Unit* GetFrontDanger(PlayerbotAI* ai);

        // Magtheridon - Manticron Cube assigned to the bot
        static GameObject* GetAssignedCube(PlayerbotAI* ai, Unit** magtheridon = nullptr);
        static bool IsCastingBlastNova(Unit* magtheridon);

        // Naj'entus - spine handling
        static GameObject* GetSpineToRemove(PlayerbotAI* ai);
        static Unit* GetShieldedNajentus(PlayerbotAI* ai);

        // Utility
        static std::list<Unit*> FindCreatures(Player* bot, uint32 entry, float range);
        static std::list<GameObject*> FindGameObjects(Player* bot, uint32 entry, float range);
        static std::vector<Player*> GetRaidMembers(Player* bot, float range, bool includeSelf = false);
        static bool IsCreatureCasting(Unit* unit, uint32 spellId);
    };

    // Blocks actions which would break the raid tactics
    class TbcRaidMultiplier : public Multiplier
    {
    public:
        TbcRaidMultiplier(PlayerbotAI* ai) : Multiplier(ai, "tbc raid") {}
        float GetValue(Action* action) override;
    };
}
