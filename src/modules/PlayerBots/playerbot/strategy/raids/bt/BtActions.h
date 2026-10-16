// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/BT/BTActions) and adapted for this core.
#pragma once

#include "playerbot/strategy/actions/MovementActions.h"
#include "playerbot/strategy/actions/AttackAction.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "BtHelpers.h"
#include <string>
#include <vector>

namespace ai
{
    // Movement helpers shared by the Black Temple actions (MoveTo is protected in MovementAction)
    template <class Base>
    class BtActionBase : public Base
    {
    public:
        BtActionBase(PlayerbotAI* ai, std::string name) : Base(ai, name) {}

    protected:
        // Moves to a point, the height is snapped to the ground
        bool MoveToPoint(float x, float y, float z)
        {
            Player* bot = this->bot;
            float groundZ = bot->GetMap()->GetHeight(x, y, z + 5.0f);
            if (groundZ > INVALID_HEIGHT && std::fabs(groundZ - z) < 10.0f)
                z = groundZ;

            return this->MoveTo(bot->GetMapId(), x, y, z, false, false, false, true);
        }

        bool MoveToPosition(const WorldPosition& position, float arrivalDist)
        {
            if (bt::Dist2d(this->bot->GetPositionX(), this->bot->GetPositionY(), position.getX(), position.getY()) <= arrivalDist)
                return false;

            return MoveToPoint(position.getX(), position.getY(), position.getZ());
        }

        // Short step towards a point (tanks drag the boss step by step)
        bool StepTowards(float x, float y, float maxStep, float arrivalDist)
        {
            Player* bot = this->bot;
            const float dist = bt::Dist2d(bot->GetPositionX(), bot->GetPositionY(), x, y);
            if (dist <= arrivalDist)
                return false;

            const float moveDist = std::min(maxStep, dist);
            const float moveX = bot->GetPositionX() + (x - bot->GetPositionX()) / dist * moveDist;
            const float moveY = bot->GetPositionY() + (y - bot->GetPositionY()) / dist * moveDist;
            return MoveToPoint(moveX, moveY, bot->GetPositionZ());
        }

        // FleePosition of mod-playerbots: get at least safeDist away from a point
        bool FleeFrom(float fromX, float fromY, float safeDist, uint32 minIntervalMs = 0)
        {
            Player* bot = this->bot;
            const float current = bt::Dist2d(bot->GetPositionX(), bot->GetPositionY(), fromX, fromY);
            if (current >= safeDist)
                return false;

            const uint32 now = WorldTimer::getMSTime();
            if (minIntervalMs && lastFleeTime && WorldTimer::getMSTimeDiff(lastFleeTime, now) < minIntervalMs)
                return false;

            float baseAngle = std::atan2(bot->GetPositionY() - fromY, bot->GetPositionX() - fromX);
            if (current < 0.1f)
                baseAngle = frand(0.0f, 2.0f * M_PI_F);

            const float offsets[] = { 0.0f, 0.5f, -0.5f, 1.0f, -1.0f, 1.5f, -1.5f, 2.2f, -2.2f };
            for (float offset : offsets)
            {
                const float angle = baseAngle + offset;
                const float x = fromX + std::cos(angle) * (safeDist + 1.0f);
                const float y = fromY + std::sin(angle) * (safeDist + 1.0f);
                const float z = bot->GetMap()->GetHeight(x, y, bot->GetPositionZ() + 5.0f);
                if (z <= INVALID_HEIGHT || std::fabs(z - bot->GetPositionZ()) > 8.0f)
                    continue;

                if (!bot->IsWithinLOS(x, y, z + 2.0f))
                    continue;

                if (this->MoveTo(bot->GetMapId(), x, y, z, false, false, false, true))
                {
                    lastFleeTime = now;
                    return true;
                }
            }

            return false;
        }

        // MoveAway of mod-playerbots: move 'distance' further away from a unit
        bool MoveAwayFrom(WorldObject* from, float distance)
        {
            if (!from || distance <= 0.0f)
                return false;

            const float current = raidboss::ExactDist2d(this->bot, from);
            return FleeFrom(from->GetPositionX(), from->GetPositionY(), current + distance);
        }

        bool IsCurrentTarget(Unit* target)
        {
            return target && this->ai->GetAiObjectContext()->template GetValue<ObjectGuid>("current target")->Get() == target->GetObjectGuid();
        }

        bool AttackTarget(Unit* target)
        {
            return target && this->Attack(this->GetMaster(), target);
        }

        // Hunter: Misdirection on the tank, then a Steady Shot on the target
        bool MisdirectTo(Player* tank, Unit* target)
        {
            if (!tank || !tank->IsAlive() || !target)
                return false;

            if (raidboss::TryCast(this->ai, "misdirection", tank))
                return true;

            return this->ai->HasAura("misdirection", this->bot) && raidboss::TryCast(this->ai, "steady shot", target);
        }

    private:
        uint32 lastFleeTime = 0;
    };

    using BtMovementAction = BtActionBase<MovementAction>;
    using BtAttackAction = BtActionBase<AttackAction>;

    // General

    class BlackTempleEraseTimersAndTrackersAction : public Action
    {
    public:
        BlackTempleEraseTimersAndTrackersAction(PlayerbotAI* ai) : Action(ai, "black temple erase timers and trackers") {}
        bool Execute(Event& event) override;
    };

    // High Warlord Naj'entus

    class HighWarlordNajentusMisdirectBossToMainTankAction : public BtAttackAction
    {
    public:
        HighWarlordNajentusMisdirectBossToMainTankAction(PlayerbotAI* ai) : BtAttackAction(ai, "high warlord naj'entus misdirect boss to main tank") {}
        bool Execute(Event& event) override;
    };

    class HighWarlordNajentusTanksPositionBossAction : public BtAttackAction
    {
    public:
        HighWarlordNajentusTanksPositionBossAction(PlayerbotAI* ai) : BtAttackAction(ai, "high warlord naj'entus tanks position boss") {}
        bool Execute(Event& event) override;
    };

    class HighWarlordNajentusDisperseRangedAction : public BtMovementAction
    {
    public:
        HighWarlordNajentusDisperseRangedAction(PlayerbotAI* ai) : BtMovementAction(ai, "high warlord naj'entus disperse ranged") {}
        bool Execute(Event& event) override;
    };

    class HighWarlordNajentusRemoveImpalingSpineAction : public BtMovementAction
    {
    public:
        HighWarlordNajentusRemoveImpalingSpineAction(PlayerbotAI* ai) : BtMovementAction(ai, "high warlord naj'entus remove impaling spine") {}
        bool Execute(Event& event) override;
    };

    class HighWarlordNajentusThrowImpalingSpineAction : public BtMovementAction
    {
    public:
        HighWarlordNajentusThrowImpalingSpineAction(PlayerbotAI* ai) : BtMovementAction(ai, "high warlord naj'entus throw impaling spine") {}
        bool Execute(Event& event) override;
    };

    // Supremus

    class SupremusMisdirectBossToMainTankAction : public BtAttackAction
    {
    public:
        SupremusMisdirectBossToMainTankAction(PlayerbotAI* ai) : BtAttackAction(ai, "supremus misdirect boss to main tank") {}
        bool Execute(Event& event) override;
    };

    class SupremusDisperseRangedAction : public BtMovementAction
    {
    public:
        SupremusDisperseRangedAction(PlayerbotAI* ai) : BtMovementAction(ai, "supremus disperse ranged") {}
        bool Execute(Event& event) override;
    };

    class SupremusKiteBossAction : public BtMovementAction
    {
    public:
        SupremusKiteBossAction(PlayerbotAI* ai) : BtMovementAction(ai, "supremus kite boss") {}
        bool Execute(Event& event) override;
    };

    class SupremusMoveAwayFromVolcanosAction : public BtMovementAction
    {
    public:
        SupremusMoveAwayFromVolcanosAction(PlayerbotAI* ai) : BtMovementAction(ai, "supremus move away from volcanos") {}
        bool Execute(Event& event) override;

    private:
        bool FindSafestNearbyPosition(const std::vector<Unit*>& volcanos, float maxRadius, float hazardRadius, float& outX, float& outY);
        bool IsPathSafe(float endX, float endY, const std::vector<Unit*>& volcanos, float hazardRadius);
    };

    class SupremusManagePhaseTimerAction : public Action
    {
    public:
        SupremusManagePhaseTimerAction(PlayerbotAI* ai) : Action(ai, "supremus manage phase timer") {}
        bool Execute(Event& event) override;
    };

    // Shade of Akama

    class ShadeOfAkamaMeleeDpsPrioritizeChannelersAction : public BtAttackAction
    {
    public:
        ShadeOfAkamaMeleeDpsPrioritizeChannelersAction(PlayerbotAI* ai) : BtAttackAction(ai, "shade of akama melee dps prioritize channelers") {}
        bool Execute(Event& event) override;
    };

    // Teron Gorefiend

    class TeronGorefiendMisdirectBossToMainTankAction : public BtAttackAction
    {
    public:
        TeronGorefiendMisdirectBossToMainTankAction(PlayerbotAI* ai) : BtAttackAction(ai, "teron gorefiend misdirect boss to main tank") {}
        bool Execute(Event& event) override;
    };

    class TeronGorefiendTanksPositionBossAction : public BtAttackAction
    {
    public:
        TeronGorefiendTanksPositionBossAction(PlayerbotAI* ai) : BtAttackAction(ai, "teron gorefiend tanks position boss") {}
        bool Execute(Event& event) override;
    };

    class TeronGorefiendPositionRangedOnBalconyAction : public BtMovementAction
    {
    public:
        TeronGorefiendPositionRangedOnBalconyAction(PlayerbotAI* ai) : BtMovementAction(ai, "teron gorefiend position ranged on balcony") {}
        bool Execute(Event& event) override;
    };

    class TeronGorefiendAvoidShadowOfDeathAction : public Action
    {
    public:
        TeronGorefiendAvoidShadowOfDeathAction(PlayerbotAI* ai) : Action(ai, "teron gorefiend avoid shadow of death") {}
        bool Execute(Event& event) override;
    };

    class TeronGorefiendMoveToCornerToDieAction : public BtMovementAction
    {
    public:
        TeronGorefiendMoveToCornerToDieAction(PlayerbotAI* ai) : BtMovementAction(ai, "teron gorefiend move to corner to die") {}
        bool Execute(Event& event) override;
    };

    class TeronGorefiendControlAndDestroyShadowyConstructsAction : public Action
    {
    public:
        TeronGorefiendControlAndDestroyShadowyConstructsAction(PlayerbotAI* ai) : Action(ai, "teron gorefiend control and destroy shadowy constructs") {}
        bool Execute(Event& event) override;
        bool isUsefulWhenStunned() override { return true; }

    private:
        bool CastSpiritSpell(Creature* spirit, uint32 spellId, Unit* target);
    };

    // Gurtogg Bloodboil

    class GurtoggBloodboilMisdirectBossToMainTankAction : public BtAttackAction
    {
    public:
        GurtoggBloodboilMisdirectBossToMainTankAction(PlayerbotAI* ai) : BtAttackAction(ai, "gurtogg bloodboil misdirect boss to main tank") {}
        bool Execute(Event& event) override;
    };

    class GurtoggBloodboilTanksPositionBossAction : public BtAttackAction
    {
    public:
        GurtoggBloodboilTanksPositionBossAction(PlayerbotAI* ai) : BtAttackAction(ai, "gurtogg bloodboil tanks position boss") {}
        bool Execute(Event& event) override;
    };

    class GurtoggBloodboilRotateRangedGroupsAction : public BtMovementAction
    {
    public:
        GurtoggBloodboilRotateRangedGroupsAction(PlayerbotAI* ai) : BtMovementAction(ai, "gurtogg bloodboil rotate ranged groups") {}
        bool Execute(Event& event) override;
    };

    class GurtoggBloodboilRangedMoveAwayFromEnragedPlayerAction : public BtMovementAction
    {
    public:
        GurtoggBloodboilRangedMoveAwayFromEnragedPlayerAction(PlayerbotAI* ai) : BtMovementAction(ai, "gurtogg bloodboil ranged move away from enraged player") {}
        bool Execute(Event& event) override;
    };

    class GurtoggBloodboilManagePhaseTimerAction : public Action
    {
    public:
        GurtoggBloodboilManagePhaseTimerAction(PlayerbotAI* ai) : Action(ai, "gurtogg bloodboil manage phase timer") {}
        bool Execute(Event& event) override;
    };

    // Reliquary of Souls

    class ReliquaryOfSoulsMisdirectBossToMainTankAction : public BtAttackAction
    {
    public:
        ReliquaryOfSoulsMisdirectBossToMainTankAction(PlayerbotAI* ai) : BtAttackAction(ai, "reliquary of souls misdirect boss to main tank") {}
        bool Execute(Event& event) override;
    };

    class ReliquaryOfSoulsAdjustDistanceFromSufferingAction : public BtMovementAction
    {
    public:
        ReliquaryOfSoulsAdjustDistanceFromSufferingAction(PlayerbotAI* ai) : BtMovementAction(ai, "reliquary of souls adjust distance from suffering") {}
        bool Execute(Event& event) override;

    private:
        bool TanksMoveToMinimumRange(Unit* suffering);
        bool MeleeDpsStayAtMaximumRange(Unit* suffering);
        bool RangedMoveAwayFromBoss(Unit* suffering);
    };

    class ReliquaryOfSoulsHealersDpsSufferingAction : public Action
    {
    public:
        ReliquaryOfSoulsHealersDpsSufferingAction(PlayerbotAI* ai) : Action(ai, "reliquary of souls healers dps suffering") {}
        bool Execute(Event& event) override;
    };

    class ReliquaryOfSoulsSpellstealRuneShieldAction : public Action
    {
    public:
        ReliquaryOfSoulsSpellstealRuneShieldAction(PlayerbotAI* ai) : Action(ai, "reliquary of souls spellsteal rune shield") {}
        bool Execute(Event& event) override;
    };

    class ReliquaryOfSoulsSpellReflectDeadenAction : public Action
    {
    public:
        ReliquaryOfSoulsSpellReflectDeadenAction(PlayerbotAI* ai) : Action(ai, "reliquary of souls spell reflect deaden") {}
        bool Execute(Event& event) override;
    };

    // Mother Shahraz

    class MotherShahrazMisdirectBossToMainTankAction : public BtAttackAction
    {
    public:
        MotherShahrazMisdirectBossToMainTankAction(PlayerbotAI* ai) : BtAttackAction(ai, "mother shahraz misdirect boss to main tank") {}
        bool Execute(Event& event) override;
    };

    class MotherShahrazTanksPositionBossUnderPillarAction : public BtAttackAction
    {
    public:
        MotherShahrazTanksPositionBossUnderPillarAction(PlayerbotAI* ai) : BtAttackAction(ai, "mother shahraz tanks position boss under pillar") {}
        bool Execute(Event& event) override;
    };

    class MotherShahrazMeleeDpsWaitAtSafePositionAction : public BtMovementAction
    {
    public:
        MotherShahrazMeleeDpsWaitAtSafePositionAction(PlayerbotAI* ai) : BtMovementAction(ai, "mother shahraz melee dps wait at safe position") {}
        bool Execute(Event& event) override;
    };

    class MotherShahrazPositionRangedUnderPillarAction : public BtMovementAction
    {
    public:
        MotherShahrazPositionRangedUnderPillarAction(PlayerbotAI* ai) : BtMovementAction(ai, "mother shahraz position ranged under pillar") {}
        bool Execute(Event& event) override;
    };

    class MotherShahrazRunAwayToBreakFatalAttractionAction : public BtMovementAction
    {
    public:
        MotherShahrazRunAwayToBreakFatalAttractionAction(PlayerbotAI* ai) : BtMovementAction(ai, "mother shahraz run away to break fatal attraction") {}
        bool Execute(Event& event) override;
    };

    // Illidari Council

    class IllidariCouncilMisdirectBossesToTanksAction : public BtAttackAction
    {
    public:
        IllidariCouncilMisdirectBossesToTanksAction(PlayerbotAI* ai) : BtAttackAction(ai, "illidari council misdirect bosses to tanks") {}
        bool Execute(Event& event) override;
    };

    class IllidariCouncilMainTankPositionGathiosAction : public BtAttackAction
    {
    public:
        IllidariCouncilMainTankPositionGathiosAction(PlayerbotAI* ai) : BtAttackAction(ai, "illidari council main tank position gathios") {}
        bool Execute(Event& event) override;
    };

    class IllidariCouncilMainTankReflectJudgementOfCommandAction : public Action
    {
    public:
        IllidariCouncilMainTankReflectJudgementOfCommandAction(PlayerbotAI* ai) : Action(ai, "illidari council main tank reflect judgement of command") {}
        bool Execute(Event& event) override;
    };

    class IllidariCouncilFirstAssistTankFocusMalandeAction : public BtAttackAction
    {
    public:
        IllidariCouncilFirstAssistTankFocusMalandeAction(PlayerbotAI* ai) : BtAttackAction(ai, "illidari council first assist tank focus malande") {}
        bool Execute(Event& event) override;
    };

    class IllidariCouncilSecondAssistTankPositionDarkshadowAction : public BtAttackAction
    {
    public:
        IllidariCouncilSecondAssistTankPositionDarkshadowAction(PlayerbotAI* ai) : BtAttackAction(ai, "illidari council second assist tank position darkshadow") {}
        bool Execute(Event& event) override;
    };

    class IllidariCouncilMageTankPositionZerevorAction : public BtAttackAction
    {
    public:
        IllidariCouncilMageTankPositionZerevorAction(PlayerbotAI* ai) : BtAttackAction(ai, "illidari council mage tank position zerevor") {}
        bool Execute(Event& event) override;
    };

    class IllidariCouncilPositionMageTankHealerAction : public BtMovementAction
    {
    public:
        IllidariCouncilPositionMageTankHealerAction(PlayerbotAI* ai) : BtMovementAction(ai, "illidari council position mage tank healer") {}
        bool Execute(Event& event) override;
    };

    class IllidariCouncilAssignDpsTargetsAction : public BtAttackAction
    {
    public:
        IllidariCouncilAssignDpsTargetsAction(PlayerbotAI* ai) : BtAttackAction(ai, "illidari council assign dps targets") {}
        bool Execute(Event& event) override;
    };

    class IllidariCouncilDisperseRangedAction : public BtMovementAction
    {
    public:
        IllidariCouncilDisperseRangedAction(PlayerbotAI* ai) : BtMovementAction(ai, "illidari council disperse ranged") {}
        bool Execute(Event& event) override;
    };

    class IllidariCouncilCommandPetsToAttackGathiosAction : public BtAttackAction
    {
    public:
        IllidariCouncilCommandPetsToAttackGathiosAction(PlayerbotAI* ai) : BtAttackAction(ai, "illidari council command pets to attack gathios") {}
        bool Execute(Event& event) override;
    };

    class IllidariCouncilManageDpsTimerAction : public Action
    {
    public:
        IllidariCouncilManageDpsTimerAction(PlayerbotAI* ai) : Action(ai, "illidari council manage dps timer") {}
        bool Execute(Event& event) override;
    };

    // Illidan Stormrage <The Betrayer>

    class IllidanStormrageMisdirectToTankAction : public BtAttackAction
    {
    public:
        IllidanStormrageMisdirectToTankAction(PlayerbotAI* ai) : BtAttackAction(ai, "illidan stormrage misdirect to tank") {}
        bool Execute(Event& event) override;

    private:
        bool TryMisdirectToFlameTanks();
        bool TryMisdirectToWarlockTank(Unit* illidan);
    };

    class IllidanStormrageMainTankRepositionBossAction : public BtAttackAction
    {
    public:
        IllidanStormrageMainTankRepositionBossAction(PlayerbotAI* ai) : BtAttackAction(ai, "illidan stormrage main tank reposition boss") {}
        bool Execute(Event& event) override;

    private:
        bool MoveToShadowTrap(GameObject* trap);
        bool FindSafestNearbyPosition(const std::vector<Unit*>& flameCrashes, float maxRadius, float hazardRadius, float& outX, float& outY);
        bool IsPathSafe(float endX, float endY, const std::vector<Unit*>& flameCrashes, float hazardRadius);

        ObjectGuid trapGuid;
        float trapDestX = 0.0f;
        float trapDestY = 0.0f;
        float trapDestZ = 0.0f;
    };

    class IllidanStormrageIsolateBotWithParasiteAction : public BtMovementAction
    {
    public:
        IllidanStormrageIsolateBotWithParasiteAction(PlayerbotAI* ai) : BtMovementAction(ai, "illidan stormrage isolate bot with parasite") {}
        bool Execute(Event& event) override;

    private:
        bool FreezeTrapShadowfiend(float targetX, float targetY, float targetZ);
    };

    class IllidanStormrageSetEarthbindTotemAction : public Action
    {
    public:
        IllidanStormrageSetEarthbindTotemAction(PlayerbotAI* ai) : Action(ai, "illidan stormrage set earthbind totem") {}
        bool Execute(Event& event) override;
    };

    class IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction : public BtAttackAction
    {
    public:
        IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction(PlayerbotAI* ai) : BtAttackAction(ai, "illidan stormrage assist tanks handle flames of azzinoth") {}
        bool Execute(Event& event) override;

    private:
        bool RepositionToAvoidEyeBlast(const bt::EyeBlastDangerArea& dangerArea);
        bool RepositionToAvoidBlaze(Unit* eastFlame, Unit* westFlame);
    };

    class IllidanStormrageControlPetAggressionAction : public Action
    {
    public:
        IllidanStormrageControlPetAggressionAction(PlayerbotAI* ai) : Action(ai, "illidan stormrage control pet aggression") {}
        bool Execute(Event& event) override;
    };

    class IllidanStormragePositionAboveGrateAction : public BtMovementAction
    {
    public:
        IllidanStormragePositionAboveGrateAction(PlayerbotAI* ai) : BtMovementAction(ai, "illidan stormrage position above grate") {}
        bool Execute(Event& event) override;
    };

    class IllidanStormrageRemoveDarkBarrageAction : public Action
    {
    public:
        IllidanStormrageRemoveDarkBarrageAction(PlayerbotAI* ai) : Action(ai, "illidan stormrage remove dark barrage") {}
        bool Execute(Event& event) override;
    };

    class IllidanStormrageMoveAwayFromLandingPointAction : public BtMovementAction
    {
    public:
        IllidanStormrageMoveAwayFromLandingPointAction(PlayerbotAI* ai) : BtMovementAction(ai, "illidan stormrage move away from landing point") {}
        bool Execute(Event& event) override;
    };

    class IllidanStormrageDisperseRangedAction : public BtMovementAction
    {
    public:
        IllidanStormrageDisperseRangedAction(PlayerbotAI* ai) : BtMovementAction(ai, "illidan stormrage disperse ranged") {}
        bool Execute(Event& event) override;

    private:
        bool FanOutBehindInHumanPhase(Unit* illidan);
        bool SpreadInCircleInDemonPhase(Unit* illidan);
    };

    class IllidanStormrageMeleeGoSomewhereToNotDieAction : public BtMovementAction
    {
    public:
        IllidanStormrageMeleeGoSomewhereToNotDieAction(PlayerbotAI* ai) : BtMovementAction(ai, "illidan stormrage melee go somewhere to not die") {}
        bool Execute(Event& event) override;
    };

    class IllidanStormrageWarlockTankHandleDemonBossAction : public BtAttackAction
    {
    public:
        IllidanStormrageWarlockTankHandleDemonBossAction(PlayerbotAI* ai) : BtAttackAction(ai, "illidan stormrage warlock tank handle demon boss") {}
        bool Execute(Event& event) override;
    };

    class IllidanStormrageDpsPrioritizeAddsAction : public BtAttackAction
    {
    public:
        IllidanStormrageDpsPrioritizeAddsAction(PlayerbotAI* ai) : BtAttackAction(ai, "illidan stormrage dps prioritize adds") {}
        bool Execute(Event& event) override;
    };

    class IllidanStormrageManageDpsTimerAndRtiAction : public Action
    {
    public:
        IllidanStormrageManageDpsTimerAndRtiAction(PlayerbotAI* ai) : Action(ai, "illidan stormrage manage dps timer and rti") {}
        bool Execute(Event& event) override;
    };
}
