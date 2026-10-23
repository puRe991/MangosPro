// Ported from mod-playerbots (src/Ai/Raid/SSC/SSCActions.h, GPL v2+) and adapted for this core.
#pragma once
#include "playerbot/strategy/actions/AttackAction.h"
#include "playerbot/strategy/actions/MovementActions.h"
#include "playerbot/strategy/actions/UseItemAction.h"
#include "playerbot/WorldPosition.h"
#include <vector>

namespace ai
{
#define SSC_ACTION(className, baseClass, actionName) \
    class className : public baseClass \
    { \
    public: \
        className(PlayerbotAI* ai) : baseClass(ai, actionName) {} \
        bool Execute(Event& event) override; \
    };

    // Base for positioning actions
    class SscMovementAction : public MovementAction
    {
    public:
        SscMovementAction(PlayerbotAI* ai, std::string name) : MovementAction(ai, name) {}

    protected:
        bool MoveToPosition(const WorldPosition& position);
        bool MoveToPosition(float x, float y, float z) { return MoveToPosition(WorldPosition(bot->GetMapId(), x, y, z)); }
        // Move to 'distance' yards from the point, on the line through the bot (FleePosition / MoveAway)
        bool MoveAwayFrom(float x, float y, float distance);
        bool MoveAwayFrom(WorldObject* object, float distance) { return MoveAwayFrom(object->GetPositionX(), object->GetPositionY(), distance); }
    };

    // Base for actions which attack and position the target
    class SscAttackAction : public AttackAction
    {
    public:
        SscAttackAction(PlayerbotAI* ai, std::string name) : AttackAction(ai, name) {}

    protected:
        bool AttackUnit(Unit* target);
        bool IsCurrentTarget(Unit* target);
        bool MoveToPosition(const WorldPosition& position);
        bool MoveAwayFrom(WorldObject* object, float distance);
        // Tank drags the boss towards a position in short steps (re-evaluated every tick)
        bool StepToPosition(const WorldPosition& position, float arrivalDistance, float maxStep);
    };

    // General
    SSC_ACTION(SscEraseTimersAndTrackersAction, Action, "serpent shrine cavern erase timers and trackers")

    // Trash
    SSC_ACTION(SscUnderbogColossusEscapeToxicPoolAction, SscMovementAction, "underbog colossus escape toxic pool")
    SSC_ACTION(SscGreyheartTidecallerMarkWaterElementalTotemAction, Action, "greyheart tidecaller mark water elemental totem")

    // Hydross the Unstable <Duke of Currents>
    SSC_ACTION(SscHydrossPositionFrostTankAction, SscAttackAction, "hydross the unstable position frost tank")
    SSC_ACTION(SscHydrossPositionNatureTankAction, SscAttackAction, "hydross the unstable position nature tank")
    SSC_ACTION(SscHydrossPrioritizeElementalAddsAction, SscAttackAction, "hydross the unstable prioritize elemental adds")
    SSC_ACTION(SscHydrossMisdirectBossToTankAction, Action, "hydross the unstable misdirect boss to tank")
    SSC_ACTION(SscHydrossStopDpsUponPhaseChangeAction, Action, "hydross the unstable stop dps upon phase change")
    SSC_ACTION(SscHydrossManageTimersAction, Action, "hydross the unstable manage timers")

    class SscHydrossFrostPhaseSpreadOutAction : public SscMovementAction
    {
    public:
        SscHydrossFrostPhaseSpreadOutAction(PlayerbotAI* ai) : SscMovementAction(ai, "hydross the unstable frost phase spread out") {}
        bool Execute(Event& event) override;

    private:
        uint32 lastMoveTime = 0;
    };

    // The Lurker Below
    SSC_ACTION(SscLurkerRunAroundBehindBossAction, SscMovementAction, "the lurker below run around behind boss")
    SSC_ACTION(SscLurkerPositionMainTankAction, SscAttackAction, "the lurker below position main tank")
    SSC_ACTION(SscLurkerSpreadRangedInArcAction, SscMovementAction, "the lurker below spread ranged in arc")
    SSC_ACTION(SscLurkerTanksPickUpAddsAction, SscAttackAction, "the lurker below tanks pick up adds")
    SSC_ACTION(SscLurkerManageSpoutTimerAction, Action, "the lurker below manage spout timer")

    // Leotheras the Blind
    SSC_ACTION(SscLeotherasTargetSpellbindersAction, Action, "leotheras the blind target spellbinders")
    SSC_ACTION(SscLeotherasDemonFormTankAttackBossAction, SscAttackAction, "leotheras the blind demon form tank attack boss")
    SSC_ACTION(SscLeotherasMeleeTanksDontAttackDemonFormAction, Action, "leotheras the blind melee tanks don't attack demon form")
    SSC_ACTION(SscLeotherasRunAwayFromWhirlwindAction, SscMovementAction, "leotheras the blind run away from whirlwind")
    SSC_ACTION(SscLeotherasMeleeDpsRunAwayFromBossAction, SscMovementAction, "leotheras the blind melee dps run away from boss")
    SSC_ACTION(SscLeotherasFinalPhaseAssignDpsPriorityAction, SscAttackAction, "leotheras the blind final phase assign dps priority")
    SSC_ACTION(SscLeotherasMisdirectBossToDemonFormTankAction, Action, "leotheras the blind misdirect boss to demon form tank")
    SSC_ACTION(SscLeotherasManageDpsWaitTimersAction, Action, "leotheras the blind manage dps wait timers")
    SSC_ACTION(SscLeotherasHealDemonFormTankAction, SscMovementAction, "leotheras the blind heal demon form tank")
    SSC_ACTION(SscLeotherasAttackMindControlledPlayerAction, SscAttackAction, "leotheras the blind attack mind controlled player")

    class SscLeotherasPositionRangedAction : public SscMovementAction
    {
    public:
        SscLeotherasPositionRangedAction(PlayerbotAI* ai) : SscMovementAction(ai, "leotheras the blind position ranged") {}
        bool Execute(Event& event) override;

    private:
        uint32 lastMoveTime = 0;
    };

    class SscLeotherasDestroyInnerDemonAction : public SscAttackAction
    {
    public:
        SscLeotherasDestroyInnerDemonAction(PlayerbotAI* ai) : SscAttackAction(ai, "leotheras the blind destroy inner demon") {}
        bool Execute(Event& event) override;

    private:
        bool HandleFeralTankStrategy(Unit* innerDemon);
        bool HandleHealerStrategy(Unit* innerDemon);
    };

    // Fathom-Lord Karathress
    SSC_ACTION(SscKarathressMainTankPositionBossAction, SscAttackAction, "fathom-lord karathress main tank position boss")
    SSC_ACTION(SscKarathressFirstAssistTankPositionCaribdisAction, SscAttackAction, "fathom-lord karathress first assist tank position caribdis")
    SSC_ACTION(SscKarathressSecondAssistTankPositionSharkkisAction, SscAttackAction, "fathom-lord karathress second assist tank position sharkkis")
    SSC_ACTION(SscKarathressThirdAssistTankPositionTidalvessAction, SscAttackAction, "fathom-lord karathress third assist tank position tidalvess")
    SSC_ACTION(SscKarathressPositionCaribdisTankHealerAction, SscMovementAction, "fathom-lord karathress position caribdis tank healer")
    SSC_ACTION(SscKarathressMisdirectBossesToTanksAction, Action, "fathom-lord karathress misdirect bosses to tanks")
    SSC_ACTION(SscKarathressAssignDpsPriorityAction, SscAttackAction, "fathom-lord karathress assign dps priority")
    SSC_ACTION(SscKarathressManageDpsTimerAction, Action, "fathom-lord karathress manage dps timer")

    // Morogrim Tidewalker
    SSC_ACTION(SscMorogrimMisdirectBossToMainTankAction, Action, "morogrim tidewalker misdirect boss to main tank")
    SSC_ACTION(SscMorogrimMoveBossToTankPositionAction, SscAttackAction, "morogrim tidewalker move boss to tank position")
    SSC_ACTION(SscMorogrimPhase2RepositionRangedAction, SscMovementAction, "morogrim tidewalker phase 2 reposition ranged")

    // Lady Vashj <Coilfang Matron>
    SSC_ACTION(SscVashjMainTankPositionBossAction, SscAttackAction, "lady vashj main tank position boss")
    SSC_ACTION(SscVashjPhase1SpreadRangedInArcAction, SscMovementAction, "lady vashj phase 1 spread ranged in arc")
    SSC_ACTION(SscVashjSetGroundingTotemInMainTankGroupAction, SscMovementAction, "lady vashj set grounding totem in main tank group")
    SSC_ACTION(SscVashjMisdirectBossToMainTankAction, Action, "lady vashj misdirect boss to main tank")
    SSC_ACTION(SscVashjStaticChargeMoveAwayFromGroupAction, SscMovementAction, "lady vashj static charge move away from group")
    SSC_ACTION(SscVashjLootTaintedCoreAction, SscAttackAction, "lady vashj loot tainted core")
    SSC_ACTION(SscVashjDestroyTaintedCoreAction, Action, "lady vashj destroy tainted core")
    SSC_ACTION(SscVashjAssignPhase2AndPhase3DpsPriorityAction, SscAttackAction, "lady vashj assign phase 2 and phase 3 dps priority")
    SSC_ACTION(SscVashjTankAttackAndMoveAwayStriderAction, SscAttackAction, "lady vashj tank attack and move away strider")
    SSC_ACTION(SscVashjUseFreeActionAbilitiesAction, Action, "lady vashj use free action abilities")

    class SscVashjAvoidToxicSporesAction : public SscMovementAction
    {
    public:
        SscVashjAvoidToxicSporesAction(PlayerbotAI* ai) : SscMovementAction(ai, "lady vashj avoid toxic spores") {}
        bool Execute(Event& event) override;

    private:
        WorldPosition FindSafestNearbyPosition(const std::vector<Unit*>& spores, float maxRadius, float hazardRadius);
        bool IsPathSafeFromSpores(const WorldPosition& start, const WorldPosition& end, const std::vector<Unit*>& spores, float hazardRadius);
    };

    // Throws the Tainted Core (item spell Throw Key) along the line of passers and
    // uses it on the nearest active Shield Generator
    class SscTaintedCoreThrower : public UseAction
    {
    public:
        SscTaintedCoreThrower(PlayerbotAI* ai) : UseAction(ai, "ssc tainted core thrower") {}
        using UseAction::UseItem;
    };

    class SscVashjPassTheTaintedCoreAction : public SscMovementAction
    {
    public:
        SscVashjPassTheTaintedCoreAction(PlayerbotAI* ai) : SscMovementAction(ai, "lady vashj pass the tainted core"), thrower(ai) {}
        bool Execute(Event& event) override;

    private:
        SscTaintedCoreThrower thrower;

        bool MoveToLineup(const WorldPosition& position);
        bool LineUpFirstCorePasser(Player* designatedLooter);
        bool LineUpNextCorePasser(Player* previousPasser, Player* beforePreviousPasser, GameObject* generator, bool last);
        bool IsPasserInPosition(Player* passer);
        bool UseCoreOnGenerator(GameObject* generator);
        bool ThrowCore(Player* receiver);
    };

#undef SSC_ACTION
}
