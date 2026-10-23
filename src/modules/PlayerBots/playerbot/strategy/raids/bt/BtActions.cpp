// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/BT/BTActions) and adapted for this core.
#include "playerbot/playerbot.h"
#include "BtActions.h"
#include "BtNajentusRules.h"
#include "BtSupremusRules.h"
#include "playerbot/strategy/generic/TbcRaidStrategy.h"
#include <algorithm>
#include <cmath>

using namespace ai;
using namespace ai::bt;

namespace
{
    uint64 GuidKey(Player* bot) { return bot->GetObjectGuid().GetRawValue(); }

    void SendPetAction(Player* bot, uint8 flag, uint32 spellId, ObjectGuid target = ObjectGuid())
    {
        Pet* pet = bot->GetPet();
        if (!pet)
            return;

        WorldPacket packet(CMSG_PET_ACTION);
        packet << pet->GetObjectGuid();
        packet << uint32((uint32(flag) << 24) | spellId);
        packet << target;
        bot->GetSession()->HandlePetAction(packet);
    }

    bool CastByName(PlayerbotAI* ai, const char* spell, Unit* target)
    {
        return raidboss::TryCast(ai, std::string(spell), target);
    }

    Unit* GetCastTarget(Unit* caster, uint32 spellId)
    {
        if (!caster)
            return nullptr;

        Spell* spell = caster->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!spell || !spell->m_spellInfo || spell->m_spellInfo->Id != spellId)
            return nullptr;

        return spell->m_targets.getUnitTarget();
    }

    // First 'max' hunter bots of the group, in group order
    std::vector<Player*> GetHunterBots(Player* bot, size_t max)
    {
        std::vector<Player*> hunters;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref && hunters.size() < max; ref = ref->next())
            {
                Player* member = ref->getSource();
                if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && member->GetPlayerbotAI())
                    hunters.push_back(member);
            }
        }

        return hunters;
    }

    std::vector<Player*> GetRangedMembers(Player* bot)
    {
        std::vector<Player*> ranged;
        for (Player* member : raidboss::GetGroupMembers(bot, 200.0f, true))
        {
            if (raidboss::IsRangedPlayer(member))
                ranged.push_back(member);
        }

        return ranged;
    }
}

// ---------------------------------------------------------------------------
// General
// ---------------------------------------------------------------------------

bool BlackTempleEraseTimersAndTrackersAction::Execute(Event& event)
{
    bool erased = false;
    const uint64 key = GuidKey(bot);
    for (BtState state : { BtState::AkamaReachedPosition, BtState::ShahrazTankStep, BtState::GathiosTankStep,
                           BtState::ZerevorHealStep, BtState::FlameTankWaypoint })
    {
        erased |= EraseState(state, key);
    }

    // A pet set to passive for Illidan gets its normal stance back
    if (EraseState(BtState::PetMadePassive, key))
    {
        SendPetAction(bot, ACT_REACTION, REACT_DEFENSIVE);
        erased = true;
    }

    // Instance timers are reset once nobody fights the boss anymore
    const uint32 instanceId = bot->GetInstanceId();
    if (!IsBossInCombat(bot, NPC_SUPREMUS))
        erased |= EraseState(BtState::SupremusPhaseTimer, instanceId);
    if (!IsBossInCombat(bot, NPC_NAJENTUS))
        erased |= EraseState(BtState::NajentusShieldTimer, instanceId);
    if (!IsBossInCombat(bot, NPC_GURTOGG_BLOODBOIL))
        erased |= EraseState(BtState::GurtoggPhaseTimer, instanceId);
    if (!IsBossInCombat(bot, NPC_GATHIOS))
        erased |= EraseState(BtState::CouncilDpsTimer, instanceId);
    if (!IsBossInCombat(bot, NPC_ILLIDAN) && !IsBossInCombat(bot, NPC_FLAME_OF_AZZINOTH))
    {
        for (BtState state : { BtState::IllidanBossDpsTimer, BtState::IllidanFlameDpsTimer, BtState::IllidanLastPhase,
                               BtState::IllidanEastFlame, BtState::IllidanWestFlame })
        {
            erased |= EraseState(state, instanceId);
        }
    }

    return erased;
}

// ---------------------------------------------------------------------------
// High Warlord Naj'entus
// ---------------------------------------------------------------------------

bool HighWarlordNajentusMisdirectBossToMainTankAction::Execute(Event& event)
{
    Unit* najentus = FindBoss(ai, NPC_NAJENTUS);
    return najentus && MisdirectTo(raidboss::GetGroupMainTank(bot), najentus);
}

bool HighWarlordNajentusTanksPositionBossAction::Execute(Event& event)
{
    Unit* najentus = FindBoss(ai, NPC_NAJENTUS);
    if (!najentus)
        return false;

    if (!IsCurrentTarget(najentus))
        return AttackTarget(najentus);

    if (najentus->GetVictim() == bot && bot->CanReachWithMeleeAttack(najentus))
        return StepTowards(NAJENTUS_TANK_POSITION.getX(), NAJENTUS_TANK_POSITION.getY(), 5.0f, 3.0f);

    return false;
}

bool HighWarlordNajentusDisperseRangedAction::Execute(Event& event)
{
    Unit* najentus = FindBoss(ai, NPC_NAJENTUS);
    if (!najentus)
        return false;

    // Each ranged player gets an own slot on a ring around the boss, a second ring once the first one gets
    // crowded. Fleeing from the nearest player within 7 yards never settled with 20+ ranged and they spent
    // the fight moving instead of casting.
    std::vector<Player*> ranged;
    for (Player* member : raidboss::GetGroupMembers(bot, 200.0f, true))
    {
        if (raidboss::IsRangedPlayer(member))
            ranged.push_back(member);
    }

    const int index = raidboss::GetIndexOf(bot, ranged);
    if (index < 0)
        return false;

    float ringRadius, angle;
    najentus::RangedSlot(index, int(ranged.size()), ringRadius, angle);
    const float slotX = najentus->GetPositionX() + ringRadius * std::cos(angle);
    const float slotY = najentus->GetPositionY() + ringRadius * std::sin(angle);

    const float arrivalTolerance = 4.0f;
    if (raidboss::ExactDist2d(bot, slotX, slotY) <= arrivalTolerance)
        return false;

    float stepX, stepY, stepZ;
    if (!raidboss::CanTakeStepTowards(bot, slotX, slotY, 8.0f, stepX, stepY, stepZ))
        return false;

    return MoveTo(bot->GetMapId(), stepX, stepY, stepZ, false, false, false, true);
}

bool HighWarlordNajentusSpreadMeleeAction::Execute(Event& event)
{
    Unit* najentus = FindBoss(ai, NPC_NAJENTUS);
    if (!najentus)
        return false;

    // Two groups on the rear flanks instead of one pile, so a Needle Spine splash only hits half of the melee
    std::vector<Player*> melee;
    for (Player* member : raidboss::GetGroupMembers(bot, 200.0f, true))
    {
        if (raidboss::IsMeleePlayer(member) && !PlayerbotAI::IsTank(member))
            melee.push_back(member);
    }

    const int index = raidboss::GetIndexOf(bot, melee);
    if (index < 0)
        return false;

    const float angle = najentus::MeleeGroupAngle(index, najentus->GetOrientation());
    const float radius = najentus::MeleeRadius(najentus->GetCombatReach());
    const float slotX = najentus->GetPositionX() + radius * std::cos(angle);
    const float slotY = najentus->GetPositionY() + radius * std::sin(angle);

    if (raidboss::ExactDist2d(bot, slotX, slotY) <= 2.0f)
        return false;

    float stepX, stepY, stepZ;
    if (!raidboss::CanTakeStepTowards(bot, slotX, slotY, 8.0f, stepX, stepY, stepZ))
        return false;

    return MoveTo(bot->GetMapId(), stepX, stepY, stepZ, false, false, false, true);
}

bool HighWarlordNajentusRemoveImpalingSpineAction::Execute(Event& event)
{
    bool impaled = false;
    for (Player* member : raidboss::GetGroupMembers(bot, 100.0f, false))
    {
        if (member->HasAura(SPELL_IMPALING_SPINE))
        {
            impaled = true;
            break;
        }
    }

    if (!impaled)
        return false;

    GameObject* spine = nullptr;
    for (GameObject* go : raidboss::FindGameObjects(bot, GO_NAJENTUS_SPINE, 30.0f))
    {
        if (!spine || bot->GetDistance(go) < bot->GetDistance(spine))
            spine = go;
    }

    if (!spine)
        return false;

    if (raidboss::ExactDist2d(bot, spine) > 3.0f)
        return MoveToPoint(spine->GetPositionX(), spine->GetPositionY(), spine->GetPositionZ());

    if (bot->IsMoving())
        bot->StopMoving();

    spine->Use(bot);
    return true;
}

bool HighWarlordNajentusThrowImpalingSpineAction::Execute(Event& event)
{
    Unit* najentus = FindBoss(ai, NPC_NAJENTUS);
    if (!najentus)
        return false;

    if (raidboss::ExactDist2d(bot, najentus) > 24.0f)
    {
        const float angle = std::atan2(bot->GetPositionY() - najentus->GetPositionY(), bot->GetPositionX() - najentus->GetPositionX());
        return MoveToPoint(najentus->GetPositionX() + 23.0f * std::cos(angle), najentus->GetPositionY() + 23.0f * std::sin(angle),
                           bot->GetPositionZ());
    }

    Item* spine = bot->GetItemByEntry(ITEM_NAJENTUS_SPINE);
    if (!spine)
        return false;

    ai->ImbueItem(spine, najentus);
    return true;
}

// ---------------------------------------------------------------------------
// Supremus
// ---------------------------------------------------------------------------

// Three hunters build threat on the three tanks after every threat reset (pull and phase change)
bool SupremusMisdirectBossToMainTankAction::Execute(Event& event)
{
    Unit* supremus = FindBoss(ai, NPC_SUPREMUS);
    if (!supremus)
        return false;

    std::vector<Player*> hunters = GetHunterBots(bot, 3);
    const int index = raidboss::GetIndexOf(bot, hunters);
    Player* tank = nullptr;
    if (index == 0)
        tank = raidboss::GetGroupMainTank(bot);
    else if (index == 1)
        tank = raidboss::GetGroupAssistTank(bot, 0);
    else if (index == 2)
        tank = raidboss::GetGroupAssistTank(bot, 1);

    return MisdirectTo(tank, supremus);
}

bool SupremusDisperseRangedAction::Execute(Event& event)
{
    if (Player* nearest = raidboss::GetNearestPlayerInRadius(bot, 8.0f))
        return FleeFrom(nearest->GetPositionX(), nearest->GetPositionY(), 8.0f, 1000);

    return false;
}

bool SupremusKiteBossAction::Execute(Event& event)
{
    Unit* supremus = FindBoss(ai, NPC_SUPREMUS);
    if (!supremus)
        return false;

    const float reach = GetSupremusMeleeReach(bot, supremus);
    if (!bt::supremus::NeedsToKite(raidboss::ExactDist2d(bot, supremus), reach))
        return false;

    return FleeFrom(supremus->GetPositionX(), supremus->GetPositionY(), bt::supremus::KiteFleeDistance(reach));
}

bool SupremusMoveAwayFromVolcanosAction::Execute(Event& event)
{
    std::vector<Unit*> volcanos = GetSupremusVolcanos(bot, 40.0f);
    if (volcanos.empty())
        return false;

    const float hazardRadius = 16.0f;
    bool inDanger = false;
    for (Unit* volcano : volcanos)
    {
        if (raidboss::ExactDist2d(bot, volcano) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    float x = 0.0f, y = 0.0f;
    if (!inDanger || !FindSafestNearbyPosition(volcanos, 40.0f, hazardRadius, x, y))
        return false;

    return MoveToPoint(x, y, bot->GetPositionZ());
}

bool SupremusMoveAwayFromVolcanosAction::FindSafestNearbyPosition(const std::vector<Unit*>& volcanos, float maxRadius,
                                                                  float hazardRadius, float& outX, float& outY)
{
    bool found = false;
    float best = 0.0f;
    for (float distance = 1.0f; distance <= maxRadius; distance += 1.0f)
    {
        for (float angle = 0.0f; angle < 2.0f * M_PI_F; angle += M_PI_F / 8.0f)
        {
            const float x = bot->GetPositionX() + distance * std::cos(angle);
            const float y = bot->GetPositionY() + distance * std::sin(angle);

            bool safe = true;
            for (Unit* volcano : volcanos)
            {
                if (raidboss::ExactDist2d(volcano, x, y) < hazardRadius)
                {
                    safe = false;
                    break;
                }
            }

            if (!safe)
                continue;

            const bool pathSafe = IsPathSafe(x, y, volcanos, hazardRadius);
            // Prefer a point which can be reached without crossing a volcano, otherwise the closest safe point
            if (pathSafe)
            {
                outX = x;
                outY = y;
                return true;
            }

            if (!found || distance < best)
            {
                found = true;
                best = distance;
                outX = x;
                outY = y;
            }
        }
    }

    return found;
}

bool SupremusMoveAwayFromVolcanosAction::IsPathSafe(float endX, float endY, const std::vector<Unit*>& volcanos, float hazardRadius)
{
    const float startX = bot->GetPositionX();
    const float startY = bot->GetPositionY();
    for (uint8 i = 1; i <= 10; ++i)
    {
        const float ratio = i / 10.0f;
        const float x = startX + (endX - startX) * ratio;
        const float y = startY + (endY - startY) * ratio;
        for (Unit* volcano : volcanos)
        {
            if (raidboss::ExactDist2d(volcano, x, y) < hazardRadius)
                return false;
        }
    }

    return true;
}

// The chased player leaves the group so the flame trail only hits one person
bool SupremusLeadMoltenFlameAwayAction::Execute(Event& event)
{
    Unit* flame = GetSupremusMoltenFlameChasingBot(bot);
    if (!flame)
        return false;

    if (Player* nearest = raidboss::GetNearestPlayerInRadius(bot, 12.0f))
        return FleeFrom(nearest->GetPositionX(), nearest->GetPositionY(), 12.0f, 1000);

    const float current = raidboss::ExactDist2d(bot, flame);
    return current < 8.0f && MoveAwayFrom(flame, 8.0f - current);
}

bool SupremusManagePhaseTimerAction::Execute(Event& event)
{
    if (Unit* supremus = FindBoss(ai, NPC_SUPREMUS))
        SetStateIfAbsent(BtState::SupremusPhaseTimer, supremus->GetInstanceId(), int64(time(nullptr)));

    return false;
}

// ---------------------------------------------------------------------------
// Shade of Akama
// ---------------------------------------------------------------------------

bool ShadeOfAkamaMeleeDpsPrioritizeChannelersAction::Execute(Event& event)
{
    int64 reached = 0;
    if (!GetState(BtState::AkamaReachedPosition, GuidKey(bot), reached))
    {
        if (MoveToPosition(AKAMA_CHANNELER_POSITION, 2.0f))
            return true;

        SetState(BtState::AkamaReachedPosition, GuidKey(bot), 1);
    }

    std::vector<Unit*> channelers = raidboss::FindUnits(bot, NPC_ASHTONGUE_CHANNELER, 30.0f);
    if (channelers.empty())
        return false;

    std::sort(channelers.begin(), channelers.end(), [](Unit* a, Unit* b) { return a->GetObjectGuid() < b->GetObjectGuid(); });
    Unit* channeler = channelers.front();

    if (raidboss::MarkTargetWithSkull(bot, channeler))
        return true;

    return !IsCurrentTarget(channeler) && AttackTarget(channeler);
}

// ---------------------------------------------------------------------------
// Teron Gorefiend
// ---------------------------------------------------------------------------

bool TeronGorefiendMisdirectBossToMainTankAction::Execute(Event& event)
{
    Unit* gorefiend = FindBoss(ai, NPC_TERON_GOREFIEND);
    return gorefiend && MisdirectTo(raidboss::GetGroupMainTank(bot), gorefiend);
}

bool TeronGorefiendTanksPositionBossAction::Execute(Event& event)
{
    Unit* gorefiend = FindBoss(ai, NPC_TERON_GOREFIEND);
    if (!gorefiend)
        return false;

    if (raidboss::MarkTargetWithSkull(bot, gorefiend))
        return true;

    if (!IsCurrentTarget(gorefiend))
        return AttackTarget(gorefiend);

    if (gorefiend->GetVictim() == bot && bot->CanReachWithMeleeAttack(gorefiend))
        return StepTowards(GOREFIEND_TANK_POSITION.getX(), GOREFIEND_TANK_POSITION.getY(), 5.0f, 3.0f);

    return false;
}

// Arc at the edge of the balcony (farthest from the Constructs)
bool TeronGorefiendPositionRangedOnBalconyAction::Execute(Event& event)
{
    std::vector<Player*> ranged = GetRangedMembers(bot);
    if (ranged.empty())
        return false;

    const size_t count = ranged.size();
    const int index = std::max(0, raidboss::GetIndexOf(bot, ranged));

    const float arcSpan = 2.0f * M_PI_F / 5.0f;
    const float arcCenter = 6.279f;
    const float arcStart = arcCenter - arcSpan / 2.0f;
    const float radius = 12.0f;
    const float angle = count == 1 ? arcCenter : arcStart + arcSpan * float(index) / float(count - 1);

    const float x = GOREFIEND_TANK_POSITION.getX() + radius * std::cos(angle);
    const float y = GOREFIEND_TANK_POSITION.getY() + radius * std::sin(angle);
    return MoveToPosition(WorldPosition(BLACK_TEMPLE_MAP_ID, x, y, GOREFIEND_TANK_POSITION.getZ()), 1.0f);
}

// Classes with an immunity survive the Shadow of Death cast, the immunity is removed right after
bool TeronGorefiendAvoidShadowOfDeathAction::Execute(Event& event)
{
    if (ai->HasAura("feign death", bot))
        return ai->RemoveAura("feign death");
    if (ai->HasAura("ice block", bot))
        return ai->RemoveAura("ice block");
    if (!PlayerbotAI::IsHeal(bot) && ai->HasAura("divine shield", bot))
        return ai->RemoveAura("divine shield");

    switch (bot->getClass())
    {
    case CLASS_HUNTER:  return CastByName(ai, "feign death", bot);
    case CLASS_MAGE:    return CastByName(ai, "ice block", bot);
    case CLASS_PALADIN: return CastByName(ai, "divine shield", bot);
    case CLASS_ROGUE:   return CastByName(ai, "vanish", bot);
    default:            return false;
    }
}

// The Shadowy Constructs spawn where the marked player stands when Shadow of Death expires
bool TeronGorefiendMoveToCornerToDieAction::Execute(Event& event)
{
    return MoveToPosition(GOREFIEND_DIE_POSITION, 2.0f);
}

bool TeronGorefiendControlAndDestroyShadowyConstructsAction::CastSpiritSpell(Creature* spirit, uint32 spellId, Unit* target)
{
    if (!spirit->IsSpellReady(spellId))
        return false;

    return spirit->CastSpell(target, spellId, TRIGGERED_NONE) == SPELL_CAST_OK;
}

bool TeronGorefiendControlAndDestroyShadowyConstructsAction::Execute(Event& event)
{
    Unit* charm = bot->GetCharm();
    if (!charm || charm->GetTypeId() != TYPEID_UNIT || !charm->IsAlive())
        return false;

    Creature* spirit = (Creature*)charm;
    if (spirit->IsNonMeleeSpellCasted(false))
        return true;

    Unit* gorefiend = FindNearest(bot, NPC_TERON_GOREFIEND, 150.0f);

    // Highest health construct first, ties go to the one closest to Gorefiend
    Unit* target = nullptr;
    for (Unit* construct : TbcRaidHelper::FindCreaturesAround(spirit, NPC_SHADOWY_CONSTRUCT, 80.0f))
    {
        if (!construct->IsAlive())
            continue;

        if (!target || construct->GetHealth() > target->GetHealth() ||
            (construct->GetHealth() == target->GetHealth() && gorefiend &&
             raidboss::ExactDist2d(gorefiend, construct) < raidboss::ExactDist2d(gorefiend, target)))
        {
            target = construct;
        }
    }

    if (target)
    {
        const float dist = raidboss::ExactDist2d(spirit, target);
        if (dist > 10.0f)
        {
            const float moveDist = dist - 8.0f;
            const float x = spirit->GetPositionX() + (target->GetPositionX() - spirit->GetPositionX()) / dist * moveDist;
            const float y = spirit->GetPositionY() + (target->GetPositionY() - spirit->GetPositionY()) / dist * moveDist;
            spirit->GetMotionMaster()->MovePoint(0, x, y, spirit->GetPositionZ());
            return true;
        }

        spirit->SetFacingToObject(target);

        // Chains first on a fresh construct, volley never on a chained one (it would break the chains)
        if (target->GetHealthPercent() >= 100.0f && CastSpiritSpell(spirit, SPELL_SPIRIT_CHAINS, target))
            return true;
        if (CastSpiritSpell(spirit, SPELL_SPIRIT_LANCE, target))
            return true;
        if (CastSpiritSpell(spirit, SPELL_SPIRIT_CHAINS, target))
            return true;
        if (!target->HasAura(SPELL_SPIRIT_CHAINS) && CastSpiritSpell(spirit, SPELL_SPIRIT_VOLLEY, target))
            return true;

        return false;
    }

    if (!gorefiend)
        return false;

    const float dist = raidboss::ExactDist2d(spirit, gorefiend);
    if (dist > 5.0f)
    {
        const float moveDist = dist - 5.0f;
        const float x = spirit->GetPositionX() + (gorefiend->GetPositionX() - spirit->GetPositionX()) / dist * moveDist;
        const float y = spirit->GetPositionY() + (gorefiend->GetPositionY() - spirit->GetPositionY()) / dist * moveDist;
        spirit->GetMotionMaster()->MovePoint(0, x, y, spirit->GetPositionZ());
        return true;
    }

    return CastSpiritSpell(spirit, SPELL_SPIRIT_STRIKE, gorefiend);
}

// ---------------------------------------------------------------------------
// Gurtogg Bloodboil
// ---------------------------------------------------------------------------

bool GurtoggBloodboilMisdirectBossToMainTankAction::Execute(Event& event)
{
    Unit* gurtogg = FindBoss(ai, NPC_GURTOGG_BLOODBOIL);
    return gurtogg && MisdirectTo(raidboss::GetGroupMainTank(bot), gurtogg);
}

bool GurtoggBloodboilTanksPositionBossAction::Execute(Event& event)
{
    Unit* gurtogg = FindBoss(ai, NPC_GURTOGG_BLOODBOIL);
    if (!gurtogg)
        return false;

    if (!IsCurrentTarget(gurtogg))
        return AttackTarget(gurtogg);

    Unit* victim = gurtogg->GetVictim();
    if (victim && victim->IsPlayer() && PlayerbotAI::IsTank((Player*)victim) && bot->CanReachWithMeleeAttack(gurtogg))
        return StepTowards(GURTOGG_TANK_POSITION.getX(), GURTOGG_TANK_POSITION.getY(), 5.0f, 2.0f);

    return false;
}

bool GurtoggBloodboilRotateRangedGroupsAction::Execute(Event& event)
{
    Unit* gurtogg = FindBoss(ai, NPC_GURTOGG_BLOODBOIL);
    if (!gurtogg)
        return false;

    std::vector<std::vector<Player*>> groups = GetGurtoggRangedRotationGroups(bot);
    const int activeGroup = GetGurtoggActiveRotationGroup(gurtogg);
    const bool inActiveGroup = activeGroup >= 0 && size_t(activeGroup) < groups.size() &&
                               raidboss::GetIndexOf(bot, groups[activeGroup]) >= 0;

    return MoveToPosition(inActiveGroup ? GURTOGG_SOAKER_POSITION : GURTOGG_RANGED_POSITION, 2.0f);
}

bool GurtoggBloodboilRangedMoveAwayFromEnragedPlayerAction::Execute(Event& event)
{
    for (Player* member : raidboss::GetGroupMembers(bot, 100.0f, false))
    {
        if (member->HasAura(SPELL_PLAYER_FEL_RAGE))
            return FleeFrom(member->GetPositionX(), member->GetPositionY(), 20.0f);
    }

    return false;
}

bool GurtoggBloodboilManagePhaseTimerAction::Execute(Event& event)
{
    Unit* gurtogg = FindBoss(ai, NPC_GURTOGG_BLOODBOIL);
    if (!gurtogg)
        return false;

    if (gurtogg->HasAura(SPELL_BOSS_FEL_RAGE))
        return EraseState(BtState::GurtoggPhaseTimer, gurtogg->GetInstanceId());

    return SetStateIfAbsent(BtState::GurtoggPhaseTimer, gurtogg->GetInstanceId(), int64(time(nullptr)));
}

// ---------------------------------------------------------------------------
// Reliquary of Souls
// ---------------------------------------------------------------------------

bool ReliquaryOfSoulsMisdirectBossToMainTankAction::Execute(Event& event)
{
    Unit* target = FindBoss(ai, NPC_ESSENCE_OF_DESIRE);
    if (!target)
        target = FindBoss(ai, NPC_ESSENCE_OF_ANGER);

    if (!target || target->GetHealthPercent() <= 95.0f)
        return false;

    return MisdirectTo(raidboss::GetGroupMainTank(bot), target);
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::Execute(Event& event)
{
    Unit* suffering = FindBoss(ai, NPC_ESSENCE_OF_SUFFERING);
    if (!suffering)
        return false;

    // Essence of Suffering fixates on the closest player: the tanks stay closest
    if (PlayerbotAI::IsTank(bot) && bot->GetHealthPercent() > 25.0f)
        return TanksMoveToMinimumRange(suffering);
    if (raidboss::IsMeleePlayer(bot) && bot->GetVictim() != suffering)
        return MeleeDpsStayAtMaximumRange(suffering);
    if (raidboss::IsRangedPlayer(bot))
        return RangedMoveAwayFromBoss(suffering);

    return false;
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::TanksMoveToMinimumRange(Unit* suffering)
{
    return StepTowards(suffering->GetPositionX(), suffering->GetPositionY(), 1.0f, 2.0f);
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::MeleeDpsStayAtMaximumRange(Unit* suffering)
{
    const float desiredDist = std::max(1.5f, suffering->GetCombatReach() + bot->GetCombatReach() - 0.5f);
    const float behindAngle = suffering->GetOrientation() + M_PI_F;
    const float x = suffering->GetPositionX() + desiredDist * std::cos(behindAngle);
    const float y = suffering->GetPositionY() + desiredDist * std::sin(behindAngle);
    return MoveToPosition(WorldPosition(BLACK_TEMPLE_MAP_ID, x, y, suffering->GetPositionZ()), 0.5f);
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::RangedMoveAwayFromBoss(Unit* suffering)
{
    return FleeFrom(suffering->GetPositionX(), suffering->GetPositionY(), 15.0f, 1000);
}

// Aura of Suffering blocks all healing: healers damage the boss instead
bool ReliquaryOfSoulsHealersDpsSufferingAction::Execute(Event& event)
{
    Unit* suffering = FindBoss(ai, NPC_ESSENCE_OF_SUFFERING);
    if (!suffering)
        return false;

    switch (bot->getClass())
    {
    case CLASS_DRUID:
    {
        if (ai->HasAura("tree of life", bot))
            ai->RemoveAura("tree of life");

        bool casted = CastByName(ai, "barkskin", bot);
        casted |= CastByName(ai, "wrath", suffering);
        return casted;
    }
    case CLASS_PALADIN:
        return CastByName(ai, "avenging wrath", bot) || CastByName(ai, "consecration", bot) ||
               CastByName(ai, "exorcism", suffering) || CastByName(ai, "hammer of wrath", suffering) ||
               CastByName(ai, "holy shock", suffering) || CastByName(ai, "judgement", suffering);
    case CLASS_PRIEST:
        return CastByName(ai, "smite", suffering);
    case CLASS_SHAMAN:
        return CastByName(ai, "earth shock", suffering) || CastByName(ai, "chain lightning", suffering) ||
               CastByName(ai, "lightning bolt", suffering);
    default:
        return false;
    }
}

bool ReliquaryOfSoulsSpellstealRuneShieldAction::Execute(Event& event)
{
    Unit* desire = FindBoss(ai, NPC_ESSENCE_OF_DESIRE);
    return desire && CastByName(ai, "spellsteal", desire);
}

bool ReliquaryOfSoulsSpellReflectDeadenAction::Execute(Event& event)
{
    return CastByName(ai, "spell reflection", bot);
}

// ---------------------------------------------------------------------------
// Mother Shahraz
// ---------------------------------------------------------------------------

bool MotherShahrazMisdirectBossToMainTankAction::Execute(Event& event)
{
    Unit* shahraz = FindBoss(ai, NPC_MOTHER_SHAHRAZ);
    return shahraz && MisdirectTo(raidboss::GetGroupMainTank(bot), shahraz);
}

bool MotherShahrazTanksPositionBossUnderPillarAction::Execute(Event& event)
{
    Unit* shahraz = FindBoss(ai, NPC_MOTHER_SHAHRAZ);
    if (!shahraz)
        return false;

    if (!IsCurrentTarget(shahraz))
        return AttackTarget(shahraz);

    Unit* victim = shahraz->GetVictim();
    if (!victim || !victim->IsPlayer() || !PlayerbotAI::IsTank((Player*)victim))
        return false;

    const uint64 key = GuidKey(bot);
    int64 raw = int64(TankPositionState::MovingToTransition);
    if (!GetState(BtState::ShahrazTankStep, key, raw))
        SetState(BtState::ShahrazTankStep, key, raw);

    const TankPositionState state = TankPositionState(raw);
    const WorldPosition& position = state == TankPositionState::MovingToTransition ? SHAHRAZ_TRANSITION_POSITION : SHAHRAZ_TANK_POSITION;
    const float maxDistance = 0.5f;
    const float dist = bt::Dist2d(bot->GetPositionX(), bot->GetPositionY(), position.getX(), position.getY());

    if (dist > maxDistance && bot->CanReachWithMeleeAttack(shahraz))
        return MoveToPoint(position.getX(), position.getY(), position.getZ());

    if (dist <= maxDistance)
    {
        if (state == TankPositionState::MovingToTransition)
            SetState(BtState::ShahrazTankStep, key, int64(TankPositionState::MovingToFinal));
        else
        {
            bot->SetFacingToObject(shahraz);
            SetState(BtState::ShahrazTankStep, key, int64(TankPositionState::Positioned));
        }
    }

    return false;
}

bool MotherShahrazMeleeDpsWaitAtSafePositionAction::Execute(Event& event)
{
    return MoveToPosition(SHAHRAZ_RANGED_POSITION, 1.0f);
}

// Doesn't matter for bots (no fall damage), but simulates the player strategy of standing under the pillar
bool MotherShahrazPositionRangedUnderPillarAction::Execute(Event& event)
{
    return MoveToPosition(SHAHRAZ_RANGED_POSITION, 1.0f);
}

// The linked players run in different directions from their center
bool MotherShahrazRunAwayToBreakFatalAttractionAction::Execute(Event& event)
{
    std::vector<Player*> attracted = GetFatalAttractionPlayers(bot);
    const int index = raidboss::GetIndexOf(bot, attracted);
    if (attracted.size() < 2 || index < 0)
        return false;

    float centerX = 0.0f, centerY = 0.0f;
    for (Player* member : attracted)
    {
        centerX += member->GetPositionX();
        centerY += member->GetPositionY();
    }

    centerX /= attracted.size();
    centerY /= attracted.size();

    const float spreadAngle = 2.0f * M_PI_F * index / attracted.size();
    float lastX = bot->GetPositionX(), lastY = bot->GetPositionY(), lastZ = bot->GetPositionZ();
    bool found = false;
    for (float distance = 1.0f; distance <= 35.0f; distance += 1.0f)
    {
        const float x = centerX + std::cos(spreadAngle) * distance;
        const float y = centerY + std::sin(spreadAngle) * distance;
        const float z = bot->GetMap()->GetHeight(x, y, lastZ + 4.0f);
        if (z <= INVALID_HEIGHT || std::fabs(z - lastZ) > 3.0f ||
            !bot->GetMap()->IsInLineOfSight(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ() + 2.0f, x, y, z + 2.0f, true))
        {
            break;
        }

        lastX = x;
        lastY = y;
        lastZ = z;
        found = true;
    }

    if (found && MoveToPoint(lastX, lastY, lastZ))
        return true;

    // Stuck: try a 5 yard random move
    const float angle = frand(0.0f, 2.0f * M_PI_F);
    return MoveToPoint(bot->GetPositionX() + std::cos(angle) * 5.0f, bot->GetPositionY() + std::sin(angle) * 5.0f, bot->GetPositionZ());
}

// ---------------------------------------------------------------------------
// Illidari Council
// ---------------------------------------------------------------------------

// Hunter 1: Zerevor -> mage tank, 2: Malande -> first assist tank, 3: Gathios -> main tank, 4: Veras -> second assist tank
bool IllidariCouncilMisdirectBossesToTanksAction::Execute(Event& event)
{
    std::vector<Player*> hunters = GetHunterBots(bot, 4);
    Unit* councilTarget = nullptr;
    Player* tank = nullptr;
    switch (raidboss::GetIndexOf(bot, hunters))
    {
    case 0:
        councilTarget = FindBoss(ai, NPC_ZEREVOR);
        tank = GetZerevorMageTank(bot);
        break;
    case 1:
        councilTarget = FindBoss(ai, NPC_MALANDE);
        tank = raidboss::GetGroupAssistTank(bot, 0);
        break;
    case 2:
        councilTarget = FindBoss(ai, NPC_GATHIOS);
        tank = raidboss::GetGroupMainTank(bot);
        break;
    case 3:
        councilTarget = FindBoss(ai, NPC_VERAS);
        tank = raidboss::GetGroupAssistTank(bot, 1);
        break;
    default:
        return false;
    }

    return councilTarget && MisdirectTo(tank, councilTarget);
}

namespace
{
    // Failsafe of mod-playerbots: bots falling through the floor on the pull
    void CouncilFloorFailsafe(Player* bot, Unit* boss)
    {
        if (bot->GetPositionZ() < COUNCIL_FLOOR_Z_THRESHOLD)
            bot->TeleportTo(BLACK_TEMPLE_MAP_ID, boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(), bot->GetOrientation());
    }
}

bool IllidariCouncilMainTankPositionGathiosAction::Execute(Event& event)
{
    Unit* gathios = FindBoss(ai, NPC_GATHIOS);
    if (!gathios)
        return false;

    CouncilFloorFailsafe(bot, gathios);

    if (raidboss::MarkTargetWithSquare(bot, gathios))
        return true;

    raidboss::SetRtiTarget(ai, "square");

    if (!IsCurrentTarget(gathios))
        return AttackTarget(gathios);

    if (gathios->GetVictim() != bot || !bot->CanReachWithMeleeAttack(gathios))
        return false;

    const uint64 key = GuidKey(bot);
    int64 index = 0;
    GetState(BtState::GathiosTankStep, key, index);

    const WorldPosition* position = &GATHIOS_TANK_POSITIONS[index % 4];
    const float dist = bt::Dist2d(bot->GetPositionX(), bot->GetPositionY(), position->getX(), position->getY());

    // Drag Gathios out of Consecration / Flamestrike / Blizzard to the next corner
    if (dist <= 2.0f && HasDangerousCouncilAura(bot))
    {
        index = (index + 1) % 4;
        SetState(BtState::GathiosTankStep, key, index);
        position = &GATHIOS_TANK_POSITIONS[index];
    }

    return StepTowards(position->getX(), position->getY(), 5.0f, 2.0f);
}

bool IllidariCouncilMainTankReflectJudgementOfCommandAction::Execute(Event& event)
{
    return CastByName(ai, "spell reflection", bot);
}

bool IllidariCouncilFirstAssistTankFocusMalandeAction::Execute(Event& event)
{
    Unit* malande = FindBoss(ai, NPC_MALANDE);
    if (!malande)
        return false;

    CouncilFloorFailsafe(bot, malande);

    if (raidboss::MarkTargetWithStar(bot, malande))
        return true;

    raidboss::SetRtiTarget(ai, "star");
    return !IsCurrentTarget(malande) && AttackTarget(malande);
}

// Veras is tanked next to Gathios so the cleave of the melee hits both
bool IllidariCouncilSecondAssistTankPositionDarkshadowAction::Execute(Event& event)
{
    Unit* darkshadow = FindBoss(ai, NPC_VERAS);
    if (!darkshadow)
        return false;

    CouncilFloorFailsafe(bot, darkshadow);

    if (raidboss::MarkTargetWithCircle(bot, darkshadow))
        return true;

    raidboss::SetRtiTarget(ai, "circle");

    if (!IsCurrentTarget(darkshadow))
        return AttackTarget(darkshadow);

    if (darkshadow->GetVictim() == bot)
    {
        if (Player* mainTank = raidboss::GetGroupMainTank(bot))
            return StepTowards(mainTank->GetPositionX(), mainTank->GetPositionY(), 5.0f, 2.0f);
    }

    return false;
}

bool IllidariCouncilMageTankPositionZerevorAction::Execute(Event& event)
{
    Unit* zerevor = FindBoss(ai, NPC_ZEREVOR);
    if (!zerevor)
        return false;

    if (zerevor->HasAura(SPELL_DAMPEN_MAGIC) && CastByName(ai, "spellsteal", zerevor))
        return true;

    if (raidboss::MarkTargetWithTriangle(bot, zerevor))
        return true;

    raidboss::SetRtiTarget(ai, "triangle");

    if (!IsCurrentTarget(zerevor))
        return AttackTarget(zerevor);

    if (zerevor->GetVictim() == bot)
        return StepTowards(ZEREVOR_TANK_POSITION.getX(), ZEREVOR_TANK_POSITION.getY(), 10.0f, 2.0f);

    return false;
}

bool IllidariCouncilPositionMageTankHealerAction::Execute(Event& event)
{
    Player* mageTank = GetZerevorMageTank(bot);
    Unit* zerevor = FindBoss(ai, NPC_ZEREVOR);
    if (!mageTank || !zerevor || zerevor->GetVictim() != mageTank)
        return false;

    const uint64 key = GuidKey(bot);
    int64 index = 0;
    GetState(BtState::ZerevorHealStep, key, index);

    const WorldPosition* position = &ZEREVOR_HEALER_POSITIONS[index % 2];
    const float dist = bt::Dist2d(bot->GetPositionX(), bot->GetPositionY(), position->getX(), position->getY());
    if (dist <= 1.0f && HasDangerousCouncilAura(bot))
    {
        index = (index + 1) % 2;
        SetState(BtState::ZerevorHealStep, key, index);
        return MoveToPosition(ZEREVOR_HEALER_POSITIONS[index], 1.0f);
    }

    return StepTowards(position->getX(), position->getY(), 10.0f, 1.0f);
}

bool IllidariCouncilAssignDpsTargetsAction::Execute(Event& event)
{
    Unit* malande = FindBoss(ai, NPC_MALANDE);
    if (!malande)
        return false;

    // Physical damage on Malande unless she has Blessing of Protection, shamans unless Spell Warding;
    // nobody on Malande while she stands in Zerevor's AoE
    bool attackMalande = false;
    Unit* zerevor = FindBoss(ai, NPC_ZEREVOR);
    if (zerevor && raidboss::ExactDist2d(zerevor, malande) < 15.0f)
        attackMalande = false;
    else if (bot->getClass() == CLASS_ROGUE || (bot->getClass() == CLASS_WARRIOR && raidboss::IsDps(bot)))
        attackMalande = !malande->HasAura(SPELL_BLESSING_OF_PROTECTION);
    else if (bot->getClass() == CLASS_SHAMAN && raidboss::IsDps(bot))
        attackMalande = !malande->HasAura(SPELL_BLESSING_OF_SPELL_WARDING);

    if (attackMalande)
    {
        raidboss::SetRtiTarget(ai, "star");
        return !IsCurrentTarget(malande) && AttackTarget(malande);
    }

    Unit* darkshadow = FindBoss(ai, NPC_VERAS);
    if (darkshadow && !darkshadow->HasAura(SPELL_VANISH))
    {
        raidboss::SetRtiTarget(ai, "circle");
        return !IsCurrentTarget(darkshadow) && AttackTarget(darkshadow);
    }

    if (Unit* gathios = FindBoss(ai, NPC_GATHIOS))
    {
        raidboss::SetRtiTarget(ai, "square");
        return !IsCurrentTarget(gathios) && AttackTarget(gathios);
    }

    return false;
}

bool IllidariCouncilDisperseRangedAction::Execute(Event& event)
{
    if (Player* nearest = raidboss::GetNearestPlayerInRadius(bot, 4.0f))
        return FleeFrom(nearest->GetPositionX(), nearest->GetPositionY(), 4.0f, 1000);

    return false;
}

bool IllidariCouncilCommandPetsToAttackGathiosAction::Execute(Event& event)
{
    Unit* gathios = FindBoss(ai, NPC_GATHIOS);
    Pet* pet = bot->GetPet();
    if (!gathios || !pet || !pet->IsAlive() || pet->GetVictim() == gathios)
        return false;

    return PetAttack(GetMaster(), gathios);
}

bool IllidariCouncilManageDpsTimerAction::Execute(Event& event)
{
    Unit* gathios = FindBoss(ai, NPC_GATHIOS);
    return gathios && SetStateIfAbsent(BtState::CouncilDpsTimer, gathios->GetInstanceId(), int64(time(nullptr)));
}

// ---------------------------------------------------------------------------
// Illidan Stormrage
// ---------------------------------------------------------------------------

bool IllidanStormrageMisdirectToTankAction::Execute(Event& event)
{
    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan)
        return false;

    const int phase = GetIllidanPhase(illidan);
    if (phase == 2 && TryMisdirectToFlameTanks())
        return true;

    return phase == 4 && TryMisdirectToWarlockTank(illidan);
}

bool IllidanStormrageMisdirectToTankAction::TryMisdirectToFlameTanks()
{
    std::vector<Player*> hunters = GetHunterBots(bot, 2);
    const int index = raidboss::GetIndexOf(bot, hunters);
    if (index < 0)
        return false;

    Unit* eastFlame = nullptr;
    Unit* westFlame = nullptr;
    GetFlamesOfAzzinoth(bot, eastFlame, westFlame);
    if (!eastFlame || !westFlame || eastFlame == westFlame)
        return false;

    Player* firstAssistTank = raidboss::GetGroupAssistTank(bot, 0);
    Player* secondAssistTank = raidboss::GetGroupAssistTank(bot, 1);
    if (!firstAssistTank || !secondAssistTank)
        return false;

    if (hunters.size() == 1)
        return eastFlame->GetHealthPercent() >= 99.0f && MisdirectTo(secondAssistTank, eastFlame);

    Player* tank = index == 0 ? secondAssistTank : firstAssistTank;
    Unit* flame = index == 0 ? eastFlame : westFlame;
    return flame->GetHealthPercent() >= 90.0f && MisdirectTo(tank, flame);
}

bool IllidanStormrageMisdirectToTankAction::TryMisdirectToWarlockTank(Unit* illidan)
{
    return MisdirectTo(GetIllidanWarlockTank(bot), illidan);
}

bool IllidanStormrageMainTankRepositionBossAction::Execute(Event& event)
{
    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan)
        return false;

    if (!IsCurrentTarget(illidan))
        return AttackTarget(illidan);

    if (GetIllidanPhase(illidan) == 5)
    {
        // Drag Illidan over Maiev's Cage Trap (it springs when he gets within 3 yards of it)
        GameObject* trap = FindNearestTrap(ai);
        if (trap && raidboss::ExactDist2d(bot, trap) < 40.0f && illidan->GetVictim() == bot)
            return MoveToShadowTrap(trap);
    }
    else
        trapGuid = ObjectGuid();

    if (illidan->GetVictim() != bot)
    {
        trapGuid = ObjectGuid();
        return false;
    }

    std::vector<Unit*> flameCrashes = GetAllFlameCrashes(bot);
    const float hazardRadius = 12.0f;
    bool inDanger = false;
    for (Unit* flameCrash : flameCrashes)
    {
        if (raidboss::ExactDist2d(bot, flameCrash) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    float x = 0.0f, y = 0.0f;
    if (!inDanger || !FindSafestNearbyPosition(flameCrashes, 30.0f, hazardRadius, x, y))
        return false;

    return MoveToPoint(x, y, bot->GetPositionZ());
}

bool IllidanStormrageMainTankRepositionBossAction::MoveToShadowTrap(GameObject* trap)
{
    if (trapGuid != trap->GetObjectGuid())
    {
        const float dist = raidboss::ExactDist2d(trap, bot);
        if (dist <= 0.0f)
            return false;

        // A point behind the trap, so Illidan has to walk over it
        const float distBeyondTrap = 6.0f;
        trapDestX = trap->GetPositionX() + (trap->GetPositionX() - bot->GetPositionX()) / dist * distBeyondTrap;
        trapDestY = trap->GetPositionY() + (trap->GetPositionY() - bot->GetPositionY()) / dist * distBeyondTrap;
        trapDestZ = trap->GetPositionZ();
        trapGuid = trap->GetObjectGuid();
    }

    if (bot->GetHealthPercent() <= 50.0f)
        return false;

    return StepTowards(trapDestX, trapDestY, 5.0f, 2.0f);
}

// Prefers the direction behind the tank (Illidan follows and the raid behind him stays safe)
bool IllidanStormrageMainTankRepositionBossAction::FindSafestNearbyPosition(const std::vector<Unit*>& flameCrashes, float maxRadius,
                                                                            float hazardRadius, float& outX, float& outY)
{
    const float backwardsAngle = bot->GetOrientation() + M_PI_F;
    bool found = false;
    for (float distance = 2.0f; distance <= maxRadius; distance += 1.0f)
    {
        for (float offset = 0.0f; offset < M_PI_F; offset += M_PI_F / 16.0f)
        {
            for (int sign = -1; sign <= 1; sign += 2)
            {
                const float angle = backwardsAngle + sign * offset;
                const float x = bot->GetPositionX() + distance * std::cos(angle);
                const float y = bot->GetPositionY() + distance * std::sin(angle);

                bool safe = true;
                for (Unit* flameCrash : flameCrashes)
                {
                    if (raidboss::ExactDist2d(flameCrash, x, y) < hazardRadius)
                    {
                        safe = false;
                        break;
                    }
                }

                if (!safe)
                    continue;

                if (IsPathSafe(x, y, flameCrashes, hazardRadius))
                {
                    outX = x;
                    outY = y;
                    return true;
                }

                if (!found)
                {
                    found = true;
                    outX = x;
                    outY = y;
                }
            }
        }
    }

    return found;
}

bool IllidanStormrageMainTankRepositionBossAction::IsPathSafe(float endX, float endY, const std::vector<Unit*>& flameCrashes, float hazardRadius)
{
    const float startX = bot->GetPositionX();
    const float startY = bot->GetPositionY();
    for (uint8 i = 1; i <= 10; ++i)
    {
        const float x = startX + (endX - startX) * i / 10.0f;
        const float y = startY + (endY - startY) * i / 10.0f;
        for (Unit* flameCrash : flameCrashes)
        {
            if (raidboss::ExactDist2d(flameCrash, x, y) < hazardRadius)
                return false;
        }
    }

    return true;
}

bool IllidanStormrageIsolateBotWithParasiteAction::Execute(Event& event)
{
    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan)
        return false;

    if (GetIllidanPhase(illidan) == 1)
    {
        const float safeDistance = 15.0f;
        if (Player* nearest = raidboss::GetNearestPlayerInRadius(bot, safeDistance))
            return MoveAwayFrom(nearest, safeDistance - raidboss::ExactDist2d(bot, nearest));

        return false;
    }

    // Behind Illidan, away from the raid
    const float angle = illidan->GetOrientation() + M_PI_F;
    const float x = illidan->GetPositionX() + std::cos(angle) * 35.0f;
    const float y = illidan->GetPositionY() + std::sin(angle) * 35.0f;

    if (HasParasiticShadowfiend(bot))
        return MoveToPosition(WorldPosition(BLACK_TEMPLE_MAP_ID, x, y, bot->GetPositionZ()), 1.0f);

    if (GetIllidanTrapperHunter(bot) == bot)
        return FreezeTrapShadowfiend(x, y, bot->GetPositionZ());

    return false;
}

// The hunter waits for the infected player behind Illidan and freezes the Shadowfiends with Frost Trap
bool IllidanStormrageIsolateBotWithParasiteAction::FreezeTrapShadowfiend(float targetX, float targetY, float targetZ)
{
    Player* infected = GetBotWithParasiticShadowfiend(bot);
    if (!infected)
        return false;

    if (MoveToPosition(WorldPosition(BLACK_TEMPLE_MAP_ID, targetX, targetY, targetZ), 2.0f))
        return true;

    return raidboss::ExactDist2d(bot, infected) < 2.0f && CastByName(ai, "frost trap", bot);
}

bool IllidanStormrageSetEarthbindTotemAction::Execute(Event& event)
{
    return CastByName(ai, "earthbind totem", bot);
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::Execute(Event& event)
{
    Unit* eastFlame = nullptr;
    Unit* westFlame = nullptr;
    GetFlamesOfAzzinoth(bot, eastFlame, westFlame);

    // The second assist tank's flame dies first: if the tank of the second flame dies after that,
    // the tank of the dead flame becomes the first assist tank and takes over the remaining flame
    const bool demonFiresNearby = !raidboss::FindUnits(bot, NPC_DEMON_FIRE, 40.0f).empty();
    if (raidboss::IsAssistTank(bot, 1))
    {
        if (eastFlame && westFlame)
        {
            if (!IsCurrentTarget(eastFlame))
                return AttackTarget(eastFlame);

            if (eastFlame->GetVictim() != bot)
            {
                if (!bot->CanReachWithMeleeAttack(eastFlame))
                    return MoveToPoint(eastFlame->GetPositionX(), eastFlame->GetPositionY(), eastFlame->GetPositionZ());

                return false;
            }
        }
        else if (!eastFlame && !westFlame)
        {
            // Before the flames spawn: waiting position, after they die: a grate away from the hazards
            if (MoveToPosition(demonFiresNearby ? ILLIDAN_E_GRATE_POSITION : ILLIDAN_E_GLAIVE_WAITING_POSITION, 0.5f))
                return true;
        }
        else if (!eastFlame && westFlame)
        {
            if (MoveToPosition(ILLIDAN_E_GRATE_POSITION, 0.5f))
                return true;
        }
    }
    else if (raidboss::IsAssistTank(bot, 0))
    {
        if (westFlame)
        {
            if (!IsCurrentTarget(westFlame))
                return AttackTarget(westFlame);

            if (westFlame->GetVictim() != bot)
            {
                if (!bot->CanReachWithMeleeAttack(westFlame))
                    return MoveToPoint(westFlame->GetPositionX(), westFlame->GetPositionY(), westFlame->GetPositionZ());

                return false;
            }
        }
        else if (MoveToPosition(demonFiresNearby ? ILLIDAN_W_GRATE_POSITION : ILLIDAN_W_GLAIVE_WAITING_POSITION, 0.5f))
            return true;
    }

    if (!FindBoss(ai, NPC_ILLIDAN))
        return false;

    // Only an Eye Blast whose trigger is within 30 yards matters for the tank
    EyeBlastDangerArea dangerArea = GetEyeBlastDangerArea(bot);
    if (dangerArea.width > 0.0f &&
        bt::Dist2d(bot->GetPositionX(), bot->GetPositionY(), dangerArea.startX, dangerArea.startY) <= 30.0f)
    {
        return RepositionToAvoidEyeBlast(dangerArea);
    }

    return RepositionToAvoidBlaze(eastFlame, westFlame);
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::RepositionToAvoidEyeBlast(const EyeBlastDangerArea& area)
{
    const float px = bot->GetPositionX();
    const float py = bot->GetPositionY();
    if (!IsPositionInEyeBlastDangerArea(px, py, area))
        return false;

    const float dx = area.endX - area.startX;
    const float dy = area.endY - area.startY;
    const float length = bt::Dist2d(area.startX, area.startY, area.endX, area.endY);
    const float projection = std::clamp(((px - area.startX) * dx + (py - area.startY) * dy) / (length * length), 0.0f, 1.0f);
    const float closestX = area.startX + projection * dx;
    const float closestY = area.startY + projection * dy;

    const float moveDist = area.width - bt::Dist2d(px, py, closestX, closestY) + 0.5f;
    if (moveDist <= 0.0f)
        return false;

    const float rawLength = bt::Dist2d(px, py, closestX, closestY);
    const float dirX = rawLength == 0.0f ? -(dy / length) : (px - closestX) / rawLength;
    const float dirY = rawLength == 0.0f ? dx / length : (py - closestY) / rawLength;
    const float safeX = px + dirX * moveDist;
    const float safeY = py + dirY * moveDist;

    // Don't drag the flame onto the raid standing on the grates
    for (const WorldPosition& grate : GRATE_POSITIONS)
    {
        if (bt::Dist2d(safeX, safeY, grate.getX(), grate.getY()) < 10.0f)
            return false;
    }

    return MoveToPoint(safeX, safeY, bot->GetPositionZ());
}

// The flame tanks walk around a circle of waypoints and leave the Blaze behind
bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::RepositionToAvoidBlaze(Unit* eastFlame, Unit* westFlame)
{
    const std::array<WorldPosition, 7>* waypoints = nullptr;
    if (raidboss::IsAssistTank(bot, 1))
    {
        if (!eastFlame || eastFlame->GetVictim() != bot || !bot->CanReachWithMeleeAttack(eastFlame))
            return false;

        waypoints = &E_GLAIVE_TANK_POSITIONS;
    }
    else if (raidboss::IsAssistTank(bot, 0))
    {
        if (!westFlame || westFlame->GetVictim() != bot || !bot->CanReachWithMeleeAttack(westFlame))
            return false;

        waypoints = &W_GLAIVE_TANK_POSITIONS;
    }

    if (!waypoints)
        return false;

    const uint64 key = GuidKey(bot);
    int64 index = 0;
    GetState(BtState::FlameTankWaypoint, key, index);
    index %= int64(waypoints->size());

    const WorldPosition* target = &(*waypoints)[index];
    bool blazeNearby = false;
    for (Unit* blaze : raidboss::FindUnits(bot, NPC_BLAZE, 8.0f))
    {
        if (raidboss::ExactDist2d(bot, blaze) <= 8.0f)
        {
            blazeNearby = true;
            break;
        }
    }

    const float dist = bt::Dist2d(bot->GetPositionX(), bot->GetPositionY(), target->getX(), target->getY());
    if (blazeNearby && dist <= 0.5f)
    {
        index = (index + 1) % int64(waypoints->size());
        SetState(BtState::FlameTankWaypoint, key, index);
        target = &(*waypoints)[index];
        return StepTowards(target->getX(), target->getY(), 5.0f, 0.5f);
    }

    return StepTowards(target->getX(), target->getY(), 3.0f, 0.5f);
}

// Pets grab aggro right away during Phase 2 and wipe the raid if not put on passive.
// Like players, pets cannot melee Illidan in Phase 4.
bool IllidanStormrageControlPetAggressionAction::Execute(Event& event)
{
    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    Pet* pet = bot->GetPet();
    if (!illidan || !pet || !pet->AI())
        return false;

    const int phase = GetIllidanPhase(illidan);
    const uint64 key = GuidKey(bot);
    if (phase == 2 || phase == 4)
    {
        if (!pet->AI()->HasReactState(REACT_PASSIVE) || pet->GetVictim())
        {
            pet->AttackStop();
            SendPetAction(bot, ACT_REACTION, REACT_PASSIVE);
            SendPetAction(bot, ACT_COMMAND, COMMAND_FOLLOW);
            SetState(BtState::PetMadePassive, key, 1);
        }
    }
    else if (EraseState(BtState::PetMadePassive, key))
    {
        SendPetAction(bot, ACT_REACTION, REACT_DEFENSIVE);
    }

    return false;
}

bool IllidanStormragePositionAboveGrateAction::Execute(Event& event)
{
    std::vector<Player*> members;
    for (Player* member : raidboss::GetGroupMembers(bot, 200.0f, true))
    {
        if (!raidboss::IsAssistTank(member, 0) && !raidboss::IsAssistTank(member, 1))
            members.push_back(member);
    }

    std::sort(members.begin(), members.end(), [](Player* a, Player* b) { return a->GetObjectGuid() < b->GetObjectGuid(); });
    const int index = raidboss::GetIndexOf(bot, members);
    if (index < 0)
        return false;

    return MoveToPosition(GRATE_POSITIONS[index % GRATE_POSITIONS.size()], 0.5f);
}

bool IllidanStormrageRemoveDarkBarrageAction::Execute(Event& event)
{
    if (ai->HasAura("ice block", bot))
        return ai->RemoveAura("ice block");
    if (!PlayerbotAI::IsHeal(bot) && ai->HasAura("divine shield", bot))
        return ai->RemoveAura("divine shield");

    if (!bot->HasAura(SPELL_DARK_BARRAGE))
        return false;

    switch (bot->getClass())
    {
    case CLASS_MAGE:    return CastByName(ai, "ice block", bot);
    case CLASS_PALADIN: return CastByName(ai, "divine shield", bot);
    case CLASS_ROGUE:   return CastByName(ai, "cloak of shadows", bot);
    default:            return false;
    }
}

bool IllidanStormrageMoveAwayFromLandingPointAction::Execute(Event& event)
{
    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan)
        return false;

    const float current = raidboss::ExactDist2d(bot, illidan);
    return current < 20.0f && MoveAwayFrom(illidan, 20.0f - current);
}

bool IllidanStormrageDisperseRangedAction::Execute(Event& event)
{
    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan)
        return false;

    if (GetIllidanPhase(illidan) == 4)
        return SpreadInCircleInDemonPhase(illidan);

    // The infected bot and its trapper have their own job
    Player* infected = GetBotWithParasiticShadowfiend(bot);
    if (infected == bot || (infected && GetIllidanTrapperHunter(bot) == bot))
        return false;

    return FanOutBehindInHumanPhase(illidan);
}

// Healers at 18 and ranged dps at 25 yards in a half circle behind Illidan
bool IllidanStormrageDisperseRangedAction::FanOutBehindInHumanPhase(Unit* illidan)
{
    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (Player* member : GetRangedMembers(bot))
        (PlayerbotAI::IsHeal(member) ? healers : rangedDps).push_back(member);

    const bool isHealer = PlayerbotAI::IsHeal(bot);
    const std::vector<Player*>& members = isHealer ? healers : rangedDps;
    const size_t count = members.size();
    const int index = std::max(0, raidboss::GetIndexOf(bot, members));

    const float arcSpan = M_PI_F;
    const float arcCenter = illidan->GetOrientation() + M_PI_F;
    const float arcStart = arcCenter - arcSpan / 2.0f;
    const float radius = isHealer ? 18.0f : 25.0f;
    const float angle = count <= 1 ? arcCenter : arcStart + arcSpan * float(index) / float(count - 1);

    const float x = illidan->GetPositionX() + radius * std::cos(angle);
    const float y = illidan->GetPositionY() + radius * std::sin(angle);

    for (Unit* flameCrash : GetAllFlameCrashes(bot))
    {
        if (raidboss::ExactDist2d(flameCrash, x, y) < 12.0f)
            return false;
    }

    return MoveToPosition(WorldPosition(BLACK_TEMPLE_MAP_ID, x, y, bot->GetPositionZ()), 1.0f);
}

// Shadow Blast splashes 20 yards around the warlock tank: spread in the 240 degrees away from him
bool IllidanStormrageDisperseRangedAction::SpreadInCircleInDemonPhase(Unit* illidan)
{
    Player* warlockTank = GetIllidanWarlockTank(bot);
    if (!warlockTank)
    {
        if (FleeFrom(illidan->GetPositionX(), illidan->GetPositionY(), 24.0f))
            return true;

        if (Player* nearest = raidboss::GetNearestPlayerInRadius(bot, 6.0f))
            return FleeFrom(nearest->GetPositionX(), nearest->GetPositionY(), 6.0f, 1000);

        return false;
    }

    std::vector<Player*> ranged = GetRangedMembers(bot);
    if (ranged.empty())
        return false;

    const size_t count = ranged.size();
    const int index = std::max(0, raidboss::GetIndexOf(bot, ranged));

    const float warlockAngle = std::atan2(warlockTank->GetPositionY() - illidan->GetPositionY(),
                                          warlockTank->GetPositionX() - illidan->GetPositionX());
    const float forbiddenArc = 2.0f / 3.0f * M_PI_F;
    const float allowedArc = 4.0f / 3.0f * M_PI_F;
    const float arcStart = warlockAngle + forbiddenArc / 2.0f;
    const float angle = count == 1 ? arcStart + allowedArc / 2.0f : arcStart + allowedArc * float(index) / float(count - 1);

    const float x = illidan->GetPositionX() + 25.0f * std::cos(angle);
    const float y = illidan->GetPositionY() + 25.0f * std::sin(angle);
    if (MoveToPosition(WorldPosition(BLACK_TEMPLE_MAP_ID, x, y, bot->GetPositionZ()), 1.0f))
        return true;

    const float distFromTank = raidboss::ExactDist2d(bot, warlockTank);
    return distFromTank < 25.0f && MoveAwayFrom(warlockTank, 25.0f - distFromTank);
}

// Melee cannot attack Demon Form Illidan, but Shadow Demons and Shadowfiends far enough from him
bool IllidanStormrageMeleeGoSomewhereToNotDieAction::Execute(Event& event)
{
    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan)
        return false;

    Unit* illidanVictim = illidan->GetVictim();
    Unit* shadowDemon = FindNearest(bot, NPC_SHADOW_DEMON, 25.0f);
    if (shadowDemon && raidboss::ExactDist2d(shadowDemon, illidan) > 15.0f &&
        (!illidanVictim || raidboss::ExactDist2d(shadowDemon, illidanVictim) > 24.0f))
    {
        return false;
    }

    Unit* shadowfiend = FindNearest(bot, NPC_PARASITIC_SHADOWFIEND, 15.0f);
    if (shadowfiend && raidboss::ExactDist2d(shadowfiend, illidan) > 15.0f && shadowfiend->GetHealthPercent() < 30.0f &&
        (!illidanVictim || raidboss::ExactDist2d(shadowfiend, illidanVictim) > 24.0f))
    {
        return false;
    }

    // 30 yards is closer than ideal but lets melee reach the adds in time
    const float distFromBoss = raidboss::ExactDist2d(bot, illidan);
    if (distFromBoss < 30.0f && MoveAwayFrom(illidan, 30.0f - distFromBoss))
        return true;

    if (Player* warlockTank = GetIllidanWarlockTank(bot))
    {
        const float distFromTank = raidboss::ExactDist2d(bot, warlockTank);
        if (distFromTank < 25.0f && MoveAwayFrom(warlockTank, 25.0f - distFromTank))
            return true;
    }

    if (Player* nearest = raidboss::GetNearestPlayerInRadius(bot, 6.0f))
        MoveAwayFrom(nearest, 6.0f - raidboss::ExactDist2d(bot, nearest));

    return true;
}

bool IllidanStormrageWarlockTankHandleDemonBossAction::Execute(Event& event)
{
    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan)
        return false;

    const float current = raidboss::ExactDist2d(bot, illidan);
    if (current < 24.0f && MoveAwayFrom(illidan, 24.0f - current))
        return true;

    if (CastByName(ai, "shadow ward", bot))
        return true;

    return CastByName(ai, "searing pain", illidan);
}

bool IllidanStormrageDpsPrioritizeAddsAction::Execute(Event& event)
{
    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan)
        return false;

    const int phase = GetIllidanPhase(illidan);
    std::vector<Unit*> targets;
    if (phase == 4)
    {
        Unit* shadowDemon = FindNearest(bot, NPC_SHADOW_DEMON, 35.0f);
        if (GetIllidanWarlockTank(bot) == bot)
            targets = { shadowDemon, illidan };
        else
        {
            Unit* shadowfiend = FindNearest(bot, NPC_PARASITIC_SHADOWFIEND, 35.0f);
            if (raidboss::IsRangedPlayer(bot))
            {
                if (shadowDemon)
                    targets = { shadowDemon };
                else if (shadowfiend && raidboss::ExactDist2d(bot, shadowfiend) > 10.0f)
                    targets = { shadowfiend };
                else
                    targets = { illidan };
            }
            else
                targets = { shadowDemon, shadowfiend };
        }
    }
    else if (raidboss::IsRangedPlayer(bot))
    {
        if (phase == 1 || phase == 3 || phase == 5)
        {
            Unit* shadowfiend = FindNearest(bot, NPC_PARASITIC_SHADOWFIEND, 35.0f);
            if (shadowfiend && raidboss::ExactDist2d(bot, shadowfiend) > 10.0f)
                targets = { shadowfiend };
            else
                targets = { illidan };
        }
        else if (phase == 2)
        {
            Unit* shadowfiend = FindNearest(bot, NPC_PARASITIC_SHADOWFIEND, 20.0f);
            if (shadowfiend && raidboss::ExactDist2d(bot, shadowfiend) > 5.0f)
                targets = { shadowfiend };
            else
            {
                Unit* eastFlame = nullptr;
                Unit* westFlame = nullptr;
                GetFlamesOfAzzinoth(bot, eastFlame, westFlame);
                targets = { eastFlame, westFlame };
            }
        }
    }

    for (Unit* candidate : targets)
    {
        if (candidate && candidate->IsAlive())
            return !IsCurrentTarget(candidate) && AttackTarget(candidate);
    }

    return false;
}

bool IllidanStormrageManageDpsTimerAndRtiAction::Execute(Event& event)
{
    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan)
        return false;

    const uint32 instanceId = illidan->GetInstanceId();
    const int64 now = int64(time(nullptr));
    const int phase = GetIllidanPhase(illidan);

    int64 lastPhase = -1;
    GetState(BtState::IllidanLastPhase, instanceId, lastPhase);
    SetState(BtState::IllidanLastPhase, instanceId, phase);

    bool updated = false;
    if (lastPhase != phase)
    {
        if (phase == 1 || phase == 3 || phase == 4 || phase == 5)
        {
            SetState(BtState::IllidanBossDpsTimer, instanceId, now);
            updated = true;
        }
        else if (phase == 2)
            updated |= EraseState(BtState::IllidanBossDpsTimer, instanceId);

        if (phase != 2)
            updated |= EraseState(BtState::IllidanFlameDpsTimer, instanceId);
    }

    if (phase == 2)
    {
        int64 raw = 0;
        if (!GetState(BtState::IllidanEastFlame, instanceId, raw) && !GetState(BtState::IllidanWestFlame, instanceId, raw))
        {
            std::list<Unit*> flames;
            for (Unit* flame : TbcRaidHelper::FindCreaturesAround(illidan, NPC_FLAME_OF_AZZINOTH, 50.0f))
            {
                if (flame->IsAlive())
                    flames.push_back(flame);
            }

            if (flames.size() == 2)
            {
                Unit* first = flames.front();
                Unit* second = flames.back();
                const float x = ILLIDAN_E_GLAIVE_WAITING_POSITION.getX();
                const float y = ILLIDAN_E_GLAIVE_WAITING_POSITION.getY();
                if (raidboss::ExactDist2d(second, x, y) < raidboss::ExactDist2d(first, x, y))
                    std::swap(first, second);

                SetState(BtState::IllidanEastFlame, instanceId, int64(first->GetObjectGuid().GetRawValue()));
                SetState(BtState::IllidanWestFlame, instanceId, int64(second->GetObjectGuid().GetRawValue()));
                SetState(BtState::IllidanFlameDpsTimer, instanceId, now);
                updated = true;
            }
        }
    }
    else
    {
        updated |= EraseState(BtState::IllidanEastFlame, instanceId);
        updated |= EraseState(BtState::IllidanWestFlame, instanceId);
    }

    return updated;
}
