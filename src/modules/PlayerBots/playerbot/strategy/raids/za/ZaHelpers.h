// Ported from mod-playerbots (AzerothCore, GPL v2+), src/Ai/Raid/ZA/ZAHelpers.h, and adapted for this core.
#pragma once

#include "playerbot/WorldPosition.h"
#include <array>
#include <list>
#include <utility>
#include <vector>

class Player;
class Unit;
class GameObject;
class PlayerbotAI;

namespace ai
{
    class Action;

    // Zul'Aman (map 568). Ids checked against src/game/AI/ScriptDevAI/scripts/eastern_kingdoms/zulaman
    namespace za
    {
        enum ZaSpells : uint32
        {
            // Akil'zon <Eagle Avatar>
            SPELL_ELECTRICAL_STORM          = 43648,

            // Nalorakk <Bear Avatar>
            SPELL_BEARFORM                  = 42377,

            // Jan'alai <Dragonhawk Avatar>
            SPELL_FIRE_BOMB_CHANNEL         = 42621,

            // Hex Lord Malacrass (abilities siphoned from the raid)
            SPELL_HEX_LORD_WHIRLWIND        = 43442,
            SPELL_HEX_LORD_SPELL_REFLECTION = 43443,
            SPELL_UNSTABLE_AFFLICTION       = 35183,    // our script (mod-playerbots: 43522)
            SPELL_UNSTABLE_AFFLICTION_ALT   = 43522,

            // Zul'jin
            SPELL_ZULJIN_WHIRLWIND          = 17207,
            SPELL_SHAPE_OF_THE_BEAR         = 42594,
            SPELL_SHAPE_OF_THE_EAGLE        = 42606,
            SPELL_SHAPE_OF_THE_LYNX         = 42607,
            SPELL_SHAPE_OF_THE_DRAGONHAWK   = 42608,
            SPELL_CREEPING_PARALYSIS        = 43095,

            // Hunter
            SPELL_MISDIRECTION              = 35079,    // aura on the hunter after casting Misdirection (34477)

            // Priest
            SPELL_MASS_DISPEL               = 32375,
        };

        enum ZaNpcs : uint32
        {
            // Bosses
            NPC_AKILZON                     = 23574,
            NPC_NALORAKK                    = 23576,
            NPC_JANALAI                     = 23578,
            NPC_HALAZZI                     = 23577,
            NPC_HEX_LORD_MALACRASS          = 24239,
            NPC_ZULJIN                      = 23863,

            // Trash
            NPC_AMANISHI_MEDICINE_MAN       = 23581,
            NPC_AMANI_HEALING_WARD          = 23757,
            NPC_AMANI_PROTECTIVE_WARD       = 23822,

            // Jan'alai: our script summons one hatcher per side with its own entry
            NPC_AMANI_DRAGONHAWK_HATCHLING  = 23598,
            NPC_AMANISHI_HATCHER_1          = 23818,    // right side
            NPC_AMANISHI_HATCHER_2          = 24504,    // left side
            NPC_FIRE_BOMB                   = 23920,

            // Halazzi
            NPC_SPIRIT_OF_THE_LYNX          = 24143,
            NPC_CORRUPTED_LIGHTNING_TOTEM   = 24224,

            // Hex Lord Malacrass companions
            NPC_ALYSON_ANTILLE              = 24240,
            NPC_THURG                       = 24241,
            NPC_SLITHER                     = 24242,
            NPC_LORD_RAADAN                 = 24243,
            NPC_GAZAKROTH                   = 24244,
            NPC_FENSTALKER                  = 24245,
            NPC_DARKHEART                   = 24246,
            NPC_KORAGG                      = 24247,
        };

        enum ZaObjects : uint32
        {
            GO_FREEZING_TRAP                = 186669,   // summoned by Malacrass' Freezing Trap (43447)
        };

        // General

        constexpr uint32 ZA_MAP_ID = 568;
        // For Hex Lord and Zul'jin. Whirlwind radius is 8y. Safe distance has 4y of padding;
        // hold distance is for the don't run back in multiplier and adds another 3y of padding.
        constexpr float ZA_WHIRLWIND_SAFE_DISTANCE = 12.0f;
        constexpr float ZA_WHIRLWIND_HOLD_DISTANCE = 15.0f;
        // For Medicine Man wards, Halazzi totem, and Jan'alai Hatchers
        constexpr float ZA_CREATURE_SEARCH_RADIUS = 40.0f;

        // A boss of Zul'Aman is engaged by the bot's group
        bool IsEncounterInProgress(Player* bot);
        Unit* GetBoss(PlayerbotAI* ai, uint32 entry);
        // Taunt a creature with the class taunt (Righteous Defense on the victim of the creature)
        bool CastTaunt(PlayerbotAI* ai, Unit* target);

        // Akil'zon <Eagle Avatar>

        // mod-playerbots assumes a fixed 60s cycle. Our script changes the weather after 42s, casts
        // Electrical Storm 8.5s later and then repeats the weather change every 54-60s. The bots
        // therefore track the pull and the start of every storm and react from AKILZON_STORM_LEAD_MS
        // before the earliest possible cast until the storm has been seen (or the window is over).
        constexpr uint32 AKILZON_FIRST_STORM_MS = 50500;
        constexpr uint32 AKILZON_NEXT_STORM_MIN_MS = 54000;
        constexpr uint32 AKILZON_NEXT_STORM_MAX_MS = 60000;
        constexpr uint32 AKILZON_STORM_LEAD_MS = 5000;
        constexpr uint32 AKILZON_STORM_DURATION_MS = 10000;

        // Centerish of the platform
        const WorldPosition AKILZON_TANK_POSITION(ZA_MAP_ID, 378.369f, 1407.718f, 74.797f);

        // engageMs / lastStormMs come from the "akil'zon electrical storm timer" values (0 = unknown)
        bool IsInStormWindow(Player* bot, uint32 engageMs, uint32 lastStormMs);
        bool IsInStormWindow(PlayerbotAI* ai);
        Player* GetElectricalStormTarget(Player* bot);

        // Nalorakk <Bear Avatar>

        // Toward the back, down the stairs from his starting position
        const WorldPosition NALORAKK_TANK_POSITION(ZA_MAP_ID, -80.208f, 1324.530f, 40.942f);

        bool IsNalorakkInBearForm(Unit* nalorakk);
        // Main tank takes bear form, the first assist tank takes troll form
        Player* GetNalorakkFormTank(Player* bot, Unit* nalorakk);

        // Jan'alai <Dragonhawk Avatar>

        constexpr float JANALAI_PLATFORM_Z = 19.146f;
        // Hatchers open eggs at increasing speed. There are 20 eggs per side. Bloodlust is held
        // until this many Hatchlings are active.
        constexpr uint32 JANALAI_BLOODLUST_HATCHLING_COUNT = 6;
        // Ranged spread evenly around the tank position to reduce players hit by Flame Breath.
        constexpr float JANALAI_RANGED_SPREAD_RADIUS = 15.0f;
        // How far bots search for a safe spot from Fire Bombs.
        constexpr float JANALAI_FIRE_BOMB_MAX_SEARCH_DISTANCE = 30.0f;
        // How far bots search for the Fire Bombs themselves.
        constexpr float JANALAI_FIRE_BOMB_SEARCH_RADIUS = 40.0f;
        // Fire Bomb (42630) has a 4y radius, 1.5y padding.
        constexpr float JANALAI_FIRE_BOMB_SAFE_DISTANCE = 5.5f;
        // Jan'alai hatches every remaining egg at once at 35% HP (33% accounts for the delay).
        constexpr float JANALAI_HATCH_ALL_HEALTH_PCT = 33.0f;

        // Centerish of the platform
        const WorldPosition JANALAI_TANK_POSITION(ZA_MAP_ID, -33.873f, 1149.571f, JANALAI_PLATFORM_Z);
        // The reachable platform without the fenced-off corner and the bridges to the eggs. The
        // Fire Wall damage comes from Fire Bomb npcs too, so avoiding the bombs keeps away from it.
        const std::vector<WorldPosition> JANALAI_SAFE_ZONE = {
            WorldPosition(ZA_MAP_ID, -12.576924f, 1131.9163f, JANALAI_PLATFORM_Z),
            WorldPosition(ZA_MAP_ID, -52.068108f, 1132.8198f, JANALAI_PLATFORM_Z),
            WorldPosition(ZA_MAP_ID, -51.508590f, 1158.4890f, JANALAI_PLATFORM_Z),
            WorldPosition(ZA_MAP_ID, -44.008713f, 1168.6125f, JANALAI_PLATFORM_Z),
            WorldPosition(ZA_MAP_ID, -12.115170f, 1167.6681f, JANALAI_PLATFORM_Z)
        };

        // Both Amani'shi Hatchers (first: right side 23818, second: left side 24504)
        std::pair<Unit*, Unit*> GetAmanishiHatcherPair(PlayerbotAI* ai);
        uint32 CountJanalaiHatchlings(PlayerbotAI* ai);
        // Jan'alai has the Fire Bomb Channel aura from the start of each Fire Bomb event until the
        // bombs detonate (the bomb npcs stay a few seconds longer).
        bool IsJanalaiBombing(Unit* janalai);
        std::list<ObjectGuid> FindNearbyFireBombGuids(Player* bot);
        std::vector<Unit*> GetNearbyFireBombs(PlayerbotAI* ai);
        bool FindSafeStepInJanalaiZone(Player* bot, const std::vector<Unit*>& hazards, const std::vector<WorldPosition>& safeZone,
                                       float maxSearchDistance, float hazardRadius, float moveDist,
                                       float& stepX, float& stepY, float& stepZ);

        // Halazzi <Lynx Avatar>

        // Right near his starting position
        const WorldPosition HALAZZI_TANK_POSITION(ZA_MAP_ID, 370.733f, 1131.202f, 6.516f);

        // Hex Lord Malacrass

        // Freezing Trap (43448) stuns for 10s across a 10y radius, padded by 1y.
        constexpr float ZA_FREEZING_TRAP_SAFE_DISTANCE = 11.0f;
        constexpr float ZA_FREEZING_TRAP_SEARCH_RADIUS = 16.0f;

        ObjectGuid FindNearbyFreezingTrapGuid(Player* bot);
        GameObject* GetNearbyFreezingTrap(PlayerbotAI* ai);
        bool HasUnstableAffliction(Unit* unit);

        // Zul'jin

        // 4 Feather Vortexes chase random raid members at run speed with a 4y radius. They can't
        // really be dodged, so ranged spread out for the Cyclones to travel farther.
        constexpr float ZULJIN_SPREAD_Z = 45.111f;

        // Centerish of the platform
        const WorldPosition ZULJIN_TANK_POSITION(ZA_MAP_ID, 120.210f, 705.564f, ZULJIN_SPREAD_Z);
        const std::array<WorldPosition, 8> ZULJIN_SPREAD_POSITIONS = {{
            // Healer slots
            WorldPosition(ZA_MAP_ID, 120.462f, 728.502f, ZULJIN_SPREAD_Z),
            WorldPosition(ZA_MAP_ID, 119.984f, 693.188f, ZULJIN_SPREAD_Z),
            // Ranged dps slots
            WorldPosition(ZA_MAP_ID, 94.939f, 713.698f, ZULJIN_SPREAD_Z),
            WorldPosition(ZA_MAP_ID, 145.286f, 713.034f, ZULJIN_SPREAD_Z),
            WorldPosition(ZA_MAP_ID, 103.869f, 728.703f, ZULJIN_SPREAD_Z),
            WorldPosition(ZA_MAP_ID, 135.847f, 728.816f, ZULJIN_SPREAD_Z),
            WorldPosition(ZA_MAP_ID, 143.862f, 697.302f, ZULJIN_SPREAD_Z),
            WorldPosition(ZA_MAP_ID, 95.618f, 698.439f, ZULJIN_SPREAD_Z)
        }};

        bool IsZuljinInAnimalForm(Unit* zuljin);
        // Ranged are ordered healers first. The first two positions are at the bottom and the top
        // of the stairs, no two spots are more than 39y apart so healers reach everyone.
        bool GetZuljinSpreadSlotIndex(Player* bot, size_t slotCount, size_t& slotIndex);
        // Closest raid member with Creeping Paralysis (Mass Dispel target)
        Player* GetZuljinCreepingParalysisDispelTarget(Player* bot);

        // Action classification used by the multipliers (by action name for this core)
        bool IsApproachMovement(Action* action);
        bool IsHazardousMovement(Action* action);
    }
}
