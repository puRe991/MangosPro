// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/TK/TKActions) and adapted for this core.
#pragma once
#include "playerbot/strategy/actions/MovementActions.h"
#include "playerbot/strategy/actions/AttackAction.h"
#include "TkHelpers.h"
#include <vector>

namespace ai
{
    // Movement helpers shared by the movement and attack actions of Tempest Keep
    // (MoveAway / FleePosition / MoveFromGroup / MoveInside of mod-playerbots)
    template <class Base>
    class TkMoveMixin : public Base
    {
    public:
        TkMoveMixin(PlayerbotAI* ai, std::string name) : Base(ai, name) {}

    protected:
        bool MoveToPosition(float x, float y, float z)
        {
            return this->MoveTo(tk::TK_MAP_ID, x, y, z, false, false, false, true);
        }

        bool MoveToPosition(const WorldPosition& position)
        {
            return MoveToPosition(position.getX(), position.getY(), position.getZ());
        }

        bool MoveToPosition(const tk::TkPos& position)
        {
            return MoveToPosition(position.x, position.y, position.z);
        }

        // Move 'distance' yards away from a point
        bool MoveAwayFrom(float x, float y, float distance)
        {
            WorldPosition point;
            return distance > 0.0f && tk::GetPointAwayFrom(this->bot, x, y, distance, point) && MoveToPosition(point);
        }

        bool MoveAwayFrom(Unit* unit, float distance)
        {
            return unit && MoveAwayFrom(unit->GetPositionX(), unit->GetPositionY(), distance);
        }

        // Move until nobody of the group is closer than 'distance'
        bool MoveFromGroup(float distance, bool ignoreTanks = false)
        {
            WorldPosition point;
            return tk::GetPointAwayFromGroup(this->bot, distance, ignoreTanks, point) && MoveToPosition(point);
        }

        // Move into a circle around a position
        bool MoveInside(const tk::TkPos& center, float radius)
        {
            WorldPosition point;
            return tk::GetPointInside(this->bot, center, radius, point) && MoveToPosition(point);
        }

        // Jump off a balcony down to a ground position. MotionMaster::MoveJump never got a bot off the balcony
        // (the spline was restarted every tick and stopped by the bot movement handling), so the drop uses the
        // bots' own jump simulation like a knockback: PlayerbotAI lands the bot at the jump time.
        bool JumpTo(const tk::TkPos& position)
        {
            Player* bot = this->bot;
            PlayerbotAI* ai = this->ai;
            if (ai->IsJumping())
                return true; // keep the running drop, restarting it would hold the bot on the balcony

            bot->InterruptNonMeleeSpells(false);
            ai->StopMoving();

            const float dx = position.x - bot->GetPositionX();
            const float dy = position.y - bot->GetPositionY();
            const float distance = std::sqrt(dx * dx + dy * dy);
            const float height = std::max(0.0f, bot->GetPositionZ() - position.z);
            const float angle = std::atan2(dy, dx);
            constexpr float gravity = 19.2911f;
            const float timeToLand = std::max(std::sqrt(2.0f * height / gravity), distance / bot->GetSpeed(MOVE_RUN));

            float landX = position.x, landY = position.y, landZ = position.z + 0.5f;
            bot->UpdateAllowedPositionZ(landX, landY, landZ);
            ai->SetJumpDestination(WorldPosition(bot->GetMapId(), landX, landY, landZ));
            bot->SetFallInformation(0, bot->GetPositionZ());

            bot->SetOrientation(angle);
            bot->m_movementInfo.jump.zspeed = 0.0f;
            bot->m_movementInfo.jump.cosAngle = std::cos(angle);
            bot->m_movementInfo.jump.sinAngle = std::sin(angle);
            bot->m_movementInfo.jump.xyspeed = timeToLand > 0.0f ? distance / timeToLand : 0.0f;
            bot->m_movementInfo.SetMovementFlags(MovementFlags(MOVEFLAG_FALLING | MOVEFLAG_FORWARD));

            WorldPacket jump(MSG_MOVE_JUMP);
            jump << bot->m_movementInfo;
            ai->QueuePacket(jump);
            ai->SetJumpTime(sWorld.GetCurrentMSTime() + sWorld.GetAverageDiff() * 2 + uint32(timeToLand * IN_MILLISECONDS));

            // Leave the balcony right away: Flame Quills hits whoever is still standing up there
            const float midX = bot->GetPositionX() + dx * 0.5f;
            const float midY = bot->GetPositionY() + dy * 0.5f;
            const float midZ = bot->GetPositionZ() - height * 0.25f;
            if (bot->GetMap()->IsInLineOfSight(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ() + 2.0f, midX, midY, midZ + 2.0f, true))
                bot->Relocate(midX, midY, midZ);
            return true;
        }
    };

    typedef TkMoveMixin<MovementAction> TkMovementAction;
    typedef TkMoveMixin<AttackAction> TkAttackAction;

    // General

    class TkResetEncounterStatesAction : public Action
    {
    public:
        TkResetEncounterStatesAction(PlayerbotAI* ai) : Action(ai, "tempest keep reset encounter states") {}
        bool Execute(Event& event) override;
    };

    class TkClearStaleFallingFlagAction : public Action
    {
    public:
        TkClearStaleFallingFlagAction(PlayerbotAI* ai) : Action(ai, "tempest keep clear stale falling flag") {}
        bool Execute(Event& event) override;
        bool isUsefulWhenStunned() override { return true; }
    };

    // For Void Reaver, Sanguinar, Telonicus, and Kael'thas.
    class TkTankPositionAction : public TkAttackAction
    {
    public:
        TkTankPositionAction(PlayerbotAI* ai, std::string name) : TkAttackAction(ai, name) {}

    protected:
        // shouldAttack is false for a tank that is not assigned to the target but that is included
        // in the action solely to reposition if it ends up with aggro.
        bool MoveToTankPosition(Unit* target, const tk::TkPos& position, float tolerance, bool shouldAttack = true);
    };

    // Trash

    class TkCrimsonHandCenturionCastPolymorphAction : public Action
    {
    public:
        TkCrimsonHandCenturionCastPolymorphAction(PlayerbotAI* ai) : Action(ai, "crimson hand centurion cast polymorph") {}
        bool Execute(Event& event) override;
    };

    // Al'ar <Phoenix God>

    class TkAlarMisdirectBossToMainTankAction : public Action
    {
    public:
        TkAlarMisdirectBossToMainTankAction(PlayerbotAI* ai) : Action(ai, "al'ar misdirect boss to main tank") {}
        bool Execute(Event& event) override;
    };

    class TkAlarBossTanksMoveBetweenPlatformsAction : public TkAttackAction
    {
    public:
        TkAlarBossTanksMoveBetweenPlatformsAction(PlayerbotAI* ai) : TkAttackAction(ai, "al'ar boss tanks move between platforms") {}
        bool Execute(Event& event) override;
    };

    class TkAlarMeleeDpsMoveBetweenPlatformsAction : public TkMovementAction
    {
    public:
        TkAlarMeleeDpsMoveBetweenPlatformsAction(PlayerbotAI* ai) : TkMovementAction(ai, "al'ar melee dps move between platforms") {}
        bool Execute(Event& event) override;
    };

    class TkAlarRangedAndEmberTankMoveUnderPlatformsAction : public TkAttackAction
    {
    public:
        TkAlarRangedAndEmberTankMoveUnderPlatformsAction(PlayerbotAI* ai) : TkAttackAction(ai, "al'ar ranged and ember tank move under platforms") {}
        bool Execute(Event& event) override;
    };

    class TkAlarAssistTanksPickUpEmbersAction : public TkAttackAction
    {
    public:
        TkAlarAssistTanksPickUpEmbersAction(PlayerbotAI* ai) : TkAttackAction(ai, "al'ar assist tanks pick up embers") {}
        bool Execute(Event& event) override;

    private:
        bool HandlePhase1Embers(Unit* alar);
        bool HandlePhase2Embers();
    };

    class TkAlarAssignNonTankTargetAction : public TkAttackAction
    {
    public:
        TkAlarAssignNonTankTargetAction(PlayerbotAI* ai) : TkAttackAction(ai, "al'ar assign non-tank target") {}
        bool Execute(Event& event) override;
    };

    class TkAlarJumpFromPlatformAction : public TkMovementAction
    {
    public:
        TkAlarJumpFromPlatformAction(PlayerbotAI* ai) : TkMovementAction(ai, "al'ar jump from platform") {}
        bool Execute(Event& event) override;
    };

    class TkAlarMoveAwayFromRebirthAction : public TkMovementAction
    {
    public:
        TkAlarMoveAwayFromRebirthAction(PlayerbotAI* ai) : TkMovementAction(ai, "al'ar move away from rebirth") {}
        bool Execute(Event& event) override;
    };

    class TkAlarSwapTanksOnBossAction : public TkAttackAction
    {
    public:
        TkAlarSwapTanksOnBossAction(PlayerbotAI* ai) : TkAttackAction(ai, "al'ar swap tanks on boss") {}
        bool Execute(Event& event) override;
    };

    class TkAlarAvoidFlamePatchesAndDiveBombsAction : public TkMovementAction
    {
    public:
        TkAlarAvoidFlamePatchesAndDiveBombsAction(PlayerbotAI* ai) : TkMovementAction(ai, "al'ar avoid flame patches and dive bombs") {}
        bool Execute(Event& event) override;

    private:
        bool AvoidFlamePatch();
        bool HandleDiveBomb(Unit* alar);
        WorldPosition FindSafestNearbyPosition(const std::vector<Unit*>& flamePatches, float hazardRadius);
        bool IsPathSafe(float startX, float startY, float endX, float endY, const std::vector<Unit*>& flamePatches, float hazardRadius);
    };

    class TkAlarManagePhaseTrackerAction : public Action
    {
    public:
        TkAlarManagePhaseTrackerAction(PlayerbotAI* ai) : Action(ai, "al'ar manage phase tracker") {}
        bool Execute(Event& event) override;
    };

    // Void Reaver

    class TkVoidReaverTanksPositionBossAction : public TkTankPositionAction
    {
    public:
        TkVoidReaverTanksPositionBossAction(PlayerbotAI* ai) : TkTankPositionAction(ai, "void reaver tanks position boss") {}
        bool Execute(Event& event) override;
    };

    class TkVoidReaverUseAggroDumpAbilityAction : public Action
    {
    public:
        TkVoidReaverUseAggroDumpAbilityAction(PlayerbotAI* ai) : Action(ai, "void reaver use aggro dump ability") {}
        bool Execute(Event& event) override;
    };

    class TkVoidReaverRangedBackOffAndSpreadAction : public TkMovementAction
    {
    public:
        TkVoidReaverRangedBackOffAndSpreadAction(PlayerbotAI* ai) : TkMovementAction(ai, "void reaver ranged back off and spread") {}
        bool Execute(Event& event) override;

    private:
        bool GetRangedSlot(Unit* voidReaver, float radius, WorldPosition& slot);
    };

    class TkVoidReaverAvoidArcaneOrbAction : public TkMovementAction
    {
    public:
        TkVoidReaverAvoidArcaneOrbAction(PlayerbotAI* ai) : TkMovementAction(ai, "void reaver avoid arcane orb") {}
        bool Execute(Event& event) override;
    };

    // High Astromancer Solarian

    class TkSolarianMainTankPickUpBossAction : public TkAttackAction
    {
    public:
        TkSolarianMainTankPickUpBossAction(PlayerbotAI* ai) : TkAttackAction(ai, "high astromancer solarian main tank pick up boss") {}
        bool Execute(Event& event) override;
    };

    class TkSolarianMoveAwayFromGroupAction : public TkMovementAction
    {
    public:
        TkSolarianMoveAwayFromGroupAction(PlayerbotAI* ai) : TkMovementAction(ai, "high astromancer solarian move away from group") {}
        bool Execute(Event& event) override;
    };

    class TkSolarianTargetSolariumPriestsAction : public TkAttackAction
    {
    public:
        TkSolarianTargetSolariumPriestsAction(PlayerbotAI* ai) : TkAttackAction(ai, "high astromancer solarian target solarium priests") {}
        bool Execute(Event& event) override;

    private:
        std::vector<Player*> GetMeleeBots();
        Unit* AssignSolariumPriestsToMeleeBots(const std::pair<Unit*, Unit*>& priestsPair, const std::vector<Player*>& meleeMembers);
    };

    // Kael'thas Sunstrider <Lord of the Blood Elves>

    class TkKaelthasKiteThaladredAction : public TkMovementAction
    {
    public:
        TkKaelthasKiteThaladredAction(PlayerbotAI* ai) : TkMovementAction(ai, "kael'thas sunstrider kite thaladred") {}
        bool Execute(Event& event) override;
    };

    class TkKaelthasMisdirectAdvisorsToTanksAction : public Action
    {
    public:
        TkKaelthasMisdirectAdvisorsToTanksAction(PlayerbotAI* ai) : Action(ai, "kael'thas sunstrider misdirect advisors to tanks") {}
        bool Execute(Event& event) override;
    };

    class TkKaelthasMeleeTanksPositionAdvisorsAction : public TkTankPositionAction
    {
    public:
        TkKaelthasMeleeTanksPositionAdvisorsAction(PlayerbotAI* ai) : TkTankPositionAction(ai, "kael'thas sunstrider melee tanks position advisors") {}
        bool Execute(Event& event) override;
    };

    class TkKaelthasWarlockTankPositionCapernianAction : public TkAttackAction
    {
    public:
        TkKaelthasWarlockTankPositionCapernianAction(PlayerbotAI* ai) : TkAttackAction(ai, "kael'thas sunstrider warlock tank position capernian") {}
        bool Execute(Event& event) override;
    };

    class TkKaelthasSpreadAndMoveAwayFromCapernianAction : public TkMovementAction
    {
    public:
        TkKaelthasSpreadAndMoveAwayFromCapernianAction(PlayerbotAI* ai) : TkMovementAction(ai, "kael'thas sunstrider spread and move away from capernian") {}
        bool Execute(Event& event) override;

    private:
        bool RangedBotsDisperse(uint32 phase, Unit* capernian);
        bool MeleeStayBackFromCapernian(Unit* capernian);
    };

    class TkKaelthasHandleAdvisorRolesInPhase3Action : public TkMovementAction
    {
    public:
        TkKaelthasHandleAdvisorRolesInPhase3Action(PlayerbotAI* ai) : TkMovementAction(ai, "kael'thas sunstrider handle advisor roles in phase 3") {}
        bool Execute(Event& event) override;
    };

    class TkKaelthasAssignAdvisorDpsPriorityAction : public TkAttackAction
    {
    public:
        TkKaelthasAssignAdvisorDpsPriorityAction(PlayerbotAI* ai) : TkAttackAction(ai, "kael'thas sunstrider assign advisor dps priority") {}
        bool Execute(Event& event) override;

    private:
        bool MeleeDpsPositionOutsideBombRange(Unit* telonicus);
    };

    class TkKaelthasManageAdvisorDpsTimerAction : public Action
    {
    public:
        TkKaelthasManageAdvisorDpsTimerAction(PlayerbotAI* ai) : Action(ai, "kael'thas sunstrider manage advisor dps timer") {}
        bool Execute(Event& event) override;
    };

    class TkKaelthasAssignLegendaryWeaponDpsPriorityAction : public TkAttackAction
    {
    public:
        TkKaelthasAssignLegendaryWeaponDpsPriorityAction(PlayerbotAI* ai) : TkAttackAction(ai, "kael'thas sunstrider assign legendary weapon dps priority") {}
        bool Execute(Event& event) override;

    private:
        bool HandleDevastationAvoidance(Unit* axe, Unit* mace, Unit* dagger, Unit* sword, bool isTank, bool isMeleeDps);
    };

    class TkKaelthasMoveDevastationAwayAction : public TkAttackAction
    {
    public:
        TkKaelthasMoveDevastationAwayAction(PlayerbotAI* ai) : TkAttackAction(ai, "kael'thas sunstrider move devastation away") {}
        bool Execute(Event& event) override;
    };

    class TkKaelthasLootLegendaryWeaponsAction : public TkMovementAction
    {
    public:
        TkKaelthasLootLegendaryWeaponsAction(PlayerbotAI* ai) : TkMovementAction(ai, "kael'thas sunstrider loot legendary weapons") {}
        bool Execute(Event& event) override;

    private:
        bool ShouldBotLootWeapon(uint32 weaponEntry);
        bool LootWeapon(uint32 weaponEntry, uint32 itemId);
        bool EquipLegendaryWeapon(uint32 itemId);
    };

    class TkKaelthasUseLegendaryWeaponsAction : public Action
    {
    public:
        TkKaelthasUseLegendaryWeaponsAction(PlayerbotAI* ai) : Action(ai, "kael'thas sunstrider use legendary weapons") {}
        bool Execute(Event& event) override;

    private:
        bool UsePhaseshiftBulwark();
        bool UseStaffOfDisintegration();
        bool UseNetherstrandLongbow();
        bool UseEquippedItem(Item* item);
    };

    class TkKaelthasReequipGearAction : public Action
    {
    public:
        TkKaelthasReequipGearAction(PlayerbotAI* ai) : Action(ai, "kael'thas sunstrider reequip gear") {}
        bool Execute(Event& event) override;
    };

    class TkKaelthasTanksPositionBossAction : public TkTankPositionAction
    {
    public:
        TkKaelthasTanksPositionBossAction(PlayerbotAI* ai) : TkTankPositionAction(ai, "kael'thas sunstrider tanks position boss") {}
        bool Execute(Event& event) override;
    };

    class TkKaelthasAvoidFlameStrikeAction : public TkMovementAction
    {
    public:
        TkKaelthasAvoidFlameStrikeAction(PlayerbotAI* ai) : TkMovementAction(ai, "kael'thas sunstrider avoid flame strike") {}
        bool Execute(Event& event) override;
    };

    class TkKaelthasAssignFinalPhaseTargetAction : public TkAttackAction
    {
    public:
        TkKaelthasAssignFinalPhaseTargetAction(PlayerbotAI* ai) : TkAttackAction(ai, "kael'thas sunstrider assign final phase target") {}
        bool Execute(Event& event) override;

    private:
        Unit* GetAssignedPhoenix();
        bool AssistTankPicksUpPhoenix(Unit* phoenix);
        bool NonTanksAssignTargetAndAvoidPhoenixes();
    };

    class TkKaelthasBreakMindControlAction : public TkAttackAction
    {
    public:
        TkKaelthasBreakMindControlAction(PlayerbotAI* ai) : TkAttackAction(ai, "kael'thas sunstrider break mind control") {}
        bool Execute(Event& event) override;
    };

    class TkKaelthasSpreadOutInMidairAction : public TkMovementAction
    {
    public:
        TkKaelthasSpreadOutInMidairAction(PlayerbotAI* ai) : TkMovementAction(ai, "kael'thas sunstrider spread out in midair") {}
        bool Execute(Event& event) override;
        bool isPossible() override { return true; }

    private:
        bool DropToGround();
        bool HoverAndSpread();
    };
}
