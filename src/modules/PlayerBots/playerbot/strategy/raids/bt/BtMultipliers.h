// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/BT/BTMultipliers) and adapted for this core.
#pragma once
#include "playerbot/strategy/Multiplier.h"

#define BT_MULTIPLIER(ClassName, multiplierName)                                    \
    class ClassName : public Multiplier                                             \
    {                                                                               \
    public:                                                                         \
        ClassName(PlayerbotAI* ai) : Multiplier(ai, multiplierName) {}              \
        float GetValue(Action* action) override;                                    \
    };

namespace ai
{
    // General
    BT_MULTIPLIER(BlackTempleDelayDpsCooldownsMultiplier, "black temple delay dps cooldowns")

    // High Warlord Naj'entus
    BT_MULTIPLIER(HighWarlordNajentusDisableCombatFormationMoveMultiplier, "high warlord naj'entus disable combat formation move")

    // Supremus
    BT_MULTIPLIER(SupremusFocusOnAvoidanceInPhase2Multiplier, "supremus focus on avoidance in phase 2")
    BT_MULTIPLIER(SupremusHitboxIsBuggedMultiplier, "supremus hitbox is bugged")

    // Shade of Akama (keeps "tbc raid attack priority target" from pulling melee off the channelers)
    BT_MULTIPLIER(ShadeOfAkamaMeleeFocusChannelersMultiplier, "shade of akama melee focus channelers")

    // Teron Gorefiend
    BT_MULTIPLIER(TeronGorefiendControlMovementMultiplier, "teron gorefiend control movement")
    BT_MULTIPLIER(TeronGorefiendMarkedBotOnlyMoveToDieMultiplier, "teron gorefiend marked bot only move to die")
    BT_MULTIPLIER(TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier, "teron gorefiend spirits attack only shadowy constructs")
    BT_MULTIPLIER(TeronGorefiendDisableAttackingConstructsMultiplier, "teron gorefiend disable attacking constructs")

    // Gurtogg Bloodboil
    BT_MULTIPLIER(GurtoggBloodboilControlMovementMultiplier, "gurtogg bloodboil control movement")

    // Reliquary of Souls
    BT_MULTIPLIER(ReliquaryOfSoulsDontWasteHealingMultiplier, "reliquary of souls dont waste healing")

    // Mother Shahraz
    BT_MULTIPLIER(MotherShahrazControlMovementMultiplier, "mother shahraz control movement")
    BT_MULTIPLIER(MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier, "mother shahraz bots with fatal attraction only run away")

    // Illidari Council
    BT_MULTIPLIER(IllidariCouncilDisableTankActionsMultiplier, "illidari council disable tank actions")
    BT_MULTIPLIER(IllidariCouncilControlMovementMultiplier, "illidari council control movement")
    BT_MULTIPLIER(IllidariCouncilControlMisdirectionMultiplier, "illidari council control misdirection")
    BT_MULTIPLIER(IllidariCouncilDisableArcaneShotOnZerevorMultiplier, "illidari council disable arcane shot on zerevor")
    BT_MULTIPLIER(IllidariCouncilDisableIceBlockMultiplier, "illidari council disable ice block")
    BT_MULTIPLIER(IllidariCouncilWaitForDpsMultiplier, "illidari council wait for dps")

    // Illidan Stormrage <The Betrayer>
    BT_MULTIPLIER(IllidanStormrageDelayDpsCooldownsMultiplier, "illidan stormrage delay dps cooldowns")
    BT_MULTIPLIER(IllidanStormrageControlTankActionsMultiplier, "illidan stormrage control tank actions")
    BT_MULTIPLIER(IllidanStormrageDisableDefaultTargetingMultiplier, "illidan stormrage disable default targeting")
    BT_MULTIPLIER(IllidanStormrageControlNonTankMovementMultiplier, "illidan stormrage control non tank movement")
    BT_MULTIPLIER(IllidanStormrageUseEarthbindTotemMultiplier, "illidan stormrage use earthbind totem")
    BT_MULTIPLIER(IllidanStormrageWaitForDpsMultiplier, "illidan stormrage wait for dps")
}

#undef BT_MULTIPLIER
