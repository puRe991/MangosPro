// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#pragma once

#include "playerbot/WorldPosition.h"
#include <list>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace ai
{
    namespace gruul
    {
        enum GruulIds : uint32
        {
            // Npcs (boss_high_king_maulgar.cpp / boss_gruul.cpp)
            NPC_HIGH_KING_MAULGAR       = 18831,
            NPC_KROSH_FIREHAND          = 18832,
            NPC_OLM_THE_SUMMONER        = 18834,
            NPC_KIGGLER_THE_CRAZED      = 18835,
            NPC_BLINDEYE_THE_SEER       = 18836,
            NPC_WILD_FEL_STALKER        = 18847,    // summoned by Olm (33131)
            NPC_GRUUL_THE_DRAGONKILLER  = 19044,

            // High King Maulgar
            SPELL_WHIRLWIND             = 33238,
            // Krosh Firehand
            SPELL_SPELL_SHIELD          = 33054,
            // Hunter
            SPELL_MISDIRECTION          = 35079,

            // Gruul the Dragonkiller
            SPELL_GROUND_SLAM_1         = 33525,
            SPELL_GROUND_SLAM_2         = 39187,    // triggered by 33525
            SPELL_GRONN_LORDS_GRASP     = 33572,
            SPELL_STONED               = 33652,
            SPELL_CAVE_IN               = 36240,
        };

        constexpr uint32 GRUUL_MAP_ID = 565;

        // High King Maulgar

        // Hold cooldowns until Blindeye is at this percent health. Also the gate for on-pull Misdirects.
        constexpr float BLINDEYE_ENGAGED_HEALTH_PCT = 75.0f;
        // Radius is 30y with 2y of padding: the moonkin can still attack Kiggler from outside of
        // Kiggler's Arcane Explosion.
        constexpr float KIGGLER_ARCANE_EXPLOSION_SAFE_DISTANCE = 32.0f;
        // Radius is 15y with 2y of padding.
        constexpr float KROSH_BLAST_WAVE_SAFE_DISTANCE = 17.0f;
        // Radius is 8y, padded more because Maulgar can end up turned away from the wall.
        constexpr float MAULGAR_WHIRLWIND_SAFE_DISTANCE = 12.0f;
        // Distance for the multiplier, a little more than the escape distance.
        constexpr float MAULGAR_WHIRLWIND_HOLD_DISTANCE = 15.0f;
        constexpr float WILD_FEL_STALKER_SEARCH_RADIUS = 50.0f;

        const WorldPosition MAULGAR_TANK_POSITION(GRUUL_MAP_ID, 90.686f, 167.047f, -13.234f);
        const WorldPosition OLM_TANK_POSITION(GRUUL_MAP_ID, 101.050f, 219.359f, -9.503f);
        const WorldPosition BLINDEYE_TANK_POSITION(GRUUL_MAP_ID, 99.681f, 213.989f, -10.345f);
        const WorldPosition KROSH_TANK_POSITION(GRUUL_MAP_ID, 116.880f, 166.208f, -14.231f);
        const WorldPosition GRUUL_TANK_POSITION(GRUUL_MAP_ID, 241.238f, 365.025f, -4.220f);

        Unit* FindMaulgar(PlayerbotAI* ai);
        Unit* FindKrosh(PlayerbotAI* ai);
        Unit* FindOlm(PlayerbotAI* ai);
        Unit* FindKiggler(PlayerbotAI* ai);
        Unit* FindBlindeye(PlayerbotAI* ai);
        Unit* FindGruul(PlayerbotAI* ai);
        bool IsGruulsLairEncounterInProgress(Player* bot);
        bool IsCouncilMember(Unit* unit);
        Unit* GetCasterRoleTarget(PlayerbotAI* ai);
        Unit* GetPrepareTarget(PlayerbotAI* ai, float& range);
        bool IsPrepared(Player* bot, Unit* target, float range);
        bool GetPreparePosition(PlayerbotAI* ai, Unit* target, float range, float& x, float& y, float& z);

        bool IsMaulgarTank(Player* bot);
        bool IsOlmTank(Player* bot);
        bool IsBlindeyeTank(Player* bot);
        ObjectGuid FindKroshMageTankGuid(Player* bot);
        Player* GetKroshMageTank(PlayerbotAI* ai);
        bool IsKroshMageTank(PlayerbotAI* ai);
        ObjectGuid FindKigglerMoonkinTankGuid(Player* bot);
        Player* GetKigglerMoonkinTank(PlayerbotAI* ai);
        bool IsKigglerMoonkinTank(PlayerbotAI* ai);
        std::list<ObjectGuid> FindNearbyWildFelStalkerGuids(Player* bot);
        std::vector<Unit*> GetNearbyWildFelStalkers(PlayerbotAI* ai);
        // Living bots of a class in the map, in group order (each bot gets its own job)
        std::vector<Player*> GetGroupBotsOfClass(Player* bot, uint8 classId);

        // Gruul the Dragonkiller

        // Radius is 8y, plus the player's combat reach, rounded up. The pool lasts 15s and is cast
        // as often as every 3.5s, so several can be on the ground at once.
        constexpr float CAVE_IN_RADIUS = 10.0f;
        // Distance for the multiplier, wider than the avoidance to prevent running back in.
        constexpr float CAVE_IN_CONTROL_RADIUS = CAVE_IN_RADIUS + 5.0f;
        constexpr float CAVE_IN_SEARCH_RADIUS = CAVE_IN_CONTROL_RADIUS + 2.0f;
        // Shatter radius is 20y (damage falls off with distance), 2y of padding.
        constexpr float GRUUL_SHATTER_SAFE_DISTANCE = 22.0f;

        // Cave In pools are read directly (no cached value): the pools appear every few seconds
        bool GetNearestCaveInPosition(Player* bot, WorldPosition& pool);
        bool IsNearCaveIn(Player* bot, float radius);
        bool IsInCaveIn(Player* bot);
        bool HasGroundSlam(Player* bot);
        bool GetShatterSpreadPosition(Player* bot, float& x, float& y, float& z);

        // Point at safeDistance from (centerX, centerY), as straight away from it as the terrain allows
        // (replaces MoveAway / FleePosition of mod-playerbots). False if no reachable point was found.
        bool GetPointAwayFrom(Player* bot, float centerX, float centerY, float safeDistance, float& x, float& y, float& z);
    }
}
