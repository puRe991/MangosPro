// Ported from mod-playerbots (src/Ai/Raid/SSC, GPL v2+) and adapted for this core (CMaNGOS TBC).
#pragma once

#include "playerbot/WorldPosition.h"
#include <array>
#include <ctime>
#include <functional>
#include <map>
#include <vector>

class Player;
class Unit;
class Creature;
class GameObject;
class PlayerbotAI;

namespace ai
{
    class Action;

    // Serpentshrine Cavern helpers. Ids checked against the ScriptDev2 scripts in
    // src/game/AI/ScriptDevAI/scripts/outland/coilfang_reservoir/serpent_shrine
    namespace ssc
    {
        constexpr uint32 SSC_MAP_ID = 548;

        enum SscSpells : uint32
        {
            // Trash
            SPELL_TOXIC_POOL             = 38718,   // Underbog Colossus death pool

            // Hydross the Unstable
            SPELL_MARK_OF_HYDROSS_10     = 38215,
            SPELL_MARK_OF_HYDROSS_25     = 38216,
            SPELL_MARK_OF_HYDROSS_50     = 38217,
            SPELL_MARK_OF_HYDROSS_100    = 38218,
            SPELL_MARK_OF_HYDROSS_250    = 38231,
            SPELL_MARK_OF_HYDROSS_500    = 40584,
            SPELL_MARK_OF_CORRUPTION_10  = 38219,
            SPELL_MARK_OF_CORRUPTION_25  = 38220,
            SPELL_MARK_OF_CORRUPTION_50  = 38221,
            SPELL_MARK_OF_CORRUPTION_100 = 38222,
            SPELL_MARK_OF_CORRUPTION_250 = 38230,
            SPELL_MARK_OF_CORRUPTION_500 = 40583,
            SPELL_CORRUPTION             = 37961,   // corrupted (nature) form

            // The Lurker Below
            SPELL_SPOUT                  = 37431,
            SPELL_SPOUT_LEFT             = 37429,   // rotation auras during Spout (this core)
            SPELL_SPOUT_RIGHT            = 37430,
            SPELL_LURKER_SUBMERGE        = 28819,

            // Leotheras the Blind
            SPELL_LEOTHERAS_BANISHED     = 37546,
            SPELL_WHIRLWIND              = 37640,
            SPELL_WHIRLWIND_CHANNEL      = 37641,
            SPELL_METAMORPHOSIS          = 37673,
            SPELL_CHAOS_BLAST            = 37675,   // stacking debuff triggered by 37674
            SPELL_INSIDIOUS_WHISPER      = 37676,
            SPELL_CONSUMING_MADNESS      = 37749,   // mind control when an Inner Demon survives

            // Lady Vashj
            SPELL_MAGIC_BARRIER          = 38112,
            SPELL_STATIC_CHARGE          = 38280,
            SPELL_ENTANGLE               = 38316,
            SPELL_PARALYZE               = 38132,   // holder of a Tainted Core is rooted

            // Druid
            SPELL_CAT_FORM               = 768,
            SPELL_BEAR_FORM              = 5487,
            SPELL_DIRE_BEAR_FORM         = 9634,
            SPELL_TREE_OF_LIFE           = 33891,

            // Hunter
            SPELL_MISDIRECTION           = 35079,   // buff on the hunter after Misdirection

            // Mage / Warlock slows
            SPELL_SLOW                   = 31589,
            SPELL_CURSE_OF_EXHAUSTION    = 18223,
        };

        enum SscNpcs : uint32
        {
            // Trash
            NPC_WATER_ELEMENTAL_TOTEM    = 22236,   // Greyheart Tidecaller

            // Hydross the Unstable
            NPC_HYDROSS                  = 21216,
            NPC_PURE_SPAWN_OF_HYDROSS    = 22035,
            NPC_TAINTED_SPAWN_OF_HYDROSS = 22036,

            // The Lurker Below
            NPC_LURKER_BELOW             = 21217,
            NPC_COILFANG_GUARDIAN        = 21873,

            // Leotheras the Blind
            NPC_LEOTHERAS                = 21215,
            NPC_GREYHEART_SPELLBINDER    = 21806,
            NPC_INNER_DEMON              = 21857,
            NPC_SHADOW_OF_LEOTHERAS      = 21875,

            // Fathom-Lord Karathress
            NPC_KARATHRESS               = 21214,
            NPC_CARIBDIS                 = 21964,
            NPC_TIDALVESS                = 21965,
            NPC_SHARKKIS                 = 21966,
            NPC_SPITFIRE_TOTEM           = 22091,

            // Morogrim Tidewalker
            NPC_MOROGRIM                 = 21213,
            NPC_TIDEWALKER_LURKER        = 21920,   // murlocs

            // Lady Vashj
            NPC_LADY_VASHJ               = 21212,
            NPC_ENCHANTED_ELEMENTAL      = 21958,
            NPC_TINY_TRIGGER             = 21987,   // channels Magic Barrier at an active generator
            NPC_TAINTED_ELEMENTAL        = 22009,
            NPC_COILFANG_ELITE           = 22055,
            NPC_COILFANG_STRIDER         = 22056,
            NPC_TOXIC_SPOREBAT           = 22140,
            NPC_SPORE_DROP_TRIGGER       = 22207,
        };

        enum SscObjects : uint32
        {
            GO_SHIELD_GENERATOR_1        = 185051,
            GO_SHIELD_GENERATOR_4        = 185054,
            ITEM_TAINTED_CORE            = 31088,
        };

        // Hydross
        extern const WorldPosition HYDROSS_FROST_TANK_POSITION;
        extern const WorldPosition HYDROSS_NATURE_TANK_POSITION;
        bool HasMarkOfHydrossAt100Percent(Player* bot);
        bool HasNoMarkOfHydross(Player* bot);
        bool HasMarkOfCorruptionAt100Percent(Player* bot);
        bool HasNoMarkOfCorruption(Player* bot);
        bool UpdateHydrossState(PlayerbotAI* ai);
        bool ShouldPauseHydrossDps(PlayerbotAI* ai);

        // The Lurker Below
        extern const WorldPosition LURKER_MAIN_TANK_POSITION;
        bool IsLurkerSpouting(Unit* lurker);
        bool IsLurkerSubmerged(Unit* lurker);

        // Leotheras the Blind
        Unit* GetLeotherasHuman(PlayerbotAI* ai);
        Unit* GetPhase2LeotherasDemon(PlayerbotAI* ai);
        Unit* GetPhase3LeotherasDemon(PlayerbotAI* ai);
        Unit* GetActiveLeotherasDemon(PlayerbotAI* ai);
        Player* GetLeotherasDemonFormTank(Player* bot);
        // Group member mind controlled by Consuming Madness, nearest first
        Player* GetLeotherasMindControlledPlayer(Player* bot);
        // Threat is wiped when Whirlwind ends: true for a few seconds afterwards
        bool IsLeotherasWhirlwindAggroReset(Player* bot);
        Unit* GetMyInnerDemon(Player* bot);
        bool CanUseLeotherasMovement(PlayerbotAI* ai, const std::string& action);

        // Fathom-Lord Karathress
        extern const WorldPosition KARATHRESS_TANK_POSITION;
        extern const WorldPosition TIDALVESS_TANK_POSITION;
        extern const WorldPosition SHARKKIS_TANK_POSITION;
        extern const WorldPosition CARIBDIS_TANK_POSITION;
        extern const WorldPosition CARIBDIS_HEALER_POSITION;
        extern const WorldPosition CARIBDIS_RANGED_DPS_POSITION;
        // Living council members without an own tank, in dps kill order, followed by Karathress.
        // With fewer than four tanks the main tank holds these together at the Karathress spot.
        std::vector<Unit*> GetKarathressMainTankUnits(PlayerbotAI* ai);

        // Morogrim Tidewalker
        extern const WorldPosition TIDEWALKER_PHASE_1_TANK_POSITION;
        extern const WorldPosition TIDEWALKER_PHASE_TRANSITION_WAYPOINT;
        extern const WorldPosition TIDEWALKER_PHASE_2_TANK_POSITION;
        extern const WorldPosition TIDEWALKER_PHASE_2_RANGED_POSITION;

        // Lady Vashj
        constexpr float VASHJ_PLATFORM_CENTER_Z = 42.902f;
        extern const WorldPosition VASHJ_PLATFORM_CENTER_POSITION;
        bool IsLadyVashjInPhase1(PlayerbotAI* ai);
        bool IsLadyVashjInPhase2(PlayerbotAI* ai);
        bool IsLadyVashjInPhase3(PlayerbotAI* ai);
        bool IsValidLadyVashjCombatNpc(Unit* unit, PlayerbotAI* ai);
        bool IsMainTankInSameSubgroup(Player* bot);
        // [0] designated looter, [1..4] first to fourth core passer (may be nullptr)
        std::array<Player*, 5> GetCoreHandlers(Player* bot);
        int GetCoreHandlerIndex(Player* bot);
        bool AnyRecentCoreInInventory(Player* bot);
        bool HasTaintedCore(Player* player);
        GameObject* GetNearestActiveShieldGenerator(Player* bot, WorldObject* from);
        std::vector<Unit*> GetSporeDropTriggers(Player* bot);

        // Shared encounter state, per instance (bots of an instance can run in other threads
        // than the bots of another instance, every access is locked)
        struct SscInstanceState
        {
            time_t hydrossFrostDpsWait = 0;
            time_t hydrossNatureDpsWait = 0;
            time_t hydrossChangeToFrostPhase = 0;
            time_t hydrossChangeToNaturePhase = 0;
            time_t lurkerSpoutEnd = 0;
            time_t leotherasHumanFormDpsWait = 0;
            time_t leotherasDemonFormDpsWait = 0;
            time_t leotherasFinalPhaseDpsWait = 0;
            time_t leotherasWhirlwindEnd = 0;
            bool leotherasWhirlwindActive = false;
            ObjectGuid leotherasDemonTank;  // chosen once per fight: resistances change with buffs and range
            time_t karathressDpsWait = 0;
            time_t lastCoreThrow = 0;
            ObjectGuid vashjGenerator;
            std::map<ObjectGuid, WorldPosition> lurkerRangedPositions;
            std::map<ObjectGuid, uint8> tidewalkerTankStep;
            std::map<ObjectGuid, uint8> tidewalkerRangedStep;
            std::map<ObjectGuid, bool> hasReachedVashjRangedPosition;
            std::map<ObjectGuid, WorldPosition> intendedLineup;
            std::map<ObjectGuid, time_t> lastCoreInInventoryTime;
        };

        // Runs the function with the locked state of the bot's instance
        void WithState(Player* bot, const std::function<void(SscInstanceState&)>& function);
        SscInstanceState GetState(Player* bot);
        bool NeedsStateReset(PlayerbotAI* ai);

        // Cached lookup of a creature of this raid (one grid search per bot and entry every 500 ms)
        Unit* FindSscUnit(PlayerbotAI* ai, uint32 entry, float range = 150.0f);

        // Members of a role, assistants first (IsAssistHealOfIndex / IsAssistRangedDpsOfIndex)
        std::vector<Player*> GetHealersAssistantsFirst(Player* bot);
        std::vector<Player*> GetRangedDpsAssistantsFirst(Player* bot);
        Player* GetHealerOfIndex(Player* bot, uint8 index);

        // Stops the auto attack / spell cast of the bot (the "botAI->Reset()" of mod-playerbots)
        bool StopAttacking(PlayerbotAI* ai);

        // Point at 'distance' from (fromX, fromY) on the line through the bot
        WorldPosition GetPointAwayFrom(Player* bot, float fromX, float fromY, float distance);
        // One step of at most maxStep yards towards a position
        WorldPosition GetStepTowards(Player* bot, const WorldPosition& position, float maxStep);
        // Ranged spread: a spot near the bot that keeps spreadDistance from the group (as far as possible)
        // and stays within [minAnchorDistance, maxAnchorDistance] of the anchor. False if no better spot.
        bool GetRangedSpreadPosition(Player* bot, Unit* anchor, float spreadDistance, float minAnchorDistance,
                                     float maxAnchorDistance, WorldPosition& position);

        bool IsActionNamed(Action* action, std::initializer_list<const char*> names);
        bool IsTbcRaidAction(Action* action);
    }
}
