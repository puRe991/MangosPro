// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/Hyjal/HyjalMultipliers) and adapted for this core.
#pragma once

#include "playerbot/strategy/Multiplier.h"
#include "HyjalHelpers.h"

namespace ai
{
    // General

    class HyjalEncounterMultiplier : public Multiplier
    {
    public:
        HyjalEncounterMultiplier(PlayerbotAI* ai, std::string name) : Multiplier(ai, name) {}
        float GetValue(Action* action) final;

    protected:
        virtual float GetValueInEncounter(Action* action) = 0;
    };

    // Hold the dps cooldowns until the boss is engaged, and Bloodlust / Heroism during the trash waves
    class HyjalDelayDpsCooldownsMultiplier : public Multiplier
    {
    public:
        HyjalDelayDpsCooldownsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "hyjal delay dps cooldowns") {}
        float GetValue(Action* action) override;
    };

#define HYJAL_MULTIPLIER(clazz, multiplierName) \
    class clazz : public HyjalEncounterMultiplier \
    { \
    public: \
        clazz(PlayerbotAI* ai) : HyjalEncounterMultiplier(ai, multiplierName) {} \
    protected: \
        float GetValueInEncounter(Action* action) override; \
    };

    HYJAL_MULTIPLIER(HyjalDisableDisperseAndTankFaceMultiplier, "hyjal disable disperse and tank face")
    HYJAL_MULTIPLIER(HyjalMainTankNoImmunityMultiplier, "hyjal main tank no immunity")

    // Rage Winterchill
    HYJAL_MULTIPLIER(RageWinterchillMeleeControlAvoidanceMultiplier, "rage winterchill melee control avoidance")
    HYJAL_MULTIPLIER(RageWinterchillRangedControlAvoidanceMultiplier, "rage winterchill ranged control avoidance")

    // Anetheron
    HYJAL_MULTIPLIER(AnetheronDisableAssistTargetingMultiplier, "anetheron disable assist targeting")
    HYJAL_MULTIPLIER(AnetheronAvoidAccidentalInfernalAggroMultiplier, "anetheron avoid accidental infernal aggro")
    HYJAL_MULTIPLIER(AnetheronInfernalTargetRunToPositionMultiplier, "anetheron infernal target run to position")
    HYJAL_MULTIPLIER(AnetheronControlMovementMultiplier, "anetheron control movement")
    HYJAL_MULTIPLIER(AnetheronControlMisdirectionMultiplier, "anetheron control misdirection")
    HYJAL_MULTIPLIER(AnetheronControlTauntMultiplier, "anetheron control taunt")

    // Kaz'rogal
    HYJAL_MULTIPLIER(KazrogalControlLowManaMovementMultiplier, "kaz'rogal control low mana movement")
    HYJAL_MULTIPLIER(KazrogalKeepAspectOfTheViperActiveMultiplier, "kaz'rogal keep aspect of the viper active")

    // Azgalor
    HYJAL_MULTIPLIER(AzgalorDisableAutoTargetingAndPositioningMultiplier, "azgalor disable auto targeting and positioning")
    HYJAL_MULTIPLIER(AzgalorDoomedBotPrioritizePositioningMultiplier, "azgalor doomed bot prioritize positioning")
    HYJAL_MULTIPLIER(AzgalorMeleeDpsControlAvoidanceMultiplier, "azgalor melee dps control avoidance")
    HYJAL_MULTIPLIER(AzgalorRangedControlAvoidanceMultiplier, "azgalor ranged control avoidance")

    // Archimonde
    HYJAL_MULTIPLIER(ArchimondeSetTremorTotemMultiplier, "archimonde set tremor totem")
    HYJAL_MULTIPLIER(ArchimondeControlDoomfireAvoidanceMultiplier, "archimonde control doomfire avoidance")

#undef HYJAL_MULTIPLIER
}
