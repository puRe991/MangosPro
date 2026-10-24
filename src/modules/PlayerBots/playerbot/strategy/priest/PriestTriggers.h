#pragma once

#include "playerbot/strategy/triggers/GenericTriggers.h"

namespace ai
{
    DEBUFF_TRIGGER(HolyFireTrigger, "holy fire");
    // Shadow Word: Pain and Vampiric Touch are per-caster DoTs, so only our own copy counts
    class PowerWordPainTrigger : public DebuffTrigger
    {
    public:
        PowerWordPainTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "shadow word: pain", 1, true) {}
    };

    DEBUFF_ENEMY_TRIGGER(PowerWordPainOnAttackerTrigger, "shadow word: pain");

    class VampiricTouchTrigger : public DebuffTrigger
    {
    public:
        VampiricTouchTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "vampiric touch", 1, true) {}
    };

    DEBUFF_ENEMY_TRIGGER(VampiricTouchOnAttackerTrigger, "vampiric touch");
    DEBUFF_TRIGGER(VampiricEmbraceTrigger, "vampiric embrace");
    CURE_TRIGGER(DispelMagicTrigger, "dispel magic", DISPEL_MAGIC);
    CURE_PARTY_TRIGGER(DispelMagicPartyMemberTrigger, "dispel magic", DISPEL_MAGIC);
    CURE_TRIGGER(CureDiseaseTrigger, "cure disease", DISPEL_DISEASE);
    CURE_PARTY_TRIGGER(PartyMemberCureDiseaseTrigger, "cure disease", DISPEL_DISEASE);
    BUFF_TRIGGER_A(InnerFireTrigger, "inner fire");
    BUFF_TRIGGER_A(ShadowformTrigger, "shadowform");
    BOOST_TRIGGER_A(InnerFocusTrigger, "inner focus");
    CC_TRIGGER(ShackleUndeadTrigger, "shackle undead");
    INTERRUPT_TRIGGER(SilenceTrigger, "silence");
    INTERRUPT_HEALER_TRIGGER(SilenceEnemyHealerTrigger, "silence");

    // racials
    DEBUFF_TRIGGER(DevouringPlagueTrigger, "devouring plague");
    BUFF_TRIGGER(TouchOfWeaknessTrigger, "touch of weakness");
    DEBUFF_TRIGGER(HexOfWeaknessTrigger, "hex of weakness");
    DEFLECT_TRIGGER(FeedbackTrigger, "feedback");
    SNARE_TRIGGER(ChastiseTrigger, "chastise");
    DEBUFF_TRIGGER(StarshardsTrigger, "starshards");

    BOOST_TRIGGER_A(ShadowfiendTrigger, "shadowfiend");
    CAN_CAST_TRIGGER(MindBlastTrigger, "mind blast");

    // Shadow Word: Death as part of the rotation, but not while its backlash could kill us
    class ShadowWordDeathTrigger : public SpellCanBeCastedTrigger
    {
    public:
        ShadowWordDeathTrigger(PlayerbotAI* ai) : SpellCanBeCastedTrigger(ai, "shadow word: death") {}
        virtual bool IsActive() override
        {
            return SpellCanBeCastedTrigger::IsActive() && AI_VALUE2(uint8, "health", "self target") >= sPlayerbotAIConfig.mediumHealth;
        }
    };

    // Keeps Renew rolling on a tank that is being hit
    class RenewOnTankTrigger : public Trigger
    {
    public:
        RenewOnTankTrigger(PlayerbotAI* ai) : Trigger(ai, "renew on tank", 1) {}

        Value<ObjectGuid>* GetTargetValue() override { return context->GetValue<ObjectGuid>("party tank without lifebloom", "renew"); }
        bool IsActive() override
        {
            Unit* target = GetTarget();
            return target
                && target->IsAlive()
                && !ai->HasAura("renew", target, false, true, -1, false, 1000)
                && ai->CanCastSpell("renew", target, 0)
                && !target->getAttackers().empty();
        }
    };
    CAN_CAST_TRIGGER(SmiteTrigger, "smite");

    class ShadowguardTrigger : public BuffTrigger
    {
    public:
        ShadowguardTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "shadowguard", 4) {}
        virtual bool IsActive() override { return BuffTrigger::IsActive() && bot->getRace() == RACE_TROLL; }
    };

    class PowerWordFortitudeOnPartyTrigger : public BuffOnPartyTrigger 
    {
    public:
        PowerWordFortitudeOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "power word: fortitude", 4) {}
        virtual bool IsActive() override { return BuffOnPartyTrigger::IsActive() && !ai->HasAura("prayer of fortitude", GetTarget()); }
    };

    class PowerWordFortitudeTrigger : public BuffTrigger 
    {
    public:
        PowerWordFortitudeTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "power word: fortitude", 4) {}
        virtual bool IsActive() override { return BuffTrigger::IsActive() && !ai->HasAura("prayer of fortitude", GetTarget()); }
    };

    class DivineSpiritOnPartyTrigger : public BuffOnPartyTrigger 
    {
    public:
        DivineSpiritOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "divine spirit", 4) {}
        virtual bool IsActive() override { return BuffOnPartyTrigger::IsActive() && !ai->HasAura("prayer of spirit", GetTarget()); }
    };

    class DivineSpiritTrigger : public BuffTrigger 
    {
    public:
        DivineSpiritTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "divine spirit", 4) {}
        virtual bool IsActive() override { return BuffTrigger::IsActive() && !ai->HasAura("prayer of spirit", GetTarget()); }
    };

    class ShadowProtectionOnPartyTrigger : public BuffOnPartyTrigger
    {
    public:
        ShadowProtectionOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "shadow protection", 4) {}
        virtual bool IsActive() override { return BuffOnPartyTrigger::IsActive() && !ai->HasAura("prayer of shadow protection", GetTarget()); }
    };

    class ShadowProtectionTrigger : public BuffTrigger 
    {
    public:
        ShadowProtectionTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "shadow protection", 4) {}
        virtual bool IsActive() override { return BuffTrigger::IsActive() && !ai->HasAura("prayer of shadow protection", GetTarget()); }
    };

    class PrayerOfFortitudeOnPartyTrigger : public GreaterBuffOnPartyTrigger
    {
    public:
        PrayerOfFortitudeOnPartyTrigger(PlayerbotAI* ai) : GreaterBuffOnPartyTrigger(ai, "prayer of fortitude", "power word: fortitude", 4) {}
    };

    class PrayerOfSpiritOnPartyTrigger : public GreaterBuffOnPartyTrigger
    {
    public:
        PrayerOfSpiritOnPartyTrigger(PlayerbotAI* ai) : GreaterBuffOnPartyTrigger(ai, "prayer of spirit", "divine spirit", 4) {}
    };

    class PrayerOfShadowProtectionOnPartyTrigger : public GreaterBuffOnPartyTrigger
    {
    public:
        PrayerOfShadowProtectionOnPartyTrigger(PlayerbotAI* ai) : GreaterBuffOnPartyTrigger(ai, "prayer of shadow protection", "shadow protection", 4) {}
    };

    class BindingHealTrigger : public PartyMemberLowHealthTrigger 
    {
    public:
        BindingHealTrigger(PlayerbotAI* ai) : PartyMemberLowHealthTrigger(ai, "binding heal", sPlayerbotAIConfig.lowHealth, 0) {}

        virtual bool IsActive() override
        {
            return PartyMemberLowHealthTrigger::IsActive() && AI_VALUE2(uint8, "health", "self target") < sPlayerbotAIConfig.mediumHealth;
        }
    };

    class PowerInfusionTrigger : public SpellTargetTrigger
    {
    public:
        PowerInfusionTrigger(PlayerbotAI* ai) : SpellTargetTrigger(ai, "power infusion", "boost targets", true, true) {}
        std::string GetTargetName() override { return "self target"; }
    };

    class FearWardTrigger : public SpellTargetTrigger
    {
    public:
        FearWardTrigger(PlayerbotAI* ai) : SpellTargetTrigger(ai, "fear ward", "buff targets", true, true) {}
        std::string GetTargetName() override { return "self target"; }
    };
}
