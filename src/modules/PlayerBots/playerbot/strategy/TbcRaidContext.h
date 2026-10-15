#pragma once
#include "NamedObjectContext.h"
#include "generic/TbcRaidStrategy.h"
#include "triggers/TbcRaidTriggers.h"
#include "actions/TbcRaidActions.h"

namespace ai
{
    // Objects of the Burning Crusade raid tactics ("tbc raid" strategy)
    class TbcRaidStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        TbcRaidStrategyContext()
        {
            creators["tbc raid"] = [](PlayerbotAI* ai) { return new TbcRaidStrategy(ai); };
        }
    };

    class TbcRaidTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        TbcRaidTriggerContext()
        {
            creators["enter tbc raid"] = [](PlayerbotAI* ai) { return new TbcRaidEnterTrigger(ai); };
            creators["leave tbc raid"] = [](PlayerbotAI* ai) { return new TbcRaidLeaveTrigger(ai); };
            creators["tbc raid in hazard"] = [](PlayerbotAI* ai) { return new TbcRaidInHazardTrigger(ai); };
            creators["tbc raid debuff run"] = [](PlayerbotAI* ai) { return new TbcRaidDebuffRunTrigger(ai); };
            creators["tbc raid spread"] = [](PlayerbotAI* ai) { return new TbcRaidSpreadTrigger(ai); };
            creators["tbc raid gather"] = [](PlayerbotAI* ai) { return new TbcRaidGatherTrigger(ai); };
            creators["tbc raid in front of boss"] = [](PlayerbotAI* ai) { return new TbcRaidInFrontOfBossTrigger(ai); };
            creators["tbc raid priority target"] = [](PlayerbotAI* ai) { return new TbcRaidPriorityTargetTrigger(ai); };
            creators["tbc raid taunt"] = [](PlayerbotAI* ai) { return new TbcRaidTauntTrigger(ai); };
            creators["tbc raid tank assigned"] = [](PlayerbotAI* ai) { return new TbcRaidTankAssignedTrigger(ai); };
            creators["tbc raid tank position"] = [](PlayerbotAI* ai) { return new TbcRaidTankPositionTrigger(ai); };
            creators["tbc raid steal aura"] = [](PlayerbotAI* ai) { return new TbcRaidStealAuraTrigger(ai); };
            creators["tbc raid interrupt"] = [](PlayerbotAI* ai) { return new TbcRaidInterruptTrigger(ai); };
            creators["tbc raid mark targets"] = [](PlayerbotAI* ai) { return new TbcRaidMarkTargetsTrigger(ai); };
            creators["tbc raid use cube"] = [](PlayerbotAI* ai) { return new TbcRaidUseCubeTrigger(ai); };
            creators["tbc raid use spine"] = [](PlayerbotAI* ai) { return new TbcRaidUseSpineTrigger(ai); };

            creators["tbc raid stack on tank"] = [](PlayerbotAI* ai) { return new TbcRaidMechanicTrigger(ai, "tbc raid stack on tank", [](PlayerbotAI* ai)
            {
                Player* mainTank = TbcRaidHelper::GetMainTankToStackOn(ai);
                return mainTank && !ai->GetBot()->IsWithinDistInMap(mainTank, 3.0f);
            }); };

            creators["tbc raid approach boss"] = [](PlayerbotAI* ai) { return new TbcRaidMechanicTrigger(ai, "tbc raid approach boss", [](PlayerbotAI* ai)
            {
                float distance = 0.0f;
                Unit* boss = TbcRaidHelper::GetApproachTarget(ai, distance);
                return boss && !ai->GetBot()->IsWithinDistInMap(boss, distance);
            }); };

            creators["tbc raid soak front"] = [](PlayerbotAI* ai) { return new TbcRaidMechanicTrigger(ai, "tbc raid soak front", [](PlayerbotAI* ai)
            {
                return TbcRaidHelper::GetSoakBoss(ai) != nullptr;
            }); };

            creators["tbc raid hydross tank"] = [](PlayerbotAI* ai) { return new TbcRaidMechanicTrigger(ai, "tbc raid hydross tank", [](PlayerbotAI* ai)
            {
                WorldPosition position;
                return TbcRaidHelper::GetHydrossTankPosition(ai, position);
            }); };

            creators["tbc raid loot core"] = [](PlayerbotAI* ai) { return new TbcRaidMechanicTrigger(ai, "tbc raid loot core", [](PlayerbotAI* ai)
            {
                return TbcRaidHelper::GetTaintedCoreCorpse(ai) != nullptr;
            }); };

            creators["tbc raid has core"] = [](PlayerbotAI* ai) { return new TbcRaidMechanicTrigger(ai, "tbc raid has core", [](PlayerbotAI* ai)
            {
                return ai->GetBot()->HasItemCount(31088, 1) && TbcRaidHelper::GetNearestActiveShieldGenerator(ai, ai->GetBot());
            }); };

            creators["tbc raid lurker spout"] = [](PlayerbotAI* ai) { return new TbcRaidMechanicTrigger(ai, "tbc raid lurker spout", [](PlayerbotAI* ai)
            {
                return !ai->GetBot()->IsInWater() && TbcRaidHelper::GetSpoutingLurker(ai) != nullptr;
            }); };

            creators["tbc raid possessing"] = [](PlayerbotAI* ai) { return new TbcRaidMechanicTrigger(ai, "tbc raid possessing", [](PlayerbotAI* ai)
            {
                return TbcRaidHelper::GetPossessedCreature(ai) != nullptr;
            }); };

            creators["tbc raid use blue orb"] = [](PlayerbotAI* ai) { return new TbcRaidMechanicTrigger(ai, "tbc raid use blue orb", [](PlayerbotAI* ai)
            {
                return TbcRaidHelper::GetBlueOrbToUse(ai) != nullptr;
            }); };

            creators["tbc raid darkness"] = [](PlayerbotAI* ai) { return new TbcRaidMechanicTrigger(ai, "tbc raid darkness", [](PlayerbotAI* ai)
            {
                Player* bot = ai->GetBot();
                if (TbcRaidHelper::IsPossessingBossCreature(bot) || !TbcRaidHelper::GetKiljaedenCastingDarkness(ai))
                    return false;

                Unit* dragon = TbcRaidHelper::GetRaidBlueDragon(ai);
                if (!dragon)
                    return false;

                const float dx = bot->GetPositionX() - dragon->GetPositionX();
                const float dy = bot->GetPositionY() - dragon->GetPositionY();
                return dx * dx + dy * dy > 6.0f * 6.0f;
            }); };
        }
    };

    class TbcRaidActionContext : public NamedObjectContext<Action>
    {
    public:
        TbcRaidActionContext()
        {
            creators["enable tbc raid strategy"] = [](PlayerbotAI* ai) { return new TbcRaidEnableStrategyAction(ai); };
            creators["disable tbc raid strategy"] = [](PlayerbotAI* ai) { return new TbcRaidDisableStrategyAction(ai); };
            creators["tbc raid leave hazard"] = [](PlayerbotAI* ai) { return new TbcRaidLeaveHazardAction(ai); };
            creators["tbc raid spread"] = [](PlayerbotAI* ai) { return new TbcRaidSpreadAction(ai); };
            creators["tbc raid gather"] = [](PlayerbotAI* ai) { return new TbcRaidGatherAction(ai); };
            creators["tbc raid leave front"] = [](PlayerbotAI* ai) { return new TbcRaidLeaveFrontAction(ai); };
            creators["tbc raid attack priority target"] = [](PlayerbotAI* ai) { return new TbcRaidAttackPriorityTargetAction(ai); };
            creators["tbc raid taunt"] = [](PlayerbotAI* ai) { return new TbcRaidTauntAction(ai); };
            creators["tbc raid tank assigned"] = [](PlayerbotAI* ai) { return new TbcRaidTankAssignedAction(ai); };
            creators["tbc raid tank position"] = [](PlayerbotAI* ai) { return new TbcRaidTankPositionAction(ai); };
            creators["tbc raid steal aura"] = [](PlayerbotAI* ai) { return new TbcRaidStealAuraAction(ai); };
            creators["tbc raid interrupt"] = [](PlayerbotAI* ai) { return new TbcRaidInterruptAction(ai); };
            creators["tbc raid mark targets"] = [](PlayerbotAI* ai) { return new TbcRaidMarkTargetsAction(ai); };
            creators["tbc raid use cube"] = [](PlayerbotAI* ai) { return new TbcRaidUseCubeAction(ai); };
            creators["tbc raid move to spine"] = [](PlayerbotAI* ai) { return new TbcRaidMoveToSpineAction(ai); };
            creators["tbc raid use spine"] = [](PlayerbotAI* ai) { return new TbcRaidUseSpineAction(ai); };
            creators["tbc raid stack on tank"] = [](PlayerbotAI* ai) { return new TbcRaidStackOnTankAction(ai); };
            creators["tbc raid approach boss"] = [](PlayerbotAI* ai) { return new TbcRaidApproachBossAction(ai); };
            creators["tbc raid soak front"] = [](PlayerbotAI* ai) { return new TbcRaidSoakFrontAction(ai); };
            creators["tbc raid hydross tank"] = [](PlayerbotAI* ai) { return new TbcRaidHydrossTankAction(ai); };
            creators["tbc raid loot core"] = [](PlayerbotAI* ai) { return new TbcRaidLootCoreAction(ai); };
            creators["tbc raid use core"] = [](PlayerbotAI* ai) { return new TbcRaidUseCoreAction(ai); };
            creators["tbc raid jump into water"] = [](PlayerbotAI* ai) { return new TbcRaidJumpIntoWaterAction(ai); };
            creators["tbc raid control possessed"] = [](PlayerbotAI* ai) { return new TbcRaidControlPossessedAction(ai); };
            creators["tbc raid use blue orb"] = [](PlayerbotAI* ai) { return new TbcRaidUseBlueOrbAction(ai); };
            creators["tbc raid gather under dragon"] = [](PlayerbotAI* ai) { return new TbcRaidGatherUnderDragonAction(ai); };
        }
    };
}
