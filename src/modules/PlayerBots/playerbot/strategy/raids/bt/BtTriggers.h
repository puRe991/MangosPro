// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/BT/BTTriggers) and adapted for this core.
#pragma once
#include "playerbot/strategy/Trigger.h"

#define BT_TRIGGER(ClassName, triggerName)                                          \
    class ClassName : public Trigger                                                \
    {                                                                               \
    public:                                                                         \
        ClassName(PlayerbotAI* ai) : Trigger(ai, triggerName) {}                    \
        bool IsActive() override;                                                   \
    };

namespace ai
{
    // General
    BT_TRIGGER(BlackTempleBotIsNotInCombatTrigger, "black temple bot is not in combat")

    // High Warlord Naj'entus
    BT_TRIGGER(HighWarlordNajentusPullingBossTrigger, "high warlord naj'entus pulling boss")
    BT_TRIGGER(HighWarlordNajentusBossEngagedByTanksTrigger, "high warlord naj'entus boss engaged by tanks")
    BT_TRIGGER(HighWarlordNajentusCastsNeedleSpinesTrigger, "high warlord naj'entus casts needle spines")
    BT_TRIGGER(HighWarlordNajentusPlayerIsImpaledTrigger, "high warlord naj'entus player is impaled")
    BT_TRIGGER(HighWarlordNajentusBossHasTidalShieldTrigger, "high warlord naj'entus boss has tidal shield")

    // Supremus
    BT_TRIGGER(SupremusPullingBossOrChangingPhaseTrigger, "supremus pulling boss or changing phase")
    BT_TRIGGER(SupremusBossEngagedByRangedTrigger, "supremus boss engaged by ranged")
    BT_TRIGGER(SupremusBossIsFixatedOnBotTrigger, "supremus boss is fixated on bot")
    BT_TRIGGER(SupremusVolcanoIsNearbyTrigger, "supremus volcano is nearby")
    BT_TRIGGER(SupremusNeedToManagePhaseTimerTrigger, "supremus need to manage phase timer")

    // Shade of Akama
    BT_TRIGGER(ShadeOfAkamaKillingChannelersStartsPhase2Trigger, "shade of akama killing channelers starts phase 2")

    // Teron Gorefiend
    BT_TRIGGER(TeronGorefiendPullingBossTrigger, "teron gorefiend pulling boss")
    BT_TRIGGER(TeronGorefiendBossEngagedByTanksTrigger, "teron gorefiend boss engaged by tanks")
    BT_TRIGGER(TeronGorefiendBossEngagedByRangedTrigger, "teron gorefiend boss engaged by ranged")
    BT_TRIGGER(TeronGorefiendBossIsCastingShadowOfDeathTrigger, "teron gorefiend boss is casting shadow of death")
    BT_TRIGGER(TeronGorefiendBotHasShadowOfDeathTrigger, "teron gorefiend bot has shadow of death")
    BT_TRIGGER(TeronGorefiendBotTransformedIntoVengefulSpiritTrigger, "teron gorefiend bot transformed into vengeful spirit")

    // Gurtogg Bloodboil
    BT_TRIGGER(GurtoggBloodboilPullingBossTrigger, "gurtogg bloodboil pulling boss")
    BT_TRIGGER(GurtoggBloodboilBossEngagedByTanksTrigger, "gurtogg bloodboil boss engaged by tanks")
    BT_TRIGGER(GurtoggBloodboilBossCastsBloodboilTrigger, "gurtogg bloodboil boss casts bloodboil")
    BT_TRIGGER(GurtoggBloodboilBotHasFelRageTrigger, "gurtogg bloodboil bot has fel rage")
    BT_TRIGGER(GurtoggBloodboilNeedToManagePhaseTimerTrigger, "gurtogg bloodboil need to manage phase timer")

    // Reliquary of Souls
    BT_TRIGGER(ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger, "reliquary of souls aggro resets upon phase change")
    BT_TRIGGER(ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger, "reliquary of souls essence of suffering fixates on closest target")
    BT_TRIGGER(ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger, "reliquary of souls essence of suffering disables healing")
    BT_TRIGGER(ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger, "reliquary of souls essence of desire has rune shield")
    BT_TRIGGER(ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger, "reliquary of souls essence of desire casting deaden")

    // Mother Shahraz
    BT_TRIGGER(MotherShahrazPullingBossTrigger, "mother shahraz pulling boss")
    BT_TRIGGER(MotherShahrazBossEngagedByTanksTrigger, "mother shahraz boss engaged by tanks")
    BT_TRIGGER(MotherShahrazTanksArePositioningBossTrigger, "mother shahraz tanks are positioning boss")
    BT_TRIGGER(MotherShahrazSinisterBeamKnocksBackPlayersTrigger, "mother shahraz sinister beam knocks back players")
    BT_TRIGGER(MotherShahrazBotsAreLinkedByFatalAttractionTrigger, "mother shahraz bots are linked by fatal attraction")

    // Illidari Council
    BT_TRIGGER(IllidariCouncilPullingBossesTrigger, "illidari council pulling bosses")
    BT_TRIGGER(IllidariCouncilGathiosEngagedByMainTankTrigger, "illidari council gathios engaged by main tank")
    BT_TRIGGER(IllidariCouncilGathiosCastingJudgementOfCommandTrigger, "illidari council gathios casting judgement of command")
    BT_TRIGGER(IllidariCouncilMalandeEngagedByFirstAssistTankTrigger, "illidari council malande engaged by first assist tank")
    BT_TRIGGER(IllidariCouncilDarkshadowEngagedBySecondAssistTankTrigger, "illidari council darkshadow engaged by second assist tank")
    BT_TRIGGER(IllidariCouncilZerevorEngagedByMageTankTrigger, "illidari council zerevor engaged by mage tank")
    BT_TRIGGER(IllidariCouncilMageTankNeedsDedicatedHealerTrigger, "illidari council mage tank needs dedicated healer")
    BT_TRIGGER(IllidariCouncilZerevorCastsDangerousAoesTrigger, "illidari council zerevor casts dangerous aoes")
    BT_TRIGGER(IllidariCouncilPetsScrewUpThePullTrigger, "illidari council pets screw up the pull")
    BT_TRIGGER(IllidariCouncilDeterminingDpsAssignmentsTrigger, "illidari council determining dps assignments")
    BT_TRIGGER(IllidariCouncilNeedToManageDpsTimerTrigger, "illidari council need to manage dps timer")

    // Illidan Stormrage <The Betrayer>
    BT_TRIGGER(IllidanStormrageTankNeedsAggroTrigger, "illidan stormrage tank needs aggro")
    BT_TRIGGER(IllidanStormrageBossCastsFlameCrashInFrontOfMainTankTrigger, "illidan stormrage boss casts flame crash in front of main tank")
    BT_TRIGGER(IllidanStormrageBotHasParasiticShadowfiendTrigger, "illidan stormrage bot has parasitic shadowfiend")
    BT_TRIGGER(IllidanStormrageParasiticShadowfiendsRunWildTrigger, "illidan stormrage parasitic shadowfiends run wild")
    BT_TRIGGER(IllidanStormrageBossSummonedFlamesOfAzzinothTrigger, "illidan stormrage boss summoned flames of azzinoth")
    BT_TRIGGER(IllidanStormragePetsDieToFireTrigger, "illidan stormrage pets die to fire")
    BT_TRIGGER(IllidanStormrageGrateIsSafeFromFlamesTrigger, "illidan stormrage grate is safe from flames")
    BT_TRIGGER(IllidanStormrageBotStruckByDarkBarrageTrigger, "illidan stormrage bot struck by dark barrage")
    BT_TRIGGER(IllidanStormrageBossIsPreparingToLandTrigger, "illidan stormrage boss is preparing to land")
    BT_TRIGGER(IllidanStormrageBossDealsSplashDamageTrigger, "illidan stormrage boss deals splash damage")
    BT_TRIGGER(IllidanStormrageThisExpansionHatesMeleeTrigger, "illidan stormrage this expansion hates melee")
    BT_TRIGGER(IllidanStormrageBossTransformsIntoDemonTrigger, "illidan stormrage boss transforms into demon")
    BT_TRIGGER(IllidanStormrageBossSpawnsAddsTrigger, "illidan stormrage boss spawns adds")
    BT_TRIGGER(IllidanStormrageNeedToManageDpsTimerAndRtiTrigger, "illidan stormrage need to manage dps timer and rti")
}

#undef BT_TRIGGER
