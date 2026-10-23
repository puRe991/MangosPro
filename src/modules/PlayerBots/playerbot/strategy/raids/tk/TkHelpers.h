// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/TK/Util/TKHelpers) and adapted for this core.
#pragma once

#include "playerbot/WorldPosition.h"
#include <list>
#include <utility>
#include <vector>

class Player;
class Unit;
class Creature;
class Item;
class PlayerbotAI;

namespace ai
{
    namespace tk
    {
        // Spell ids (ScriptDev2 the_eye boss scripts, TBC 2.4.3)
        enum TkSpells : uint32
        {
            // Crimson Hand Centurion
            SPELL_ARCANE_FLURRY             = 37268,

            // Al'ar (boss_alar.cpp)
            SPELL_REBIRTH_PHASE2            = 34342,    // cast once Al'ar reached the room center after the first "death"
            SPELL_DIVE_BOMB_VISUAL          = 35367,    // fire ball form before the Dive Bomb
            SPELL_REBIRTH_DIVE              = 35369,    // cast after the Dive Bomb to turn back
            SPELL_MELT_ARMOR                = 35410,

            // Void Reaver (boss_void_reaver.cpp)
            SPELL_ARCANE_ORB                = 34172,    // missile, the explosion (34190) hits 20 yards around the target spot

            // High Astromancer Solarian (boss_astromancer.cpp)
            SPELL_SOLARIAN_TRANSFORM        = 39117,
            SPELL_WRATH_OF_THE_ASTROMANCER  = 42783,

            // Kael'thas Sunstrider (boss_kaelthas.cpp)
            SPELL_PERMANENT_FEIGN_DEATH     = 29266,    // advisors "killed" in phase 1
            SPELL_RESURRECTION              = 36450,    // Kael'thas revives the advisors (phase 3)
            SPELL_GRAVITY_LAPSE_AURA        = 39432,    // fly aura on the players
            SPELL_MENTAL_PROTECTION_FIELD   = 36480,    // Staff of Disintegration
            SPELL_ARCANE_BARRIER            = 36481,    // Phaseshift Bulwark
            SPELL_KAELTHAS_MIND_CONTROL     = 36797,
            SPELL_SHOCK_BARRIER             = 36815,

            // Hunter
            SPELL_MISDIRECTION              = 35079,
        };

        enum TkNpcs : uint32
        {
            // Trash
            NPC_CRIMSON_HAND_CENTURION      = 20048,

            // Al'ar
            NPC_ALAR                        = 19514,
            NPC_EMBER_OF_ALAR               = 19551,
            NPC_FLAME_PATCH                 = 20602,

            // Void Reaver
            NPC_VOID_REAVER                 = 19516,

            // High Astromancer Solarian
            NPC_SOLARIAN                    = 18805,
            NPC_SOLARIUM_PRIEST             = 18806,
            NPC_SOLARIUM_AGENT              = 18925,

            // Kael'thas Sunstrider
            NPC_KAELTHAS                    = 19622,
            NPC_SANGUINAR                   = 20060,
            NPC_CAPERNIAN                   = 20062,
            NPC_TELONICUS                   = 20063,
            NPC_THALADRED                   = 20064,
            NPC_NETHERSTRAND_LONGBOW        = 21268,
            NPC_DEVASTATION                 = 21269,
            NPC_COSMIC_INFUSER              = 21270,
            NPC_INFINITY_BLADES             = 21271,
            NPC_WARP_SLICER                 = 21272,
            NPC_PHASESHIFT_BULWARK          = 21273,
            NPC_STAFF_OF_DISINTEGRATION     = 21274,
            NPC_PHOENIX                     = 21362,
            NPC_PHOENIX_EGG                 = 21364,
            NPC_FLAME_STRIKE_TRIGGER        = 21369,
        };

        enum TkItems : uint32
        {
            ITEM_WARP_SLICER                = 30311,
            ITEM_INFINITY_BLADE             = 30312,
            ITEM_STAFF_OF_DISINTEGRATION    = 30313,
            ITEM_PHASESHIFT_BULWARK         = 30314,
            ITEM_DEVASTATION                = 30316,
            ITEM_COSMIC_INFUSER             = 30317,
            ITEM_NETHERSTRAND_LONGBOW       = 30318,
            ITEM_NETHER_SPIKES              = 30319,
        };

        constexpr uint32 TK_MAP_ID = 550;

        struct TkPos
        {
            float x;
            float y;
            float z;
        };

        // General

        bool IsTkEncounterInProgress(Player* bot);
        // Lowest / highest guid of the attackers with this entry (splits two adds between two groups)
        std::pair<Unit*, Unit*> GetTargetUnitPair(PlayerbotAI* ai, uint32 entry);
        Player* GetNearestNonTankPlayerInRadius(Player* bot, float radius);
        float Dist2d(const WorldObject* a, const TkPos& pos);
        Unit* GetCurrentTarget(PlayerbotAI* ai);
        // Raid tank taunt by class ("taunt", "growl", "righteous defense")
        bool CastTaunt(PlayerbotAI* ai, Unit* target);
        // Stops attacking and clears the current target
        void ClearTarget(PlayerbotAI* ai);
        // Rough "is a caster" check (ranged and no hunter)
        bool IsCaster(Player* player);
        bool IsRangedDps(Player* player);
        bool IsMeleeDps(Player* player);
        // First healer of the group (assistants first)
        bool IsFirstAssistHealer(Player* bot);

        // Destination search for the movement actions (replaces MoveAway / FleePosition / MoveFromGroup / MoveInside)
        bool GetPointAwayFrom(Player* bot, float fromX, float fromY, float distance, WorldPosition& out);
        bool GetPointAwayFromGroup(Player* bot, float distance, bool ignoreTanks, WorldPosition& out);
        bool GetPointInside(Player* bot, const TkPos& center, float radius, WorldPosition& out);

        // Trash

        Unit* GetCenturionCastingArcaneFlurry(PlayerbotAI* ai);

        // Al'ar <Phoenix God>
        // CombatReach is 15 yards

        enum AlarLocationIndex : int8
        {
            PLATFORM_0_IDX = 0,     // West
            PLATFORM_1_IDX = 1,     // Northwest
            PLATFORM_2_IDX = 2,     // Northeast
            PLATFORM_3_IDX = 3,     // East
            POINT_QUILL_OR_DIVE_IDX = 4,
            POINT_MIDDLE_IDX = 5,
            LOCATION_NONE = -1
        };

        // 17.0f is intentionally a little lower than the actual balcony positions to capture melee
        // that may be slightly down the ramps from Platform 0 or 3.
        constexpr float ALAR_BALCONY_Z = 17.0f;

        // Landing points of Al'ar (aPlatformLocation / aCenterLocation of boss_alar.cpp)
        constexpr TkPos ALAR_LANDING_PLATFORM_POSITIONS[4] =
        {
            { 335.5587f,  58.56138f, 18.30103f },   // West
            { 387.8628f,  32.18992f, 20.23814f },   // Northwest
            { 388.777f,  -32.10653f, 20.26319f },   // Northeast
            { 333.6099f, -60.69928f, 18.01521f },   // East
        };
        constexpr TkPos ALAR_POINT_QUILLS    = { 330.3943f, -2.02203f,   42.4762f  };
        constexpr TkPos ALAR_POINT_MIDDLE    = { 333.589f,  -0.768249f, -2.38949f };
        constexpr TkPos ALAR_POINT_DIVE_BOMB = { 329.9976f, -0.2719078f, 42.42101f };

        constexpr TkPos ALAR_TANK_PLATFORM_POSITIONS[4] =
        {
            { 329.433f,  59.914f, 17.548f },
            { 395.772f,  26.779f, 20.181f },
            { 387.498f, -41.909f, 20.182f },
            { 330.342f, -61.902f, 17.719f },
        };

        constexpr TkPos ALAR_MELEE_DPS_PLATFORM_POSITIONS[4] =
        {
            { 344.037f,  64.339f, 18.350f },
            { 387.558f,  40.300f, 20.182f },
            { 395.275f, -28.573f, 20.182f },
            { 346.427f, -69.361f, 18.750f },
        };

        constexpr TkPos ALAR_GROUND_POSITIONS[4] =
        {
            { 336.439f,  48.181f, -2.389f },    // Underish West platform
            { 379.122f,  25.146f, -2.385f },    // Underish Northwest platform
            { 378.583f, -27.481f, -2.385f },    // Underish Northeast platform
            { 331.631f, -49.716f, -2.389f },    // Underish East platform
        };

        constexpr TkPos ALAR_ROOM_CENTER   = { 330.611f,  -2.540f, -2.389f };
        constexpr TkPos ALAR_SE_RAMP_BASE  = { 281.064f, -36.590f, -2.389f };
        constexpr TkPos ALAR_SW_RAMP_BASE  = { 281.064f,  36.590f, -2.389f };
        constexpr TkPos ALAR_ROOM_S_CENTER = { 281.064f,   0.000f, -2.389f };

        constexpr float ALAR_FLAME_PATCH_SEARCH_DISTANCE = 40.0f;

        // Entry into phase 2 is measured as the moment that Rebirth (34342) finishes casting.
        bool IsAlarInPhase2(uint32 instanceId);
        // Phase tracker (mechanic tracker bot): true once phase 2 started
        bool UpdateAlarPhaseTracker(Unit* alar);
        int8 GetAlarDestinationLocationIndex(Unit* alar);
        int8 GetAlarCurrentLocationIndex(Unit* alar);
        int8 GetAlarPlatformIndex(Unit* alar);
        // Al'ar is passive while "dead" between phase 1 and 2
        bool IsAlarPassive(Unit* alar);
        const TkPos& GetClosestGroundPosition(Player* bot);
        bool IsPrimaryEmberTank(Player* bot);
        bool IsFirstAlarTank(Player* bot);
        bool IsSecondAlarTank(Player* bot);
        Player* GetSecondaryEmberTank(Player* bot);
        std::list<ObjectGuid> FindFlamePatchGuids(Player* bot);
        std::vector<Unit*> GetFlamePatches(PlayerbotAI* ai);

        // Void Reaver
        // CombatReach is 15 yards

        // Actual hazard range is 20 yards; the rest is buffer for the escape & multiplier.
        constexpr float ARCANE_ORB_SAFE_DISTANCE = 21.0f;  // explosion radius 20 yards
        constexpr float ARCANE_ORB_BUFFER_DISTANCE = 30.0f;

        // Center of the room
        constexpr TkPos VOID_REAVER_TANK_POSITION = { 423.845f, 371.733f, 14.897f };

        // Target spots of the Arcane Orbs which are still flying (read from the spell events of Void Reaver)
        std::vector<WorldPosition> GetActiveArcaneOrbs(Unit* voidReaver);
        bool IsNearArcaneOrb(Player* bot, const std::vector<WorldPosition>& orbs, float radius);
        bool IsNearActiveArcaneOrb(PlayerbotAI* ai, float radius);

        // High Astromancer Solarian

        bool HasWrathOfTheAstromancer(Player* bot);
        // False while Solarian is split (invisible) or transforming
        bool IsSolarianTankable(Unit* astromancer);

        // Kael'thas Sunstrider <Lord of the Blood Elves>

        // Same phases as the mod-playerbots port. The ScriptDev2 phase of Kael'thas is not readable
        // from outside the script, so it is deduced from the state of Kael'thas, the advisors and
        // the weapons and remembered per instance.
        enum KTPhases : uint32
        {
            PHASE_NONE           = 0,
            PHASE_SINGLE_ADVISOR = 1,
            PHASE_WEAPONS        = 2,
            PHASE_TRANSITION     = 3,   // weapon timer is over, advisors are about to be revived
            PHASE_ALL_ADVISORS   = 4,
            PHASE_FINAL          = 5
        };

        // About the exact distance from Kael to the entrances to his room.
        constexpr float KAELTHAS_ROOM_SEARCH_DISTANCE = 125.0f;
        constexpr TkPos KAELTHAS_ROOM_CENTER = { 796.641f, -0.588817f, 48.72847f };
        // Weapon phase length of boss_kaelthas.cpp (KAEL_PHASE_TWO timer)
        constexpr uint32 KAELTHAS_WEAPON_PHASE_MS = 120000;

        // To the left of the advisors' starting position (when facing them), up against the wall.
        constexpr TkPos SANGUINAR_TANK_POSITION    = { 775.478f,  39.888f, 46.780f };
        constexpr TkPos SANGUINAR_WAITING_POSITION = { 761.850f,  27.459f, 46.779f };
        // Next to Sanguinar
        constexpr TkPos TELONICUS_TANK_POSITION    = { 773.717f,  44.091f, 46.780f };
        constexpr TkPos TELONICUS_WAITING_POSITION = { 754.347f,  31.739f, 46.796f };
        constexpr TkPos CAPERNIAN_WAITING_POSITION = { 743.897f, -11.575f, 46.779f };
        // A bit aways from the Sanguinar and Telonicus tanking positions.
        constexpr TkPos ADVISOR_HEAL_POSITION      = { 752.171f,  19.494f, 46.779f };
        // Towards the front-center of the platform to ensure an open area for Gravity Lapse.
        constexpr TkPos KAELTHAS_TANK_POSITION     = { 774.008f,  -0.631f, 48.729f };

        Unit* FindKaelthas(PlayerbotAI* ai);
        uint32 GetKaelthasTkPhase(PlayerbotAI* ai, Unit* kaelthas);
        // 10 second dps wait at the start of each phase 1 advisor
        void ResetAdvisorDpsTimer(uint32 instanceId);
        // Returns true when the timer was started now
        bool UpdateAdvisorDpsTimer(uint32 instanceId, bool advisorAtFullHp);
        bool IsAdvisorDpsWaitOver(uint32 instanceId, uint32 waitMs);
        bool IsAdvisorActive(Unit* advisor);
        // The advisor was revived and is still talking (not selectable) in phase 3
        bool IsAdvisorReviving(Unit* advisor);
        Player* GetCapernianTank(Player* bot);
        bool IsCapernianTank(Player* bot);
        bool IsSanguinarDebuffHunter(Player* bot);
        std::list<ObjectGuid> FindLegendaryWeaponGuids(Player* bot);
        Unit* GetLegendaryWeapon(PlayerbotAI* ai, uint32 weaponEntry);
        Creature* GetDeadLegendaryWeapon(PlayerbotAI* ai, uint32 weaponEntry);
        bool HasDeadLegendaryWeapon(PlayerbotAI* ai);
        bool IsLegendaryWeaponItem(uint32 itemId);
        // Item in the bags that can be equipped in the slot
        Item* GetEquippableItemForSlot(Player* bot, uint8 slot);
        Item* GetEquippedItemInSlot(Player* bot, uint8 slot, uint32 itemId);
        Creature* GetNearestFlameStrikeInRadius(Player* bot, float radius);
        Creature* GetPhoenixEgg(Player* bot);

        // Resets every remembered encounter state of the instance
        bool ResetEncounterStates(uint32 instanceId);
    }
}
