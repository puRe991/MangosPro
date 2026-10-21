// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "GruulActions.h"
#include "GruulHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/raids/RaidBossStrategies.h"
#include <algorithm>
#include <cmath>

using namespace ai;
using namespace ai::gruul;

namespace
{
    Unit* GetCurrentTarget(PlayerbotAI* ai)
    {
        return ai->GetUnit(ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Get());
    }

    void LogTankPosition(PlayerbotAI* ai, Unit* maulgar, const WorldPosition& position, const char* status)
    {
        auto* debug = dynamic_cast<RaidBossDebugValue*>(ai->GetAiObjectContext()->GetUntypedValue("raid boss debug"));
        if (!debug || !debug->Get())
            return;

        const time_t now = time(nullptr);
        time_t& last = debug->lastLog[std::string("tank position ") + maulgar->GetName()];
        if (now - last < 3)
            return;
        last = now;

        Player* tank = ai->GetBot();
        Unit* victim = maulgar->GetVictim();
        sLog.outString("[RaidPosition] bot=%s boss=%s status=%s tank=(%.2f,%.2f,%.2f) boss=(%.2f,%.2f,%.2f) tankSpot=(%.2f,%.2f,%.2f) tankDistance=%.2f victim=%s melee=%u",
            tank->GetName(), maulgar->GetName(), status, tank->GetPositionX(), tank->GetPositionY(), tank->GetPositionZ(),
            maulgar->GetPositionX(), maulgar->GetPositionY(), maulgar->GetPositionZ(),
            position.getX(), position.getY(), position.getZ(), raidboss::ExactDist2d(tank, position.getX(), position.getY()),
            victim ? victim->GetName() : "none", tank->CanReachWithMeleeAttack(maulgar));
    }
}

// General

bool GruulsLairResetEncounterStatesAction::Execute(Event& /*event*/)
{
    bool reset = false;

    if (auto* spread = dynamic_cast<GruulTheDragonkillerSpreadRangedAction*>(context->GetAction("gruul the dragonkiller spread ranged")))
        reset = spread->ResetInitialPosition();

    if (raidboss::IsMechanicTrackerBot(bot, GRUUL_MAP_ID) && !bot->IsInCombat())
    {
        reset |= raidboss::ClearTargetIcon(bot, raidboss::ICON_SKULL);
        reset |= raidboss::ClearTargetIcon(bot, raidboss::ICON_CROSS);
    }

    return reset;
}

// High King Maulgar <Lord of the Ogres>

bool HighKingMaulgarPrepareAction::isUseful()
{
    float range = 0;
    Unit* target = GetPrepareTarget(ai, range);
    return target && !IsPrepared(bot, target, range) && MovementAction::isUseful();
}

bool HighKingMaulgarPrepareAction::Execute(Event& /*event*/)
{
    float range = 0;
    Unit* target = GetPrepareTarget(ai, range);
    if (!target || IsPrepared(bot, target, range))
        return false;
    float x, y, z;
    if (!GetPreparePosition(ai, target, range, x, y, z))
        return false;
    // Movement only: never set an attack target or send the pet before the pull.
    return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, false);
}

bool HighKingMaulgarMeleeTanksPositionBossesAction::Execute(Event& /*event*/)
{
    Unit* target = nullptr;
    WorldPosition position;
    const bool maulgarTank = IsMaulgarTank(bot);
    if (maulgarTank)
    {
        target = FindMaulgar(ai);
        position = MAULGAR_TANK_POSITION;
    }
    else if (IsOlmTank(bot))
    {
        target = FindOlm(ai);
        position = OLM_TANK_POSITION;
    }
    else if (IsBlindeyeTank(bot))
    {
        target = FindBlindeye(ai);
        position = BLINDEYE_TANK_POSITION;
    }

    if (!target)
        return false;

    if (GetCurrentTarget(ai) != target)
    {
        if (maulgarTank)
            LogTankPosition(ai, target, position, "select_target");
        return Attack(GetMaster(), target);
    }

    if (target->GetVictim() != bot)
    {
        if (maulgarTank)
            LogTankPosition(ai, target, position, "no_aggro");
        return false;
    }

    // Maulgar must be allowed to follow between short steps even when the tank has
    // temporarily left melee range. Otherwise normal melee chasing moves him back.
    if (!maulgarTank && !bot->CanReachWithMeleeAttack(target))
        return false;

    const float arrivalDist = 3.0f;
    float moveX, moveY;
    bool backwards;
    if (!raidboss::GetStepToPosition(bot, position, arrivalDist, target, moveX, moveY, backwards))
    {
        if (maulgarTank)
            LogTankPosition(ai, target, position, "tank_at_spot");
        return false;
    }

    const bool moved = MoveTo(GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, true);
    if (maulgarTank)
        LogTankPosition(ai, target, position, moved ? "move_sent" : "move_failed");
    return moved;
}

bool HighKingMaulgarMageTankAttackKroshAction::Execute(Event& /*event*/)
{
    Unit* krosh = FindKrosh(ai);
    if (!krosh)
        return false;

    if (AttackAndCast(krosh))
        return true;

    if (krosh->GetVictim() != bot)
        return false;

    return MoveToDesiredDistance(krosh);
}

bool HighKingMaulgarMageTankAttackKroshAction::AttackAndCast(Unit* krosh)
{
    // Steal Spell Shield: Krosh loses his fire resistance and the mage gains it
    if (krosh->HasAura(SPELL_SPELL_SHIELD) && raidboss::TryCast(ai, "spellsteal", krosh))
        return true;

    if (GetCurrentTarget(ai) != krosh)
        return Attack(GetMaster(), krosh);

    if (bot->HasAura(SPELL_SPELL_SHIELD))
        return false;

    return raidboss::TryCast(ai, "fire ward", bot);
}

// The mage tank moves to a designated position only if Krosh is far enough from that position to
// be tanked from it without standing in Blast Wave, and close enough to still be tanked at all.
bool HighKingMaulgarMageTankAttackKroshAction::MoveToDesiredDistance(Unit* krosh)
{
    const WorldPosition& position = KROSH_TANK_POSITION;
    const float distanceKroshToPosition = raidboss::ExactDist2d(krosh, position.getX(), position.getY());
    const float minDistance = KROSH_BLAST_WAVE_SAFE_DISTANCE;
    const float maxDistance = 30.0f;

    if (distanceKroshToPosition > minDistance && distanceKroshToPosition < maxDistance &&
        raidboss::ExactDist2d(bot, position.getX(), position.getY()) > 1.0f)
    {
        return MoveTo(GRUUL_MAP_ID, position.getX(), position.getY(), position.getZ(), false, false, false, true);
    }

    if (raidboss::ExactDist2d(bot, krosh) >= KROSH_BLAST_WAVE_SAFE_DISTANCE)
        return false;

    float x, y, z;
    if (!GetPointAwayFrom(bot, krosh->GetPositionX(), krosh->GetPositionY(), KROSH_BLAST_WAVE_SAFE_DISTANCE, x, y, z))
        return false;

    bot->InterruptNonMeleeSpells(true);
    return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, true);
}

// The moonkin tank has no tank position, but usually Kiggler remains close to where he starts.
bool HighKingMaulgarMoonkinTankAttackKigglerAction::Execute(Event& /*event*/)
{
    Unit* kiggler = FindKiggler(ai);
    if (!kiggler)
        return false;

    if (GetCurrentTarget(ai) != kiggler)
        return Attack(GetMaster(), kiggler);

    if (kiggler->GetVictim() != bot || raidboss::ExactDist2d(bot, kiggler) >= KIGGLER_ARCANE_EXPLOSION_SAFE_DISTANCE)
        return false;

    float x, y, z;
    if (!GetPointAwayFrom(bot, kiggler->GetPositionX(), kiggler->GetPositionY(), KIGGLER_ARCANE_EXPLOSION_SAFE_DISTANCE, x, y, z))
        return false;

    return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, true);
}

// Priority: (1) Blindeye, (2) Olm, (3) Krosh (ranged only), (4) Kiggler, and (5) Maulgar
bool HighKingMaulgarAssignDpsPriorityAction::Execute(Event& /*event*/)
{
    Unit* target = nullptr;
    Unit* krosh = nullptr;
    if (Unit* blindeye = FindBlindeye(ai))
        target = blindeye;
    else if (Unit* olm = FindOlm(ai))
        target = olm;
    else if (raidboss::IsRangedPlayer(bot) && (krosh = FindKrosh(ai)))
        target = krosh;
    else if (Unit* kiggler = FindKiggler(ai))
        target = kiggler;
    else if (Unit* maulgar = FindMaulgar(ai))
        target = maulgar;

    if (!target)
        return false;

    if (target == krosh)
    {
        if (raidboss::MarkTargetWithCross(bot, target))
            return true;
    }
    else if (raidboss::MarkTargetWithSkull(bot, target))
    {
        return true;
    }

    return GetCurrentTarget(ai) != target && Attack(GetMaster(), target);
}

bool HighKingMaulgarRunAwayFromWhirlwindAction::isUseful()
{
    // Reaction selection interrupts casts before Execute, so reject stale or
    // unnecessary escapes here as well as in the trigger.
    Unit* maulgar = FindMaulgar(ai);
    return MovementAction::isUseful() && IsGruulsLairEncounterInProgress(bot) &&
        maulgar && maulgar->HasAura(SPELL_WHIRLWIND) && !IsMaulgarTank(bot) &&
        raidboss::ExactDist2d(bot, maulgar) < MAULGAR_WHIRLWIND_SAFE_DISTANCE;
}

bool HighKingMaulgarRunAwayFromWhirlwindAction::Execute(Event& /*event*/)
{
    if (!isUseful())
        return false;

    Unit* maulgar = FindMaulgar(ai);
    if (!maulgar || raidboss::ExactDist2d(bot, maulgar) >= MAULGAR_WHIRLWIND_SAFE_DISTANCE)
        return false;

    float x, y, z;
    if (!GetPointAwayFrom(bot, maulgar->GetPositionX(), maulgar->GetPositionY(), MAULGAR_WHIRLWIND_SAFE_DISTANCE, x, y, z))
        return false;

    bot->InterruptNonMeleeSpells(true);
    return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, true);
}

bool HighKingMaulgarBackAwayFromKroshAction::isUseful()
{
    Unit* krosh = FindKrosh(ai);
    return MovementAction::isUseful() && IsGruulsLairEncounterInProgress(bot) &&
        !PlayerbotAI::IsTank(bot) && !IsKroshMageTank(ai) && krosh &&
        raidboss::ExactDist2d(bot, krosh) < KROSH_BLAST_WAVE_SAFE_DISTANCE;
}

bool HighKingMaulgarBackAwayFromKroshAction::Execute(Event& /*event*/)
{
    if (!isUseful())
        return false;

    Unit* krosh = FindKrosh(ai);
    if (!krosh || raidboss::ExactDist2d(bot, krosh) >= KROSH_BLAST_WAVE_SAFE_DISTANCE)
        return false;

    float x, y, z;
    if (!GetPointAwayFrom(bot, krosh->GetPositionX(), krosh->GetPositionY(), KROSH_BLAST_WAVE_SAFE_DISTANCE, x, y, z))
        return false;

    bot->InterruptNonMeleeSpells(true);
    return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, true);
}

bool HighKingMaulgarBanishFelStalkerAction::Execute(Event& /*event*/)
{
    // Both lists are ordered the same way for every bot, so each warlock banishes its own stalker
    const std::vector<Unit*> felStalkers = GetNearbyWildFelStalkers(ai);
    const int warlockIndex = raidboss::GetIndexOf(bot, GetGroupBotsOfClass(bot, CLASS_WARLOCK));
    if (warlockIndex < 0 || size_t(warlockIndex) >= felStalkers.size())
        return false;

    Unit* assignedFelStalker = felStalkers[warlockIndex];
    if (ai->HasAura("banish", assignedFelStalker))
        return false;

    return raidboss::TryCast(ai, "banish", assignedFelStalker);
}

// Misdirect order: Blindeye, Olm, Kiggler, Krosh
bool HighKingMaulgarMisdirectOgresToTanksAction::Execute(Event& /*event*/)
{
    std::vector<Player*> hunters = GetGroupBotsOfClass(bot, CLASS_HUNTER);
    if (hunters.size() > 4)
        hunters.resize(4);

    const int hunterIndex = raidboss::GetIndexOf(bot, hunters);
    Unit* ogre = nullptr;
    Player* tank = nullptr;
    switch (hunterIndex)
    {
    case 0:
        ogre = FindBlindeye(ai);
        tank = raidboss::GetGroupAssistTankOrStandIn(bot, 1);
        break;
    case 1:
        ogre = FindOlm(ai);
        tank = raidboss::GetGroupAssistTankOrStandIn(bot, 0);
        break;
    case 2:
        ogre = FindKiggler(ai);
        tank = GetKigglerMoonkinTank(ai);
        break;
    case 3:
        ogre = FindKrosh(ai);
        tank = GetKroshMageTank(ai);
        break;
    default:
        return false;
    }

    if (!ogre || !tank || !tank->IsAlive())
        return false;

    if (raidboss::TryCast(ai, "misdirection", tank))
        return true;

    if (!bot->HasAura(SPELL_MISDIRECTION))
        return false;

    return raidboss::TryCast(ai, "steady shot", ogre);
}

// Gruul the Dragonkiller

bool GruulTheDragonkillerTanksPositionBossAction::Execute(Event& /*event*/)
{
    Unit* gruul = FindGruul(ai);
    if (!gruul)
        return false;

    if (GetCurrentTarget(ai) != gruul)
        return Attack(GetMaster(), gruul);

    if (gruul->GetVictim() != bot)
        return false;

    const float arrivalDist = 3.0f;
    float moveX, moveY;
    bool backwards;
    if (!raidboss::GetStepToPosition(bot, GRUUL_TANK_POSITION, arrivalDist, gruul, moveX, moveY, backwards))
    {
        LogTankPosition(ai, gruul, GRUUL_TANK_POSITION, "tank_at_spot");
        return false;
    }

    const bool moved = MoveTo(GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, true);
    LogTankPosition(ai, gruul, GRUUL_TANK_POSITION, moved ? "move_sent" : "move_failed");
    return moved;
}

bool GruulTheDragonkillerSpreadRangedAction::Execute(Event& /*event*/)
{
    Unit* gruul = FindGruul(ai);
    if (!gruul || HasGroundSlam(bot))
        return false;

    const WorldPosition position(gruul);
    const float minBossDistance = std::max(20.0f, gruul->GetCombatReach() + bot->GetCombatReach() + 5.0f);
    // Ground Slam and combat resurrection can leave a healer/caster in Hurtful
    // Strike range after its original spread position was already reached.
    if (raidboss::ExactDist2d(bot, gruul) < minBossDistance)
    {
        float x, y, z;
        if (!GetPointAwayFrom(bot, gruul->GetPositionX(), gruul->GetPositionY(), minBossDistance, x, y, z))
            return false;
        return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, true);
    }

    // Ordinary spacing must not repeatedly interrupt healing or damage casts.
    if (bot->IsNonMeleeSpellCasted(false))
        return false;

    if (!hasInitialPosition || (!hasReachedInitialPosition &&
        (raidboss::ExactDist2d(gruul, initialX, initialY) < minBossDistance ||
         raidboss::ExactDist2d(gruul, initialX, initialY) > 35.0f)))
    {
        // Slots spread evenly over the whole ring, handed out in the order of the current angles
        // around the boss: neighbours keep their order, so nobody crosses Hurtful Strike range.
        // Only keeping the own side clumped nearly all ranged on one side (8 deaths to one Shatter).
        auto angleOf = [&position](Player* player)
        {
            return std::atan2(player->GetPositionY() - position.getY(), player->GetPositionX() - position.getX());
        };

        std::vector<std::pair<float, Player*>> ranged;
        for (Player* member : raidboss::GetGroupMembers(bot, 80.0f, true))
        {
            if (member->IsAlive() && raidboss::IsRangedPlayer(member))
                ranged.emplace_back(angleOf(member), member);
        }
        std::sort(ranged.begin(), ranged.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

        float angle = angleOf(bot);
        for (size_t i = 0; i < ranged.size(); ++i)
        {
            if (ranged[i].second == bot)
            {
                angle = ranged.front().first + 2.0f * M_PI_F * float(i) / float(ranged.size());
                break;
            }
        }

        const float radius = 30.0f;
        initialX = position.getX() + radius * std::cos(angle);
        initialY = position.getY() + radius * std::sin(angle);
        hasInitialPosition = true;
    }

    if (!hasReachedInitialPosition)
    {
        const float distToTarget = raidboss::ExactDist2d(bot, initialX, initialY);
        if (distToTarget <= 2.0f)
        {
            hasReachedInitialPosition = true;
            return false;
        }

        const float moveDist = std::min(3.5f, distToTarget);
        float moveX = bot->GetPositionX() + ((initialX - bot->GetPositionX()) / distToTarget) * moveDist;
        float moveY = bot->GetPositionY() + ((initialY - bot->GetPositionY()) / distToTarget) * moveDist;

        // A slot on the far side is reached around the ring, not through the boss
        const float stepBossDistance = raidboss::ExactDist2d(gruul, moveX, moveY);
        if (stepBossDistance < minBossDistance && stepBossDistance > 0.1f)
        {
            const float scale = (minBossDistance + 2.0f) / stepBossDistance;
            moveX = position.getX() + (moveX - position.getX()) * scale;
            moveY = position.getY() + (moveY - position.getY()) * scale;
        }

        if (MoveTo(GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, true))
            return true;

        // Unreachable slot (wall): keep the spot and let the ordinary spacing below take over
        hasReachedInitialPosition = true;
        return false;
    }

    const float minSpreadDistance = 10.0f;
    Player* nearestPlayer = raidboss::GetNearestPlayerInRadius(bot, minSpreadDistance);
    if (!nearestPlayer)
        return false;

    float x, y, z;
    if (!GetPointAwayFrom(bot, nearestPlayer->GetPositionX(), nearestPlayer->GetPositionY(), minSpreadDistance, x, y, z))
        return false;

    // Do not solve crowding by stepping back into the boss or out of spell range.
    const float bossDistance = raidboss::ExactDist2d(gruul, x, y);
    if (bossDistance < minBossDistance || bossDistance > 35.0f)
        return false;

    return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, true);
}

// More preemptive avoidance than "avoid aoe", which only reacts once the bot has the damaging aura
// and forgets the pool as soon as the bot is out of it.
bool GruulTheDragonkillerGetOutOfCaveInAction::isUseful()
{
    Unit* gruul = FindGruul(ai);
    return MovementAction::isUseful() && IsGruulsLairEncounterInProgress(bot) && gruul &&
        gruul->GetVictim() != bot && IsInCaveIn(bot) && bot->GetSpeed(MOVE_RUN) > 0.0f;
}

bool GruulTheDragonkillerGetOutOfCaveInAction::Execute(Event& /*event*/)
{
    if (!isUseful())
        return false;

    WorldPosition pool;
    if (!GetNearestCaveInPosition(bot, pool))
        return false;

    float x, y, z;
    if (!GetPointAwayFrom(bot, pool.getX(), pool.getY(), CAVE_IN_RADIUS, x, y, z))
        return false;

    bot->InterruptNonMeleeSpells(true);
    return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, true);
}

bool GruulTheDragonkillerShatterSpreadAction::isUseful()
{
    if (!MovementAction::isUseful() || !IsGruulsLairEncounterInProgress(bot) || !FindGruul(ai))
        return false;

    float x, y, z;
    return GetShatterSpreadPosition(bot, x, y, z);
}

bool GruulTheDragonkillerShatterSpreadAction::Execute(Event& /*event*/)
{
    float x, y, z;
    if (!GetShatterSpreadPosition(bot, x, y, z))
        return false;

    bot->InterruptNonMeleeSpells(true);
    return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, true);
}
