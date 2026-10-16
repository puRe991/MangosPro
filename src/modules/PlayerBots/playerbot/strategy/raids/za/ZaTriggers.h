// Ported from mod-playerbots (AzerothCore, GPL v2+), src/Ai/Raid/ZA/ZATriggers.h, and adapted for this core.
#pragma once
#include "playerbot/strategy/Trigger.h"
#include "ZaHelpers.h"
#include <string>

namespace ai
{
    // Base: active only while a Zul'Aman boss is engaged
    class ZaEncounterTrigger : public Trigger
    {
    public:
        ZaEncounterTrigger(PlayerbotAI* ai, std::string name, int checkInterval = 1) : Trigger(ai, name, checkInterval) {}

        bool IsActive() final { return za::IsEncounterInProgress(bot) && IsActiveInEncounter(); }

    protected:
        virtual bool IsActiveInEncounter() = 0;
    };

    // General

    // Between encounters: reset the encounter state of the bot (throttled, no urgency)
    class ZaNoEncounterInProgressTrigger : public Trigger
    {
    public:
        ZaNoEncounterInProgressTrigger(PlayerbotAI* ai) : Trigger(ai, "zul'aman no encounter in progress", 2) {}
        bool IsActive() override;
    };

    // Trash

    class ZaAmanishiMedicineManSummonedWardTrigger : public Trigger
    {
    public:
        ZaAmanishiMedicineManSummonedWardTrigger(PlayerbotAI* ai) : Trigger(ai, "amani'shi medicine man summoned ward") {}
        bool IsActive() override;
    };

    // For Misdirection to the boss on the pull. Used by every boss.
    class ZaPullingBossTrigger : public ZaEncounterTrigger
    {
    public:
        ZaPullingBossTrigger(PlayerbotAI* ai, std::string name, uint32 bossEntry) : ZaEncounterTrigger(ai, name), bossEntry(bossEntry) {}

    protected:
        bool IsActiveInEncounter() override;

    private:
        uint32 bossEntry;
    };

    // Akil'zon <Eagle Avatar>

    class ZaAkilzonShouldBeTankedTrigger : public ZaEncounterTrigger
    {
    public:
        ZaAkilzonShouldBeTankedTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "akil'zon should be tanked") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaAkilzonSpreadForStaticDisruptionTrigger : public ZaEncounterTrigger
    {
    public:
        ZaAkilzonSpreadForStaticDisruptionTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "akil'zon spread for static disruption") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaAkilzonElectricalStormIncomingTrigger : public ZaEncounterTrigger
    {
    public:
        ZaAkilzonElectricalStormIncomingTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "akil'zon electrical storm incoming") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaAkilzonShouldTrackElectricalStormTrigger : public ZaEncounterTrigger
    {
    public:
        ZaAkilzonShouldTrackElectricalStormTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "akil'zon should track electrical storm") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    // Nalorakk <Bear Avatar>

    class ZaNalorakkBothFormsShouldBeTankedTrigger : public ZaEncounterTrigger
    {
    public:
        ZaNalorakkBothFormsShouldBeTankedTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "nalorakk both forms should be tanked") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaNalorakkSpreadForSurgeTrigger : public ZaEncounterTrigger
    {
    public:
        ZaNalorakkSpreadForSurgeTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "nalorakk spread for surge") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    // Jan'alai <Dragonhawk Avatar>

    class ZaJanalaiShouldBeTankedTrigger : public ZaEncounterTrigger
    {
    public:
        ZaJanalaiShouldBeTankedTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "jan'alai should be tanked") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaJanalaiSpreadForFlameBreathTrigger : public ZaEncounterTrigger
    {
    public:
        ZaJanalaiSpreadForFlameBreathTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "jan'alai spread for flame breath") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaJanalaiIsFireBombingTrigger : public ZaEncounterTrigger
    {
    public:
        ZaJanalaiIsFireBombingTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "jan'alai is fire bombing") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaJanalaiAmanishiHatchersSpawnedTrigger : public ZaEncounterTrigger
    {
    public:
        ZaJanalaiAmanishiHatchersSpawnedTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "jan'alai amani'shi hatchers spawned") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    // Halazzi <Lynx Avatar>

    class ZaHalazziShouldBeTankedTrigger : public ZaEncounterTrigger
    {
    public:
        ZaHalazziShouldBeTankedTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "halazzi should be tanked") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaHalazziSpiritLynxHasAppearedTrigger : public ZaEncounterTrigger
    {
    public:
        ZaHalazziSpiritLynxHasAppearedTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "halazzi spirit lynx has appeared") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaHalazziShouldFocusDpsTrigger : public ZaEncounterTrigger
    {
    public:
        ZaHalazziShouldFocusDpsTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "halazzi should focus dps") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    // Hex Lord Malacrass

    class ZaHexLordMalacrassShouldPrioritizeAddsTrigger : public ZaEncounterTrigger
    {
    public:
        ZaHexLordMalacrassShouldPrioritizeAddsTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "hex lord malacrass should prioritize adds") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaHexLordMalacrassChannelingWhirlwindTrigger : public ZaEncounterTrigger
    {
    public:
        ZaHexLordMalacrassChannelingWhirlwindTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "hex lord malacrass channeling whirlwind") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaHexLordMalacrassFreezingTrapPlacedTrigger : public ZaEncounterTrigger
    {
    public:
        ZaHexLordMalacrassFreezingTrapPlacedTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "hex lord malacrass freezing trap placed") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    // Zul'jin

    class ZaZuljinShouldBeTankedTrigger : public ZaEncounterTrigger
    {
    public:
        ZaZuljinShouldBeTankedTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "zul'jin should be tanked") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaZuljinChannelingWhirlwindInTrollFormTrigger : public ZaEncounterTrigger
    {
    public:
        ZaZuljinChannelingWhirlwindInTrollFormTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "zul'jin channeling whirlwind in troll form") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaZuljinCreepingParalysisInBearFormTrigger : public ZaEncounterTrigger
    {
    public:
        ZaZuljinCreepingParalysisInBearFormTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "zul'jin creeping paralysis in bear form") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaZuljinSummoningCyclonesInEagleFormTrigger : public ZaEncounterTrigger
    {
    public:
        ZaZuljinSummoningCyclonesInEagleFormTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "zul'jin summoning cyclones in eagle form") {}

    protected:
        bool IsActiveInEncounter() override;
    };

    class ZaZuljinSpreadForDragonhawkAoeTrigger : public ZaEncounterTrigger
    {
    public:
        ZaZuljinSpreadForDragonhawkAoeTrigger(PlayerbotAI* ai) : ZaEncounterTrigger(ai, "zul'jin spread for dragonhawk aoe") {}

    protected:
        bool IsActiveInEncounter() override;
    };
}
