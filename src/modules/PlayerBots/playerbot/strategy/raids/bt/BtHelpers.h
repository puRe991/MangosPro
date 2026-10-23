// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/BT/BTHelpers) and adapted for this core.
#pragma once

#include "playerbot/WorldPosition.h"
#include <array>
#include <ctime>
#include <string>
#include <vector>

class Player;
class Unit;
class GameObject;
class PlayerbotAI;

namespace ai
{
    class Action;

    // Black Temple (map 564) helpers, ids checked against the ScriptDev2 scripts in
    // src/game/AI/ScriptDevAI/scripts/outland/black_temple
    namespace bt
    {
        constexpr uint32 BLACK_TEMPLE_MAP_ID = 564;

        enum BtNpcs : uint32
        {
            NPC_NAJENTUS                = 22887,
            NPC_SUPREMUS                = 22898,
            NPC_SUPREMUS_VOLCANO        = 23085,
            NPC_SUPREMUS_MOLTEN_FLAME   = 23095,    // stalker of Molten Punch, chases a random player
            NPC_SHADE_OF_AKAMA          = 22841,
            NPC_ASHTONGUE_CHANNELER     = 23421,
            NPC_TERON_GOREFIEND         = 22871,
            NPC_SHADOWY_CONSTRUCT       = 23111,
            NPC_GURTOGG_BLOODBOIL       = 22948,
            NPC_RELIQUARY_OF_SOULS      = 22856,
            NPC_ESSENCE_OF_SUFFERING    = 23418,
            NPC_ESSENCE_OF_DESIRE       = 23419,
            NPC_ESSENCE_OF_ANGER        = 23420,
            NPC_MOTHER_SHAHRAZ          = 22947,
            NPC_GATHIOS                 = 22949,
            NPC_ZEREVOR                 = 22950,
            NPC_MALANDE                 = 22951,
            NPC_VERAS                   = 22952,
            NPC_ILLIDAN                 = 22917,
            NPC_FLAME_OF_AZZINOTH       = 22997,
            NPC_DEMON_FIRE              = 23069,
            NPC_ILLIDAN_DB_TARGET       = 23070,    // Eye Blast target
            NPC_BLAZE                   = 23259,
            NPC_FLAME_CRASH             = 23336,
            NPC_SHADOW_DEMON            = 23375,
            NPC_PARASITIC_SHADOWFIEND   = 23498,
        };

        enum BtSpells : uint32
        {
            // High Warlord Naj'entus
            SPELL_IMPALING_SPINE            = 39837,
            SPELL_TIDAL_SHIELD              = 39872,

            // Supremus
            SPELL_SNARE_SELF                = 41922,    // phase 2 (Gaze)

            // Teron Gorefiend
            SPELL_SHADOW_OF_DEATH           = 40251,
            SPELL_SPIRITUAL_VENGEANCE       = 40268,
            SPELL_SPIRIT_LANCE              = 40157,
            SPELL_SPIRIT_CHAINS             = 40175,
            SPELL_SPIRIT_VOLLEY             = 40314,
            SPELL_SPIRIT_STRIKE             = 40325,

            // Gurtogg Bloodboil
            SPELL_BOSS_FEL_RAGE             = 40594,
            SPELL_PLAYER_FEL_RAGE           = 40604,

            // Reliquary of Souls
            SPELL_DEADEN                    = 41410,
            SPELL_RUNE_SHIELD               = 41431,

            // Mother Shahraz (40869 is the dummy which triggers 41001)
            SPELL_FATAL_ATTRACTION          = 41001,

            // Illidari Council
            SPELL_BLESSING_OF_PROTECTION    = 41450,
            SPELL_BLESSING_OF_SPELL_WARDING = 41451,
            SPELL_JUDGEMENT                 = 41467,
            SPELL_SEAL_OF_COMMAND           = 41469,
            SPELL_CONSECRATION              = 41541,
            SPELL_VANISH                    = 41476,
            SPELL_DAMPEN_MAGIC              = 41478,
            SPELL_FLAMESTRIKE               = 41481,
            SPELL_BLIZZARD                  = 41482,

            // Illidan Stormrage
            SPELL_DEMON_TRANSFORM_1         = 40511,
            SPELL_DEMON_TRANSFORM_2         = 40398,
            SPELL_DEMON_TRANSFORM_3         = 40510,
            SPELL_DEMON_FORM                = 40506,
            SPELL_DARK_BARRAGE              = 40585,
            SPELL_SHADOW_PRISON             = 40647,
            SPELL_CAGED                     = 40695,
            SPELL_PARASITIC_SHADOWFIEND_1   = 41917,    // cast by Illidan (primary infection)
            SPELL_PARASITIC_SHADOWFIEND_2   = 41914,    // cast by a Shadowfiend on contact

            // Shaman
            SPELL_EARTHBIND_TOTEM           = 2484,
        };

        enum BtItems : uint32
        {
            ITEM_NAJENTUS_SPINE = 32408,
        };

        enum BtObjects : uint32
        {
            GO_NAJENTUS_SPINE   = 185584,
            GO_SHADOW_TRAP      = 185916,   // Maiev's Cage Trap
        };

        // Relevances: mod ACTION_EMERGENCY + n compressed into 91..99 (order kept, everything < 100)
        constexpr float BT_EMERGENCY_1  = 91.0f;
        constexpr float BT_EMERGENCY_2  = 92.0f;
        constexpr float BT_EMERGENCY_3  = 93.0f;
        constexpr float BT_EMERGENCY_6  = 95.0f;
        constexpr float BT_EMERGENCY_7  = 96.0f;
        constexpr float BT_EMERGENCY_9  = 97.0f;
        constexpr float BT_EMERGENCY_10 = 98.0f;
        constexpr float BT_EMERGENCY_11 = 99.0f;

        enum class TankPositionState : uint8
        {
            MovingToTransition = 0,
            MovingToFinal      = 1,
            Positioned         = 2,
            Unknown            = 255,
        };

        // Shared encounter state (timers per instance, steps per bot). Thread safe: maps can be
        // updated in parallel, so every access goes through these functions.
        enum class BtState : uint8
        {
            NajentusShieldTimer,
            SupremusPhaseTimer,
            GurtoggPhaseTimer,
            CouncilDpsTimer,
            IllidanBossDpsTimer,
            IllidanFlameDpsTimer,
            IllidanLastPhase,
            IllidanEastFlame,
            IllidanWestFlame,
            AkamaReachedPosition,   // per bot
            ShahrazTankStep,        // per bot
            GathiosTankStep,        // per bot
            ZerevorHealStep,        // per bot
            FlameTankWaypoint,      // per bot
            PetMadePassive,         // per bot
        };

        bool GetState(BtState type, uint64 key, int64& value);
        void SetState(BtState type, uint64 key, int64 value);
        bool SetStateIfAbsent(BtState type, uint64 key, int64 value);
        bool EraseState(BtState type, uint64 key);
        // Seconds since the timer was started, -1 if not running
        int64 GetTimerElapsed(BtState type, uint32 instanceId);

        // Creature of an entry in combat with the bot (the "find target" value of mod-playerbots)
        Unit* FindBoss(PlayerbotAI* ai, uint32 entry);
        // Alive creature of an entry within range (grid search)
        Unit* FindNearest(Player* bot, uint32 entry, float range);
        // Encounter still fought by somebody (used to reset the instance timers out of combat)
        bool IsBossInCombat(Player* bot, uint32 entry);

        // Action classification by name / class (our core has no CombatFormationMoveAction)
        bool IsFormationMoveAction(Action* action);
        bool IsFleeOrFollowAction(Action* action);
        bool IsReachAction(Action* action);
        bool IsAttackOrSpellAction(Action* action);   // attacks and non healing spells

        // Positions
        extern const WorldPosition NAJENTUS_TANK_POSITION;
        extern const WorldPosition AKAMA_CHANNELER_POSITION;
        extern const WorldPosition GOREFIEND_TANK_POSITION;
        extern const WorldPosition GOREFIEND_DIE_POSITION;
        extern const WorldPosition GURTOGG_TANK_POSITION;
        extern const WorldPosition GURTOGG_RANGED_POSITION;
        extern const WorldPosition GURTOGG_SOAKER_POSITION;
        extern const WorldPosition SHAHRAZ_TANK_POSITION;
        extern const WorldPosition SHAHRAZ_TRANSITION_POSITION;
        extern const WorldPosition SHAHRAZ_RANGED_POSITION;
        constexpr float COUNCIL_FLOOR_Z_THRESHOLD = 270.0f;
        extern const std::array<WorldPosition, 4> GATHIOS_TANK_POSITIONS;
        extern const WorldPosition MALANDE_TANK_POSITION;
        extern const WorldPosition ZEREVOR_TANK_POSITION;
        extern const std::array<WorldPosition, 2> ZEREVOR_HEALER_POSITIONS;
        extern const WorldPosition ILLIDAN_LANDING_POSITION;
        extern const WorldPosition ILLIDAN_N_GRATE_POSITION;
        extern const WorldPosition ILLIDAN_E_GRATE_POSITION;
        extern const WorldPosition ILLIDAN_W_GRATE_POSITION;
        extern const std::array<WorldPosition, 3> GRATE_POSITIONS;
        extern const WorldPosition ILLIDAN_E_GLAIVE_WAITING_POSITION;
        extern const std::array<WorldPosition, 7> E_GLAIVE_TANK_POSITIONS;
        extern const WorldPosition ILLIDAN_W_GLAIVE_WAITING_POSITION;
        extern const std::array<WorldPosition, 7> W_GLAIVE_TANK_POSITIONS;

        // Supremus
        bool HasSupremusVolcanoNearby(Player* bot);
        std::vector<Unit*> GetSupremusVolcanos(Player* bot, float range);
        Unit* GetSupremusMoltenFlameChasingBot(Player* bot);
        float GetSupremusMeleeReach(Player* bot, Unit* supremus);

        // Gurtogg Bloodboil
        std::vector<std::vector<Player*>> GetGurtoggRangedRotationGroups(Player* bot);
        int GetGurtoggActiveRotationGroup(Unit* gurtogg);

        // Mother Shahraz
        TankPositionState GetShahrazTankPositionState(Player* bot);
        std::vector<Player*> GetFatalAttractionPlayers(Player* bot);

        // Illidari Council
        Player* GetZerevorMageTank(Player* bot);
        Player* GetZerevorHealer(Player* bot);
        bool HasDangerousCouncilAura(Unit* unit);

        // Illidan Stormrage: -1 none/outro, 0 landing, 1 > 65%, 2 flying, 3 normal, 4 demon, 5 <= 30%
        int GetIllidanPhase(Unit* illidan);
        bool IsIllidanDefeated(Unit* illidan);
        std::vector<Unit*> GetAllFlameCrashes(Player* bot);
        void GetFlamesOfAzzinoth(Player* bot, Unit*& eastFlame, Unit*& westFlame);
        Player* GetIllidanWarlockTank(Player* bot);
        bool HasParasiticShadowfiend(Player* member);
        Player* GetIllidanTrapperHunter(Player* bot);
        Player* GetBotWithParasiticShadowfiend(Player* bot);
        GameObject* FindNearestTrap(PlayerbotAI* ai);

        struct EyeBlastDangerArea
        {
            float startX = 0.0f, startY = 0.0f;
            float endX = 0.0f, endY = 0.0f;
            float width = 0.0f;
        };
        EyeBlastDangerArea GetEyeBlastDangerArea(Player* bot);
        bool IsPositionInEyeBlastDangerArea(float x, float y, const EyeBlastDangerArea& area);

        float Dist2d(float x1, float y1, float x2, float y2);
    }
}
