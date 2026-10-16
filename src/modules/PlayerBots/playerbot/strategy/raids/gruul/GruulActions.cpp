// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#include "playerbot/playerbot.h"
#include "GruulActions.h"
#include "GruulHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
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

bool HighKingMaulgarMeleeTanksPositionBossesAction::Execute(Event& /*event*/)
{
    Unit* target = nullptr;
    WorldPosition position;
    if (IsMaulgarTank(bot))
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
        return Attack(GetMaster(), target);

    if (target->GetVictim() != bot || !bot->CanReachWithMeleeAttack(target))
        return false;

    const float arrivalDist = 3.0f;
    float moveX, moveY;
    bool backwards;
    if (!raidboss::GetStepToPosition(bot, position, arrivalDist, target, moveX, moveY, backwards))
        return false;

    return MoveTo(GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, true);
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

bool HighKingMaulgarRunAwayFromWhirlwindAction::Execute(Event& /*event*/)
{
    Unit* maulgar = FindMaulgar(ai);
    if (!maulgar || raidboss::ExactDist2d(bot, maulgar) >= MAULGAR_WHIRLWIND_SAFE_DISTANCE)
        return false;

    float x, y, z;
    if (!GetPointAwayFrom(bot, maulgar->GetPositionX(), maulgar->GetPositionY(), MAULGAR_WHIRLWIND_SAFE_DISTANCE, x, y, z))
        return false;

    bot->InterruptNonMeleeSpells(true);
    return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, true);
}

bool HighKingMaulgarBackAwayFromKroshAction::Execute(Event& /*event*/)
{
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
        tank = raidboss::GetGroupAssistTank(bot, 1);
        break;
    case 1:
        ogre = FindOlm(ai);
        tank = raidboss::GetGroupAssistTank(bot, 0);
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

    if (gruul->GetVictim() != bot || !bot->CanReachWithMeleeAttack(gruul))
        return false;

    const float arrivalDist = 3.0f;
    float moveX, moveY;
    bool backwards;
    if (!raidboss::GetStepToPosition(bot, GRUUL_TANK_POSITION, arrivalDist, gruul, moveX, moveY, backwards))
        return false;

    return MoveTo(GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, true);
}

bool GruulTheDragonkillerSpreadRangedAction::Execute(Event& /*event*/)
{
    const WorldPosition& position = GRUUL_TANK_POSITION;

    if (!hasInitialPosition)
    {
        // Each ranged bot gets its own angle on a ring around the tank position
        std::vector<Player*> members;
        for (Player* member : raidboss::GetGroupMembers(bot, 200.0f, true))
        {
            if (member->GetPlayerbotAI() && raidboss::IsRangedPlayer(member))
                members.push_back(member);
        }

        if (members.empty())
            return false;

        const int index = std::max(0, raidboss::GetIndexOf(bot, members));
        const float minRadius = 25.0f;
        const float maxRadius = 40.0f;
        const float angle = 2.0f * M_PI_F * index / members.size();
        const float radius = frand(minRadius, maxRadius);
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
        const float moveX = bot->GetPositionX() + ((initialX - bot->GetPositionX()) / distToTarget) * moveDist;
        const float moveY = bot->GetPositionY() + ((initialY - bot->GetPositionY()) / distToTarget) * moveDist;
        return MoveTo(GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, true);
    }

    const float minSpreadDistance = 10.0f;
    Player* nearestPlayer = raidboss::GetNearestPlayerInRadius(bot, minSpreadDistance);
    if (!nearestPlayer)
        return false;

    float x, y, z;
    if (!GetPointAwayFrom(bot, nearestPlayer->GetPositionX(), nearestPlayer->GetPositionY(), minSpreadDistance, x, y, z))
        return false;

    return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, true);
}

// More preemptive avoidance than "avoid aoe", which only reacts once the bot has the damaging aura
// and forgets the pool as soon as the bot is out of it.
bool GruulTheDragonkillerGetOutOfCaveInAction::Execute(Event& /*event*/)
{
    WorldPosition pool;
    if (!GetNearestCaveInPosition(bot, pool))
        return false;

    float x, y, z;
    if (!GetPointAwayFrom(bot, pool.getX(), pool.getY(), CAVE_IN_RADIUS, x, y, z))
        return false;

    bot->InterruptNonMeleeSpells(true);
    return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, true);
}

bool GruulTheDragonkillerShatterSpreadAction::Execute(Event& /*event*/)
{
    Player* nearestPlayer = raidboss::GetNearestPlayerInRadius(bot, GRUUL_SHATTER_SAFE_DISTANCE);
    if (!nearestPlayer)
        return false;

    // Small steps away from the nearest player, re-evaluated every tick
    const float distToNearest = raidboss::ExactDist2d(bot, nearestPlayer);
    const float wantedDistance = std::min(GRUUL_SHATTER_SAFE_DISTANCE, distToNearest + 3.5f);

    float x, y, z;
    if (!GetPointAwayFrom(bot, nearestPlayer->GetPositionX(), nearestPlayer->GetPositionY(), wantedDistance, x, y, z))
        return false;

    return MoveTo(GRUUL_MAP_ID, x, y, z, false, false, false, true);
}
