#pragma once
#include "DungeonTriggers.h"
#include "GenericTriggers.h"
#include "playerbot/strategy/values/GuidPositionValues.h"

namespace ai
{
    // While Broodlord fights, only devices whose suppression reaches his fight matter; the rest
    // rearm within 2 minutes anyway and pull the rogues away from the boss.
    std::list<GuidPosition> FilterSuppressionDevices(Player* bot, const std::list<GuidPosition>& devices);

    class BlackwingLairEnterDungeonTrigger : public EnterDungeonTrigger
    {
    public:
        BlackwingLairEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter blackwing lair", "blackwing lair", 469) {}
    };

    class BlackwingLairLeaveDungeonTrigger : public LeaveDungeonTrigger
    {
    public:
        BlackwingLairLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave blackwing lair", "blackwing lair", 469) {}
    };

    class SuppressionDeviceNeedStealthTrigger : public Trigger
    {
    public:
        SuppressionDeviceNeedStealthTrigger(PlayerbotAI* ai) : Trigger(ai, "suppression device need stealth", 1) {}

        bool IsActive() override
        {
            if (bot->getClass() != CLASS_ROGUE)
                return false;

            if (ai->HasAura("stealth", bot))
                return false;

            std::list<GuidPosition> gos = FilterSuppressionDevices(bot, AI_VALUE(std::list<GuidPosition>, "go usable filter::go trapped filter::entry filter::{gos in sight,suppression devices}"));
            return !gos.empty();
        }
    };

    class SuppressionDeviceInSightTrigger : public Trigger
    {
    public:
        SuppressionDeviceInSightTrigger(PlayerbotAI* ai) : Trigger(ai, "suppression device in sight", 1) {}

        bool IsActive() override
        {
            if (bot->getClass() != CLASS_ROGUE)
                return false;

            std::list<GuidPosition> gosInSight = FilterSuppressionDevices(bot, AI_VALUE(std::list<GuidPosition>, "go usable filter::go trapped filter::entry filter::{gos in sight,suppression devices}"));
            std::list<GuidPosition> gosClose = FilterSuppressionDevices(bot, AI_VALUE(std::list<GuidPosition>, "entry filter::{gos close,suppression devices}"));
            
            return !gosInSight.empty() && gosClose.empty();
        }
    };

    class SuppressionDeviceCloseTrigger : public Trigger
    {
    public:
        SuppressionDeviceCloseTrigger(PlayerbotAI* ai) : Trigger(ai, "suppression device close", 1) {}

        bool IsActive() override
        {
            if (bot->getClass() != CLASS_ROGUE)
                return false;

            std::list<GuidPosition> gos = FilterSuppressionDevices(bot, AI_VALUE(std::list<GuidPosition>, "go usable filter::go trapped filter::entry filter::{gos close,suppression devices}"));
            return !gos.empty();
        }
    };
}