// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/BT/BTTriggers) and adapted for this core.
#include "playerbot/playerbot.h"
#include "BtTriggers.h"
#include "BtHelpers.h"
#include "playerbot/AiFactory.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "Entities/Totem.h"
#include <limits>

using namespace ai;
using namespace ai::bt;

namespace
{
    bool IsHunter(Player* bot) { return bot->getClass() == CLASS_HUNTER; }

    bool IsCastingOnBot(Unit* caster, uint32 spellId, Player* bot)
    {
        if (!caster)
            return false;

        Spell* spell = caster->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        return spell && spell->m_spellInfo && spell->m_spellInfo->Id == spellId && spell->m_targets.getUnitTarget() == bot;
    }

    // Closest alive bot (no tank, optionally excluding one unit's victim) to a point
    Player* GetClosestBot(Player* bot, WorldObject* to, Player* exclude, Unit* excludeVictimOf)
    {
        Player* closest = nullptr;
        float closestDist = std::numeric_limits<float>::max();
        for (Player* member : raidboss::GetGroupMembers(bot, 200.0f, true))
        {
            if (member == exclude || !member->GetPlayerbotAI() || PlayerbotAI::IsTank(member))
                continue;

            if (excludeVictimOf && excludeVictimOf->GetVictim() == member)
                continue;

            const float dist = member->GetDistance(to);
            if (dist < closestDist)
            {
                closestDist = dist;
                closest = member;
            }
        }

        return closest;
    }
}

// General

bool BlackTempleBotIsNotInCombatTrigger::IsActive()
{
    return bot->IsInWorld() && bot->GetMapId() == BLACK_TEMPLE_MAP_ID && !bot->IsInCombat();
}

// High Warlord Naj'entus

bool HighWarlordNajentusPullingBossTrigger::IsActive()
{
    if (!IsHunter(bot))
        return false;

    Unit* najentus = FindBoss(ai, NPC_NAJENTUS);
    return najentus && najentus->GetHealthPercent() > 95.0f;
}

bool HighWarlordNajentusBossEngagedByTanksTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && FindBoss(ai, NPC_NAJENTUS);
}

bool HighWarlordNajentusCastsNeedleSpinesTrigger::IsActive()
{
    return raidboss::IsRangedPlayer(bot) && FindBoss(ai, NPC_NAJENTUS);
}

// The closest bot frees the impaled player
bool HighWarlordNajentusPlayerIsImpaledTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot) || !FindBoss(ai, NPC_NAJENTUS))
        return false;

    for (Player* member : raidboss::GetGroupMembers(bot, 100.0f, false))
    {
        if (member->HasAura(SPELL_IMPALING_SPINE))
            return GetClosestBot(bot, member, member, nullptr) == bot;
    }

    return false;
}

bool HighWarlordNajentusBossHasTidalShieldTrigger::IsActive()
{
    Unit* najentus = FindBoss(ai, NPC_NAJENTUS);
    return najentus && najentus->HasAura(SPELL_TIDAL_SHIELD) && bot->HasItemCount(ITEM_NAJENTUS_SPINE, 1);
}

// Supremus

// Threat is reset on the pull and on every phase change (every 60 seconds)
bool SupremusPullingBossOrChangingPhaseTrigger::IsActive()
{
    if (!IsHunter(bot))
        return false;

    Unit* supremus = FindBoss(ai, NPC_SUPREMUS);
    if (!supremus)
        return false;

    const int64 elapsed = GetTimerElapsed(BtState::SupremusPhaseTimer, supremus->GetInstanceId());
    return elapsed >= 0 && (elapsed < 10 || (elapsed >= 60 && elapsed % 60 < 10));
}

bool SupremusBossEngagedByRangedTrigger::IsActive()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    Unit* supremus = FindBoss(ai, NPC_SUPREMUS);
    return supremus && !supremus->HasAura(SPELL_SNARE_SELF);
}

bool SupremusBossIsFixatedOnBotTrigger::IsActive()
{
    Unit* supremus = FindBoss(ai, NPC_SUPREMUS);
    return supremus && supremus->GetVictim() == bot && supremus->HasAura(SPELL_SNARE_SELF);
}

bool SupremusVolcanoIsNearbyTrigger::IsActive()
{
    return FindBoss(ai, NPC_SUPREMUS) && HasSupremusVolcanoNearby(bot);
}

bool SupremusNeedToManagePhaseTimerTrigger::IsActive()
{
    return raidboss::IsMechanicTrackerBot(bot, BLACK_TEMPLE_MAP_ID) && FindBoss(ai, NPC_SUPREMUS);
}

// Shade of Akama

bool ShadeOfAkamaKillingChannelersStartsPhase2Trigger::IsActive()
{
    if (!raidboss::IsDps(bot) || !raidboss::IsMeleePlayer(bot))
        return false;

    Unit* channeler = FindNearest(bot, NPC_ASHTONGUE_CHANNELER, 30.0f);
    return channeler && !channeler->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
}

// Teron Gorefiend

bool TeronGorefiendPullingBossTrigger::IsActive()
{
    if (!IsHunter(bot))
        return false;

    Unit* gorefiend = FindBoss(ai, NPC_TERON_GOREFIEND);
    return gorefiend && gorefiend->GetHealthPercent() > 95.0f;
}

bool TeronGorefiendBossEngagedByTanksTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && FindBoss(ai, NPC_TERON_GOREFIEND);
}

bool TeronGorefiendBossEngagedByRangedTrigger::IsActive()
{
    return raidboss::IsRangedPlayer(bot) && FindBoss(ai, NPC_TERON_GOREFIEND);
}

bool TeronGorefiendBossIsCastingShadowOfDeathTrigger::IsActive()
{
    const uint8 cls = bot->getClass();
    if (cls != CLASS_HUNTER && cls != CLASS_MAGE && cls != CLASS_PALADIN && cls != CLASS_ROGUE)
        return false;

    Unit* gorefiend = FindBoss(ai, NPC_TERON_GOREFIEND);
    if (!gorefiend)
        return false;

    // The immunity used against the cast is cancelled right away
    if (ai->HasAura("feign death", bot) || ai->HasAura("ice block", bot) ||
        (!PlayerbotAI::IsHeal(bot) && ai->HasAura("divine shield", bot)))
    {
        return true;
    }

    return IsCastingOnBot(gorefiend, SPELL_SHADOW_OF_DEATH, bot);
}

bool TeronGorefiendBotHasShadowOfDeathTrigger::IsActive()
{
    SpellAuraHolder* holder = bot->GetSpellAuraHolder(SPELL_SHADOW_OF_DEATH);
    return holder && holder->GetAuraDuration() < 12000;
}

bool TeronGorefiendBotTransformedIntoVengefulSpiritTrigger::IsActive()
{
    return bot->HasAura(SPELL_SPIRITUAL_VENGEANCE);
}

// Gurtogg Bloodboil

bool GurtoggBloodboilPullingBossTrigger::IsActive()
{
    if (!IsHunter(bot))
        return false;

    Unit* gurtogg = FindBoss(ai, NPC_GURTOGG_BLOODBOIL);
    if (!gurtogg)
        return false;

    const int64 elapsed = GetTimerElapsed(BtState::GurtoggPhaseTimer, gurtogg->GetInstanceId());
    return elapsed >= 0 && elapsed < 10;
}

bool GurtoggBloodboilBossEngagedByTanksTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* gurtogg = FindBoss(ai, NPC_GURTOGG_BLOODBOIL);
    return gurtogg && !gurtogg->HasAura(SPELL_BOSS_FEL_RAGE);
}

bool GurtoggBloodboilBossCastsBloodboilTrigger::IsActive()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    Unit* gurtogg = FindBoss(ai, NPC_GURTOGG_BLOODBOIL);
    return gurtogg && !gurtogg->HasAura(SPELL_BOSS_FEL_RAGE);
}

bool GurtoggBloodboilBotHasFelRageTrigger::IsActive()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    Unit* gurtogg = FindBoss(ai, NPC_GURTOGG_BLOODBOIL);
    if (!gurtogg || !gurtogg->HasAura(SPELL_BOSS_FEL_RAGE))
        return false;

    for (Player* member : raidboss::GetGroupMembers(bot, 100.0f, true))
    {
        if (member->HasAura(SPELL_PLAYER_FEL_RAGE))
            return true;
    }

    return false;
}

bool GurtoggBloodboilNeedToManagePhaseTimerTrigger::IsActive()
{
    return raidboss::IsMechanicTrackerBot(bot, BLACK_TEMPLE_MAP_ID) && FindBoss(ai, NPC_GURTOGG_BLOODBOIL);
}

// Reliquary of Souls

bool ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger::IsActive()
{
    return IsHunter(bot) && (FindBoss(ai, NPC_ESSENCE_OF_DESIRE) || FindBoss(ai, NPC_ESSENCE_OF_ANGER));
}

bool ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger::IsActive()
{
    return FindBoss(ai, NPC_ESSENCE_OF_SUFFERING);
}

bool ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger::IsActive()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    // Discipline priests keep shielding
    if (bot->getClass() == CLASS_PRIEST && AiFactory::GetPlayerSpecTab(bot) == 0)
        return false;

    return FindBoss(ai, NPC_ESSENCE_OF_SUFFERING);
}

bool ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* desire = FindBoss(ai, NPC_ESSENCE_OF_DESIRE);
    return desire && desire->HasAura(SPELL_RUNE_SHIELD);
}

bool ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot) || bot->getClass() != CLASS_WARRIOR)
        return false;

    return IsCastingOnBot(FindBoss(ai, NPC_ESSENCE_OF_DESIRE), SPELL_DEADEN, bot);
}

// Mother Shahraz

bool MotherShahrazPullingBossTrigger::IsActive()
{
    if (!IsHunter(bot))
        return false;

    Unit* shahraz = FindBoss(ai, NPC_MOTHER_SHAHRAZ);
    return shahraz && shahraz->GetHealthPercent() > 95.0f;
}

bool MotherShahrazBossEngagedByTanksTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && FindBoss(ai, NPC_MOTHER_SHAHRAZ) && !bot->HasAura(SPELL_FATAL_ATTRACTION);
}

bool MotherShahrazTanksArePositioningBossTrigger::IsActive()
{
    if (!raidboss::IsMeleePlayer(bot) || !raidboss::IsDps(bot))
        return false;

    Unit* shahraz = FindBoss(ai, NPC_MOTHER_SHAHRAZ);
    if (!shahraz || shahraz->GetHealthPercent() < 90.0f)
        return false;

    return GetShahrazTankPositionState(bot) != TankPositionState::Positioned;
}

bool MotherShahrazSinisterBeamKnocksBackPlayersTrigger::IsActive()
{
    return raidboss::IsRangedPlayer(bot) && FindBoss(ai, NPC_MOTHER_SHAHRAZ) && !bot->HasAura(SPELL_FATAL_ATTRACTION);
}

bool MotherShahrazBotsAreLinkedByFatalAttractionTrigger::IsActive()
{
    return bot->HasAura(SPELL_FATAL_ATTRACTION);
}

// Illidari Council

bool IllidariCouncilPullingBossesTrigger::IsActive()
{
    if (!IsHunter(bot))
        return false;

    Unit* gathios = FindBoss(ai, NPC_GATHIOS);
    return gathios && gathios->GetHealthPercent() > 95.0f;
}

bool IllidariCouncilGathiosEngagedByMainTankTrigger::IsActive()
{
    return raidboss::IsMainTank(bot) && FindBoss(ai, NPC_GATHIOS);
}

bool IllidariCouncilGathiosCastingJudgementOfCommandTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARRIOR || !raidboss::IsMainTank(bot))
        return false;

    Unit* gathios = FindBoss(ai, NPC_GATHIOS);
    return gathios && gathios->HasAura(SPELL_SEAL_OF_COMMAND) && IsCastingOnBot(gathios, SPELL_JUDGEMENT, bot);
}

bool IllidariCouncilMalandeEngagedByFirstAssistTankTrigger::IsActive()
{
    return raidboss::IsAssistTank(bot, 0) && FindBoss(ai, NPC_MALANDE);
}

bool IllidariCouncilDarkshadowEngagedBySecondAssistTankTrigger::IsActive()
{
    if (!raidboss::IsAssistTank(bot, 1))
        return false;

    Unit* darkshadow = FindBoss(ai, NPC_VERAS);
    return darkshadow && !darkshadow->HasAura(SPELL_VANISH);
}

bool IllidariCouncilZerevorEngagedByMageTankTrigger::IsActive()
{
    return bot->getClass() == CLASS_MAGE && GetZerevorMageTank(bot) == bot && FindBoss(ai, NPC_ZEREVOR);
}

bool IllidariCouncilMageTankNeedsDedicatedHealerTrigger::IsActive()
{
    return PlayerbotAI::IsHeal(bot) && GetZerevorHealer(bot) == bot && FindBoss(ai, NPC_ZEREVOR);
}

bool IllidariCouncilZerevorCastsDangerousAoesTrigger::IsActive()
{
    return raidboss::IsRangedPlayer(bot) && FindBoss(ai, NPC_ZEREVOR) && !HasDangerousCouncilAura(bot);
}

bool IllidariCouncilPetsScrewUpThePullTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK)
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive() && FindBoss(ai, NPC_GATHIOS);
}

bool IllidariCouncilDeterminingDpsAssignmentsTrigger::IsActive()
{
    if (PlayerbotAI::IsHeal(bot) || !FindBoss(ai, NPC_GATHIOS))
        return false;

    if (raidboss::IsMainTank(bot) || raidboss::IsAssistTank(bot, 0) || GetZerevorMageTank(bot) == bot)
        return false;

    Unit* darkshadow = FindBoss(ai, NPC_VERAS);
    if (raidboss::IsAssistTank(bot, 1) && darkshadow && !darkshadow->HasAura(SPELL_VANISH))
        return false;

    return true;
}

bool IllidariCouncilNeedToManageDpsTimerTrigger::IsActive()
{
    return raidboss::IsMechanicTrackerBot(bot, BLACK_TEMPLE_MAP_ID) && FindBoss(ai, NPC_GATHIOS);
}

// Illidan Stormrage <The Betrayer>

bool IllidanStormrageTankNeedsAggroTrigger::IsActive()
{
    if (!IsHunter(bot))
        return false;

    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    return illidan && !IsIllidanDefeated(illidan);
}

bool IllidanStormrageBossCastsFlameCrashInFrontOfMainTankTrigger::IsActive()
{
    if (!raidboss::IsMainTank(bot))
        return false;

    const int phase = GetIllidanPhase(FindBoss(ai, NPC_ILLIDAN));
    return phase == 1 || phase == 3 || phase == 5;
}

bool IllidanStormrageBotHasParasiticShadowfiendTrigger::IsActive()
{
    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan || IsIllidanDefeated(illidan) || illidan->GetVictim() == bot || raidboss::IsMainTank(bot))
        return false;

    const int phase = GetIllidanPhase(illidan);
    if (phase == 2 || phase == 4)
        return false;

    // The main tank drags Illidan into the trap, nobody runs around
    if (phase == 5 && FindNearestTrap(ai))
        return false;

    Player* infected = GetBotWithParasiticShadowfiend(bot);
    if (!infected)
        return false;

    return infected == bot || (phase != 1 && IsHunter(bot));
}

bool IllidanStormrageParasiticShadowfiendsRunWildTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan || IsIllidanDefeated(illidan) || GetIllidanPhase(illidan) == 2)
        return false;

    Totem* totem = bot->GetTotem(TOTEM_SLOT_EARTH);
    return !totem || raidboss::ExactDist2d(totem, bot) > 20.0f || totem->GetUInt32Value(UNIT_CREATED_BY_SPELL) != SPELL_EARTHBIND_TOTEM;
}

bool IllidanStormrageBossSummonedFlamesOfAzzinothTrigger::IsActive()
{
    if (!raidboss::IsAssistTank(bot, 0) && !raidboss::IsAssistTank(bot, 1))
        return false;

    return GetIllidanPhase(FindBoss(ai, NPC_ILLIDAN)) == 2;
}

bool IllidanStormragePetsDieToFireTrigger::IsActive()
{
    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan || IsIllidanDefeated(illidan))
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive();
}

bool IllidanStormrageGrateIsSafeFromFlamesTrigger::IsActive()
{
    if (raidboss::IsAssistTank(bot, 0) || raidboss::IsAssistTank(bot, 1))
        return false;

    return GetIllidanPhase(FindBoss(ai, NPC_ILLIDAN)) == 2;
}

bool IllidanStormrageBotStruckByDarkBarrageTrigger::IsActive()
{
    const uint8 cls = bot->getClass();
    if (cls != CLASS_MAGE && cls != CLASS_PALADIN && cls != CLASS_ROGUE)
        return false;

    if (!FindBoss(ai, NPC_ILLIDAN))
        return false;

    if (ai->HasAura("ice block", bot) || (!PlayerbotAI::IsHeal(bot) && ai->HasAura("divine shield", bot)))
        return true;

    return bot->HasAura(SPELL_DARK_BARRAGE);
}

bool IllidanStormrageBossIsPreparingToLandTrigger::IsActive()
{
    return !raidboss::IsMainTank(bot) && GetIllidanPhase(FindBoss(ai, NPC_ILLIDAN)) == 0;
}

bool IllidanStormrageBossDealsSplashDamageTrigger::IsActive()
{
    if (!raidboss::IsRangedPlayer(bot))
        return false;

    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan || illidan->HasAura(SPELL_CAGED))
        return false;

    const int phase = GetIllidanPhase(illidan);
    if (phase == 4 && GetIllidanWarlockTank(bot) == bot)
        return false;

    return phase == 3 || phase == 4 || phase == 5;
}

bool IllidanStormrageThisExpansionHatesMeleeTrigger::IsActive()
{
    return raidboss::IsMeleePlayer(bot) && GetIllidanPhase(FindBoss(ai, NPC_ILLIDAN)) == 4;
}

bool IllidanStormrageBossTransformsIntoDemonTrigger::IsActive()
{
    return bot->getClass() == CLASS_WARLOCK && GetIllidanPhase(FindBoss(ai, NPC_ILLIDAN)) == 4 && GetIllidanWarlockTank(bot) == bot;
}

bool IllidanStormrageBossSpawnsAddsTrigger::IsActive()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    if (!illidan || IsIllidanDefeated(illidan))
        return false;

    return !PlayerbotAI::IsTank(bot) || GetIllidanPhase(illidan) == 4;
}

bool IllidanStormrageNeedToManageDpsTimerAndRtiTrigger::IsActive()
{
    if (!raidboss::IsMechanicTrackerBot(bot, BLACK_TEMPLE_MAP_ID))
        return false;

    Unit* illidan = FindBoss(ai, NPC_ILLIDAN);
    return illidan && !IsIllidanDefeated(illidan);
}
