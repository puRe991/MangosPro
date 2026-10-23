#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "BtStrategy.h"
#include "BtActions.h"
#include "BtTriggers.h"

// Black Temple (map 564) - strategy "boss black temple"
namespace ai
{
    class BtStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        BtStrategyContext()
        {
            creators["boss black temple"] = [](PlayerbotAI* ai) { return new BtStrategy(ai); };
        }
    };

    class BtActionContext : public NamedObjectContext<Action>
    {
    public:
        BtActionContext()
        {
            creators["black temple erase timers and trackers"] = [](PlayerbotAI* ai) { return new BlackTempleEraseTimersAndTrackersAction(ai); };
            creators["high warlord naj'entus misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new HighWarlordNajentusMisdirectBossToMainTankAction(ai); };
            creators["high warlord naj'entus tanks position boss"] = [](PlayerbotAI* ai) { return new HighWarlordNajentusTanksPositionBossAction(ai); };
            creators["high warlord naj'entus disperse ranged"] = [](PlayerbotAI* ai) { return new HighWarlordNajentusDisperseRangedAction(ai); };
            creators["high warlord naj'entus remove impaling spine"] = [](PlayerbotAI* ai) { return new HighWarlordNajentusRemoveImpalingSpineAction(ai); };
            creators["high warlord naj'entus throw impaling spine"] = [](PlayerbotAI* ai) { return new HighWarlordNajentusThrowImpalingSpineAction(ai); };
            creators["high warlord naj'entus spread melee"] = [](PlayerbotAI* ai) { return new HighWarlordNajentusSpreadMeleeAction(ai); };
            creators["supremus misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new SupremusMisdirectBossToMainTankAction(ai); };
            creators["supremus disperse ranged"] = [](PlayerbotAI* ai) { return new SupremusDisperseRangedAction(ai); };
            creators["supremus kite boss"] = [](PlayerbotAI* ai) { return new SupremusKiteBossAction(ai); };
            creators["supremus move away from volcanos"] = [](PlayerbotAI* ai) { return new SupremusMoveAwayFromVolcanosAction(ai); };
            creators["supremus manage phase timer"] = [](PlayerbotAI* ai) { return new SupremusManagePhaseTimerAction(ai); };
            creators["supremus lead molten flame away"] = [](PlayerbotAI* ai) { return new SupremusLeadMoltenFlameAwayAction(ai); };
            creators["shade of akama melee dps prioritize channelers"] = [](PlayerbotAI* ai) { return new ShadeOfAkamaMeleeDpsPrioritizeChannelersAction(ai); };
            creators["teron gorefiend misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new TeronGorefiendMisdirectBossToMainTankAction(ai); };
            creators["teron gorefiend tanks position boss"] = [](PlayerbotAI* ai) { return new TeronGorefiendTanksPositionBossAction(ai); };
            creators["teron gorefiend position ranged on balcony"] = [](PlayerbotAI* ai) { return new TeronGorefiendPositionRangedOnBalconyAction(ai); };
            creators["teron gorefiend avoid shadow of death"] = [](PlayerbotAI* ai) { return new TeronGorefiendAvoidShadowOfDeathAction(ai); };
            creators["teron gorefiend move to corner to die"] = [](PlayerbotAI* ai) { return new TeronGorefiendMoveToCornerToDieAction(ai); };
            creators["teron gorefiend control and destroy shadowy constructs"] = [](PlayerbotAI* ai) { return new TeronGorefiendControlAndDestroyShadowyConstructsAction(ai); };
            creators["gurtogg bloodboil misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new GurtoggBloodboilMisdirectBossToMainTankAction(ai); };
            creators["gurtogg bloodboil tanks position boss"] = [](PlayerbotAI* ai) { return new GurtoggBloodboilTanksPositionBossAction(ai); };
            creators["gurtogg bloodboil rotate ranged groups"] = [](PlayerbotAI* ai) { return new GurtoggBloodboilRotateRangedGroupsAction(ai); };
            creators["gurtogg bloodboil ranged move away from enraged player"] = [](PlayerbotAI* ai) { return new GurtoggBloodboilRangedMoveAwayFromEnragedPlayerAction(ai); };
            creators["gurtogg bloodboil manage phase timer"] = [](PlayerbotAI* ai) { return new GurtoggBloodboilManagePhaseTimerAction(ai); };
            creators["reliquary of souls misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new ReliquaryOfSoulsMisdirectBossToMainTankAction(ai); };
            creators["reliquary of souls adjust distance from suffering"] = [](PlayerbotAI* ai) { return new ReliquaryOfSoulsAdjustDistanceFromSufferingAction(ai); };
            creators["reliquary of souls healers dps suffering"] = [](PlayerbotAI* ai) { return new ReliquaryOfSoulsHealersDpsSufferingAction(ai); };
            creators["reliquary of souls spellsteal rune shield"] = [](PlayerbotAI* ai) { return new ReliquaryOfSoulsSpellstealRuneShieldAction(ai); };
            creators["reliquary of souls spell reflect deaden"] = [](PlayerbotAI* ai) { return new ReliquaryOfSoulsSpellReflectDeadenAction(ai); };
            creators["mother shahraz misdirect boss to main tank"] = [](PlayerbotAI* ai) { return new MotherShahrazMisdirectBossToMainTankAction(ai); };
            creators["mother shahraz tanks position boss under pillar"] = [](PlayerbotAI* ai) { return new MotherShahrazTanksPositionBossUnderPillarAction(ai); };
            creators["mother shahraz melee dps wait at safe position"] = [](PlayerbotAI* ai) { return new MotherShahrazMeleeDpsWaitAtSafePositionAction(ai); };
            creators["mother shahraz position ranged under pillar"] = [](PlayerbotAI* ai) { return new MotherShahrazPositionRangedUnderPillarAction(ai); };
            creators["mother shahraz run away to break fatal attraction"] = [](PlayerbotAI* ai) { return new MotherShahrazRunAwayToBreakFatalAttractionAction(ai); };
            creators["illidari council misdirect bosses to tanks"] = [](PlayerbotAI* ai) { return new IllidariCouncilMisdirectBossesToTanksAction(ai); };
            creators["illidari council main tank position gathios"] = [](PlayerbotAI* ai) { return new IllidariCouncilMainTankPositionGathiosAction(ai); };
            creators["illidari council main tank reflect judgement of command"] = [](PlayerbotAI* ai) { return new IllidariCouncilMainTankReflectJudgementOfCommandAction(ai); };
            creators["illidari council first assist tank focus malande"] = [](PlayerbotAI* ai) { return new IllidariCouncilFirstAssistTankFocusMalandeAction(ai); };
            creators["illidari council second assist tank position darkshadow"] = [](PlayerbotAI* ai) { return new IllidariCouncilSecondAssistTankPositionDarkshadowAction(ai); };
            creators["illidari council mage tank position zerevor"] = [](PlayerbotAI* ai) { return new IllidariCouncilMageTankPositionZerevorAction(ai); };
            creators["illidari council position mage tank healer"] = [](PlayerbotAI* ai) { return new IllidariCouncilPositionMageTankHealerAction(ai); };
            creators["illidari council assign dps targets"] = [](PlayerbotAI* ai) { return new IllidariCouncilAssignDpsTargetsAction(ai); };
            creators["illidari council disperse ranged"] = [](PlayerbotAI* ai) { return new IllidariCouncilDisperseRangedAction(ai); };
            creators["illidari council command pets to attack gathios"] = [](PlayerbotAI* ai) { return new IllidariCouncilCommandPetsToAttackGathiosAction(ai); };
            creators["illidari council manage dps timer"] = [](PlayerbotAI* ai) { return new IllidariCouncilManageDpsTimerAction(ai); };
            creators["illidan stormrage misdirect to tank"] = [](PlayerbotAI* ai) { return new IllidanStormrageMisdirectToTankAction(ai); };
            creators["illidan stormrage main tank reposition boss"] = [](PlayerbotAI* ai) { return new IllidanStormrageMainTankRepositionBossAction(ai); };
            creators["illidan stormrage isolate bot with parasite"] = [](PlayerbotAI* ai) { return new IllidanStormrageIsolateBotWithParasiteAction(ai); };
            creators["illidan stormrage set earthbind totem"] = [](PlayerbotAI* ai) { return new IllidanStormrageSetEarthbindTotemAction(ai); };
            creators["illidan stormrage assist tanks handle flames of azzinoth"] = [](PlayerbotAI* ai) { return new IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction(ai); };
            creators["illidan stormrage control pet aggression"] = [](PlayerbotAI* ai) { return new IllidanStormrageControlPetAggressionAction(ai); };
            creators["illidan stormrage position above grate"] = [](PlayerbotAI* ai) { return new IllidanStormragePositionAboveGrateAction(ai); };
            creators["illidan stormrage remove dark barrage"] = [](PlayerbotAI* ai) { return new IllidanStormrageRemoveDarkBarrageAction(ai); };
            creators["illidan stormrage move away from landing point"] = [](PlayerbotAI* ai) { return new IllidanStormrageMoveAwayFromLandingPointAction(ai); };
            creators["illidan stormrage disperse ranged"] = [](PlayerbotAI* ai) { return new IllidanStormrageDisperseRangedAction(ai); };
            creators["illidan stormrage melee go somewhere to not die"] = [](PlayerbotAI* ai) { return new IllidanStormrageMeleeGoSomewhereToNotDieAction(ai); };
            creators["illidan stormrage warlock tank handle demon boss"] = [](PlayerbotAI* ai) { return new IllidanStormrageWarlockTankHandleDemonBossAction(ai); };
            creators["illidan stormrage dps prioritize adds"] = [](PlayerbotAI* ai) { return new IllidanStormrageDpsPrioritizeAddsAction(ai); };
            creators["illidan stormrage manage dps timer and rti"] = [](PlayerbotAI* ai) { return new IllidanStormrageManageDpsTimerAndRtiAction(ai); };
        }
    };

    class BtTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        BtTriggerContext()
        {
            creators["black temple bot is not in combat"] = [](PlayerbotAI* ai) { return new BlackTempleBotIsNotInCombatTrigger(ai); };
            creators["high warlord naj'entus pulling boss"] = [](PlayerbotAI* ai) { return new HighWarlordNajentusPullingBossTrigger(ai); };
            creators["high warlord naj'entus boss engaged by tanks"] = [](PlayerbotAI* ai) { return new HighWarlordNajentusBossEngagedByTanksTrigger(ai); };
            creators["high warlord naj'entus casts needle spines"] = [](PlayerbotAI* ai) { return new HighWarlordNajentusCastsNeedleSpinesTrigger(ai); };
            creators["high warlord naj'entus player is impaled"] = [](PlayerbotAI* ai) { return new HighWarlordNajentusPlayerIsImpaledTrigger(ai); };
            creators["high warlord naj'entus boss has tidal shield"] = [](PlayerbotAI* ai) { return new HighWarlordNajentusBossHasTidalShieldTrigger(ai); };
            creators["high warlord naj'entus melee spread"] = [](PlayerbotAI* ai) { return new HighWarlordNajentusMeleeSpreadTrigger(ai); };
            creators["supremus pulling boss or changing phase"] = [](PlayerbotAI* ai) { return new SupremusPullingBossOrChangingPhaseTrigger(ai); };
            creators["supremus boss engaged by ranged"] = [](PlayerbotAI* ai) { return new SupremusBossEngagedByRangedTrigger(ai); };
            creators["supremus boss is fixated on bot"] = [](PlayerbotAI* ai) { return new SupremusBossIsFixatedOnBotTrigger(ai); };
            creators["supremus volcano is nearby"] = [](PlayerbotAI* ai) { return new SupremusVolcanoIsNearbyTrigger(ai); };
            creators["supremus need to manage phase timer"] = [](PlayerbotAI* ai) { return new SupremusNeedToManagePhaseTimerTrigger(ai); };
            creators["supremus molten flame chases bot"] = [](PlayerbotAI* ai) { return new SupremusMoltenFlameChasesBotTrigger(ai); };
            creators["shade of akama killing channelers starts phase 2"] = [](PlayerbotAI* ai) { return new ShadeOfAkamaKillingChannelersStartsPhase2Trigger(ai); };
            creators["teron gorefiend pulling boss"] = [](PlayerbotAI* ai) { return new TeronGorefiendPullingBossTrigger(ai); };
            creators["teron gorefiend boss engaged by tanks"] = [](PlayerbotAI* ai) { return new TeronGorefiendBossEngagedByTanksTrigger(ai); };
            creators["teron gorefiend boss engaged by ranged"] = [](PlayerbotAI* ai) { return new TeronGorefiendBossEngagedByRangedTrigger(ai); };
            creators["teron gorefiend boss is casting shadow of death"] = [](PlayerbotAI* ai) { return new TeronGorefiendBossIsCastingShadowOfDeathTrigger(ai); };
            creators["teron gorefiend bot has shadow of death"] = [](PlayerbotAI* ai) { return new TeronGorefiendBotHasShadowOfDeathTrigger(ai); };
            creators["teron gorefiend bot transformed into vengeful spirit"] = [](PlayerbotAI* ai) { return new TeronGorefiendBotTransformedIntoVengefulSpiritTrigger(ai); };
            creators["gurtogg bloodboil pulling boss"] = [](PlayerbotAI* ai) { return new GurtoggBloodboilPullingBossTrigger(ai); };
            creators["gurtogg bloodboil boss engaged by tanks"] = [](PlayerbotAI* ai) { return new GurtoggBloodboilBossEngagedByTanksTrigger(ai); };
            creators["gurtogg bloodboil boss casts bloodboil"] = [](PlayerbotAI* ai) { return new GurtoggBloodboilBossCastsBloodboilTrigger(ai); };
            creators["gurtogg bloodboil bot has fel rage"] = [](PlayerbotAI* ai) { return new GurtoggBloodboilBotHasFelRageTrigger(ai); };
            creators["gurtogg bloodboil need to manage phase timer"] = [](PlayerbotAI* ai) { return new GurtoggBloodboilNeedToManagePhaseTimerTrigger(ai); };
            creators["reliquary of souls aggro resets upon phase change"] = [](PlayerbotAI* ai) { return new ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger(ai); };
            creators["reliquary of souls essence of suffering fixates on closest target"] = [](PlayerbotAI* ai) { return new ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger(ai); };
            creators["reliquary of souls essence of suffering disables healing"] = [](PlayerbotAI* ai) { return new ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger(ai); };
            creators["reliquary of souls essence of desire has rune shield"] = [](PlayerbotAI* ai) { return new ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger(ai); };
            creators["reliquary of souls essence of desire casting deaden"] = [](PlayerbotAI* ai) { return new ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger(ai); };
            creators["mother shahraz pulling boss"] = [](PlayerbotAI* ai) { return new MotherShahrazPullingBossTrigger(ai); };
            creators["mother shahraz boss engaged by tanks"] = [](PlayerbotAI* ai) { return new MotherShahrazBossEngagedByTanksTrigger(ai); };
            creators["mother shahraz tanks are positioning boss"] = [](PlayerbotAI* ai) { return new MotherShahrazTanksArePositioningBossTrigger(ai); };
            creators["mother shahraz sinister beam knocks back players"] = [](PlayerbotAI* ai) { return new MotherShahrazSinisterBeamKnocksBackPlayersTrigger(ai); };
            creators["mother shahraz bots are linked by fatal attraction"] = [](PlayerbotAI* ai) { return new MotherShahrazBotsAreLinkedByFatalAttractionTrigger(ai); };
            creators["illidari council pulling bosses"] = [](PlayerbotAI* ai) { return new IllidariCouncilPullingBossesTrigger(ai); };
            creators["illidari council gathios engaged by main tank"] = [](PlayerbotAI* ai) { return new IllidariCouncilGathiosEngagedByMainTankTrigger(ai); };
            creators["illidari council gathios casting judgement of command"] = [](PlayerbotAI* ai) { return new IllidariCouncilGathiosCastingJudgementOfCommandTrigger(ai); };
            creators["illidari council malande engaged by first assist tank"] = [](PlayerbotAI* ai) { return new IllidariCouncilMalandeEngagedByFirstAssistTankTrigger(ai); };
            creators["illidari council darkshadow engaged by second assist tank"] = [](PlayerbotAI* ai) { return new IllidariCouncilDarkshadowEngagedBySecondAssistTankTrigger(ai); };
            creators["illidari council zerevor engaged by mage tank"] = [](PlayerbotAI* ai) { return new IllidariCouncilZerevorEngagedByMageTankTrigger(ai); };
            creators["illidari council mage tank needs dedicated healer"] = [](PlayerbotAI* ai) { return new IllidariCouncilMageTankNeedsDedicatedHealerTrigger(ai); };
            creators["illidari council zerevor casts dangerous aoes"] = [](PlayerbotAI* ai) { return new IllidariCouncilZerevorCastsDangerousAoesTrigger(ai); };
            creators["illidari council pets screw up the pull"] = [](PlayerbotAI* ai) { return new IllidariCouncilPetsScrewUpThePullTrigger(ai); };
            creators["illidari council determining dps assignments"] = [](PlayerbotAI* ai) { return new IllidariCouncilDeterminingDpsAssignmentsTrigger(ai); };
            creators["illidari council need to manage dps timer"] = [](PlayerbotAI* ai) { return new IllidariCouncilNeedToManageDpsTimerTrigger(ai); };
            creators["illidan stormrage tank needs aggro"] = [](PlayerbotAI* ai) { return new IllidanStormrageTankNeedsAggroTrigger(ai); };
            creators["illidan stormrage boss casts flame crash in front of main tank"] = [](PlayerbotAI* ai) { return new IllidanStormrageBossCastsFlameCrashInFrontOfMainTankTrigger(ai); };
            creators["illidan stormrage bot has parasitic shadowfiend"] = [](PlayerbotAI* ai) { return new IllidanStormrageBotHasParasiticShadowfiendTrigger(ai); };
            creators["illidan stormrage parasitic shadowfiends run wild"] = [](PlayerbotAI* ai) { return new IllidanStormrageParasiticShadowfiendsRunWildTrigger(ai); };
            creators["illidan stormrage boss summoned flames of azzinoth"] = [](PlayerbotAI* ai) { return new IllidanStormrageBossSummonedFlamesOfAzzinothTrigger(ai); };
            creators["illidan stormrage pets die to fire"] = [](PlayerbotAI* ai) { return new IllidanStormragePetsDieToFireTrigger(ai); };
            creators["illidan stormrage grate is safe from flames"] = [](PlayerbotAI* ai) { return new IllidanStormrageGrateIsSafeFromFlamesTrigger(ai); };
            creators["illidan stormrage bot struck by dark barrage"] = [](PlayerbotAI* ai) { return new IllidanStormrageBotStruckByDarkBarrageTrigger(ai); };
            creators["illidan stormrage boss is preparing to land"] = [](PlayerbotAI* ai) { return new IllidanStormrageBossIsPreparingToLandTrigger(ai); };
            creators["illidan stormrage boss deals splash damage"] = [](PlayerbotAI* ai) { return new IllidanStormrageBossDealsSplashDamageTrigger(ai); };
            creators["illidan stormrage this expansion hates melee"] = [](PlayerbotAI* ai) { return new IllidanStormrageThisExpansionHatesMeleeTrigger(ai); };
            creators["illidan stormrage boss transforms into demon"] = [](PlayerbotAI* ai) { return new IllidanStormrageBossTransformsIntoDemonTrigger(ai); };
            creators["illidan stormrage boss spawns adds"] = [](PlayerbotAI* ai) { return new IllidanStormrageBossSpawnsAddsTrigger(ai); };
            creators["illidan stormrage need to manage dps timer and rti"] = [](PlayerbotAI* ai) { return new IllidanStormrageNeedToManageDpsTimerAndRtiTrigger(ai); };
        }
    };

    class BtValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        BtValueContext()
        {
        }
    };
}
