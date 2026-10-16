// Ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara/KaraHelpers) and adapted for this core.
#pragma once

#include "playerbot/WorldPosition.h"
#include <array>
#include <ctime>
#include <vector>

class Player;
class Unit;
class PlayerbotAI;

namespace ai
{
    namespace karazhan
    {
        constexpr uint32 KARAZHAN_MAP_ID = 532;

        // Entries checked against the ScriptDev2 scripts (eastern_kingdoms/karazhan)
        enum KarazhanNpcs : uint32
        {
            // Trash
            NPC_MANA_WARP                   = 16530,
            NPC_SPECTRAL_CHARGER            = 15547,

            // Attumen the Huntsman (boss_midnight.cpp). In this core Midnight and the unmounted
            // Attumen despawn when the mounted Attumen is summoned.
            NPC_ATTUMEN                     = 15550,
            NPC_MIDNIGHT                    = 16151,
            NPC_ATTUMEN_MOUNTED             = 16152,

            // Moroes
            NPC_MOROES                      = 15687,
            NPC_LADY_KEIRA_BERRYBUCK        = 17007,
            NPC_LADY_CATRIONA_VON_INDI      = 19872,
            NPC_LORD_CRISPIN_FERENCE        = 19873,
            NPC_BARON_RAFE_DREUGER          = 19874,
            NPC_BARONESS_DOROTHEA_MILLSTIPE = 19875,
            NPC_LORD_ROBIN_DARIS            = 19876,

            // Maiden of Virtue
            NPC_MAIDEN_OF_VIRTUE            = 16457,

            // Opera
            NPC_BIG_BAD_WOLF                = 17521,
            NPC_ROMULO                      = 17533,
            NPC_JULIANNE                    = 17534,
            NPC_DOROTHEE                    = 17535,
            NPC_TITO                        = 17548,
            NPC_ROAR                        = 17546,
            NPC_STRAWMAN                    = 17543,
            NPC_TINHEAD                     = 17547,
            NPC_CRONE                       = 18168,

            // The Curator
            NPC_CURATOR                     = 15691,
            NPC_ASTRAL_FLARE_1              = 17096,
            NPC_ASTRAL_FLARE_2              = 19781,
            NPC_ASTRAL_FLARE_3              = 19782,
            NPC_ASTRAL_FLARE_4              = 19783,

            // Terestian Illhoof
            NPC_TERESTIAN_ILLHOOF           = 15688,
            NPC_DEMON_CHAINS                = 17248,
            NPC_KILREK                      = 17229,
            NPC_FIENDISH_IMP                = 17267,

            // Shade of Aran
            NPC_SHADE_OF_ARAN               = 16524,
            NPC_CONJURED_ELEMENTAL          = 17167,

            // Netherspite
            NPC_NETHERSPITE                 = 15689,
            NPC_VOID_ZONE                   = 16697,
            NPC_GREEN_PORTAL                = 17367,    // Serenity <Healing Portal>
            NPC_BLUE_PORTAL                 = 17368,    // Dominance <Damage Portal>
            NPC_RED_PORTAL                  = 17369,    // Perseverance <Tanking Portal>

            // Prince Malchezaar
            NPC_PRINCE_MALCHEZAAR           = 15690,
            NPC_NETHERSPITE_INFERNAL        = 17646,

            // Nightbane
            NPC_NIGHTBANE                   = 17225,
            NPC_RESTLESS_SKELETON           = 17261,
        };

        enum KarazhanSpells : uint32
        {
            // Maiden of Virtue
            SPELL_REPENTANCE                = 29511,

            // Opera - Little Red Riding Hood (30768 triggers the fixate 30753 and the transform 30756)
            SPELL_LITTLE_RED_RIDING_HOOD    = 30768,
            SPELL_RED_RIDING_HOOD_FIXATE    = 30753,
            SPELL_RED_RIDING_HOOD_TRANSFORM = 30756,

            // The Curator
            SPELL_CURATOR_EVOCATION         = 30254,

            // Shade of Aran
            SPELL_FLAME_WREATH_CAST         = 30004,
            SPELL_FLAME_WREATH_AURA         = 29946,
            SPELL_BLIZZARD                  = 29951,
            SPELL_ARCANE_EXPLOSION          = 29973,

            // Netherspite
            SPELL_RED_BEAM_DEBUFF           = 30421,    // Nether Portal - Perseverance (player aura)
            SPELL_GREEN_BEAM_DEBUFF         = 30422,    // Nether Portal - Serenity (player aura)
            SPELL_BLUE_BEAM_DEBUFF          = 30423,    // Nether Portal - Dominance (player aura)
            SPELL_NETHER_EXHAUSTION_RED     = 38637,
            SPELL_NETHER_EXHAUSTION_GREEN   = 38638,
            SPELL_NETHER_EXHAUSTION_BLUE    = 38639,
            SPELL_NETHERSPITE_BANISH_VISUAL = 38542,    // banish phase aura of the ScriptDev2 script
            SPELL_NETHERSPITE_BANISHED      = 39833,    // Vortex Shade Black (AzerothCore)

            // Prince Malchezaar
            SPELL_ENFEEBLE                  = 30843,

            // Nightbane
            SPELL_CHARRED_EARTH             = 30129,
            SPELL_RAIN_OF_BONES             = 37098,    // cast by Nightbane in this core
            SPELL_RAIN_OF_BONES_AURA        = 37091,
        };

        struct KaraPosition
        {
            float x;
            float y;
            float z;
        };

        // General
        bool IsSafePosition(float x, float y, const std::vector<Unit*>& hazards, float hazardRadius);
        // Ground height at (x, y) reachable from the bot (no wall in between, no big height difference)
        bool GetValidGroundPoint(Player* bot, float x, float y, float& z);
        // Stop casting (spells and channels, auto attack keeps running)
        void StopCasting(Player* bot);
        // Alive creature of an entry near the bot (from the cached "karazhan nearby creatures" value)
        Unit* GetKaraUnit(PlayerbotAI* ai, uint32 entry);
        std::vector<Unit*> GetKaraUnits(PlayerbotAI* ai, uint32 entry);
        bool IsBotCurrentTarget(PlayerbotAI* ai, Unit* unit);
        // Creatures the "karazhan nearby creatures" value looks for
        bool IsTrackedEntry(uint32 entry);

        // Encounter timers and trackers shared by the bots of one instance
        enum class KaraTimer : uint8
        {
            ATTUMEN_DPS_WAIT,
            NETHERSPITE_DPS_WAIT,
            NIGHTBANE_DPS_WAIT,
            NIGHTBANE_FLIGHT_START,
        };
        bool GetTimer(uint32 instanceId, KaraTimer timer, time_t& start);
        bool StartTimer(uint32 instanceId, KaraTimer timer, time_t now);   // true if the timer was not running
        bool ClearTimer(uint32 instanceId, KaraTimer timer);               // true if the timer was running
        // Seconds since the timer started, -1 if it is not running
        int64 GetTimerElapsed(uint32 instanceId, KaraTimer timer);

        // Attumen the Huntsman
        constexpr KaraPosition ATTUMEN_TANK_POSITION = { -11123.762f, -1926.619f, 49.215f };
        Unit* GetAttumenMounted(PlayerbotAI* ai);
        bool IsAttumenEncounterActive(PlayerbotAI* ai);

        // Maiden of Virtue
        constexpr KaraPosition MAIDEN_OF_VIRTUE_TANK_POSITION = { -10945.881f, -2103.782f, 92.712f };
        constexpr std::array<KaraPosition, 8> MAIDEN_OF_VIRTUE_RANGED_POSITIONS = { {
            { -10959.242f, -2119.617f, 92.180f },   // SE (opposite entrance)
            { -10944.495f, -2123.857f, 92.180f },   // E
            { -10966.017f, -2105.288f, 92.175f },   // S
            { -10931.178f, -2116.580f, 92.179f },   // NE
            { -10960.912f, -2090.437f, 92.179f },   // SW
            { -10925.828f, -2102.425f, 92.180f },   // N
            { -10947.590f, -2082.815f, 92.180f },   // W
            { -10933.089f, -2088.502f, 92.180f },   // NW (entrance)
        } };

        // The Big Bad Wolf
        constexpr KaraPosition BIG_BAD_WOLF_TANK_POSITION = { -10913.391f, -1773.508f, 90.477f };
        constexpr std::array<KaraPosition, 4> BIG_BAD_WOLF_RUN_POSITIONS = { {
            { -10875.456f, -1779.036f, 90.477f },
            { -10872.281f, -1751.638f, 90.477f },
            { -10910.492f, -1747.401f, 90.477f },
            { -10913.391f, -1773.508f, 90.477f },
        } };
        bool HasLittleRedRidingHood(Player* bot);

        // Wizard of Oz (kill order)
        constexpr std::array<uint32, 5> OZ_TARGETS = { NPC_DOROTHEE, NPC_TITO, NPC_ROAR, NPC_STRAWMAN, NPC_TINHEAD };

        // The Curator
        constexpr KaraPosition THE_CURATOR_TANK_POSITION = { -11139.463f, -1884.645f, 165.765f };
        Unit* GetAstralFlare(PlayerbotAI* ai);

        // Shade of Aran
        bool IsAranCastingArcaneExplosion(Unit* aran);
        bool IsFlameWreathActive(PlayerbotAI* ai);

        // Netherspite
        bool IsBanishPhase(Unit* netherspite);
        struct BeamBlockers
        {
            Player* red = nullptr;
            Player* green = nullptr;
            Player* blue = nullptr;
        };
        BeamBlockers GetCurrentBeamBlockers(Player* bot);
        void ClearBeamBlockers(uint32 instanceId, bool& changed);
        std::vector<Unit*> GetAllVoidZones(PlayerbotAI* ai);
        bool FindBeamPosition(Unit* netherspite, Unit* portal, const std::vector<Unit*>& voidZones,
                              float idealDistance, float& outX, float& outY);

        // Prince Malchezaar
        std::vector<Unit*> GetSpawnedInfernals(PlayerbotAI* ai);
        bool IsStraightPathSafe(float startX, float startY, float targetX, float targetY,
                                const std::vector<Unit*>& hazards, float hazardRadius);
        bool TryFindSafePositionWithSafePath(Player* bot, float originX, float originY, float centerX, float centerY,
                                             const std::vector<Unit*>& hazards, float safeDistance, float maxSampleDist,
                                             float& outX, float& outY);

        // Nightbane
        constexpr float NIGHTBANE_FLIGHT_Z = 95.000f;
        constexpr float NIGHTBANE_GROUND_Z = 91.473f;
        constexpr uint8 NIGHTBANE_FLIGHT_PHASE_SECONDS = 35;
        constexpr KaraPosition TERRACE_DOME_CENTER = { -11126.015f, -1925.271f, 91.473f };
        constexpr KaraPosition TERRACE_EAST_END    = { -11115.958f, -1972.058f, 91.457f };
        constexpr KaraPosition TERRACE_WEST_END    = { -11077.521f, -1913.315f, 91.471f };
        constexpr std::array<KaraPosition, 2> NIGHTBANE_FLIGHT_STACK_POSITIONS = { {
            { -11156.233f, -1888.353f, 91.473f },   // primary
            { -11149.115f, -1897.154f, 91.473f },   // backup in case of charred earth
        } };
        constexpr std::array<KaraPosition, 2> NIGHTBANE_RAIN_OF_BONES_POSITIONS = { {
            { -11166.516f, -1901.405f, 91.473f },   // primary
            { -11158.752f, -1909.394f, 91.473f },   // backup in case of charred earth
        } };
        constexpr KaraPosition NIGHTBANE_TELEPORT_POSITION = { -11159.555f, -1893.526f, 91.473f };
        bool IsNightbaneFlying(Unit* nightbane);
        bool IsNightbaneFlightPhaseActive(Unit* nightbane);
        // Fear casters (Tremor Totem): grounded Nightbane, Spectral Chargers, the Big Bad Wolf, Roar
        bool AreFearCastersPresent(PlayerbotAI* ai);
    }
}
