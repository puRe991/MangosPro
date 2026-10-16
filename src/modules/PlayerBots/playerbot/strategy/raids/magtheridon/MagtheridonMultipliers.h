// Ported from mod-playerbots (src/Ai/Raid/Mag, GPL v2+) and adapted for this core.
#pragma once

#include "playerbot/strategy/Multiplier.h"

namespace ai
{
    class MagtheridonEncounterMultiplier : public Multiplier
    {
    public:
        MagtheridonEncounterMultiplier(PlayerbotAI* ai, std::string name) : Multiplier(ai, name) {}
        float GetValue(Action* action) final;

    protected:
        virtual float GetValueInEncounter(Action* action) = 0;
    };

#define MAGTHERIDON_MULTIPLIER(clazz, name) \
    class clazz : public MagtheridonEncounterMultiplier \
    { \
    public: \
        clazz(PlayerbotAI* ai) : MagtheridonEncounterMultiplier(ai, name) {} \
    protected: \
        float GetValueInEncounter(Action* action) override; \
    }

    MAGTHERIDON_MULTIPLIER(MagtheridonUseManticronCubeMultiplier, "magtheridon use manticron cube");
    MAGTHERIDON_MULTIPLIER(MagtheridonHoldDpsMultiplier, "magtheridon hold dps");
    MAGTHERIDON_MULTIPLIER(MagtheridonControlTankActionsMultiplier, "magtheridon control tank actions");
    MAGTHERIDON_MULTIPLIER(MagtheridonAvoidDebrisDangerMultiplier, "magtheridon avoid debris danger");

#undef MAGTHERIDON_MULTIPLIER
}
