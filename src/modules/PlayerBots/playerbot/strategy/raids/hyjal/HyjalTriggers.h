// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/Hyjal/HyjalTriggers) and adapted for this core.
#pragma once

#include "playerbot/strategy/Trigger.h"
#include "HyjalHelpers.h"

namespace ai
{
    // General

    class HyjalEncounterTrigger : public Trigger
    {
    public:
        HyjalEncounterTrigger(PlayerbotAI* ai, std::string name, int checkInterval = 1) : Trigger(ai, name, checkInterval) {}
        bool IsActive() final;

    protected:
        virtual bool IsActiveInEncounter() = 0;
    };

    // Clean-up between encounters (checked every 2 seconds)
    class HyjalNoEncounterInProgressTrigger : public Trigger
    {
    public:
        HyjalNoEncounterInProgressTrigger(PlayerbotAI* ai) : Trigger(ai, "hyjal no encounter in progress", 2) {}
        bool IsActive() override;
    };

    // Hunters misdirect the boss to the main tank at the pull. Anetheron is not included because
    // Misdirection is used there for the Infernals too.
    class HyjalPullingBossTrigger : public HyjalEncounterTrigger
    {
    public:
        HyjalPullingBossTrigger(PlayerbotAI* ai, std::string name, uint32 bossEntry) : HyjalEncounterTrigger(ai, name), bossEntry(bossEntry) {}

    protected:
        bool IsActiveInEncounter() override;

    private:
        uint32 bossEntry;
    };

    // All five boss tanking triggers. activeAboveHealthPct is used for Archimonde only. Anetheron,
    // Kaz'rogal and Azgalor need their off tanks for the Infernals, the Malevolent Cleave split and
    // the Doomguards, so those three are main tank only.
    class HyjalBossShouldBeTankedTrigger : public HyjalEncounterTrigger
    {
    public:
        HyjalBossShouldBeTankedTrigger(PlayerbotAI* ai, std::string name, uint32 bossEntry, float activeAboveHealthPct = 0.0f, bool mainTankOnly = true)
            : HyjalEncounterTrigger(ai, name), bossEntry(bossEntry), activeAboveHealthPct(activeAboveHealthPct), mainTankOnly(mainTankOnly) {}

    protected:
        bool IsActiveInEncounter() override;

    private:
        uint32 bossEntry;
        float activeAboveHealthPct;
        bool mainTankOnly;
    };

#define HYJAL_TRIGGER(clazz, triggerName) \
    class clazz : public HyjalEncounterTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : HyjalEncounterTrigger(ai, triggerName) {} \
    protected: \
        bool IsActiveInEncounter() override; \
    };

    // Rage Winterchill
    HYJAL_TRIGGER(RageWinterchillRangedShouldSpreadTrigger, "rage winterchill ranged should spread")
    HYJAL_TRIGGER(RageWinterchillMeleeNearDeathAndDecayTrigger, "rage winterchill melee near death and decay")
    HYJAL_TRIGGER(RageWinterchillRangedInDeathAndDecayTrigger, "rage winterchill ranged in death and decay")

    // Anetheron
    HYJAL_TRIGGER(AnetheronPullingBossOrInfernalTrigger, "anetheron pulling boss or infernal")
    HYJAL_TRIGGER(AnetheronRangedShouldSpreadTrigger, "anetheron ranged should spread")
    HYJAL_TRIGGER(AnetheronNearInfernoTargetTrigger, "anetheron near inferno target")
    HYJAL_TRIGGER(AnetheronTargetedByInfernalTrigger, "anetheron targeted by infernal")
    HYJAL_TRIGGER(AnetheronInfernalsPulseImmolationTrigger, "anetheron infernals pulse immolation")
    HYJAL_TRIGGER(AnetheronInfernalsShouldBeTankedAwayTrigger, "anetheron infernals should be tanked away")
    HYJAL_TRIGGER(AnetheronShouldDivideDpsTrigger, "anetheron should divide dps")

    // Kaz'rogal
    HYJAL_TRIGGER(KazrogalCanSplitMalevolentCleaveDamageTrigger, "kaz'rogal can split malevolent cleave damage")
    HYJAL_TRIGGER(KazrogalRangedShouldAvoidWarStompTrigger, "kaz'rogal ranged should avoid war stomp")
    HYJAL_TRIGGER(KazrogalLowOnManaTrigger, "kaz'rogal low on mana")
    HYJAL_TRIGGER(KazrogalHunterShouldPreserveManaTrigger, "kaz'rogal hunter should preserve mana")
    HYJAL_TRIGGER(KazrogalMarkOnMageOrPaladinTrigger, "kaz'rogal mark on mage or paladin")
    HYJAL_TRIGGER(KazrogalImmunityNoLongerNeededTrigger, "kaz'rogal immunity no longer needed")
    HYJAL_TRIGGER(KazrogalWarlockShouldManageManaTrigger, "kaz'rogal warlock should manage mana")

    // Azgalor
    HYJAL_TRIGGER(AzgalorRangedShouldSpreadTrigger, "azgalor ranged should spread")
    HYJAL_TRIGGER(AzgalorMeleeNearRainOfFireTrigger, "azgalor melee near rain of fire")
    HYJAL_TRIGGER(AzgalorRangedInRainOfFireTrigger, "azgalor ranged in rain of fire")
    HYJAL_TRIGGER(AzgalorBotIsDoomedTrigger, "azgalor bot is doomed")
    HYJAL_TRIGGER(AzgalorShouldControlDoomguardsTrigger, "azgalor should control doomguards")
    HYJAL_TRIGGER(AzgalorShouldDivideDpsTrigger, "azgalor should divide dps")

    // Archimonde
    HYJAL_TRIGGER(ArchimondeShamanShouldProtectAgainstFearTrigger, "archimonde shaman should protect against fear")
    HYJAL_TRIGGER(ArchimondeCastingAirBurstTrigger, "archimonde casting air burst")
    HYJAL_TRIGGER(ArchimondeRangedShouldSpreadTrigger, "archimonde ranged should spread")
    HYJAL_TRIGGER(ArchimondeNearDoomfireTrigger, "archimonde near doomfire")
    HYJAL_TRIGGER(ArchimondeStoodInDoomfireTrigger, "archimonde stood in doomfire")
    HYJAL_TRIGGER(ArchimondeKnockedIntoTheAirTrigger, "archimonde knocked into the air")
    HYJAL_TRIGGER(ArchimondeGripOfTheLegionTrigger, "archimonde grip of the legion")
    HYJAL_TRIGGER(ArchimondeOutOfTremorTotemRangeTrigger, "archimonde out of tremor totem range")
    HYJAL_TRIGGER(ArchimondeBossNearDoomfireTrigger, "archimonde boss near doomfire")
    HYJAL_TRIGGER(ArchimondeMeleeShouldSpreadTrigger, "archimonde melee should spread")
    HYJAL_TRIGGER(ArchimondeBotInDangerOfDyingTrigger, "archimonde bot in danger of dying")

    // Remembers the pull of the bot's own view of the fight (each bot has its own trigger object)
    class ArchimondeHandOfDeathSoonTrigger : public HyjalEncounterTrigger
    {
    public:
        ArchimondeHandOfDeathSoonTrigger(PlayerbotAI* ai) : HyjalEncounterTrigger(ai, "archimonde hand of death soon") {}
    protected:
        bool IsActiveInEncounter() override;
    private:
        time_t pullTime = 0;
        time_t lastSeen = 0;
    };

#undef HYJAL_TRIGGER
}
