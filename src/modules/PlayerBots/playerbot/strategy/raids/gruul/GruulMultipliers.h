// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#pragma once

#include "playerbot/strategy/Multiplier.h"

namespace ai
{
    // General

    class HighKingMaulgarHoldPreparationMultiplier : public Multiplier
    {
    public:
        HighKingMaulgarHoldPreparationMultiplier(PlayerbotAI* ai) : Multiplier(ai, "high king maulgar hold preparation") {}
        float GetValue(Action* action) override;
    };

    class GruulsLairEncounterMultiplier : public Multiplier
    {
    public:
        GruulsLairEncounterMultiplier(PlayerbotAI* ai, std::string name) : Multiplier(ai, name) {}
        float GetValue(Action* action) final;

    protected:
        virtual float GetValueInEncounter(Action* action) = 0;
    };

#define GRUUL_MULTIPLIER(clazz, name) \
    class clazz : public GruulsLairEncounterMultiplier \
    { \
    public: \
        clazz(PlayerbotAI* ai) : GruulsLairEncounterMultiplier(ai, name) {} \
    protected: \
        float GetValueInEncounter(Action* action) override; \
    }

    GRUUL_MULTIPLIER(GruulsLairDelayDpsCooldownsMultiplier, "gruul's lair delay dps cooldowns");

    // High King Maulgar <Lord of the Ogres>
    GRUUL_MULTIPLIER(HighKingMaulgarControlTankActionsMultiplier, "high king maulgar control tank actions");
    GRUUL_MULTIPLIER(HighKingMaulgarRestrictTauntingMultiplier, "high king maulgar restrict taunting");
    GRUUL_MULTIPLIER(HighKingMaulgarDisableDpsAssistMultiplier, "high king maulgar disable dps assist");
    GRUUL_MULTIPLIER(HighKingMaulgarAvoidWhirlwindMultiplier, "high king maulgar avoid whirlwind");
    GRUUL_MULTIPLIER(HighKingMaulgarControlHunterActionsMultiplier, "high king maulgar control hunter actions");
    GRUUL_MULTIPLIER(HighKingMaulgarControlMageTankActionsMultiplier, "high king maulgar control mage tank actions");

    // Gruul the Dragonkiller
    GRUUL_MULTIPLIER(GruulTheDragonkillerControlTankMovementMultiplier, "gruul the dragonkiller control tank movement");
    GRUUL_MULTIPLIER(GruulTheDragonkillerStaySpreadForShatterMultiplier, "gruul the dragonkiller stay spread for shatter");
    GRUUL_MULTIPLIER(GruulTheDragonkillerControlAvoidanceMultiplier, "gruul the dragonkiller control avoidance");
    GRUUL_MULTIPLIER(GruulTheDragonkillerHoldWhileSnaredMultiplier, "gruul the dragonkiller hold while snared");

#undef GRUUL_MULTIPLIER
}
