// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/Hyjal/HyjalTriggers) and adapted for this core.
#include "playerbot/playerbot.h"
#include "HyjalTriggers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "Spells/SpellAuras.h"

using namespace ai;
using namespace ai::hyjal;
using namespace ai::raidboss;

// General

bool HyjalEncounterTrigger::IsActive()
{
    return IsHyjalEncounterInProgress(bot) && IsActiveInEncounter();
}

bool HyjalNoEncounterInProgressTrigger::IsActive()
{
    return bot->GetMapId() == HYJAL_MAP_ID && !IsHyjalEncounterInProgress(bot);
}

bool HyjalPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = GetBoss(ai, bossEntry);
    return boss && boss->GetHealthPercent() > BOSS_ENGAGED_HEALTH_PCT;
}

bool HyjalBossShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (mainTankOnly && !IsMainTank(bot))
        return false;

    Unit* boss = GetBoss(ai, bossEntry);
    return boss && boss->GetHealthPercent() > activeAboveHealthPct;
}

// Rage Winterchill

bool RageWinterchillRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return IsRangedPlayer(bot) && GetBoss(ai, NPC_RAGE_WINTERCHILL);
}

bool RageWinterchillMeleeNearDeathAndDecayTrigger::IsActiveInEncounter()
{
    if (!IsMeleePlayer(bot))
        return false;

    Unit* winterchill = GetBoss(ai, NPC_RAGE_WINTERCHILL);
    if (!winterchill || winterchill->GetVictim() == bot)
        return false;

    if (IsMainTank(bot))
        return false;

    return IsNearDeathAndDecay(ai, DEATH_AND_DECAY_CONTROL_RADIUS);
}

bool RageWinterchillRangedInDeathAndDecayTrigger::IsActiveInEncounter()
{
    return IsRangedPlayer(bot) && GetBoss(ai, NPC_RAGE_WINTERCHILL) && IsInDeathAndDecay(ai);
}

// Anetheron

bool AnetheronPullingBossOrInfernalTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_HUNTER && GetBoss(ai, NPC_ANETHERON);
}

bool AnetheronRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!IsRangedPlayer(bot))
        return false;

    Unit* anetheron = GetBoss(ai, NPC_ANETHERON);
    if (!anetheron)
        return false;

    if (GetInfernoTarget(anetheron) == bot)
        return false;

    return !GetInfernalToAttack(ai, anetheron);
}

bool AnetheronNearInfernoTargetTrigger::IsActiveInEncounter()
{
    Unit* anetheron = GetBoss(ai, NPC_ANETHERON);
    if (!anetheron || anetheron->GetVictim() == bot)
        return false;

    Player* infernoTarget = GetInfernoTarget(anetheron);
    if (!infernoTarget || infernoTarget == bot)
        return false;

    return ExactDist2d(bot, infernoTarget) < INFERNAL_ESCAPE_DISTANCE;
}

bool AnetheronTargetedByInfernalTrigger::IsActiveInEncounter()
{
    Unit* anetheron = GetBoss(ai, NPC_ANETHERON);
    if (!anetheron || anetheron->GetVictim() == bot)
        return false;

    if (GetInfernoTarget(anetheron) == bot)
        return true;

    if (IsInfernalTank(bot))
        return false;

    return GetInfernalTargetingBot(ai) != nullptr;
}

bool AnetheronInfernalsPulseImmolationTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    if (!GetBoss(ai, NPC_ANETHERON))
        return false;

    Unit* infernal = GetNearestInfernal(ai);
    if (!infernal || infernal->GetVictim() == bot)
        return false;

    return ExactDist2d(bot, infernal) < INFERNAL_DANGER_RADIUS;
}

bool AnetheronInfernalsShouldBeTankedAwayTrigger::IsActiveInEncounter()
{
    if (!IsInfernalTank(bot))
        return false;

    if (!GetBoss(ai, NPC_ANETHERON))
        return false;

    Unit* infernal = GetInfernalTargetingBot(ai);
    return infernal && bot->CanReachWithMeleeAttack(infernal);
}

bool AnetheronShouldDivideDpsTrigger::IsActiveInEncounter()
{
    return IsDps(bot) && GetBoss(ai, NPC_ANETHERON);
}

// Kaz'rogal

bool KazrogalCanSplitMalevolentCleaveDamageTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot) || IsMainTank(bot))
        return false;

    if (!GetBoss(ai, NPC_KAZROGAL))
        return false;

    if (bot->getClass() != CLASS_PALADIN)
        return true;

    return !AI_VALUE(bool, "kaz'rogal below mana threshold");
}

bool KazrogalRangedShouldAvoidWarStompTrigger::IsActiveInEncounter()
{
    if (!IsRangedPlayer(bot))
        return false;

    if (!GetBoss(ai, NPC_KAZROGAL))
        return false;

    return !AI_VALUE(bool, "kaz'rogal below mana threshold");
}

bool KazrogalLowOnManaTrigger::IsActiveInEncounter()
{
    if (!IsKazrogalManaUser(bot))
        return false;

    // Hunters and Warlocks never run away. They rely on Aspect of the Viper and Life Tap / Shadow Ward.
    if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_WARLOCK)
        return false;

    // A tank running away drags the boss along: the assist tank follows it with the aggro
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* kazrogal = GetBoss(ai, NPC_KAZROGAL);
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return false;

    if (bot->GetPower(POWER_MANA) <= MARK_DANGER_MANA)
    {
        SET_AI_VALUE(bool, "kaz'rogal below mana threshold", true);
        return true;
    }

    return AI_VALUE(bool, "kaz'rogal below mana threshold");
}

bool KazrogalHunterShouldPreserveManaTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!GetBoss(ai, NPC_KAZROGAL))
        return false;

    if (ai->HasAura("aspect of the viper", bot))
        return false;

    // Allowed to switch back at MARK_REJOIN_MANA (see the multiplier)
    return bot->GetPower(POWER_MANA) <= MARK_DANGER_MANA;
}

bool KazrogalMarkOnMageOrPaladinTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_PALADIN)
        return false;

    Unit* kazrogal = GetBoss(ai, NPC_KAZROGAL);
    if (!kazrogal || kazrogal->GetVictim() == bot)
        return false;

    SpellAuraHolder* mark = bot->GetSpellAuraHolder(SPELL_MARK_OF_KAZROGAL);
    if (!mark)
        return false;

    const uint32 mana = bot->GetPower(POWER_MANA);
    if (mana >= MARK_FULL_DRAIN)
        return false;

    // Ice Block / Divine Shield is worth it only when the Mark outlasts the mana:
    //   2400-2999 needs 5s left, 1800-2399 4s, 1200-1799 3s, 600-1199 2s, 0-599 1s
    const int32 requiredMs = int32((mana / MARK_TICK_DRAIN + 1) * IN_MILLISECONDS);
    return mark->GetAuraDuration() >= requiredMs;
}

bool KazrogalWarlockShouldManageManaTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!GetBoss(ai, NPC_KAZROGAL))
        return false;

    if (bot->GetPower(POWER_MANA) <= MARK_LIFE_TAP_MANA)
        return true;

    if (!HasMarkOfKazrogal(bot) || ai->HasAura("shadow ward", bot))
        return false;

    return bot->GetPower(POWER_MANA) <= MARK_TICK_DRAIN;
}

bool KazrogalImmunityNoLongerNeededTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE && (bot->getClass() != CLASS_PALADIN || PlayerbotAI::IsHeal(bot)))
        return false;

    const std::string immunity = GetSelfImmunitySpellName(bot);
    if (immunity.empty() || !ai->HasAura(immunity, bot))
        return false;

    if (HasMarkOfKazrogal(bot))
        return false;

    // Below 50% the bot may be in range of other exploding bots, keep the immunity
    const float keepImmunityHealthPct = 50.0f;
    if (bot->GetHealthPercent() <= keepImmunityHealthPct)
        return false;

    return GetBoss(ai, NPC_KAZROGAL) != nullptr;
}

// Azgalor

bool AzgalorRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!IsRangedPlayer(bot))
        return false;

    Unit* azgalor = GetBoss(ai, NPC_AZGALOR);
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    if (IsDoomed(bot))
        return false;

    return !IsNearRainOfFire(ai, RAIN_OF_FIRE_CONTROL_RADIUS);
}

bool AzgalorMeleeNearRainOfFireTrigger::IsActiveInEncounter()
{
    if (!IsMeleePlayer(bot))
        return false;

    Unit* azgalor = GetBoss(ai, NPC_AZGALOR);
    if (!azgalor || azgalor->GetVictim() == bot)
        return false;

    if (IsDoomed(bot))
        return false;

    // The Doomguard tank holds the Doomguard tanking position
    if (IsDoomguardTank(bot))
        return false;

    return IsNearRainOfFire(ai, RAIN_OF_FIRE_CONTROL_RADIUS);
}

bool AzgalorRangedInRainOfFireTrigger::IsActiveInEncounter()
{
    return IsRangedPlayer(bot) && GetBoss(ai, NPC_AZGALOR) && !IsDoomed(bot) && IsInRainOfFire(ai);
}

bool AzgalorBotIsDoomedTrigger::IsActiveInEncounter()
{
    return IsDoomed(bot);
}

bool AzgalorShouldControlDoomguardsTrigger::IsActiveInEncounter()
{
    if (!IsDoomguardTank(bot))
        return false;

    if (!GetBoss(ai, NPC_AZGALOR))
        return false;

    if (FindUnit(ai, NPC_LESSER_DOOMGUARD))
        return true;

    return AnyGroupMemberHasDoom(bot);
}

bool AzgalorShouldDivideDpsTrigger::IsActiveInEncounter()
{
    return IsDps(bot) && GetBoss(ai, NPC_AZGALOR);
}

// Archimonde

bool ArchimondeShamanShouldProtectAgainstFearTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* archimonde = GetBoss(ai, NPC_ARCHIMONDE);
    if (!archimonde || !archimonde->IsInCombat())
        return false;

    return !HasProtectionOfElune(bot) && !AI_VALUE2(bool, "has totem", "tremor totem");
}

// Air Burst knocks up everyone within 13y of its target. Only the active tank (and the bot itself)
// as target matter: losing the whole melee group lets Archimonde cast Finger of Death on the ranged.
bool ArchimondeCastingAirBurstTrigger::IsActiveInEncounter()
{
    Unit* archimonde = GetBoss(ai, NPC_ARCHIMONDE);
    if (!archimonde || archimonde->GetVictim() == bot)
        return false;

    if (HasProtectionOfElune(bot))
        return false;

    Unit* activeTank = archimonde->GetVictim();
    Player* target = GetAirBurstTarget(archimonde);
    if (!activeTank || !target || (target != activeTank && target != bot))
        return false;

    return ExactDist2d(bot, activeTank) < AIR_BURST_SAFE_DISTANCE;
}

bool ArchimondeRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return IsRangedPlayer(bot) && GetBoss(ai, NPC_ARCHIMONDE) && !HasProtectionOfElune(bot);
}

bool ArchimondeNearDoomfireTrigger::IsActiveInEncounter()
{
    if (!GetBoss(ai, NPC_ARCHIMONDE))
        return false;

    if (HasProtectionOfElune(bot))
        return false;

    return IsNearDoomfire(ai, DOOMFIRE_CONTROL_RADIUS);
}

bool ArchimondeStoodInDoomfireTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_PALADIN)
        return false;

    if (HasProtectionOfElune(bot))
        return false;

    if (!bot->HasAura(SPELL_DOOMFIRE_TRAIL) && !bot->HasAura(SPELL_DOOMFIRE) && !bot->HasAura(SPELL_DOOMFIRE_DOT))
        return false;

    const float dangerHealthPct = 40.0f;
    return bot->GetHealthPercent() < dangerHealthPct;
}

// Air Burst throws the bot high up; the fall kills without slow fall
bool ArchimondeKnockedIntoTheAirTrigger::IsActiveInEncounter()
{
    return GetBoss(ai, NPC_ARCHIMONDE) && ai->IsJumping() && !bot->HasAura(SPELL_ELUNES_EMBRACE) &&
           !HasProtectionOfElune(bot);
}

// Mages and caster druids decurse Grip of the Legion anywhere in the raid
bool ArchimondeGripOfTheLegionTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE && (bot->getClass() != CLASS_DRUID || !IsRangedPlayer(bot)))
        return false;

    if (!GetBoss(ai, NPC_ARCHIMONDE) || HasProtectionOfElune(bot))
        return false;

    return GetGripOfTheLegionTarget(bot, 60.0f) != nullptr;
}

// Ranged stay within the Tremor Totem of their party (melee have to stay at the boss)
bool ArchimondeOutOfTremorTotemRangeTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot) || !IsRangedPlayer(bot) || HasProtectionOfElune(bot))
        return false;

    if (!GetBoss(ai, NPC_ARCHIMONDE))
        return false;

    Unit* totem = GetPartyTremorTotem(bot);
    return totem && ExactDist2d(bot, totem) > TREMOR_TOTEM_RANGE;
}

// Doomfire next to Archimonde: the tank walks him off it so the melee can stand safely
bool ArchimondeBossNearDoomfireTrigger::IsActiveInEncounter()
{
    if (!IsMainTank(bot) || HasProtectionOfElune(bot))
        return false;

    Unit* archimonde = GetBoss(ai, NPC_ARCHIMONDE);
    if (!archimonde || archimonde->GetVictim() != bot)
        return false;

    return IsPositionNearDoomfire(ai, archimonde->GetPositionX(), archimonde->GetPositionY(),
                                  DOOMFIRE_DANGER_RADIUS + archimonde->GetCombatReach());
}

bool ArchimondeMeleeShouldSpreadTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot) || !IsMeleePlayer(bot) || HasProtectionOfElune(bot))
        return false;

    return GetBoss(ai, NPC_ARCHIMONDE) != nullptr;
}

// Every death casts Soul Charge on the whole raid: survival comes first
bool ArchimondeBotInDangerOfDyingTrigger::IsActiveInEncounter()
{
    if (!GetBoss(ai, NPC_ARCHIMONDE) || HasProtectionOfElune(bot))
        return false;

    const float dangerHealthPct = 35.0f;
    return bot->IsAlive() && bot->GetHealthPercent() < dangerHealthPct;
}

// Hand of Death wipes the raid 10 minutes after the pull: from 8 minutes on, the shamans
// use Bloodlust/Heroism again for a last burn
bool ArchimondeHandOfDeathSoonTrigger::IsActiveInEncounter()
{
    const time_t now = time(nullptr);
    Unit* archimonde = GetBoss(ai, NPC_ARCHIMONDE);
    if (!archimonde || !archimonde->IsInCombat())
    {
        pullTime = 0;
        return false;
    }

    // Not checked outside of encounters: a long gap means a new pull
    if (!pullTime || now - lastSeen > 30)
        pullTime = now;
    lastSeen = now;

    constexpr time_t burnAfterSeconds = HAND_OF_DEATH_SECONDS - 120;
    return bot->getClass() == CLASS_SHAMAN && now - pullTime >= burnAfterSeconds && !HasProtectionOfElune(bot);
}
