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

        // A creature of the encounter and the raid member holding it
        struct Assignment
        {
            ObjectGuid creature;
            ObjectGuid holder;
            const TankAssignment* rule;
        };

        // Per bot state, cached for a short time to keep the raid triggers cheap
        struct RaidState
        {
            uint32 assignCheckTime = 0;
            std::vector<Assignment> assignments;
            std::vector<ObjectGuid> interrupters;
            const Encounter* announcedEncounter = nullptr;

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

        // Tank assignments of the encounter (who holds which creature, refreshed every second)
        static const std::vector<tbcraid::Assignment>& GetAssignments(PlayerbotAI* ai);
        // Creatures the bot has to hold, in the order of the encounter table
        static std::vector<std::pair<Unit*, const tbcraid::TankAssignment*>> GetMyAssignments(PlayerbotAI* ai);
        static bool IsAssignedToMe(PlayerbotAI* ai, Unit* creature);
        // Spot of the holder of a creature (relative to its spawn point, checked for height and line of sight)
        static bool GetAssignmentSpot(Unit* creature, const tbcraid::TankAssignment& rule, WorldPosition& spot);

        // Raid boss: tank this healer keeps up first (healers and tanks paired in guid order)
        static Player* GetAssignedTankToHeal(PlayerbotAI* ai);

        // Interrupt the bot has to cast right now (fixed interrupters of the encounter)
        static Unit* GetInterruptTarget(PlayerbotAI* ai, std::string& spellName);
        static bool IsInterrupter(PlayerbotAI* ai, Player* member);

        // Creature a raid icon of the encounter belongs on right now (index into encounter->marks)
        static Unit* GetMarkTarget(PlayerbotAI* ai, const tbcraid::Encounter* encounter, size_t markIndex);
        // Living, attackable creatures of an entry sorted by guid (same order for every bot)
        static std::vector<Unit*> GetSortedCreatures(Player* bot, uint32 entry, float range);

        // Bot that sets the raid icons and announces the tactic (lowest guid bot of the group)
        static bool IsRaidLeaderBot(PlayerbotAI* ai);
        // Short description of the tactic and the assignments, for the raid chat
        static std::string DescribeTactic(PlayerbotAI* ai, const tbcraid::Encounter* encounter);
        static std::string DescribeMyTask(PlayerbotAI* ai);

        // Magtheridon - Manticron Cube assigned to the bot
        static GameObject* GetAssignedCube(PlayerbotAI* ai, Unit** magtheridon = nullptr);
        static bool IsCastingBlastNova(Unit* magtheridon);

        // Naj'entus - spine handling
        static GameObject* GetSpineToRemove(PlayerbotAI* ai);
        static Unit* GetShieldedNajentus(PlayerbotAI* ai);

        // Off tank stays second on threat / stacks on the main tank
        static Player* GetMainTankToStackOn(PlayerbotAI* ai);

        // Bloodboil rotation: boss to come closer to (and the distance)
        static Unit* GetApproachTarget(PlayerbotAI* ai, float& distance);

        // Meteor Slash soaking: boss the bot has to stand in front of
        static Unit* GetSoakBoss(PlayerbotAI* ai);
        static bool ShouldSoakFront(PlayerbotAI* ai, Unit* boss);

        // Hydross: position the tank has to drag the boss to
        static bool GetHydrossTankPosition(PlayerbotAI* ai, WorldPosition& position);

        // Lady Vashj: Tainted Core handling
        static Creature* GetTaintedCoreCorpse(PlayerbotAI* ai);
        static GameObject* GetNearestActiveShieldGenerator(PlayerbotAI* ai, WorldObject* from);
        static Player* GetTaintedCoreReceiver(PlayerbotAI* ai, GameObject* generator);

        // The Lurker Below: boss currently spouting
        static Unit* GetSpoutingLurker(PlayerbotAI* ai);

        // Creature possessed by the bot through a boss mechanic (Vengeful Spirit, Blue Dragon)
        static Creature* GetPossessedCreature(PlayerbotAI* ai);
        static bool IsPossessingBossCreature(Player* bot);

        // Kil'jaeden: Blue Dragon orbs and Darkness of a Thousand Souls
        static GameObject* GetBlueOrbToUse(PlayerbotAI* ai);
        static Unit* GetKiljaedenCastingDarkness(PlayerbotAI* ai);
        static Unit* GetRaidBlueDragon(PlayerbotAI* ai);

        // Cast a spell of a possessed creature, selected by (part of) its name
        static uint32 FindCharmSpell(Creature* charm, const std::string& namePart);

        // Utility
        static std::list<Unit*> FindCreatures(Player* bot, uint32 entry, float range);
        static std::list<Unit*> FindCreaturesAround(WorldObject* center, uint32 entry, float range);
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
