// Ported from mod-playerbots (src/Ai/Raid/Mag, GPL v2+) and adapted for this core.
#pragma once

#include "playerbot/strategy/Trigger.h"

namespace ai
{
    class MagtheridonEncounterTrigger : public Trigger
    {
    public:
        MagtheridonEncounterTrigger(PlayerbotAI* ai, std::string name) : Trigger(ai, name, 1) {}
        bool IsActive() final;

    protected:
        virtual bool IsActiveInEncounter() = 0;
    };

    class MagtheridonNoEncounterInProgressTrigger : public Trigger
    {
    public:
        // Throttled: true for all trash and downtime, no urgency for the clean-up.
        MagtheridonNoEncounterInProgressTrigger(PlayerbotAI* ai) : Trigger(ai, "magtheridon no encounter in progress", 2) {}
        bool IsActive() override;
    };

#define MAGTHERIDON_TRIGGER(clazz, name) \
    class clazz : public MagtheridonEncounterTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : MagtheridonEncounterTrigger(ai, name) {} \
    protected: \
        bool IsActiveInEncounter() override; \
    }

    MAGTHERIDON_TRIGGER(MagtheridonMainTankShouldTankChannelersTrigger, "magtheridon main tank should tank channelers");
    MAGTHERIDON_TRIGGER(MagtheridonAssistTanksShouldTankChannelersTrigger, "magtheridon assist tanks should tank channelers");
    MAGTHERIDON_TRIGGER(MagtheridonPullingWestAndEastChannelersTrigger, "magtheridon pulling west and east channelers");
    MAGTHERIDON_TRIGGER(MagtheridonDeterminingKillOrderTrigger, "magtheridon determining kill order");
    MAGTHERIDON_TRIGGER(MagtheridonBurningAbyssalSpawnedTrigger, "magtheridon burning abyssal spawned");
    MAGTHERIDON_TRIGGER(MagtheridonChannelerNeedsInterruptTrigger, "magtheridon channeler needs interrupt");
    MAGTHERIDON_TRIGGER(MagtheridonShouldBeTankedTrigger, "magtheridon should be tanked");
    MAGTHERIDON_TRIGGER(MagtheridonShouldSpreadRangedTrigger, "magtheridon should spread ranged");
    MAGTHERIDON_TRIGGER(MagtheridonStandingInDebrisTrigger, "magtheridon standing in debris");
    MAGTHERIDON_TRIGGER(MagtheridonIncomingBlastNovaTrigger, "magtheridon incoming blast nova");
    MAGTHERIDON_TRIGGER(MagtheridonShouldManageTimersAndAssignmentsTrigger, "magtheridon should manage timers and assignments");

#undef MAGTHERIDON_TRIGGER
}
