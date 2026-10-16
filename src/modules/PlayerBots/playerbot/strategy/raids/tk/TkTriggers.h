// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/TK/TKTriggers) and adapted for this core.
#pragma once
#include "playerbot/strategy/Trigger.h"
#include "TkHelpers.h"

namespace ai
{
    // General

    class TkEncounterTrigger : public Trigger
    {
    public:
        TkEncounterTrigger(PlayerbotAI* ai, std::string name, int checkInterval = 1) : Trigger(ai, name, checkInterval) {}
        bool IsActive() final { return tk::IsTkEncounterInProgress(bot) && IsActiveInEncounter(); }

    protected:
        virtual bool IsActiveInEncounter() = 0;
    };

    // Once per second. True for all trash and downtime and, being for between-encounter clean-up,
    // has no real urgency to it.
    class TkNoEncounterInProgressTrigger : public Trigger
    {
    public:
        TkNoEncounterInProgressTrigger(PlayerbotAI* ai) : Trigger(ai, "tempest keep no encounter in progress", 1) {}
        bool IsActive() override;
    };

    // Same idea as above; this is to address a fringe wipe scenario during Kael'thas.
    class TkStuckFallingTrigger : public Trigger
    {
    public:
        TkStuckFallingTrigger(PlayerbotAI* ai) : Trigger(ai, "tempest keep stuck falling", 1) {}
        bool IsActive() override;
    };

    // Trash

    class TkCrimsonHandCenturionCastsArcaneFlurryTrigger : public Trigger
    {
    public:
        TkCrimsonHandCenturionCastsArcaneFlurryTrigger(PlayerbotAI* ai) : Trigger(ai, "crimson hand centurion casts arcane flurry") {}
        bool IsActive() override;
    };

    // Al'ar <Phoenix God>

#define TK_ENCOUNTER_TRIGGER(clazz, name) \
    class clazz : public TkEncounterTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : TkEncounterTrigger(ai, name) {} \
    protected: \
        bool IsActiveInEncounter() override; \
    }

    TK_ENCOUNTER_TRIGGER(TkAlarPullingBossTrigger, "al'ar pulling boss");
    TK_ENCOUNTER_TRIGGER(TkAlarFliesBetweenPlatformsTrigger, "al'ar flies between platforms");
    TK_ENCOUNTER_TRIGGER(TkAlarEmbersExplodeUponDeathTrigger, "al'ar embers explode upon death");
    TK_ENCOUNTER_TRIGGER(TkAlarShouldAssignNonTankTargetTrigger, "al'ar should assign non-tank target");
    TK_ENCOUNTER_TRIGGER(TkAlarIncomingFlameQuillsTrigger, "al'ar incoming flame quills");
    TK_ENCOUNTER_TRIGGER(TkAlarRisingFromTheAshesTrigger, "al'ar rising from the ashes");
    TK_ENCOUNTER_TRIGGER(TkAlarInPhase2Trigger, "al'ar in phase 2");
    TK_ENCOUNTER_TRIGGER(TkAlarShouldManagePhaseTrackerTrigger, "al'ar should manage phase tracker");

    // Void Reaver

    TK_ENCOUNTER_TRIGGER(TkVoidReaverShouldBeTankedTrigger, "void reaver should be tanked");
    TK_ENCOUNTER_TRIGGER(TkVoidReaverKnockAwayPullsAggroToNonTanksTrigger, "void reaver knock away pulls aggro to non-tanks");
    TK_ENCOUNTER_TRIGGER(TkVoidReaverRangedShouldStandBackTrigger, "void reaver ranged should stand back");
    TK_ENCOUNTER_TRIGGER(TkVoidReaverArcaneOrbIsIncomingTrigger, "void reaver arcane orb is incoming");

    // High Astromancer Solarian

    TK_ENCOUNTER_TRIGGER(TkSolarianShouldBeTankedTrigger, "high astromancer solarian should be tanked");
    TK_ENCOUNTER_TRIGGER(TkSolarianWrathOfTheAstromancerTrigger, "high astromancer solarian wrath of the astromancer");
    TK_ENCOUNTER_TRIGGER(TkSolarianSolariumPriestsSpawnedTrigger, "high astromancer solarian solarium priests spawned");

    // Kael'thas Sunstrider <Lord of the Blood Elves>

    TK_ENCOUNTER_TRIGGER(TkKaelthasChasedByThaladredTrigger, "kael'thas sunstrider chased by thaladred");
    TK_ENCOUNTER_TRIGGER(TkKaelthasPullingTankableAdvisorsTrigger, "kael'thas sunstrider pulling tankable advisors");
    TK_ENCOUNTER_TRIGGER(TkKaelthasSanguinarOrTelonicusShouldBeTankedTrigger, "kael'thas sunstrider sanguinar or telonicus should be tanked");
    TK_ENCOUNTER_TRIGGER(TkKaelthasCapernianShouldBeTankedByWarlockTrigger, "kael'thas sunstrider capernian should be tanked by warlock");
    TK_ENCOUNTER_TRIGGER(TkKaelthasShouldStandBackFromCapernianTrigger, "kael'thas sunstrider should stand back from capernian");
    TK_ENCOUNTER_TRIGGER(TkKaelthasShouldHoldPhase3PositionsTrigger, "kael'thas sunstrider should hold phase 3 positions");
    TK_ENCOUNTER_TRIGGER(TkKaelthasDeterminingAdvisorKillOrderTrigger, "kael'thas sunstrider determining advisor kill order");
    TK_ENCOUNTER_TRIGGER(TkKaelthasShouldManageAdvisorDpsTimerTrigger, "kael'thas sunstrider should manage advisor dps timer");
    TK_ENCOUNTER_TRIGGER(TkKaelthasLegendaryWeaponsAreAliveTrigger, "kael'thas sunstrider legendary weapons are alive");
    TK_ENCOUNTER_TRIGGER(TkKaelthasLegendaryAxeCastsWhirlwindTrigger, "kael'thas sunstrider legendary axe casts whirlwind");
    TK_ENCOUNTER_TRIGGER(TkKaelthasLegendaryWeaponsAreDeadTrigger, "kael'thas sunstrider legendary weapons are dead");
    TK_ENCOUNTER_TRIGGER(TkKaelthasLegendaryWeaponsAreEquippedTrigger, "kael'thas sunstrider legendary weapons are equipped");
    TK_ENCOUNTER_TRIGGER(TkKaelthasHasEnteredTheFightTrigger, "kael'thas sunstrider has entered the fight");
    TK_ENCOUNTER_TRIGGER(TkKaelthasShouldAssignFinalPhaseTargetTrigger, "kael'thas sunstrider should assign final phase target");
    TK_ENCOUNTER_TRIGGER(TkKaelthasRaidMemberIsMindControlledTrigger, "kael'thas sunstrider raid member is mind controlled");
    TK_ENCOUNTER_TRIGGER(TkKaelthasInGravityLapsePhaseTrigger, "kael'thas sunstrider in gravity lapse phase");

#undef TK_ENCOUNTER_TRIGGER

    // Out of the encounter: the legendary weapons were removed (Remove Enchanted Weapons) and
    // the weapon slots are empty
    class TkKaelthasLegendaryWeaponsWereLostTrigger : public Trigger
    {
    public:
        TkKaelthasLegendaryWeaponsWereLostTrigger(PlayerbotAI* ai) : Trigger(ai, "kael'thas sunstrider legendary weapons were lost", 2) {}
        bool IsActive() override;
    };
}
