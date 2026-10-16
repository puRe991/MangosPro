// Ported from mod-playerbots (src/Ai/Raid/SSC/SSCTriggers.h, GPL v2+) and adapted for this core.
#pragma once
#include "playerbot/strategy/Trigger.h"

namespace ai
{
#define SSC_TRIGGER(className, triggerName) \
    class className : public Trigger \
    { \
    public: \
        className(PlayerbotAI* ai) : Trigger(ai, triggerName) {} \
        bool IsActive() override; \
    };

    // General
    SSC_TRIGGER(SscBotIsNotInCombatTrigger, "serpent shrine cavern bot is not in combat")

    // Trash
    SSC_TRIGGER(SscUnderbogColossusSpawnedToxicPoolAfterDeathTrigger, "underbog colossus spawned toxic pool after death")
    SSC_TRIGGER(SscGreyheartTidecallerWaterElementalTotemSpawnedTrigger, "greyheart tidecaller water elemental totem spawned")

    // Hydross the Unstable <Duke of Currents>
    SSC_TRIGGER(SscHydrossBotIsFrostTankTrigger, "hydross the unstable bot is frost tank")
    SSC_TRIGGER(SscHydrossBotIsNatureTankTrigger, "hydross the unstable bot is nature tank")
    SSC_TRIGGER(SscHydrossElementalsSpawnedTrigger, "hydross the unstable elementals spawned")
    SSC_TRIGGER(SscHydrossDangerFromWaterTombsTrigger, "hydross the unstable danger from water tombs")
    SSC_TRIGGER(SscHydrossTankNeedsAggroUponPhaseChangeTrigger, "hydross the unstable tank needs aggro upon phase change")
    SSC_TRIGGER(SscHydrossAggroResetsUponPhaseChangeTrigger, "hydross the unstable aggro resets upon phase change")
    SSC_TRIGGER(SscHydrossNeedToManageTimersTrigger, "hydross the unstable need to manage timers")

    // The Lurker Below
    SSC_TRIGGER(SscLurkerSpoutIsActiveTrigger, "the lurker below spout is active")
    SSC_TRIGGER(SscLurkerBossIsActiveForMainTankTrigger, "the lurker below boss is active for main tank")
    SSC_TRIGGER(SscLurkerBossCastsGeyserTrigger, "the lurker below boss casts geyser")
    SSC_TRIGGER(SscLurkerBossIsSubmergedTrigger, "the lurker below boss is submerged")
    SSC_TRIGGER(SscLurkerNeedToPrepareTimerForSpoutTrigger, "the lurker below need to prepare timer for spout")

    // Leotheras the Blind
    SSC_TRIGGER(SscLeotherasBossIsInactiveTrigger, "leotheras the blind boss is inactive")
    SSC_TRIGGER(SscLeotherasBossTransformedIntoDemonFormTrigger, "leotheras the blind boss transformed into demon form")
    SSC_TRIGGER(SscLeotherasOnlyWarlockShouldTankDemonFormTrigger, "leotheras the blind only warlock should tank demon form")
    SSC_TRIGGER(SscLeotherasBossEngagedByRangedTrigger, "leotheras the blind boss engaged by ranged")
    SSC_TRIGGER(SscLeotherasBossChannelingWhirlwindTrigger, "leotheras the blind boss channeling whirlwind")
    SSC_TRIGGER(SscLeotherasBotHasTooManyChaosBlastStacksTrigger, "leotheras the blind bot has too many chaos blast stacks")
    SSC_TRIGGER(SscLeotherasInnerDemonHasAwakenedTrigger, "leotheras the blind inner demon has awakened")
    SSC_TRIGGER(SscLeotherasEnteredFinalPhaseTrigger, "leotheras the blind entered final phase")
    SSC_TRIGGER(SscLeotherasDemonFormTankNeedsAggroTrigger, "leotheras the blind demon form tank needs aggro")
    SSC_TRIGGER(SscLeotherasBossWipesAggroUponPhaseChangeTrigger, "leotheras the blind boss wipes aggro upon phase change")

    // Fathom-Lord Karathress
    SSC_TRIGGER(SscKarathressBossEngagedByMainTankTrigger, "fathom-lord karathress boss engaged by main tank")
    SSC_TRIGGER(SscKarathressCaribdisEngagedByFirstAssistTankTrigger, "fathom-lord karathress caribdis engaged by first assist tank")
    SSC_TRIGGER(SscKarathressSharkkisEngagedBySecondAssistTankTrigger, "fathom-lord karathress sharkkis engaged by second assist tank")
    SSC_TRIGGER(SscKarathressTidalvessEngagedByThirdAssistTankTrigger, "fathom-lord karathress tidalvess engaged by third assist tank")
    SSC_TRIGGER(SscKarathressCaribdisTankNeedsDedicatedHealerTrigger, "fathom-lord karathress caribdis tank needs dedicated healer")
    SSC_TRIGGER(SscKarathressPullingBossesTrigger, "fathom-lord karathress pulling bosses")
    SSC_TRIGGER(SscKarathressDeterminingKillOrderTrigger, "fathom-lord karathress determining kill order")
    SSC_TRIGGER(SscKarathressTanksNeedToEstablishAggroTrigger, "fathom-lord karathress tanks need to establish aggro")

    // Morogrim Tidewalker
    SSC_TRIGGER(SscMorogrimPullingBossTrigger, "morogrim tidewalker pulling boss")
    SSC_TRIGGER(SscMorogrimBossEngagedByMainTankTrigger, "morogrim tidewalker boss engaged by main tank")
    SSC_TRIGGER(SscMorogrimWaterGlobulesAreIncomingTrigger, "morogrim tidewalker water globules are incoming")

    // Lady Vashj <Coilfang Matron>
    SSC_TRIGGER(SscVashjBossEngagedByMainTankTrigger, "lady vashj boss engaged by main tank")
    SSC_TRIGGER(SscVashjBossEngagedByRangedInPhase1Trigger, "lady vashj boss engaged by ranged in phase 1")
    SSC_TRIGGER(SscVashjCastsShockBlastOnHighestAggroTrigger, "lady vashj casts shock blast on highest aggro")
    SSC_TRIGGER(SscVashjBotHasStaticChargeTrigger, "lady vashj bot has static charge")
    SSC_TRIGGER(SscVashjPullingBossInPhase1AndPhase3Trigger, "lady vashj pulling boss in phase 1 and phase 3")
    SSC_TRIGGER(SscVashjTaintedCoreCanBeLootedTrigger, "lady vashj tainted core can be looted")
    SSC_TRIGGER(SscVashjTaintedCoreWasLootedTrigger, "lady vashj tainted core was looted")
    SSC_TRIGGER(SscVashjTaintedCoreIsUnusableTrigger, "lady vashj tainted core is unusable")
    SSC_TRIGGER(SscVashjAddsSpawnInPhase2AndPhase3Trigger, "lady vashj adds spawn in phase 2 and phase 3")
    SSC_TRIGGER(SscVashjCoilfangStriderIsApproachingTrigger, "lady vashj coilfang strider is approaching")
    SSC_TRIGGER(SscVashjToxicSporebatsAreSpewingPoisonCloudsTrigger, "lady vashj toxic sporebats are spewing poison clouds")
    SSC_TRIGGER(SscVashjBotIsEntangledInToxicSporesOrStaticChargeTrigger, "lady vashj bot is entangled in toxic spores or static charge")

#undef SSC_TRIGGER
}
