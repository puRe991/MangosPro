// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/TK/TKTriggers) and adapted for this core.
#include "playerbot/playerbot.h"
#include "TkTriggers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "Movement/MoveSpline.h"

using namespace ai;
using namespace ai::tk;

// General

bool TkNoEncounterInProgressTrigger::IsActive()
{
    if (bot->GetMapId() != TK_MAP_ID || IsTkEncounterInProgress(bot))
        return false;

    return raidboss::IsMechanicTrackerBot(bot, TK_MAP_ID);
}

bool TkStuckFallingTrigger::IsActive()
{
    if (bot->GetMapId() != TK_MAP_ID || !bot->IsFalling() || !bot->movespline->Finalized())
        return false;

    return !IsTkEncounterInProgress(bot);
}

// Trash

bool TkCrimsonHandCenturionCastsArcaneFlurryTrigger::IsActive()
{
    return bot->GetMapId() == TK_MAP_ID && bot->getClass() == CLASS_MAGE && GetCenturionCastingArcaneFlurry(ai);
}

// Al'ar <Phoenix God>

bool TkAlarPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    return alar && alar->GetHealthPercent() > raidboss::BOSS_ENGAGED_HEALTH_PCT;
}

bool TkAlarFliesBetweenPlatformsTrigger::IsActiveInEncounter()
{
    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
        locationIndex = GetAlarDestinationLocationIndex(alar);

    return locationIndex != POINT_QUILL_OR_DIVE_IDX && locationIndex != POINT_MIDDLE_IDX;
}

bool TkAlarEmbersExplodeUponDeathTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && raidboss::FindUnit(ai, NPC_EMBER_OF_ALAR);
}

bool TkAlarShouldAssignNonTankTargetTrigger::IsActiveInEncounter()
{
    return !PlayerbotAI::IsTank(bot) && raidboss::FindUnit(ai, NPC_ALAR);
}

bool TkAlarIncomingFlameQuillsTrigger::IsActiveInEncounter()
{
    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return false;

    return GetAlarCurrentLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX ||
        GetAlarDestinationLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX;
}

bool TkAlarRisingFromTheAshesTrigger::IsActiveInEncounter()
{
    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return false;

    // Al'ar prevents its death at 0% and turns passive for the rebirth
    if (alar->GetHealthPercent() > 5.0f && !IsAlarPassive(alar))
        return false;

    return GetAlarCurrentLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX &&
        GetAlarDestinationLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX;
}

bool TkAlarInPhase2Trigger::IsActiveInEncounter()
{
    Unit* alar = raidboss::FindUnit(ai, NPC_ALAR);
    return alar && IsAlarInPhase2(alar->GetInstanceId());
}

bool TkAlarShouldManagePhaseTrackerTrigger::IsActiveInEncounter()
{
    return raidboss::IsMechanicTrackerBot(bot, TK_MAP_ID) && raidboss::FindUnit(ai, NPC_ALAR);
}

// Void Reaver

bool TkVoidReaverShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && raidboss::FindUnit(ai, NPC_VOID_REAVER);
}

bool TkVoidReaverKnockAwayPullsAggroToNonTanksTrigger::IsActiveInEncounter()
{
    // Classes without an aggro dump ability
    if (bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_SHAMAN || bot->getClass() == CLASS_WARRIOR)
        return false;

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* voidReaver = raidboss::FindUnit(ai, NPC_VOID_REAVER);
    return voidReaver && voidReaver->GetVictim() == bot;
}

bool TkVoidReaverRangedShouldStandBackTrigger::IsActiveInEncounter()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    Unit* voidReaver = raidboss::FindUnit(ai, NPC_VOID_REAVER);
    if (!voidReaver || voidReaver->GetVictim() == bot)
        return false;

    return !IsNearArcaneOrb(bot, GetActiveArcaneOrbs(voidReaver), ARCANE_ORB_BUFFER_DISTANCE);
}

bool TkVoidReaverArcaneOrbIsIncomingTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* voidReaver = raidboss::FindUnit(ai, NPC_VOID_REAVER);
    if (!voidReaver || voidReaver->GetVictim() == bot)
        return false;

    return IsNearArcaneOrb(bot, GetActiveArcaneOrbs(voidReaver), ARCANE_ORB_SAFE_DISTANCE);
}

// High Astromancer Solarian

bool TkSolarianShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!raidboss::IsMainTank(bot))
        return false;

    return IsSolarianTankable(raidboss::FindUnit(ai, NPC_SOLARIAN));
}

bool TkSolarianWrathOfTheAstromancerTrigger::IsActiveInEncounter()
{
    return HasWrathOfTheAstromancer(bot);
}

bool TkSolarianSolariumPriestsSpawnedTrigger::IsActiveInEncounter()
{
    if (!raidboss::IsMeleePlayer(bot) || raidboss::IsMainTank(bot))
        return false;

    return raidboss::FindUnit(ai, NPC_SOLARIUM_PRIEST);
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool TkKaelthasChasedByThaladredTrigger::IsActiveInEncounter()
{
    Unit* thaladred = raidboss::FindUnit(ai, NPC_THALADRED);
    if (!thaladred || thaladred->GetVictim() != bot)
        return false;

    const uint32 phase = GetKaelthasTkPhase(ai, FindKaelthas(ai));
    if (PlayerbotAI::IsTank(bot) && phase == PHASE_ALL_ADVISORS)
        return false;

    return phase != PHASE_NONE;
}

bool TkKaelthasPullingTankableAdvisorsTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    const uint32 phase = GetKaelthasTkPhase(ai, FindKaelthas(ai));
    return phase == PHASE_SINGLE_ADVISOR || phase == PHASE_ALL_ADVISORS;
}

bool TkKaelthasSanguinarOrTelonicusShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (raidboss::IsMainTank(bot))
        return IsAdvisorActive(raidboss::FindUnit(ai, NPC_SANGUINAR));

    if (raidboss::IsAssistTank(bot, 0))
        return IsAdvisorActive(raidboss::FindUnit(ai, NPC_TELONICUS));

    return false;
}

bool TkKaelthasCapernianShouldBeTankedByWarlockTrigger::IsActiveInEncounter()
{
    return IsCapernianTank(bot) && IsAdvisorActive(raidboss::FindUnit(ai, NPC_CAPERNIAN));
}

bool TkKaelthasShouldStandBackFromCapernianTrigger::IsActiveInEncounter()
{
    return IsAdvisorActive(raidboss::FindUnit(ai, NPC_CAPERNIAN)) && !IsCapernianTank(bot);
}

bool TkKaelthasShouldHoldPhase3PositionsTrigger::IsActiveInEncounter()
{
    if (GetKaelthasTkPhase(ai, FindKaelthas(ai)) != PHASE_ALL_ADVISORS)
        return false;

    Unit* sanguinar = raidboss::FindUnit(ai, NPC_SANGUINAR);

    // The designated healer stays in position by the melee tanks while the rest of the raid runs
    // all over the place to kite and kill Thaladred.
    if (IsFirstAssistHealer(bot))
        return sanguinar != nullptr;

    // Sanguinar is a proxy for the revival/Kael talk phase (any non-selectable advisor would do,
    // since all four revive together, but Sanguinar is already needed for the healer).
    if (!IsAdvisorReviving(sanguinar))
        return false;

    return raidboss::IsMainTank(bot) || raidboss::IsAssistTank(bot, 0) || IsCapernianTank(bot);
}

bool TkKaelthasDeterminingAdvisorKillOrderTrigger::IsActiveInEncounter()
{
    if (raidboss::IsMainTank(bot) || raidboss::IsAssistTank(bot, 0))
        return false;

    const uint32 phase = GetKaelthasTkPhase(ai, FindKaelthas(ai));
    return phase == PHASE_SINGLE_ADVISOR || phase == PHASE_ALL_ADVISORS;
}

bool TkKaelthasShouldManageAdvisorDpsTimerTrigger::IsActiveInEncounter()
{
    if (!raidboss::IsMechanicTrackerBot(bot, TK_MAP_ID))
        return false;

    return GetKaelthasTkPhase(ai, FindKaelthas(ai)) == PHASE_SINGLE_ADVISOR;
}

bool TkKaelthasLegendaryWeaponsAreAliveTrigger::IsActiveInEncounter()
{
    if (raidboss::IsMainTank(bot))
        return false;

    return GetKaelthasTkPhase(ai, FindKaelthas(ai)) == PHASE_WEAPONS;
}

bool TkKaelthasLegendaryAxeCastsWhirlwindTrigger::IsActiveInEncounter()
{
    if (!raidboss::IsMainTank(bot))
        return false;

    const uint32 phase = GetKaelthasTkPhase(ai, FindKaelthas(ai));
    if (phase < PHASE_WEAPONS || phase > PHASE_ALL_ADVISORS)
        return false;

    return GetLegendaryWeapon(ai, NPC_DEVASTATION) != nullptr;
}

bool TkKaelthasLegendaryWeaponsAreDeadTrigger::IsActiveInEncounter()
{
    const uint32 phase = GetKaelthasTkPhase(ai, FindKaelthas(ai));
    if (phase < PHASE_WEAPONS || phase > PHASE_ALL_ADVISORS)
        return false;

    Unit* axe = GetLegendaryWeapon(ai, NPC_DEVASTATION);
    if (axe && axe->GetVictim() == bot)
        return false;

    return HasDeadLegendaryWeapon(ai);
}

bool TkKaelthasLegendaryWeaponsAreEquippedTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsHeal(bot) || IsMeleeDps(bot))
        return false;

    if (!FindKaelthas(ai))
        return false;

    return GetEquippedItemInSlot(bot, EQUIPMENT_SLOT_MAINHAND, ITEM_STAFF_OF_DISINTEGRATION) ||
        GetEquippedItemInSlot(bot, EQUIPMENT_SLOT_RANGED, ITEM_NETHERSTRAND_LONGBOW) ||
        GetEquippedItemInSlot(bot, EQUIPMENT_SLOT_OFFHAND, ITEM_PHASESHIFT_BULWARK);
}

bool TkKaelthasLegendaryWeaponsWereLostTrigger::IsActive()
{
    if (bot->GetMapId() != TK_MAP_ID || bot->IsInCombat() || !bot->IsAlive())
        return false;

    if (IsTkEncounterInProgress(bot))
        return false;

    if (Dist2d(bot, KAELTHAS_ROOM_CENTER) > KAELTHAS_ROOM_SEARCH_DISTANCE)
        return false;

    const uint8 weaponSlots[] = { EQUIPMENT_SLOT_MAINHAND, EQUIPMENT_SLOT_OFFHAND, EQUIPMENT_SLOT_RANGED };
    for (uint8 slot : weaponSlots)
    {
        if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot) && GetEquippableItemForSlot(bot, slot))
            return true;
    }

    return false;
}

bool TkKaelthasHasEnteredTheFightTrigger::IsActiveInEncounter()
{
    return GetKaelthasTkPhase(ai, FindKaelthas(ai)) == PHASE_FINAL;
}

bool TkKaelthasShouldAssignFinalPhaseTargetTrigger::IsActiveInEncounter()
{
    if (raidboss::IsMainTank(bot))
        return false;

    Unit* kaelthas = FindKaelthas(ai);
    if (!kaelthas || kaelthas->GetVictim() == bot)
        return false;

    return GetKaelthasTkPhase(ai, kaelthas) == PHASE_FINAL;
}

bool TkKaelthasRaidMemberIsMindControlledTrigger::IsActiveInEncounter()
{
    if (IsCaster(bot))
        return false;

    Unit* kaelthas = FindKaelthas(ai);
    if (!kaelthas)
        return false;

    if (PlayerbotAI::IsTank(bot) && kaelthas->GetVictim() == bot)
        return false;

    if (!bot->HasItemCount(ITEM_INFINITY_BLADE, 1))
        return false;

    for (Player* member : raidboss::GetGroupMembers(bot, 100.0f, false))
    {
        if (member->HasAura(SPELL_KAELTHAS_MIND_CONTROL))
            return true;
    }

    return false;
}

bool TkKaelthasInGravityLapsePhaseTrigger::IsActiveInEncounter()
{
    constexpr float gravityLapseHpThreshold = 50.0f;
    Unit* kaelthas = FindKaelthas(ai);
    return kaelthas && kaelthas->GetHealthPercent() <= gravityLapseHpThreshold;
}
