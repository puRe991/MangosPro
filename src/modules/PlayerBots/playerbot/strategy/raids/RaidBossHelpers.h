#pragma once

#include "playerbot/WorldPosition.h"
#include <string>
#include <vector>

class Player;
class Unit;
class Creature;
class GameObject;
class PlayerbotAI;

namespace ai
{
    class Action;

    // Shared helpers for the per raid boss strategies in strategy/raids
    // (ported from the mod-playerbots EncounterHelpers and adapted to this core).
    namespace raidboss
    {
        // Rough proxies for the progress of an encounter: at 95% the raid is positioned and the
        // tank has threat (time for cooldowns), at 10% the adds are ignored to finish the boss.
        constexpr float BOSS_ENGAGED_HEALTH_PCT = 95.0f;
        constexpr float BOSS_BURN_HEALTH_PCT = 10.0f;

        // Relevance of raid mechanics: above every heal (critical heal = 80) and below emergency (90).
        // mod-playerbots ACTION_RAID (60) is above its heals (10-30), the same order is kept here.
        constexpr float ACTION_RAID = 85.0f;

        // Raid target icon indexes (same order as the client)
        enum RaidIcon : uint8
        {
            ICON_STAR     = 0,
            ICON_CIRCLE   = 1,
            ICON_DIAMOND  = 2,
            ICON_TRIANGLE = 3,
            ICON_MOON     = 4,
            ICON_SQUARE   = 5,
            ICON_CROSS    = 6,
            ICON_SKULL    = 7,
        };

        // A boss of the map is in combat with the bot's group. The instance scripts of this core
        // don't report every encounter as in progress, so an engaged creature of the given entries
        // (or any engaged creature when the list is empty) also counts.
        bool IsEncounterInProgress(Player* bot, uint32 mapId, const std::vector<uint32>& bossEntries = {});

        // Alive creature of an entry near the bot (attack targets first, then a grid search)
        Unit* FindUnit(PlayerbotAI* ai, uint32 entry, float range = 150.0f);
        // Alive creature by (case insensitive) name near the bot ("find target" of mod-playerbots)
        Unit* FindUnitByName(PlayerbotAI* ai, const std::string& name, float range = 150.0f);
        std::vector<Unit*> FindUnits(Player* bot, uint32 entry, float range, bool aliveOnly = true);
        std::vector<GameObject*> FindGameObjects(Player* bot, uint32 entry, float range);

        // Incremental movement: projects a step of moveDist towards the destination and checks
        // that the step can be taken (ground height and line of sight). stepZ is snapped to the ground.
        bool CanTakeStepTowards(Player* bot, float destinationX, float destinationY, float moveDist,
                                float& stepX, float& stepY, float& stepZ);
        // Step towards a position without collision checks, false once within arrivalDist.
        // 'facing' (the tanked mob) allows walking backwards while tanking.
        bool GetStepToPosition(Player* bot, const WorldPosition& position, float arrivalDist, Unit* facing,
                               float& stepX, float& stepY, bool& backwards);

        // Raid target icons (the bot never removes an icon set by a player during the encounter)
        bool MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
        inline bool MarkTargetWithSkull(Player* bot, Unit* target) { return MarkTargetWithIcon(bot, target, ICON_SKULL); }
        inline bool MarkTargetWithSquare(Player* bot, Unit* target) { return MarkTargetWithIcon(bot, target, ICON_SQUARE); }
        inline bool MarkTargetWithStar(Player* bot, Unit* target) { return MarkTargetWithIcon(bot, target, ICON_STAR); }
        inline bool MarkTargetWithCircle(Player* bot, Unit* target) { return MarkTargetWithIcon(bot, target, ICON_CIRCLE); }
        inline bool MarkTargetWithDiamond(Player* bot, Unit* target) { return MarkTargetWithIcon(bot, target, ICON_DIAMOND); }
        inline bool MarkTargetWithTriangle(Player* bot, Unit* target) { return MarkTargetWithIcon(bot, target, ICON_TRIANGLE); }
        inline bool MarkTargetWithCross(Player* bot, Unit* target) { return MarkTargetWithIcon(bot, target, ICON_CROSS); }
        inline bool MarkTargetWithMoon(Player* bot, Unit* target) { return MarkTargetWithIcon(bot, target, ICON_MOON); }
        bool ClearTargetIcon(Player* bot, uint8 iconId);
        Unit* GetIconTarget(PlayerbotAI* ai, uint8 iconId);
        // Raid target icon the bot attacks ("skull", "cross", ...)
        void SetRtiTarget(PlayerbotAI* ai, const std::string& rtiName);

        // First alive bot of the group in the map: the one that marks targets / tracks the mechanics
        bool IsMechanicTrackerBot(Player* bot, uint32 mapId);

        // Tanks of the group: the main tank is the first alive tank (assistants first),
        // the assist tanks are the following ones (index 0 = first off tank)
        Player* GetGroupMainTank(Player* bot);
        Player* GetGroupAssistTank(Player* bot, uint8 index);
        bool IsMainTank(Player* bot);
        bool IsAssistTank(Player* bot, uint8 index);
        // Members of a role in group order (used to give each bot its own job)
        std::vector<Player*> GetGroupTanks(Player* bot);
        std::vector<Player*> GetGroupHealers(Player* bot);
        std::vector<Player*> GetGroupRangedDps(Player* bot);
        std::vector<Player*> GetGroupMeleeDps(Player* bot);
        // Index of the bot in a list of players (-1 if not present)
        int GetIndexOf(Player* bot, const std::vector<Player*>& players);

        bool IsDps(Player* player);
        bool IsRangedPlayer(Player* player);
        bool IsMeleePlayer(Player* player);

        Player* GetNearestPlayerInRadius(Player* bot, float radius);
        std::vector<Player*> GetGroupMembers(Player* bot, float range = 200.0f, bool includeSelf = true);

        // Positions of the dynamic objects (ground effects) of a spell around the bot
        std::vector<WorldPosition> GetDynamicObjectPositions(Player* bot, float searchRadius, uint32 spellId);

        // Ice Block, Divine Shield or Cloak of Shadows (0 if the class has none)
        uint32 GetSelfImmunitySpell(Player* bot);

        // Action classification by action name (used by the multipliers)
        bool IsDpsCooldownAction(Player* bot, Action* action);
        bool IsTauntAction(Player* bot, Action* action);
        bool IsAoeThreatAction(Player* bot, Action* action);
        bool IsMovementAction(Action* action);
        bool IsAttackTargetAction(Action* action);

        // Distance between centers, ignoring the bounding radius (GetExactDist2d of AzerothCore)
        float ExactDist2d(const WorldObject* a, const WorldObject* b);
        float ExactDist2d(const WorldObject* a, float x, float y);

        // Bot casts a spell by id / name when possible
        bool TryCast(PlayerbotAI* ai, uint32 spellId, Unit* target);
        bool TryCast(PlayerbotAI* ai, const std::string& spell, Unit* target);
    }
}
