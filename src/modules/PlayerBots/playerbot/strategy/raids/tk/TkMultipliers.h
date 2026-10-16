// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/TK/TKMultipliers) and adapted for this core.
#pragma once
#include "playerbot/strategy/Multiplier.h"
#include "TkHelpers.h"

namespace ai
{
    class TkEncounterMultiplier : public Multiplier
    {
    public:
        TkEncounterMultiplier(PlayerbotAI* ai, std::string name) : Multiplier(ai, name) {}
        float GetValue(Action* action) final;

    protected:
        virtual float GetValueInEncounter(Action* action) = 0;
    };

#define TK_MULTIPLIER(clazz, name) \
    class clazz : public TkEncounterMultiplier \
    { \
    public: \
        clazz(PlayerbotAI* ai) : TkEncounterMultiplier(ai, name) {} \
    protected: \
        float GetValueInEncounter(Action* action) override; \
    }

    // Al'ar <Phoenix God>
    TK_MULTIPLIER(TkAlarSuppressGapClosersMultiplier, "al'ar suppress gap closers");
    TK_MULTIPLIER(TkAlarControlMovementMultiplier, "al'ar control movement");
    TK_MULTIPLIER(TkAlarDisableAutomaticTargetingMultiplier, "al'ar disable automatic targeting");
    TK_MULTIPLIER(TkAlarStayAwayFromRebirthMultiplier, "al'ar stay away from rebirth");
    TK_MULTIPLIER(TkAlarControlTauntingMultiplier, "al'ar control taunting");

    // Void Reaver
    TK_MULTIPLIER(TkVoidReaverMaintainPositionsMultiplier, "void reaver maintain positions");

    // High Astromancer Solarian
    TK_MULTIPLIER(TkSolarianDisableMeleeTargetingMultiplier, "high astromancer solarian disable melee targeting");
    TK_MULTIPLIER(TkSolarianWrathStayAwayMultiplier, "high astromancer solarian wrath stay away");

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    TK_MULTIPLIER(TkKaelthasWaitForDpsMultiplier, "kael'thas sunstrider wait for dps");
    TK_MULTIPLIER(TkKaelthasKiteThaladredMultiplier, "kael'thas sunstrider kite thaladred");
    TK_MULTIPLIER(TkKaelthasControlMisdirectionMultiplier, "kael'thas sunstrider control misdirection");
    TK_MULTIPLIER(TkKaelthasKeepDistanceFromCapernianMultiplier, "kael'thas sunstrider keep distance from capernian");
    TK_MULTIPLIER(TkKaelthasManageWeaponTankingMultiplier, "kael'thas sunstrider manage weapon tanking");
    TK_MULTIPLIER(TkKaelthasSuppressEquipUpgradeMultiplier, "kael'thas sunstrider suppress equip upgrade");
    TK_MULTIPLIER(TkKaelthasManageAutomaticTargetingMultiplier, "kael'thas sunstrider manage automatic targeting");
    TK_MULTIPLIER(TkKaelthasDisableDisperseMultiplier, "kael'thas sunstrider disable disperse");
    TK_MULTIPLIER(TkKaelthasPrepareForPhase3Multiplier, "kael'thas sunstrider prepare for phase 3");
    TK_MULTIPLIER(TkKaelthasDelayCooldownsMultiplier, "kael'thas sunstrider delay cooldowns");
    TK_MULTIPLIER(TkKaelthasStaySpreadDuringGravityLapseMultiplier, "kael'thas sunstrider stay spread during gravity lapse");

#undef TK_MULTIPLIER
}
