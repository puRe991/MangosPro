// Ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara/KaraTriggers) and adapted for this core.
#include "playerbot/playerbot.h"
#include "KarazhanTriggers.h"
#include "KarazhanHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;
using namespace ai::karazhan;

// General

// Non combat trigger: resets the encounter state (timers, beam blockers, run index) after a fight
bool KarazhanBotIsNotInCombatTrigger::IsActive()
{
    return bot->GetMapId() == KARAZHAN_MAP_ID && !bot->IsInCombat();
}

bool KarazhanEnemiesCastFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    if (AI_VALUE2(bool, "has totem", "tremor totem"))
        return false;

    return AreFearCastersPresent(ai);
}

// Trash

bool KarazhanManaWarpIsAboutToExplodeTrigger::IsActive()
{
    // Classes without a stun
    if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_PRIEST)
        return false;

    for (Unit* manaWarp : GetKaraUnits(ai, NPC_MANA_WARP))
    {
        if (manaWarp->GetHealthPercent() <= 15.0f && raidboss::ExactDist2d(bot, manaWarp) <= 40.0f)
            return true;
    }

    return false;
}

// Attumen the Huntsman
// In this core Midnight and the unmounted Attumen despawn when the mounted Attumen is summoned,
// so phase two is detected by the mounted Attumen alone.

bool KarazhanAttumenPhaseOneActiveTrigger::IsActive()
{
    return GetKaraUnit(ai, NPC_MIDNIGHT) && !GetAttumenMounted(ai);
}

bool KarazhanAttumenPhaseTwoActiveTrigger::IsActive()
{
    return GetAttumenMounted(ai) != nullptr;
}

bool KarazhanAttumenPhaseTransitionTrigger::IsActive()
{
    if (!raidboss::IsMechanicTrackerBot(bot, KARAZHAN_MAP_ID))
        return false;

    Unit* attumen = GetAttumenMounted(ai);
    time_t start = 0;
    return attumen && !GetTimer(bot->GetInstanceId(), KaraTimer::ATTUMEN_DPS_WAIT, start);
}

// Moroes

bool KarazhanMoroesShouldPrioritizeAddsTrigger::IsActive()
{
    return raidboss::IsMechanicTrackerBot(bot, KARAZHAN_MAP_ID) && GetKaraUnit(ai, NPC_MOROES);
}

// Maiden of Virtue

bool KarazhanMaidenBossEngagedByTanksTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && GetKaraUnit(ai, NPC_MAIDEN_OF_VIRTUE);
}

bool KarazhanMaidenGroundingTotemConsumesHolyFireTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    if (!GetKaraUnit(ai, NPC_MAIDEN_OF_VIRTUE))
        return false;

    return !AI_VALUE2(bool, "has totem", "grounding totem");
}

bool KarazhanMaidenHolyWrathDealsChainDamageTrigger::IsActive()
{
    return raidboss::IsRangedPlayer(bot) && GetKaraUnit(ai, NPC_MAIDEN_OF_VIRTUE);
}

// The Big Bad Wolf

bool KarazhanBigBadWolfBossEngagedByTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!GetKaraUnit(ai, NPC_BIG_BAD_WOLF))
        return false;

    return !HasLittleRedRidingHood(bot);
}

bool KarazhanBigBadWolfChasingLittleRedRidingHoodTrigger::IsActive()
{
    return HasLittleRedRidingHood(bot) && GetKaraUnit(ai, NPC_BIG_BAD_WOLF);
}

// Romulo and Julianne

bool KarazhanRomuloAndJulianneBothBossesRevivedTrigger::IsActive()
{
    if (!raidboss::IsMechanicTrackerBot(bot, KARAZHAN_MAP_ID))
        return false;

    return GetKaraUnit(ai, NPC_ROMULO) && GetKaraUnit(ai, NPC_JULIANNE);
}

// The Wizard of Oz

bool KarazhanWizardOfOzNeedTargetPriorityTrigger::IsActive()
{
    if (!raidboss::IsMechanicTrackerBot(bot, KARAZHAN_MAP_ID))
        return false;

    for (uint32 entry : OZ_TARGETS)
    {
        if (GetKaraUnit(ai, entry))
            return true;
    }

    return false;
}

bool KarazhanWizardOfOzStrawmanIsVulnerableToFireTrigger::IsActive()
{
    return (bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_WARLOCK) &&
        GetKaraUnit(ai, NPC_STRAWMAN);
}

// The Curator

bool KarazhanCuratorAstralFlareSpawnedTrigger::IsActive()
{
    return raidboss::IsMechanicTrackerBot(bot, KARAZHAN_MAP_ID) && GetAstralFlare(ai);
}

bool KarazhanCuratorBossEngagedByTanksTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && GetKaraUnit(ai, NPC_CURATOR);
}

bool KarazhanCuratorBossEngagedByRangedTrigger::IsActive()
{
    return raidboss::IsRangedPlayer(bot) && GetKaraUnit(ai, NPC_CURATOR);
}

// Terestian Illhoof

bool KarazhanIllhoofShouldPrioritizeChainsTrigger::IsActive()
{
    return raidboss::IsMechanicTrackerBot(bot, KARAZHAN_MAP_ID) && GetKaraUnit(ai, NPC_TERESTIAN_ILLHOOF);
}

// Shade of Aran

bool KarazhanAranArcaneExplosionIsCastingTrigger::IsActive()
{
    Unit* aran = GetKaraUnit(ai, NPC_SHADE_OF_ARAN);
    return aran && IsAranCastingArcaneExplosion(aran) && !IsFlameWreathActive(ai) &&
        raidboss::ExactDist2d(bot, aran) < ARAN_ESCAPE_DISTANCE;
}

bool KarazhanAranFlameWreathIsActiveTrigger::IsActive()
{
    return IsFlameWreathActive(ai) && bot->IsMoving();
}

bool KarazhanAranConjuredElementalsSummonedTrigger::IsActive()
{
    return raidboss::IsMechanicTrackerBot(bot, KARAZHAN_MAP_ID) && GetKaraUnit(ai, NPC_CONJURED_ELEMENTAL);
}

bool KarazhanAranBossCastsCounterspellNearbyTrigger::IsActive()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    Unit* aran = GetKaraUnit(ai, NPC_SHADE_OF_ARAN);
    if (!aran)
        return false;

    if (bot->HasAura(SPELL_BLIZZARD))
        return false;

    return !IsAranCastingArcaneExplosion(aran) && !IsFlameWreathActive(ai);
}

// Netherspite

namespace
{
    bool IsBeamActive(PlayerbotAI* ai, uint32 portalEntry)
    {
        Unit* netherspite = GetKaraUnit(ai, NPC_NETHERSPITE);
        if (!netherspite || IsBanishPhase(netherspite))
            return false;

        if (!GetKaraUnit(ai, portalEntry))
            return false;
        const auto blockers = GetCurrentBeamBlockers(ai->GetBot());
        Player* assigned = portalEntry == NPC_RED_PORTAL ? blockers.red :
            portalEntry == NPC_BLUE_PORTAL ? blockers.blue : blockers.green;
        return assigned == ai->GetBot();
    }
}

bool KarazhanNetherspiteRedBeamIsActiveTrigger::IsActive()
{
    return IsBeamActive(ai, NPC_RED_PORTAL);
}

bool KarazhanNetherspiteBlueBeamIsActiveTrigger::IsActive()
{
    return IsBeamActive(ai, NPC_BLUE_PORTAL);
}

bool KarazhanNetherspiteGreenBeamIsActiveTrigger::IsActive()
{
    return IsBeamActive(ai, NPC_GREEN_PORTAL);
}

bool KarazhanNetherspiteBotIsNotBeamBlockerTrigger::IsActive()
{
    Unit* netherspite = GetKaraUnit(ai, NPC_NETHERSPITE);
    if (!netherspite || IsBanishPhase(netherspite))
        return false;

    const BeamBlockers blockers = GetCurrentBeamBlockers(bot);
    return bot != blockers.red && bot != blockers.blue && bot != blockers.green;
}

bool KarazhanNetherspiteBossIsBanishedTrigger::IsActive()
{
    Unit* netherspite = GetKaraUnit(ai, NPC_NETHERSPITE);
    return netherspite && IsBanishPhase(netherspite);
}

bool KarazhanNetherspiteManageTimersAndTrackersTrigger::IsActive()
{
    return GetKaraUnit(ai, NPC_NETHERSPITE) != nullptr;
}

// Prince Malchezaar

bool KarazhanMalchezaarBotIsEnfeebledTrigger::IsActive()
{
    return bot->HasAura(SPELL_ENFEEBLE) && NeedsEmergencyEscape(ai);
}

bool KarazhanMalchezaarEngagedByNonTanksTrigger::IsActive()
{
    Unit* malchezaar = GetKaraUnit(ai, NPC_PRINCE_MALCHEZAAR);
    if (!malchezaar)
        return false;

    if (bot->HasAura(SPELL_ENFEEBLE))
        return false;

    if ((PlayerbotAI::IsTank(bot) && malchezaar->GetVictim() == bot) || raidboss::IsMainTank(bot))
        return false;

    return true;
}

bool KarazhanMalchezaarBossEngagedByTanksTrigger::IsActive()
{
    Unit* malchezaar = GetKaraUnit(ai, NPC_PRINCE_MALCHEZAAR);
    if (!malchezaar)
        return false;

    return (PlayerbotAI::IsTank(bot) && malchezaar->GetVictim() == bot) || raidboss::IsMainTank(bot);
}

// Nightbane

bool KarazhanNightbaneBossEngagedByTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* nightbane = GetKaraUnit(ai, NPC_NIGHTBANE);
    return nightbane && !IsNightbaneFlying(nightbane);
}

bool KarazhanNightbaneGroundPhaseEngagedByRangedTrigger::IsActive()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    Unit* nightbane = GetKaraUnit(ai, NPC_NIGHTBANE);
    return nightbane && !IsNightbaneFlying(nightbane);
}

bool KarazhanNightbanePetsIgnoreCollisionTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!GetKaraUnit(ai, NPC_NIGHTBANE))
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive();
}

bool KarazhanNightbaneBossIsFlyingTrigger::IsActive()
{
    return IsNightbaneFlightPhaseActive(GetKaraUnit(ai, NPC_NIGHTBANE));
}

bool KarazhanNightbaneBotWentOutOfBoundsTrigger::IsActive()
{
    if (!GetKaraUnit(ai, NPC_NIGHTBANE))
        return false;

    // Only around the Master's Terrace (the tower below is in range of the creature search)
    if (raidboss::ExactDist2d(bot, TERRACE_DOME_CENTER.x, TERRACE_DOME_CENTER.y) > 80.0f)
        return false;

    const float outOfBoundsLeeway = 5.0f;
    return bot->GetPositionZ() < NIGHTBANE_GROUND_Z - outOfBoundsLeeway;
}

bool KarazhanNightbaneManageTimersAndTrackersTrigger::IsActive()
{
    return GetKaraUnit(ai, NPC_NIGHTBANE) != nullptr;
}
