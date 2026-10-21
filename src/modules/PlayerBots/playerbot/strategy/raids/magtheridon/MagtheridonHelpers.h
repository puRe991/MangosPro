// Ported from mod-playerbots (src/Ai/Raid/Mag, GPL v2+) and adapted for this core.
#pragma once

#include "playerbot/WorldPosition.h"
#include <list>
#include <vector>

class Creature;
class GameObject;
class Player;
class PlayerbotAI;
class Unit;

namespace ai
{
    namespace magtheridon
    {
        enum MagtheridonIds : uint32
        {
            // Npcs / objects (magtheridons_lair.h)
            NPC_MAGTHERIDON             = 17257,
            NPC_HELLFIRE_CHANNELER      = 17256,
            NPC_BURNING_ABYSSAL         = 17454,
            GO_MANTICRON_CUBE           = 181713,
            GO_BLAZE                    = 181832,

            // Magtheridon (boss_magtheridon.cpp)
            SPELL_SHADOW_CAGE_DUMMY     = 30205,    // still banished by the channelers
            SPELL_SHADOW_CAGE           = 30168,    // banished by the 5 cubes
            SPELL_SHADOW_GRASP          = 30410,    // channeled by a cube clicker
            SPELL_MIND_EXHAUSTION       = 44032,    // a clicker cannot use a cube again for a while
            SPELL_BLAST_NOVA            = 30616,
            SPELL_QUAKE                 = 30657,
            // This core: Debris 30630 summons a trigger that casts the 30632 ground marker, the
            // damage (30631) hits when the marker ends. mod-playerbots looks for 30630 objects.
            SPELL_DEBRIS_MARKER         = 30632,

            // Hunter
            SPELL_MISDIRECTION          = 35079,
        };

        constexpr uint32 MAG_MAP_ID = 544;

        // Center of the room (leash check of the boss script), used to tell the channelers apart.
        // This core has other database guids than AzerothCore, so the channelers are identified by the
        // direction of their spawn point from the center (+x = north, +y = west).
        constexpr float ROOM_CENTER_X = -16.683f;
        constexpr float ROOM_CENTER_Y = 2.34519f;

        enum ChannelerSlot : uint8
        {
            CHANNELER_SOUTH     = 0,
            CHANNELER_WEST      = 1,
            CHANNELER_EAST      = 2,
            CHANNELER_NORTHWEST = 3,
            CHANNELER_NORTHEAST = 4,
        };

        // Hold dps for this long after Magtheridon first becomes attackable.
        constexpr uint32 MAG_DPS_HOLD_MS = 8000;
        // Time since the previous Blast Nova (or since Magtheridon joined the fight) after which the
        // cube clickers go wait by their cube. Blast Nova repeats every ~55s, delayed by every Quake.
        constexpr uint32 BLAST_NOVA_INTERIM_MS = 30000;
        constexpr float DEBRIS_HAZARD_RADIUS = 10.0f;
        constexpr float CONFLAGRATION_HAZARD_RADIUS = 5.0f;
        // Quake delays Blast Nova by (essentially) 7s (the boss script stuns him for 7s)
        constexpr uint32 QUAKE_DELAY_MS = 6999;
        // At 30% health the ceiling collapses (phase 3), which delays Quake and Blast Nova by 18s.
        constexpr float CEILING_COLLAPSE_HP_PCT = 30.0f;
        constexpr uint32 CEILING_COLLAPSE_DELAY_MS = 18000;

        // The waiting position is SE-ish of Magtheridon.
        const WorldPosition WAITING_FOR_MAGTHERIDON_POSITION(MAG_MAP_ID, -31.962f, -8.514f, -0.304f, 0.657f);
        // The tank position is up against the eastern wall.
        const WorldPosition MAGTHERIDON_TANK_POSITION(MAG_MAP_ID, -6.147f, -37.812f, -0.411f);
        const WorldPosition NW_CHANNELER_TANK_POSITION(MAG_MAP_ID, -11.764f, 30.818f, -0.411f);
        const WorldPosition NE_CHANNELER_TANK_POSITION(MAG_MAP_ID, -12.490f, -26.211f, -0.411f);

        struct CubeInfo
        {
            ObjectGuid guid;
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
        };

        Unit* FindMagtheridon(PlayerbotAI* ai);
        Unit* GetCombatTarget(PlayerbotAI* ai);
        bool CanUseCubeMovement(PlayerbotAI* ai, const std::string& action);
        bool IsMagtheridonActive(Unit* magtheridon);
        bool IsBlastNovaCasting(Unit* magtheridon);

        // Channelers
        ChannelerSlot GetChannelerSlot(Creature* channeler);
        std::list<ObjectGuid> FindChannelerGuids(Player* bot);
        Unit* GetChanneler(PlayerbotAI* ai, ChannelerSlot slot);
        Unit* GetChannelerInterruptTarget(PlayerbotAI* ai, std::string& spell);
        Unit* GetAssignedAbyssalCcTarget(PlayerbotAI* ai, std::string& spell);
        bool NeedsCubeAction(PlayerbotAI* ai);

        // Burning Abyssals, ordered by guid so every warlock indexes the same abyssal
        std::list<ObjectGuid> FindBurningAbyssalGuids(Player* bot);
        std::vector<Unit*> GetBurningAbyssals(PlayerbotAI* ai);

        // Living bots of a class in the map, in group order
        std::vector<Player*> GetGroupBotsOfClass(Player* bot, uint8 classId);

        // Manticron Cubes ordered by guid
        std::vector<CubeInfo> FindCubes(Player* bot);

        // Hazards after the ceiling collapse: falling Debris and the Blaze fires (Conflagration)
        std::vector<WorldPosition> FindDebrisPositions(Player* bot);
        bool IsPositionInDebris(const std::vector<WorldPosition>& debris, float x, float y, float radius = DEBRIS_HAZARD_RADIUS);
        bool IsPositionInActiveDebris(Player* bot, float x, float y, float radius = DEBRIS_HAZARD_RADIUS);
        std::vector<GameObject*> GetActiveConflagrations(Player* bot);
        bool IsPositionInConflagration(const std::vector<GameObject*>& blazes, float x, float y);
        bool IsPositionInActiveConflagration(Player* bot, float x, float y);

        // Point at safeDistance from (centerX, centerY), as straight away from it as the terrain allows
        bool GetPointAwayFrom(Player* bot, float centerX, float centerY, float safeDistance, float& x, float& y, float& z);

        // Encounter state shared by the bots of an instance (timers and cube assignment). Only the
        // mechanic tracker bot updates it. The bots of different instances run in different map
        // threads, so the state is guarded by a mutex.
        bool GetBlastNovaTime(uint32 instanceId, uint32& time);
        bool GetDpsWaitTime(uint32 instanceId, uint32& time);
        bool IsCeilingCollapsed(Player* bot);
        bool IsCubeClicker(Player* bot);
        bool GetAssignedCube(Player* bot, CubeInfo& cube);
        // True when the Blast Nova timer says the clickers must wait by their cube
        bool IsBlastNovaSoon(Player* bot);
        bool ResetEncounterState(uint32 instanceId);
        bool UpdateEncounterState(Player* tracker, Unit* magtheridon);
    }
}
