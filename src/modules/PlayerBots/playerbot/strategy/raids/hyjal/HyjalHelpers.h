// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/Hyjal/Util/HyjalHelpers) and adapted for this core.
#pragma once

#include "playerbot/WorldPosition.h"
#include <functional>
#include <list>
#include <string>
#include <vector>

class Player;
class Unit;
class PlayerbotAI;

namespace ai
{
    class Action;

    namespace hyjal
    {
        // Creature entries (ScriptDev2 hyjal.h / boss scripts)
        enum HyjalNpcs : uint32
        {
            NPC_RAGE_WINTERCHILL  = 17767,
            NPC_ANETHERON         = 17808,
            NPC_KAZROGAL          = 17888,
            NPC_AZGALOR           = 17842,
            NPC_ARCHIMONDE        = 17968,

            NPC_TOWERING_INFERNAL = 17818,  // Anetheron - Inferno
            NPC_LESSER_DOOMGUARD  = 17864,  // Azgalor - Doom
            NPC_DOOMFIRE          = 18095,  // Archimonde - drops the Doomfire pools
        };

        enum HyjalSpells : uint32
        {
            // Rage Winterchill
            SPELL_DEATH_AND_DECAY     = 31258,

            // Anetheron
            SPELL_INFERNO             = 31299,

            // Kaz'rogal
            SPELL_MARK_OF_KAZROGAL    = 31447,

            // Azgalor
            SPELL_RAIN_OF_FIRE        = 31340,
            SPELL_DOOM                = 31347,

            // Archimonde
            SPELL_DOOMFIRE_TRAIL      = 31943,  // 6y persistent area aura dropped along the trail
            SPELL_DOOMFIRE            = 31944,  // damaging part of the trail
            SPELL_DOOMFIRE_DOT        = 31969,  // DoT after leaving the trail
            SPELL_AIR_BURST           = 32014,
            SPELL_PROTECTION_OF_ELUNE = 38528,

            // Hunter
            SPELL_MISDIRECTION_AURA   = 35079,
        };

        struct BlockedArc
        {
            float center;
            float halfWidth;
        };

        struct Point
        {
            float x;
            float y;
            float z;
        };

        // General

        constexpr uint32 HYJAL_MAP_ID = 534;
        // Hazard positions are cached this long (ms)
        constexpr uint32 HAZARD_CACHE_INTERVAL = 100;
        // Additional distance beyond the edge of a hazard in which the multipliers still suppress
        // other movement (Death and Decay, Rain of Fire)
        constexpr float HAZARD_CONTROL_MARGIN = 5.0f;
        // Additional distance beyond the control margin in which hazards are searched
        constexpr float HAZARD_SEARCH_MARGIN = 2.0f;
        // Subtracted from the edge of melee range so rounding and drift cannot leave a bot out of reach
        constexpr float MELEE_RANGE_INSET = 1.0f;

        // Boss of the current encounter (cached by the "hyjal boss" value), nullptr if the entry differs
        Unit* GetBoss(PlayerbotAI* ai, uint32 entry);
        Unit* GetAnyBoss(PlayerbotAI* ai);
        // Search for the engaged boss (used by the "hyjal boss" value)
        Unit* FindEngagedBoss(PlayerbotAI* ai);
        bool IsHyjalEncounterInProgress(Player* bot);

        Unit* GetCurrentTarget(PlayerbotAI* ai);
        float GetMeleeRange(Player* bot, Unit* target);
        WorldPosition ToPosition(const Point& point);

        bool GetMeleeHazardManeuverStep(Player* bot, Unit* boss, const std::vector<WorldPosition>& hazards, float hazardRadius,
                                        const std::vector<BlockedArc>& extraBlocked, float& stepX, float& stepY, float& stepZ,
                                        const std::function<bool(float, float)>& isAcceptable = {});
        // Ranged members of the group (bots only), in group order
        std::vector<Player*> GetRangedMembers(Player* bot);
        // Step towards the place of the bot on a ring (healers on the inner, ranged dps on the outer one)
        bool GetRangedRingStep(Player* bot, const WorldPosition& center, float healerRadius, float dpsRadius,
                               float& stepX, float& stepY, float& stepZ, bool& reached);
        // Step towards a point on a circle around center, at the angle nearest to the preferred one
        bool FindStepToCircle(Player* bot, const WorldPosition& center, float radius, float preferredAngle, float moveDist,
                              float& stepX, float& stepY, float& stepZ, const std::function<bool(float, float)>& isAcceptable,
                              float* chosenX, float* chosenY);

        // Rage Winterchill

        // 20y radius + the combat reach of the player
        constexpr float DEATH_AND_DECAY_RADIUS = 22.0f;
        constexpr float DEATH_AND_DECAY_CONTROL_RADIUS = DEATH_AND_DECAY_RADIUS + HAZARD_CONTROL_MARGIN;
        constexpr float DEATH_AND_DECAY_SEARCH_RADIUS = DEATH_AND_DECAY_CONTROL_RADIUS + HAZARD_SEARCH_MARGIN;

        // Back towards the center of the Alliance base
        constexpr Point WINTERCHILL_TANK_POSITION = { 5031.061f, -1784.521f, 1321.626f };

        bool GetDeathAndDecayPosition(PlayerbotAI* ai, WorldPosition& deathAndDecay);
        bool IsNearDeathAndDecay(PlayerbotAI* ai, float radius);
        bool IsInDeathAndDecay(PlayerbotAI* ai);

        // Anetheron

        constexpr float INFERNAL_SEARCH_RADIUS = 100.0f;
        constexpr uint32 INFERNAL_CACHE_INTERVAL = 200;
        // A landing Infernal stuns everybody within 10y and then burns everything within 10y (Immolation)
        constexpr float INFERNAL_DANGER_RADIUS = 10.0f;
        constexpr float INFERNAL_ESCAPE_DISTANCE = INFERNAL_DANGER_RADIUS + 2.0f;
        // Past this, ranged stay on the boss rather than switching to the Infernal
        constexpr float INFERNAL_RANGED_ENGAGE_DISTANCE = 50.0f;

        // Back towards the center of the Alliance base, near the crossroads
        constexpr Point ANETHERON_TANK_POSITION       = { 5033.177f, -1765.996f, 1324.195f };
        constexpr Point ANETHERON_E_INFERNAL_POSITION = { 5016.578f, -1800.233f, 1323.070f };
        constexpr Point ANETHERON_W_INFERNAL_POSITION = { 5048.911f, -1722.164f, 1321.408f };
        // Whichever of the two spots the Infernal tank stands nearer
        WorldPosition GetInfernalTankPosition(Player* bot);

        // Player Anetheron is casting Inferno on (the Infernal lands at the feet of that player)
        Player* GetInfernoTarget(Unit* anetheron);
        // Every living Towering Infernal, oldest first
        std::list<ObjectGuid> FindInfernalGuids(Player* bot);
        std::list<ObjectGuid> GetInfernalGuids(PlayerbotAI* ai);
        // The first Infernal the Infernal tank doesn't have aggro on
        Unit* GetLooseInfernal(PlayerbotAI* ai);
        Unit* GetNearestInfernal(PlayerbotAI* ai);
        // The Infernal a ranged bot should attack instead of the boss, if any
        Unit* GetInfernalToAttack(PlayerbotAI* ai, Unit* anetheron);
        Unit* GetInfernalTargetingBot(PlayerbotAI* ai);
        // The first assist tank picks up the Infernals
        bool IsInfernalTank(Player* bot);
        Player* GetInfernalTank(Player* bot);

        // Kaz'rogal

        // War Stomp is 12y + up to 1.95y combat reach
        constexpr float KAZROGAL_RANGED_ARC_RADIUS = 15.0f;
        // Extra distance during the pull to avoid War Stomp
        constexpr float KAZROGAL_RANGED_ARC_APPROACH_RADIUS = 25.0f;
        // Heading from Kaz'rogal for the center of the ranged arc
        constexpr float KAZROGAL_RANGED_ARC_CENTER = 4.225f;
        constexpr float KAZROGAL_RANGED_ARC_HALF_WIDTH = 10.0f;
        // Mark of Kaz'rogal drains 600 mana a tick, five 1s ticks; it explodes when the victim has less than 600 mana
        constexpr uint32 MARK_TICK_DRAIN = 600;
        constexpr uint32 MARK_FULL_DRAIN = 3000;
        // Mana levels for running away from the group and coming back to it (also the Aspect of the Viper levels)
        constexpr uint32 MARK_DANGER_MANA = 3200;
        constexpr uint32 MARK_REJOIN_MANA = 4000;
        // Warlocks Life Tap below this mana
        constexpr uint32 MARK_LIFE_TAP_MANA = 5000;
        // The explosion (31463) has 15y radius, 1y buffer
        constexpr float MARK_ESCAPE_DISTANCE = 16.0f;

        // Near the gate, so the raid can start immediately and beat the soft enrage of the Marks
        constexpr Point KAZROGAL_TANK_POSITION = { 5505.440f, -2665.059f, 1480.598f };

        float GetKazrogalRangedArcRadius(Unit* kazrogal);
        float GetKazrogalRangedArcSpan(float radius);
        bool IsKazrogalManaUser(Player* bot);
        bool HasMarkOfKazrogal(Player* bot);

        // Azgalor

        // 15y radius + combat reach of the player
        constexpr float RAIN_OF_FIRE_RADIUS = 17.0f;
        constexpr float RAIN_OF_FIRE_CONTROL_RADIUS = RAIN_OF_FIRE_RADIUS + HAZARD_CONTROL_MARGIN;
        constexpr float RAIN_OF_FIRE_SEARCH_RADIUS = RAIN_OF_FIRE_CONTROL_RADIUS + HAZARD_SEARCH_MARGIN;
        // Cleave chains from Azgalor's victim to more players within 12y inside his frontal arc
        constexpr float CLEAVE_CHAIN_RADIUS = 12.0f;
        constexpr float CLEAVE_DANGER_ARC = 200.0f * float(M_PI) / 180.0f;

        // Back toward the Horde base, just past the path leading to Thrall's keep
        constexpr Point AZGALOR_TANK_POSITION      = { 5494.594f, -2747.069f, 1487.800f };
        // On top of Thrall's starting position, so Thrall helps with the Doomguards
        constexpr Point AZGALOR_DOOMGUARD_POSITION = { 5452.166f, -2723.282f, 1485.480f };

        bool IsSafeFromAzgalorCleave(Unit* azgalor, float x, float y);
        // Each Rain of Fire is its own dynamic object, up to two at a time
        std::vector<WorldPosition> GetRainOfFirePositions(PlayerbotAI* ai);
        bool GetNearestRainOfFirePosition(PlayerbotAI* ai, WorldPosition& pool);
        bool IsNearRainOfFire(PlayerbotAI* ai, float radius);
        bool IsInRainOfFire(PlayerbotAI* ai);
        bool IsDoomed(Player* bot);
        // The first assist tank picks up the Doomguards, the second one when the first is Doomed
        bool IsDoomguardTank(Player* bot);
        bool AnyGroupMemberHasDoom(Player* bot);

        // Archimonde

        constexpr float AIR_BURST_SAFE_DISTANCE = 15.0f;
        // Damaging radius of a Doomfire pool: 6y + combat reach
        constexpr float DOOMFIRE_BURN_RADIUS = 8.0f;
        // How far from the pools avoidance places the bot
        constexpr float DOOMFIRE_DANGER_RADIUS = DOOMFIRE_BURN_RADIUS + 2.0f;
        // Within this distance the movement is controlled only by the avoidance action
        constexpr float DOOMFIRE_CONTROL_RADIUS = DOOMFIRE_DANGER_RADIUS + 2.0f;
        // Pools farther away than this don't affect the escape direction
        constexpr float DOOMFIRE_FIELD_RADIUS = 18.0f;
        constexpr float DOOMFIRE_SEARCH_RADIUS = DOOMFIRE_FIELD_RADIUS + DOOMFIRE_DANGER_RADIUS + HAZARD_SEARCH_MARGIN;
        // General spread against Air Burst
        constexpr float ARCHIMONDE_RANGED_SPREAD_DISTANCE = 10.0f;

        // Up the hill a bit, away from the World Tree. The tank walks him here at the pull only.
        constexpr Point ARCHIMONDE_INITIAL_POSITION = { 5640.502f, -3421.238f, 1587.453f };

        // Protection of Elune: Archimonde is beaten, the boss tactics stop
        bool HasProtectionOfElune(Player* bot);
        bool IsNearDoomfire(PlayerbotAI* ai, float radius);
        bool IsPositionNearDoomfire(PlayerbotAI* ai, float x, float y, float radius);
        std::vector<WorldPosition> GetDoomfirePositions(PlayerbotAI* ai);
        // Player Archimonde is casting Air Burst on
        Player* GetAirBurstTarget(Unit* archimonde);

        // Ice Block / Divine Shield / Cloak of Shadows of the bot ("" if the class has none)
        std::string GetSelfImmunitySpellName(Player* bot);
        // Highest known rank of that spell (0 if unknown)
        uint32 GetSelfImmunitySpellId(PlayerbotAI* ai);

        // Action classification by name (this core has no common base classes for these actions)
        bool IsAvoidAoeAction(Action* action);           // "flee" of "avoid aoe", "tbc raid leave hazard"
        bool IsFormationMoveAction(Action* action);      // "tbc raid spread", "tbc raid leave front"
        bool IsReachTargetAction(Action* action);        // "reach melee", "reach spell", charge, ...
        bool IsSetBehindAction(Action* action);
        bool IsAssistTargetingAction(Action* action);    // "dps assist", "tank assist", ...
        bool IsAttackAction(Action* action);
    }
}
