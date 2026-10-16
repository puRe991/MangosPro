// Ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara/KaraActions) and adapted for this core.
#include "playerbot/playerbot.h"
#include "KarazhanActions.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/values/LastMovementValue.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>

using namespace ai;
using namespace ai::karazhan;

// ---------------------------------------------------------------------------
// Movement helpers
// ---------------------------------------------------------------------------

template <class Base>
bool KarazhanMoveHelper<Base>::MoveAwayFromPoint(float fromX, float fromY, float distance)
{
    Player* bot = this->bot;
    float dx = bot->GetPositionX() - fromX;
    float dy = bot->GetPositionY() - fromY;
    const float current = std::sqrt(dx * dx + dy * dy);
    if (current >= distance)
        return false;

    const float baseAngle = current > 0.1f ? std::atan2(dy, dx) : bot->GetOrientation() + M_PI_F;
    // Straight away first, then turn a bit to the sides if a wall is in the way
    for (float offset : { 0.0f, 0.5f, -0.5f, 1.0f, -1.0f, 1.5f, -1.5f })
    {
        const float angle = baseAngle + offset;
        const float x = fromX + std::cos(angle) * distance;
        const float y = fromY + std::sin(angle) * distance;
        float z = 0.0f;
        if (GetValidGroundPoint(bot, x, y, z))
            return MoveToPoint(x, y, z);
    }

    return false;
}

template <class Base>
bool KarazhanMoveHelper<Base>::MoveToDistanceFrom(Unit* target, float distance)
{
    Player* bot = this->bot;
    const float dx = bot->GetPositionX() - target->GetPositionX();
    const float dy = bot->GetPositionY() - target->GetPositionY();
    const float length = std::sqrt(dx * dx + dy * dy);
    if (length < 0.1f)
        return false;

    const float x = target->GetPositionX() + dx / length * distance;
    const float y = target->GetPositionY() + dy / length * distance;
    float z = bot->GetPositionZ();
    if (!GetValidGroundPoint(bot, x, y, z))
        z = target->GetPositionZ();

    return MoveToPoint(x, y, z);
}

template <class Base>
bool KarazhanMoveHelper<Base>::MoveAwayFromGroup(float distance)
{
    Player* bot = this->bot;
    float sumX = 0.0f;
    float sumY = 0.0f;
    uint32 count = 0;
    for (Player* member : raidboss::GetGroupMembers(bot, distance, false))
    {
        sumX += member->GetPositionX();
        sumY += member->GetPositionY();
        ++count;
    }

    if (!count)
        return false;

    return MoveAwayFromPoint(sumX / count, sumY / count, distance);
}

template <class Base>
bool KarazhanMoveHelper<Base>::StepToPosition(const KaraPosition& position, float arrivalDist, Unit* boss)
{
    Player* bot = this->bot;
    float stepX = 0.0f;
    float stepY = 0.0f;
    bool backwards = false;
    if (!raidboss::GetStepToPosition(bot, WorldPosition(bot->GetMapId(), position.x, position.y, position.z), arrivalDist,
                                     boss, stepX, stepY, backwards))
        return false;

    return MoveToPoint(stepX, stepY, position.z);
}

template class ai::KarazhanMoveHelper<MovementAction>;
template class ai::KarazhanMoveHelper<AttackAction>;

namespace
{
    Player* GetRequester(PlayerbotAI* ai, Event& event)
    {
        return event.getOwner() ? event.getOwner() : ai->GetMaster();
    }

    void ClearCurrentTarget(PlayerbotAI* ai)
    {
        ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Set(ObjectGuid());
        ai->GetBot()->SetSelectionGuid(ObjectGuid());
        ai->GetBot()->AttackStop();
    }

    template <class T>
    T* GetKaraAction(AiObjectContext* context, const std::string& name)
    {
        return dynamic_cast<T*>(context->GetAction(name));
    }
}

// ---------------------------------------------------------------------------
// General
// ---------------------------------------------------------------------------

bool KarazhanResetEncounterStatesAction::Execute(Event& event)
{
    const uint32 instanceId = bot->GetInstanceId();
    const bool isMechanicTracker = raidboss::IsMechanicTrackerBot(bot, KARAZHAN_MAP_ID);
    bool reset = false;

    auto isEngaged = [&](uint32 entry)
    {
        Unit* unit = GetKaraUnit(ai, entry);
        return unit && unit->IsInCombat();
    };

    if (isMechanicTracker)
    {
        if (!isEngaged(NPC_MIDNIGHT) && !isEngaged(NPC_ATTUMEN_MOUNTED) && ClearTimer(instanceId, KaraTimer::ATTUMEN_DPS_WAIT))
            reset = true;

        if (!isEngaged(NPC_NIGHTBANE))
        {
            reset |= ClearTimer(instanceId, KaraTimer::NIGHTBANE_DPS_WAIT);
            reset |= ClearTimer(instanceId, KaraTimer::NIGHTBANE_FLIGHT_START);
        }
    }

    if (!isEngaged(NPC_BIG_BAD_WOLF))
    {
        if (auto* action = GetKaraAction<KarazhanBigBadWolfRunAwayAction>(context, "big bad wolf little red riding hood run away"))
            reset |= action->ResetRunIndex();
    }

    if (!isEngaged(NPC_NIGHTBANE))
    {
        if (auto* action = GetKaraAction<KarazhanNightbaneFlightPhaseStackAction>(context, "nightbane flight phase stack and move"))
            reset |= action->ResetRainOfBonesHit();
    }

    if (!isEngaged(NPC_NETHERSPITE))
    {
        if (isMechanicTracker)
        {
            reset |= ClearTimer(instanceId, KaraTimer::NETHERSPITE_DPS_WAIT);
            ClearBeamBlockers(instanceId, reset);
        }

        if (auto* action = GetKaraAction<KarazhanNetherspiteBlockRedBeamAction>(context, "netherspite block red beam"))
            reset |= action->ResetRedBeamState();
        if (auto* action = GetKaraAction<KarazhanNetherspiteBlockBlueBeamAction>(context, "netherspite block blue beam"))
            reset |= action->ResetBlueBeamState();
        if (auto* action = GetKaraAction<KarazhanNetherspiteBlockGreenBeamAction>(context, "netherspite block green beam"))
            reset |= action->ResetGreenBeamState();
    }

    return reset;
}

bool KarazhanSetTremorTotemAction::Execute(Event& event)
{
    return raidboss::TryCast(ai, "tremor totem", bot);
}

// ---------------------------------------------------------------------------
// Trash
// ---------------------------------------------------------------------------

bool KarazhanManaWarpStunAction::Execute(Event& event)
{
    Unit* target = nullptr;
    for (Unit* manaWarp : GetKaraUnits(ai, NPC_MANA_WARP))
    {
        if (manaWarp->GetHealthPercent() > 15.0f || raidboss::ExactDist2d(bot, manaWarp) > 40.0f)
            continue;

        // Lowest guid: all bots stun the same Mana Warp
        if (!target || manaWarp->GetObjectGuid() < target->GetObjectGuid())
            target = manaWarp;
    }

    if (!target)
        return false;

    for (const char* spell : { "bash", "concussion blow", "hammer of justice", "kidney shot", "maim", "shadowfury", "war stomp" })
    {
        if (raidboss::TryCast(ai, spell, target))
            return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// Attumen the Huntsman
// ---------------------------------------------------------------------------

bool KarazhanAttumenHandlePhaseOneAction::Execute(Event& event)
{
    Unit* midnight = GetKaraUnit(ai, NPC_MIDNIGHT);
    if (!midnight)
        return false;

    Player* requester = GetRequester(ai, event);
    if (raidboss::IsAssistTank(bot, 0))
    {
        Unit* attumen = GetKaraUnit(ai, NPC_ATTUMEN);
        return attumen && AssistTankMoveAttumenFromGroup(requester, midnight, attumen);
    }

    return !IsBotCurrentTarget(ai, midnight) && Attack(requester, midnight);
}

bool KarazhanAttumenHandlePhaseOneAction::AssistTankMoveAttumenFromGroup(Player* requester, Unit* midnight, Unit* attumen)
{
    if (!IsBotCurrentTarget(ai, attumen))
        return Attack(requester, attumen);

    if (attumen->GetVictim() != bot || midnight->GetVictim() == bot)
        return false;

    const float safeDistance = 8.0f;
    Player* nearestPlayer = raidboss::GetNearestPlayerInRadius(bot, safeDistance);
    if (!nearestPlayer || raidboss::ExactDist2d(attumen, nearestPlayer) >= safeDistance)
        return false;

    return MoveAwayFromGroup(safeDistance);
}

bool KarazhanAttumenHandlePhaseTwoAction::Execute(Event& event)
{
    Unit* attumen = GetAttumenMounted(ai);
    if (!attumen)
        return false;

    if (!IsBotCurrentTarget(ai, attumen))
        return Attack(GetRequester(ai, event), attumen);

    if (PlayerbotAI::IsTank(bot))
        return CurrentTankPositionAttumen(attumen);

    return StackBehindAttumen(attumen);
}

bool KarazhanAttumenHandlePhaseTwoAction::CurrentTankPositionAttumen(Unit* attumen)
{
    if (attumen->GetVictim() != bot || !bot->CanReachWithMeleeAttack(attumen))
        return false;

    const KaraPosition& position = ATTUMEN_TANK_POSITION;
    if (!bot->IsWithinLOS(position.x, position.y, position.z))
        return false;

    return StepToPosition(position, 2.0f, attumen);
}

bool KarazhanAttumenHandlePhaseTwoAction::StackBehindAttumen(Unit* attumen)
{
    const float distanceBehind = bot->getClass() == CLASS_HUNTER ? 8.0f : 2.0f;
    const float orientation = attumen->GetOrientation() + M_PI_F;
    const float rearX = attumen->GetPositionX() + std::cos(orientation) * distanceBehind;
    const float rearY = attumen->GetPositionY() + std::sin(orientation) * distanceBehind;

    if (raidboss::ExactDist2d(bot, rearX, rearY) <= 1.0f)
        return false;

    return MoveToPoint(rearX, rearY, attumen->GetPositionZ());
}

bool KarazhanAttumenSetDpsTimerAction::Execute(Event& event)
{
    return StartTimer(bot->GetInstanceId(), KaraTimer::ATTUMEN_DPS_WAIT, std::time(nullptr));
}

// ---------------------------------------------------------------------------
// Moroes
// ---------------------------------------------------------------------------

bool KarazhanMoroesMarkTargetAction::Execute(Event& event)
{
    for (uint32 entry : { NPC_BARONESS_DOROTHEA_MILLSTIPE, NPC_LADY_CATRIONA_VON_INDI, NPC_LADY_KEIRA_BERRYBUCK,
                          NPC_BARON_RAFE_DREUGER, NPC_LORD_ROBIN_DARIS, NPC_LORD_CRISPIN_FERENCE })
    {
        if (Unit* guest = GetKaraUnit(ai, entry))
            return raidboss::MarkTargetWithSkull(bot, guest);
    }

    return false;
}

// ---------------------------------------------------------------------------
// Maiden of Virtue
// ---------------------------------------------------------------------------

bool KarazhanMaidenTankPositionBossAction::Execute(Event& event)
{
    Unit* maiden = GetKaraUnit(ai, NPC_MAIDEN_OF_VIRTUE);
    if (!maiden)
        return false;

    if (!IsBotCurrentTarget(ai, maiden))
        return Attack(GetRequester(ai, event), maiden);

    if (maiden->GetVictim() != bot)
        return false;

    // Drag Maiden onto a healer stunned by Repentance so Holy Ground silences nobody else
    for (Player* healer : raidboss::GetGroupHealers(bot))
    {
        if (healer->HasAura(SPELL_REPENTANCE))
            return MoveBossToStunnedHealer(healer);
    }

    if (!bot->CanReachWithMeleeAttack(maiden))
        return false;

    return StepToPosition(MAIDEN_OF_VIRTUE_TANK_POSITION, 2.0f, maiden);
}

bool KarazhanMaidenTankPositionBossAction::MoveBossToStunnedHealer(Player* healer)
{
    const float endDistance = 6.0f;
    const float angle = healer->GetOrientation();
    const float targetX = healer->GetPositionX() + std::cos(angle) * endDistance;
    const float targetY = healer->GetPositionY() + std::sin(angle) * endDistance;
    if (raidboss::ExactDist2d(bot, targetX, targetY) <= 1.0f)
        return false;

    return MoveToPoint(targetX, targetY, healer->GetPositionZ());
}

bool KarazhanMaidenPositionRangedAction::Execute(Event& event)
{
    std::vector<Player*> ranged;
    for (Player* member : raidboss::GetGroupMembers(bot, 500.0f, true))
    {
        if (raidboss::IsRangedPlayer(member))
            ranged.push_back(member);
    }

    const int index = raidboss::GetIndexOf(bot, ranged);
    if (index < 0)
        return false;

    const KaraPosition& position = MAIDEN_OF_VIRTUE_RANGED_POSITIONS[index % MAIDEN_OF_VIRTUE_RANGED_POSITIONS.size()];
    if (raidboss::ExactDist2d(bot, position.x, position.y) <= 2.0f)
        return false;

    return MoveToPoint(position);
}

bool KarazhanMaidenSetGroundingTotemAction::Execute(Event& event)
{
    return raidboss::TryCast(ai, "grounding totem", bot);
}

// ---------------------------------------------------------------------------
// The Big Bad Wolf
// ---------------------------------------------------------------------------

bool KarazhanBigBadWolfPositionBossAction::Execute(Event& event)
{
    Unit* wolf = GetKaraUnit(ai, NPC_BIG_BAD_WOLF);
    if (!wolf)
        return false;

    if (!IsBotCurrentTarget(ai, wolf))
        return Attack(GetRequester(ai, event), wolf);

    if (wolf->GetVictim() != bot || !bot->CanReachWithMeleeAttack(wolf))
        return false;

    return StepToPosition(BIG_BAD_WOLF_TANK_POSITION, 2.0f, wolf);
}

// Run away, little girl, run away
bool KarazhanBigBadWolfRunAwayAction::Execute(Event& event)
{
    const KaraPosition& target = BIG_BAD_WOLF_RUN_POSITIONS[runIndex];
    if (raidboss::ExactDist2d(bot, target.x, target.y) < 2.0f)
        runIndex = (runIndex + 1) % BIG_BAD_WOLF_RUN_POSITIONS.size();

    StopCasting(bot);
    return MoveToPoint(BIG_BAD_WOLF_RUN_POSITIONS[runIndex]);
}

// ---------------------------------------------------------------------------
// Romulo and Julianne
// ---------------------------------------------------------------------------

bool KarazhanRomuloAndJulianneMarkTargetAction::Execute(Event& event)
{
    Unit* romulo = GetKaraUnit(ai, NPC_ROMULO);
    Unit* julianne = GetKaraUnit(ai, NPC_JULIANNE);
    if (!romulo || !julianne)
        return false;

    // Both have to die within 10 seconds: keep their health close to each other
    const float maxPctDifference = 10.0f;
    const float romuloPct = romulo->GetHealthPercent();
    const float juliannePct = julianne->GetHealthPercent();

    Unit* target = nullptr;
    if (juliannePct + maxPctDifference < romuloPct || juliannePct < 1.0f)
        target = romulo;
    else if (romuloPct + maxPctDifference < juliannePct || romuloPct < 1.0f)
        target = julianne;
    else
        target = romuloPct >= juliannePct ? romulo : julianne;

    return raidboss::MarkTargetWithSkull(bot, target);
}

// ---------------------------------------------------------------------------
// The Wizard of Oz
// ---------------------------------------------------------------------------

bool KarazhanWizardOfOzMarkTargetAction::Execute(Event& event)
{
    for (uint32 entry : OZ_TARGETS)
    {
        if (Unit* target = GetKaraUnit(ai, entry))
            return raidboss::MarkTargetWithSkull(bot, target);
    }

    return false;
}

bool KarazhanWizardOfOzScorchStrawmanAction::Execute(Event& event)
{
    Unit* strawman = GetKaraUnit(ai, NPC_STRAWMAN);
    return strawman && raidboss::TryCast(ai, "scorch", strawman);
}

// ---------------------------------------------------------------------------
// The Curator
// ---------------------------------------------------------------------------

bool KarazhanCuratorMarkAstralFlareAction::Execute(Event& event)
{
    Unit* flare = GetAstralFlare(ai);
    return flare && raidboss::MarkTargetWithSkull(bot, flare);
}

bool KarazhanCuratorPositionBossAction::Execute(Event& event)
{
    Unit* curator = GetKaraUnit(ai, NPC_CURATOR);
    if (!curator)
        return false;

    if (!IsBotCurrentTarget(ai, curator))
        return Attack(GetRequester(ai, event), curator);

    if (curator->GetVictim() != bot || !bot->CanReachWithMeleeAttack(curator))
        return false;

    return StepToPosition(THE_CURATOR_TANK_POSITION, 2.0f, curator);
}

bool KarazhanCuratorSpreadRangedAction::Execute(Event& event)
{
    const float minDistance = 5.0f;
    Player* nearestPlayer = raidboss::GetNearestPlayerInRadius(bot, minDistance);
    return nearestPlayer && MoveAwayFromPoint(nearestPlayer->GetPositionX(), nearestPlayer->GetPositionY(), minDistance + 1.0f);
}

// ---------------------------------------------------------------------------
// Terestian Illhoof
// ---------------------------------------------------------------------------

bool KarazhanIllhoofMarkTargetAction::Execute(Event& event)
{
    for (uint32 entry : { NPC_DEMON_CHAINS, NPC_KILREK, NPC_TERESTIAN_ILLHOOF })
    {
        if (Unit* target = GetKaraUnit(ai, entry))
            return raidboss::MarkTargetWithSkull(bot, target);
    }

    return false;
}

// ---------------------------------------------------------------------------
// Shade of Aran
// ---------------------------------------------------------------------------

bool KarazhanAranRunAwayFromArcaneExplosionAction::Execute(Event& event)
{
    Unit* aran = GetKaraUnit(ai, NPC_SHADE_OF_ARAN);
    if (!aran)
        return false;

    const float safeDistance = 21.0f;
    if (raidboss::ExactDist2d(bot, aran) >= safeDistance)
        return false;

    StopCasting(bot);
    return MoveAwayFromPoint(aran->GetPositionX(), aran->GetPositionY(), safeDistance);
}

// I will not move when Flame Wreath is cast or the raid blows up
bool KarazhanAranStopMovingDuringFlameWreathAction::Execute(Event& event)
{
    AI_VALUE(LastMovement&, "last movement").clear();
    if (!bot->IsMoving())
        return false;

    ai->StopMoving();
    return true;
}

bool KarazhanAranMarkConjuredElementalAction::Execute(Event& event)
{
    for (Unit* elemental : GetKaraUnits(ai, NPC_CONJURED_ELEMENTAL))
    {
        if (!ai->HasAura("banish", elemental))
            return raidboss::MarkTargetWithSkull(bot, elemental);
    }

    return false;
}

bool KarazhanAranRangedMaintainDistanceAction::Execute(Event& event)
{
    Unit* aran = GetKaraUnit(ai, NPC_SHADE_OF_ARAN);
    if (!aran)
        return false;

    // Close enough to stay out of Counterspell trouble, far enough from the melee
    const float minDistance = 10.0f;
    const float maxDistance = 15.0f;
    const float distanceToBoss = raidboss::ExactDist2d(bot, aran);

    if (distanceToBoss > maxDistance)
        return MoveToDistanceFrom(aran, maxDistance - 0.5f);

    if (distanceToBoss < minDistance)
        return MoveToDistanceFrom(aran, minDistance + 0.5f);

    return false;
}

// ---------------------------------------------------------------------------
// Netherspite
// ---------------------------------------------------------------------------

namespace
{
    bool MoveToBeamPoint(PlayerbotAI* ai, float x, float y, float refZ, const std::function<bool(float, float, float)>& move)
    {
        float z = refZ;
        if (!GetValidGroundPoint(ai->GetBot(), x, y, z))
            z = refZ;

        if (raidboss::ExactDist2d(ai->GetBot(), x, y) < 0.5f)
            return false;

        return move(x, y, z);
    }
}

bool KarazhanNetherspiteBlockRedBeamAction::Execute(Event& event)
{
    Unit* redPortal = GetKaraUnit(ai, NPC_RED_PORTAL);
    Unit* netherspite = GetKaraUnit(ai, NPC_NETHERSPITE);
    if (!redPortal || !netherspite)
        return false;

    const BeamBlockers blockers = GetCurrentBeamBlockers(bot);
    if (bot != blockers.red)
    {
        wasBlockingRedBeam = false;
        redBeamTimerWasSet = false;
        return false;
    }

    if (bot->HasAura(SPELL_RED_BEAM_DEBUFF))
    {
        if (!redBeamTimerWasSet)
        {
            redBeamMoveTimer = std::time(nullptr);
            redBeamTimerWasSet = true;
        }
    }
    else
        redBeamTimerWasSet = false;

    if (!wasBlockingRedBeam)
        bot->Yell(std::string(bot->GetName()) + " is moving to block the red beam!", LANG_UNIVERSAL);
    wasBlockingRedBeam = true;

    const time_t intervalSecs = 5;
    if (redBeamTimerWasSet && std::time(nullptr) - redBeamMoveTimer >= intervalSecs)
    {
        lastBeamMoveSideways = !lastBeamMoveSideways;
        redBeamMoveTimer = std::time(nullptr);
    }

    const float idealDistance = 18.0f;
    float beamX = 0.0f;
    float beamY = 0.0f;
    if (!FindBeamPosition(netherspite, redPortal, std::vector<Unit*>(), idealDistance, beamX, beamY))
        return false;

    auto move = [this](float x, float y, float z) { return MoveToPoint(x, y, z); };
    if (!lastBeamMoveSideways)
        return MoveToBeamPoint(ai, beamX, beamY, bot->GetPositionZ(), move);

    const float length = raidboss::ExactDist2d(netherspite, redPortal);
    if (length == 0.0f)
        return false;

    const float dx = (redPortal->GetPositionX() - netherspite->GetPositionX()) / length;
    const float dy = (redPortal->GetPositionY() - netherspite->GetPositionY()) / length;
    return MoveToBeamPoint(ai, beamX - dy * 3.0f, beamY + dx * 3.0f, bot->GetPositionZ(), move);
}

bool KarazhanNetherspiteBlockBlueBeamAction::Execute(Event& event)
{
    Unit* bluePortal = GetKaraUnit(ai, NPC_BLUE_PORTAL);
    Unit* netherspite = GetKaraUnit(ai, NPC_NETHERSPITE);
    if (!bluePortal || !netherspite)
        return false;

    const BeamBlockers blockers = GetCurrentBeamBlockers(bot);
    if (bot != blockers.blue)
    {
        if (wasBlockingBlueBeam)
            bot->Yell(std::string(bot->GetName()) + " is leaving the blue beam. Next blocker up!", LANG_UNIVERSAL);
        wasBlockingBlueBeam = false;
        return false;
    }

    if (!wasBlockingBlueBeam)
        bot->Yell(std::string(bot->GetName()) + " is moving to block the blue beam!", LANG_UNIVERSAL);
    wasBlockingBlueBeam = true;

    const float idealDistance = bot->getClass() == CLASS_HUNTER ? 25.0f : 18.0f;
    float beamX = 0.0f;
    float beamY = 0.0f;
    if (!FindBeamPosition(netherspite, bluePortal, GetAllVoidZones(ai), idealDistance, beamX, beamY))
        return false;

    StopCasting(bot);
    return MoveToBeamPoint(ai, beamX, beamY, bot->GetPositionZ(), [this](float x, float y, float z) { return MoveToPoint(x, y, z); });
}

bool KarazhanNetherspiteBlockGreenBeamAction::Execute(Event& event)
{
    Unit* greenPortal = GetKaraUnit(ai, NPC_GREEN_PORTAL);
    Unit* netherspite = GetKaraUnit(ai, NPC_NETHERSPITE);
    if (!greenPortal || !netherspite)
        return false;

    const BeamBlockers blockers = GetCurrentBeamBlockers(bot);
    if (bot != blockers.green)
    {
        if (wasBlockingGreenBeam)
            bot->Yell(std::string(bot->GetName()) + " is leaving the green beam. Next blocker up!", LANG_UNIVERSAL);
        wasBlockingGreenBeam = false;
        return false;
    }

    if (!wasBlockingGreenBeam)
        bot->Yell(std::string(bot->GetName()) + " is moving to block the green beam!", LANG_UNIVERSAL);
    wasBlockingGreenBeam = true;

    float beamX = 0.0f;
    float beamY = 0.0f;
    if (!FindBeamPosition(netherspite, greenPortal, GetAllVoidZones(ai), 18.0f, beamX, beamY))
        return false;

    StopCasting(bot);
    return MoveToBeamPoint(ai, beamX, beamY, bot->GetPositionZ(), [this](float x, float y, float z) { return MoveToPoint(x, y, z); });
}

bool KarazhanNetherspiteAvoidBeamAndVoidZoneAction::Execute(Event& event)
{
    Unit* netherspite = GetKaraUnit(ai, NPC_NETHERSPITE);
    if (!netherspite)
        return false;

    const std::vector<Unit*> voidZones = GetAllVoidZones(ai);
    const float hazardRadius = 4.0f;
    const bool nearVoidZone = !IsSafePosition(bot->GetPositionX(), bot->GetPositionY(), voidZones, hazardRadius);

    const float bossX = netherspite->GetPositionX();
    const float bossY = netherspite->GetPositionY();
    std::vector<BeamAvoid> beams;
    for (uint32 entry : { NPC_RED_PORTAL, NPC_BLUE_PORTAL, NPC_GREEN_PORTAL })
    {
        if (Unit* portal = GetKaraUnit(ai, entry))
        {
            const float len = raidboss::ExactDist2d(netherspite, portal);
            beams.push_back({ 0.0f, len,
                len > 0.0f ? (portal->GetPositionX() - bossX) / len : 0.0f,
                len > 0.0f ? (portal->GetPositionY() - bossY) / len : 0.0f });
        }
    }

    const bool nearBeam = !IsAwayFromBeams(bot->GetPositionX(), bot->GetPositionY(), bossX, bossY, beams);
    if (!nearVoidZone && !nearBeam)
        return false;

    struct Candidate
    {
        float x;
        float y;
        float distSq;
    };

    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();
    std::vector<Candidate> candidates;
    for (uint8 i = 0; i < 36; ++i)
    {
        const float angle = i * M_PI_F / 18.0f;
        for (uint8 j = 0; j <= 56; ++j)
        {
            const float dist = 2.0f + j * 0.5f;
            const float x = botX + std::cos(angle) * dist;
            const float y = botY + std::sin(angle) * dist;
            if (!IsSafePosition(x, y, voidZones, hazardRadius) || !IsAwayFromBeams(x, y, bossX, bossY, beams))
                continue;

            candidates.push_back({ x, y, dist * dist });
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) { return a.distSq < b.distSq; });

    uint8 checks = 0;
    for (const Candidate& candidate : candidates)
    {
        if (++checks > 20)
            break;

        float z = 0.0f;
        if (!GetValidGroundPoint(bot, candidate.x, candidate.y, z))
            continue;

        StopCasting(bot);
        return MoveToPoint(candidate.x, candidate.y, z);
    }

    return false;
}

bool KarazhanNetherspiteAvoidBeamAndVoidZoneAction::IsAwayFromBeams(float x, float y, float bossX, float bossY, const std::vector<BeamAvoid>& beams)
{
    for (const BeamAvoid& beam : beams)
    {
        if (beam.maxDist == 0.0f)
            continue;

        const float dx = x - bossX;
        const float dy = y - bossY;
        const float distanceAlongBeam = dx * beam.dirX + dy * beam.dirY;
        const float beamX = bossX + beam.dirX * distanceAlongBeam;
        const float beamY = bossY + beam.dirY * distanceAlongBeam;
        const float distToBeamSq = (x - beamX) * (x - beamX) + (y - beamY) * (y - beamY);

        const float minDistFromBeamSq = 25.0f;
        if (distToBeamSq < minDistFromBeamSq && distanceAlongBeam > beam.minDist && distanceAlongBeam < beam.maxDist)
            return false;
    }

    return true;
}

bool KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction::Execute(Event& event)
{
    const float safeDistance = 4.0f;
    for (Unit* voidZone : GetAllVoidZones(ai))
    {
        if (raidboss::ExactDist2d(bot, voidZone) < safeDistance)
            return MoveAwayFromPoint(voidZone->GetPositionX(), voidZone->GetPositionY(), safeDistance + 1.0f);
    }

    return false;
}

bool KarazhanNetherspiteManageTimersAndTrackersAction::Execute(Event& event)
{
    Unit* netherspite = GetKaraUnit(ai, NPC_NETHERSPITE);
    if (!netherspite)
        return false;

    const uint32 instanceId = bot->GetInstanceId();
    const bool isMechanicTracker = raidboss::IsMechanicTrackerBot(bot, KARAZHAN_MAP_ID);
    bool didSomething = false;

    if (IsBanishPhase(netherspite))
    {
        if (isMechanicTracker)
            didSomething |= ClearTimer(instanceId, KaraTimer::NETHERSPITE_DPS_WAIT);

        if (auto* action = GetKaraAction<KarazhanNetherspiteBlockRedBeamAction>(context, "netherspite block red beam"))
            didSomething |= action->ResetRedBeamState();
        if (auto* action = GetKaraAction<KarazhanNetherspiteBlockBlueBeamAction>(context, "netherspite block blue beam"))
            didSomething |= action->ResetBlueBeamState();
        if (auto* action = GetKaraAction<KarazhanNetherspiteBlockGreenBeamAction>(context, "netherspite block green beam"))
            didSomething |= action->ResetGreenBeamState();
    }
    else if (isMechanicTracker)
        didSomething |= StartTimer(instanceId, KaraTimer::NETHERSPITE_DPS_WAIT, std::time(nullptr));

    return didSomething;
}

// ---------------------------------------------------------------------------
// Prince Malchezaar
// ---------------------------------------------------------------------------

// Enfeebled bots (1 hp) leave Shadow Nova range without running through an infernal
bool KarazhanMalchezaarEnfeebledAvoidHazardAction::Execute(Event& event)
{
    Unit* malchezaar = GetKaraUnit(ai, NPC_PRINCE_MALCHEZAAR);
    if (!malchezaar)
        return false;

    const float minSafeBossDistance = 32.0f;
    if (raidboss::ExactDist2d(bot, malchezaar) >= minSafeBossDistance)
        return false;

    struct Candidate
    {
        float x;
        float y;
        float moveDistSq;
    };

    const std::vector<Unit*> infernals = GetSpawnedInfernals(ai);
    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();
    const float bossX = malchezaar->GetPositionX();
    const float bossY = malchezaar->GetPositionY();

    std::vector<Candidate> candidates;
    for (int i = 0; i < 32; ++i)
    {
        const float angle = (2.0f * M_PI_F * i) / 32;
        for (uint8 j = 0; j <= 28; ++j)
        {
            const float dist = minSafeBossDistance + j * 1.0f;
            const float x = bossX + std::cos(angle) * dist;
            const float y = bossY + std::sin(angle) * dist;
            const float mdx = x - botX;
            const float mdy = y - botY;
            candidates.push_back({ x, y, mdx * mdx + mdy * mdy });
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) { return a.moveDistSq < b.moveDistSq; });

    const float safeInfernalDistance = 22.0f;
    uint8 checks = 0;
    for (const Candidate& candidate : candidates)
    {
        if (!IsStraightPathSafe(botX, botY, candidate.x, candidate.y, infernals, safeInfernalDistance))
            continue;

        if (++checks > 20)
            break;

        float z = 0.0f;
        if (!GetValidGroundPoint(bot, candidate.x, candidate.y, z))
            continue;

        StopCasting(bot);
        return MoveToPoint(candidate.x, candidate.y, z);
    }

    return false;
}

bool KarazhanMalchezaarNonTankAvoidInfernalAction::Execute(Event& event)
{
    Unit* malchezaar = GetKaraUnit(ai, NPC_PRINCE_MALCHEZAAR);
    if (!malchezaar)
        return false;

    const float safeInfernalDistance = 22.0f;
    const std::vector<Unit*> infernals = GetSpawnedInfernals(ai);
    if (IsSafePosition(bot->GetPositionX(), bot->GetPositionY(), infernals, safeInfernalDistance))
        return false;

    const float maxDistanceFromBoss = 33.0f;
    float destX = 0.0f;
    float destY = 0.0f;
    if (!TryFindSafePositionWithSafePath(bot, bot->GetPositionX(), bot->GetPositionY(), malchezaar->GetPositionX(),
                                         malchezaar->GetPositionY(), infernals, safeInfernalDistance, maxDistanceFromBoss, destX, destY))
        return false;

    float z = bot->GetPositionZ();
    GetValidGroundPoint(bot, destX, destY, z);
    StopCasting(bot);
    return MoveToPoint(destX, destY, z);
}

bool KarazhanMalchezaarTanksPositionBossAction::Execute(Event& event)
{
    Unit* malchezaar = GetKaraUnit(ai, NPC_PRINCE_MALCHEZAAR);
    if (!malchezaar)
        return false;

    if (!IsBotCurrentTarget(ai, malchezaar))
        return Attack(GetRequester(ai, event), malchezaar);

    if (malchezaar->GetVictim() != bot || !bot->CanReachWithMeleeAttack(malchezaar))
        return false;

    const float safeInfernalDistance = 30.0f;
    const std::vector<Unit*> infernals = GetSpawnedInfernals(ai);
    if (IsSafePosition(bot->GetPositionX(), bot->GetPositionY(), infernals, safeInfernalDistance))
        return false;

    const float maxSampleDist = 75.0f;
    float destX = 0.0f;
    float destY = 0.0f;
    if (!TryFindSafePositionWithSafePath(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionX(), bot->GetPositionY(),
                                         infernals, safeInfernalDistance, maxSampleDist, destX, destY))
        return false;

    float z = bot->GetPositionZ();
    GetValidGroundPoint(bot, destX, destY, z);
    return MoveToPoint(destX, destY, z);
}

// ---------------------------------------------------------------------------
// Nightbane
// ---------------------------------------------------------------------------

// The tank keeps Nightbane on the arc of the terrace between the dome and the east / west ends
bool KarazhanNightbaneTanksPositionBossAction::Execute(Event& event)
{
    Unit* nightbane = GetKaraUnit(ai, NPC_NIGHTBANE);
    if (!nightbane)
        return false;

    if (!IsBotCurrentTarget(ai, nightbane))
        return Attack(GetRequester(ai, event), nightbane);

    if (nightbane->GetVictim() != bot || !bot->CanReachWithMeleeAttack(nightbane))
        return false;

    const KaraPosition& domeCenter = TERRACE_DOME_CENTER;
    const KaraPosition& eastEnd = TERRACE_EAST_END;
    const KaraPosition& westEnd = TERRACE_WEST_END;

    const float radius = std::sqrt((eastEnd.x - domeCenter.x) * (eastEnd.x - domeCenter.x) + (eastEnd.y - domeCenter.y) * (eastEnd.y - domeCenter.y));
    const float thetaA = std::atan2(eastEnd.y - domeCenter.y, eastEnd.x - domeCenter.x);
    const float thetaB = std::atan2(westEnd.y - domeCenter.y, westEnd.x - domeCenter.x);

    float deltaAB = thetaB - thetaA;
    if (deltaAB < 0.0f)
        deltaAB += 2.0f * M_PI_F;

    float arcStart = 0.0f;
    float arcEnd = 0.0f;
    if (deltaAB < M_PI_F)
    {
        arcStart = thetaB;
        arcEnd = thetaA + 2.0f * M_PI_F;
    }
    else
    {
        arcStart = thetaA;
        arcEnd = thetaB;
    }

    float thetaN = std::atan2(nightbane->GetPositionY() - domeCenter.y, nightbane->GetPositionX() - domeCenter.x);
    if (thetaN < arcStart)
        thetaN += 2.0f * M_PI_F;
    else if (thetaN >= arcStart + 2.0f * M_PI_F)
        thetaN -= 2.0f * M_PI_F;

    const float thetaClamped = std::max(arcStart, std::min(arcEnd, thetaN));
    const KaraPosition destination = { domeCenter.x + radius * std::cos(thetaClamped), domeCenter.y + radius * std::sin(thetaClamped), bot->GetPositionZ() };
    return StepToPosition(destination, 0.5f, nightbane);
}

bool KarazhanNightbaneCoordinateRangedAction::Execute(Event& event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* rangedLeader = nullptr;
    for (Player* member : raidboss::GetGroupMembers(bot, 500.0f, true))
    {
        if (!raidboss::IsRangedPlayer(member))
            continue;

        if (group->IsAssistant(member->GetObjectGuid()))
        {
            rangedLeader = member;
            break;
        }

        if (!rangedLeader || member->GetObjectGuid() < rangedLeader->GetObjectGuid())
            rangedLeader = member;
    }

    if (!rangedLeader)
        return false;

    if (bot == rangedLeader)
        return MoveRangedLeaderToSafeSpot();

    return StackOnRangedLeader(rangedLeader);
}

bool KarazhanNightbaneCoordinateRangedAction::MoveRangedLeaderToSafeSpot()
{
    Unit* nightbane = GetKaraUnit(ai, NPC_NIGHTBANE);
    if (!nightbane)
        return false;

    const float minBossDist = 15.0f;
    const std::vector<WorldPosition> charredEarths = raidboss::GetDynamicObjectPositions(bot, 40.0f, SPELL_CHARRED_EARTH);
    if (charredEarths.empty())
    {
        if (raidboss::ExactDist2d(bot, nightbane) < minBossDist)
        {
            StopCasting(bot);
            return MoveAwayFromPoint(nightbane->GetPositionX(), nightbane->GetPositionY(), minBossDist);
        }

        return false;
    }

    const float safeDistSq = 12.0f * 12.0f;
    auto isInCharredEarth = [&](float x, float y)
    {
        for (const WorldPosition& charredEarth : charredEarths)
        {
            const float dx = x - charredEarth.getX();
            const float dy = y - charredEarth.getY();
            if (dx * dx + dy * dy < safeDistSq)
                return true;
        }

        return false;
    };

    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();
    if (!isInCharredEarth(botX, botY))
        return false;

    struct Candidate
    {
        float x;
        float y;
        float moveDistSq;
    };

    const float bossX = nightbane->GetPositionX();
    const float bossY = nightbane->GetPositionY();
    std::vector<Candidate> candidates;
    for (uint8 i = 0; i <= 20; ++i)
    {
        const float dist = minBossDist + i * 1.0f;
        for (uint8 j = 0; j < 36; ++j)
        {
            const float angle = j * M_PI_F / 18.0f;
            const float x = bossX + std::cos(angle) * dist;
            const float y = bossY + std::sin(angle) * dist;
            if (isInCharredEarth(x, y))
                continue;

            candidates.push_back({ x, y, (x - botX) * (x - botX) + (y - botY) * (y - botY) });
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) { return a.moveDistSq < b.moveDistSq; });

    uint8 checks = 0;
    for (const Candidate& candidate : candidates)
    {
        if (++checks > 20)
            break;

        float z = 0.0f;
        if (!GetValidGroundPoint(bot, candidate.x, candidate.y, z))
            continue;

        if (!nightbane->IsWithinLOS(candidate.x, candidate.y, z + 2.0f))
            continue;

        StopCasting(bot);
        return MoveToPoint(candidate.x, candidate.y, z);
    }

    return false;
}

bool KarazhanNightbaneCoordinateRangedAction::StackOnRangedLeader(Player* rangedLeader)
{
    if (raidboss::ExactDist2d(bot, rangedLeader) < 1.0f)
        return false;

    StopCasting(bot);
    return MoveToPoint(rangedLeader->GetPositionX(), rangedLeader->GetPositionY(), rangedLeader->GetPositionZ());
}

// Pets tend to run out of bounds while chasing the flying boss and pull adds from inside the tower
bool KarazhanNightbaneControlPetAggressionAction::Execute(Event& event)
{
    Unit* nightbane = GetKaraUnit(ai, NPC_NIGHTBANE);
    Pet* pet = bot->GetPet();
    if (!nightbane || !pet || !pet->IsAlive() || !pet->AI())
        return false;

    if (!IsNightbaneFlying(nightbane))
    {
        if (setPassive && pet->AI()->HasReactState(REACT_PASSIVE))
            pet->AI()->SetReactState(REACT_DEFENSIVE);

        setPassive = false;
        return false;
    }

    if (!pet->AI()->HasReactState(REACT_PASSIVE))
    {
        pet->AI()->SetReactState(REACT_PASSIVE);
        setPassive = true;
    }

    if (pet->GetVictim() == nightbane)
    {
        pet->AttackStop();
        pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    }

    return false;
}

bool KarazhanNightbaneFlightPhaseStackAction::Execute(Event& event)
{
    Unit* nightbane = GetKaraUnit(ai, NPC_NIGHTBANE);
    if (!nightbane)
        return false;

    // Nightbane can't be reached in the air: fight the skeletons instead
    bool switched = false;
    if (IsBotCurrentTarget(ai, nightbane))
    {
        StopCasting(bot);
        ClearCurrentTarget(ai);
        switched = true;
    }

    Unit* current = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
    if (!current || !current->IsAlive())
    {
        Unit* skeleton = nullptr;
        for (Unit* unit : GetKaraUnits(ai, NPC_RESTLESS_SKELETON))
        {
            if (raidboss::ExactDist2d(bot, unit) <= 30.0f && (!skeleton || raidboss::ExactDist2d(bot, unit) < raidboss::ExactDist2d(bot, skeleton)))
                skeleton = unit;
        }

        if (skeleton && Attack(GetRequester(ai, event), skeleton))
            switched = true;
    }

    if (bot->HasAura(SPELL_RAIN_OF_BONES) || bot->HasAura(SPELL_RAIN_OF_BONES_AURA))
        rainOfBonesHit = true;

    const auto& positions = rainOfBonesHit ? NIGHTBANE_RAIN_OF_BONES_POSITIONS : NIGHTBANE_FLIGHT_STACK_POSITIONS;
    const std::vector<WorldPosition> charredEarths = raidboss::GetDynamicObjectPositions(bot, 40.0f, SPELL_CHARRED_EARTH);
    const float charredEarthSafeDist = 12.0f;

    for (const KaraPosition& position : positions)
    {
        bool inCharredEarth = false;
        for (const WorldPosition& charredEarth : charredEarths)
        {
            const float dx = charredEarth.getX() - position.x;
            const float dy = charredEarth.getY() - position.y;
            if (dx * dx + dy * dy < charredEarthSafeDist * charredEarthSafeDist)
            {
                inCharredEarth = true;
                break;
            }
        }

        if (inCharredEarth)
            continue;

        if (raidboss::ExactDist2d(bot, position.x, position.y) < 1.0f)
            return switched;

        StopCasting(bot);
        return MoveToPoint(position) || switched;
    }

    return switched;
}

bool KarazhanNightbaneTeleportBackAction::Execute(Event& event)
{
    const KaraPosition& position = NIGHTBANE_TELEPORT_POSITION;
    return bot->TeleportTo(KARAZHAN_MAP_ID, position.x, position.y, position.z, bot->GetOrientation());
}

bool KarazhanNightbaneManageTimersAndTrackersAction::Execute(Event& event)
{
    Unit* nightbane = GetKaraUnit(ai, NPC_NIGHTBANE);
    if (!nightbane)
        return false;

    const uint32 instanceId = bot->GetInstanceId();
    const time_t now = std::time(nullptr);
    const bool isMechanicTracker = raidboss::IsMechanicTrackerBot(bot, KARAZHAN_MAP_ID);
    bool didSomething = false;

    if (!IsNightbaneFlying(nightbane))
    {
        if (auto* action = GetKaraAction<KarazhanNightbaneFlightPhaseStackAction>(context, "nightbane flight phase stack and move"))
            didSomething |= action->ResetRainOfBonesHit();

        if (isMechanicTracker)
        {
            didSomething |= ClearTimer(instanceId, KaraTimer::NIGHTBANE_FLIGHT_START);
            didSomething |= StartTimer(instanceId, KaraTimer::NIGHTBANE_DPS_WAIT, now);
        }
    }
    else if (isMechanicTracker)
    {
        didSomething |= ClearTimer(instanceId, KaraTimer::NIGHTBANE_DPS_WAIT);
        didSomething |= StartTimer(instanceId, KaraTimer::NIGHTBANE_FLIGHT_START, now);
    }

    return didSomething;
}
