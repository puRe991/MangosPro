#pragma once
#include "playerbot/strategy/triggers/GenericTriggers.h"

namespace ai 
{
    class MarkOfTheWildOnPartyTrigger : public BuffOnPartyTrigger 
    {
    public:
        MarkOfTheWildOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "mark of the wild", 4) {}
        virtual bool IsActive() override { return BuffOnPartyTrigger::IsActive() && !ai->HasAura("gift of the wild", GetTarget()); }
    };

    class MarkOfTheWildTrigger : public BuffTrigger 
    {
    public:
        MarkOfTheWildTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "mark of the wild", 4) {}
        virtual bool IsActive() override { return BuffTrigger::IsActive() && !ai->HasAura("gift of the wild", GetTarget()); }
    };

    class GiftOfTheWildOnPartyTrigger : public GreaterBuffOnPartyTrigger
    {
    public:
        GiftOfTheWildOnPartyTrigger(PlayerbotAI* ai) : GreaterBuffOnPartyTrigger(ai, "gift of the wild", "mark of the wild", 4) {}
    };

    class ThornsOnPartyTrigger : public BuffOnTankTrigger
    {
    public:
        ThornsOnPartyTrigger(PlayerbotAI* ai) : BuffOnTankTrigger(ai, "thorns", 4) {}

        virtual bool IsActive() override
        {
            Unit* target = GetTarget();
            if (target && BuffOnTankTrigger::IsActive() && (!target->IsPlayer() || !ai->IsRanged((Player*)target)))
            {
                // Don't apply thorns if fire shield (conflict) is on the target
                return !ai->HasAura("fire shield", target);
            }

            return false;
        }
    };

    class ThornsTrigger : public BuffTrigger
    {
    public:
        ThornsTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "thorns", 4) {}

        bool IsActive() override
        {
            if (BuffTrigger::IsActive())
            {
                // Don't apply thorns if fire shield (conflict) is on the target
                return !ai->HasAura("fire shield", GetTarget());
            }

            return false;
        }
    };

    class OmenOfClarityTrigger : public BuffTrigger
    {
    public:
        OmenOfClarityTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "omen of clarity") {}
    };

    class RakeTrigger : public DebuffTrigger
    {
    public:
        RakeTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "rake") {}
    };

    class RipTrigger : public NoDebuffAndComboPointsAvailableTrigger
    {
    public:
        RipTrigger(PlayerbotAI* ai, uint8 comboPoints =
#ifdef MANGOSBOT_ONE
            4
#else
            3
#endif
        ) : NoDebuffAndComboPointsAvailableTrigger(ai, "rip", comboPoints) {}
    };

    class InsectSwarmTrigger : public DebuffTrigger
    {
    public:
        InsectSwarmTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "insect swarm") {}
    };

    class MoonfireTrigger : public DebuffTrigger
    {
    public:
        // Moonfire is a per-caster DoT, so only our own copy counts
        MoonfireTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "moonfire", 1, true) {}
        virtual bool IsActive() override { return DebuffTrigger::IsActive() && !GetTarget()->IsRooted(); }
    };

    class MoonfireOnAttackerTrigger : public DebuffOnAttackerTrigger
    {
    public:
        MoonfireOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "moonfire") {}
    };

    // Hurricane from 4 targets up (the generic ranged aoe triggers use 3 and 5)
    class HurricaneTrigger : public AoeTrigger
    {
    public:
        HurricaneTrigger(PlayerbotAI* ai) : AoeTrigger(ai, 4) {}
        std::string getName() override { return "hurricane"; }
    };

    BOOST_TRIGGER_A(ForceOfNatureTrigger, "force of nature");

    class FaerieFireTrigger : public DebuffTrigger
    {
    public:
        FaerieFireTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "faerie fire") {}
        // Both versions share the armor debuff and do not stack
        virtual bool IsActive() override { return DebuffTrigger::IsActive() && !ai->HasAura("faerie fire (feral)", GetTarget()); }
    };

    class FaerieFireFeralTrigger : public DebuffTrigger
    {
    public:
        FaerieFireFeralTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "faerie fire (feral)") {}
        virtual bool IsActive() override { return DebuffTrigger::IsActive() && !ai->HasAura("faerie fire", GetTarget()); }
    };

    // Keep the mangle debuff (+30% bleed/shred damage) up, no matter if a cat or a bear applied it
    class MangleCatDebuffTrigger : public SpellCanBeCastedTrigger
    {
    public:
        MangleCatDebuffTrigger(PlayerbotAI* ai) : SpellCanBeCastedTrigger(ai, "mangle (cat)") {}
        bool IsActive() override
        {
            Unit* target = GetTarget();
            return target && SpellCanBeCastedTrigger::IsActive() &&
                   !ai->HasAura("mangle (cat)", target) && !ai->HasAura("mangle (bear)", target);
        }
    };

    class DemoralizingRoarTrigger : public DebuffTrigger
    {
    public:
        DemoralizingRoarTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "demoralizing roar") {}
        virtual bool IsActive() override { return DebuffTrigger::IsActive() && !ai->HasAura("demoralizing shout", GetTarget()); }
    };

    class BashInterruptSpellTrigger : public InterruptSpellTrigger
    {
    public:
        BashInterruptSpellTrigger(PlayerbotAI* ai) : InterruptSpellTrigger(ai, "bash") {}
    };

    class TigersFuryTrigger : public BoostTrigger
    {
    public:
        TigersFuryTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "tiger's fury") {}
    };

    class NaturesGraspTrigger : public BoostTrigger
    {
    public:
        NaturesGraspTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "nature's grasp") {}
    };

    class EntanglingRootsTrigger : public HasCcTargetTrigger
    {
    public:
        EntanglingRootsTrigger(PlayerbotAI* ai) : HasCcTargetTrigger(ai, "entangling roots") {}
    };

    class EntanglingRootsKiteTrigger : public DebuffTrigger
    {
    public:
        EntanglingRootsKiteTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "entangling roots") {}
        virtual bool IsActive() override;
    };

    class EntanglingRootsSnareTrigger : public SnareTargetTrigger
    {
    public:
        EntanglingRootsSnareTrigger(PlayerbotAI* ai) : SnareTargetTrigger(ai, "entangling roots", 5) {}
    };

    class HibernateTrigger : public HasCcTargetTrigger
    {
    public:
        HibernateTrigger(PlayerbotAI* ai) : HasCcTargetTrigger(ai, "hibernate") {}
    };

    class CurePoisonTrigger : public NeedCureTrigger
    {
    public:
        CurePoisonTrigger(PlayerbotAI* ai) : NeedCureTrigger(ai, "cure poison", DISPEL_POISON) {}
    };

    class PartyMemberCurePoisonTrigger : public PartyMemberNeedCureTrigger
    {
    public:
        PartyMemberCurePoisonTrigger(PlayerbotAI* ai) : PartyMemberNeedCureTrigger(ai, "cure poison", DISPEL_POISON) {}
    };

    CURE_TRIGGER(RemoveCurseTrigger, "remove curse", DISPEL_CURSE);
    CURE_PARTY_TRIGGER(RemoveCurseOnPartyTrigger, "remove curse", DISPEL_CURSE);

    class BearFormTrigger : public BuffTrigger
    {
    public:
        BearFormTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "bear form") {}
        virtual bool IsActive() override { return !ai->HasAnyAuraOf(bot, "bear form", "dire bear form", NULL); }
    };

    class TreeFormTrigger : public BuffTrigger
    {
    public:
        TreeFormTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "tree of life") {}
        virtual bool IsActive() override { return !ai->HasAura("tree of life", bot); }
    };

    class MoonkinFormTrigger : public BuffTrigger
    {
    public:
        MoonkinFormTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "moonkin form") {}
        virtual bool IsActive() override { return !ai->HasAura("moonkin form", bot); }
    };

    class CatFormTrigger : public BuffTrigger
    {
    public:
        CatFormTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "cat form") {}
        virtual bool IsActive() override { return !ai->HasAura("cat form", bot); }
    };

    class EclipseSolarTrigger : public HasAuraTrigger
    {
    public:
        EclipseSolarTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "eclipse (solar)") {}
    };

    class EclipseLunarTrigger : public HasAuraTrigger
    {
    public:
        EclipseLunarTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "eclipse (lunar)") {}
    };

    class BashInterruptEnemyHealerSpellTrigger : public InterruptEnemyHealerTrigger
    {
    public:
        BashInterruptEnemyHealerSpellTrigger(PlayerbotAI* ai) : InterruptEnemyHealerTrigger(ai, "bash") {}
    };

    class NaturesSwiftnessTrigger : public BuffTrigger
    {
    public:
        NaturesSwiftnessTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "nature's swiftness") {}
    };

    class EnrageTrigger : public BuffTrigger
    {
    public:
        EnrageTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "enrage") {}

    private:
        bool IsActive() override
        {
            return AI_VALUE2(uint8, "health", "self target") >= sPlayerbotAIConfig.mediumHealth &&
                   AI_VALUE2(uint8, "rage", "self target") < 20;
        }
    };

    class LacerateTrigger : public DebuffTrigger
    {
    public:
        LacerateTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "lacerate") {}

    private:
        bool IsActive() override
        {
            Unit* target = GetTarget();
#ifdef MANGOSBOT_ONE
            return target && !ai->HasAura("lacerate", target, true, true, -1, false, 3000) && !HasMaxDebuffs();
#else
            return target && !ai->HasAura("lacerate", target, true) && !HasMaxDebuffs();
#endif
        }
    };

    class InStealthTrigger : public HasAuraTrigger
    {
    public:
        InStealthTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "prowl") {}
    };

    class NoStealthTrigger : public HasNoAuraTrigger
    {
    public:
        NoStealthTrigger(PlayerbotAI* ai) : HasNoAuraTrigger(ai, "prowl")
        {
            checkInterval = 2;
        }
    };

    class DruidUnstealthTrigger : public BuffTrigger
    {
    public:
        DruidUnstealthTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "prowl", 2) {}

        bool IsActive() override
        {
            if (ai->HasAura("prowl", bot) && !bot->InBattleGround())
            {
                if (!AI_VALUE(bool, "has attackers") && !AI_VALUE(bool, "has enemy player targets"))
                {
                    if (AI_VALUE2(bool, "moving", "self target"))
                    {
                        if (ai->GetMaster())
                        {
                            return sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "master target"), 10.0f);
                        }
                        else
                        {
                            return true;
                        }
                    }
                }
            }

            return false;
        }
    };

    class StealthTrigger : public Trigger
    {
    public:
        StealthTrigger(PlayerbotAI* ai) : Trigger(ai, "prowl") {}

        bool IsActive() override
        {
            if (ai->HasAura("prowl", bot) || sServerFacade.IsInCombat(bot) || !sServerFacade.IsSpellReady(bot, 5215))
            {
                return false;
            }

            Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, "enemy player target"));
            if (!target)
            {
                target = ai->GetUnit(AI_VALUE(ObjectGuid, "grind target"));
            }
            if (!target)
            {
                target = ai->GetUnit(AI_VALUE(ObjectGuid, "dps target"));
            }
            if (!target)
            {
                return false;
            }

            float distance = 30.0f;
            if (target && target->GetVictim())
            {
                distance -= 10;
            }

            if (sServerFacade.isMoving(target) && target->GetVictim())
            {
                distance -= 10;
            }

            if (bot->InBattleGround())
            {
                distance += 20;
            }

#ifndef MANGOSBOT_ZERO
            if (bot->InArena())
            {
                distance += 20;
            }
#endif

            return (target && sServerFacade.GetDistance2d(bot, target) < distance);
        }
    };

    class PowershiftTrigger : public Trigger
    {
    public:
        PowershiftTrigger(PlayerbotAI* ai) : Trigger(ai, "powershift") {}

        bool IsActive() override
        {
#ifdef MANGOSBOT_ONE
            // Shifting only pays with 5/5 Furor (40 energy on Cat Form) and never
            // while a free Omen of Clarity ability is waiting. At 21 energy or
            // less the next tick does not reach Shred's 42 energy cost.
            if (!bot->HasSpell(17061) || ai->HasAura("clearcasting", bot))
                return false;

            const uint8 maxEnergy = 21;
#else
            const uint8 maxEnergy = 19;
#endif
            if (ai->HasAura("cat form", bot) &&
                AI_VALUE2(uint8, "energy", "self target") <= maxEnergy &&
                AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig.lowMana)
            {
                return true;
            }

            return false;
        }
    };

    class FerociousBiteTrigger : public ComboPointsAvailableTrigger
    {
    public:
        FerociousBiteTrigger(PlayerbotAI* ai) : ComboPointsAvailableTrigger(ai, 5) {}
    };

    class RebirthTrigger : public SpellTargetTrigger
    {
    public:
        RebirthTrigger(PlayerbotAI* ai) : SpellTargetTrigger(ai, "rebirth", "revive targets") {}
        std::string GetTargetName() override { return "party member to resurrect"; }

        bool IsTargetValid(Unit* target) override 
        { 
            return SpellTargetTrigger::IsTargetValid(target) && target->IsDead(); 
        }
    };

    class InnervateTrigger : public SpellTargetTrigger
    {
    public:
        InnervateTrigger(PlayerbotAI* ai) : SpellTargetTrigger(ai, "innervate", "boost targets", true, true) {}
        std::string GetTargetName() override { return "self target"; }

        bool IsTargetValid(Unit* target) override
        {
            if (SpellTargetTrigger::IsTargetValid(target))
            {
                const uint32 currentMana = target->GetPower(POWER_MANA);
                if (currentMana > 0)
                {
                    const uint32 maxMana = target->GetMaxPower(POWER_MANA);
                    const uint32 currentManaPct = (uint32)(currentMana / maxMana) * 100;
                    return currentManaPct < sPlayerbotAIConfig.lowMana;
                }
            }

            return false;
        }
    };

    class LifebloomTankTrigger : public Trigger
    {
    public:
        explicit LifebloomTankTrigger(PlayerbotAI* ai) : Trigger(ai, "lifebloom", 1) {}

        Value<ObjectGuid>* GetTargetValue() override;
        bool IsActive() override
        {
            Unit* target = GetTarget();
            return target
                && ai->IsTank((Player*)target)                                          //target is tank 
                && target->IsAlive()                                                    //target is alive
                && !ai->HasAura("lifebloom", target, true, true, -1, false, 2000, 8)    //target dont have max stacked aura or aura will expire soon
                && ai->CanCastSpell("lifebloom", target, 0)                             //bot can cast spell
                && !target->getAttackers().empty();                                     //target have attackers
        }
    };

    // Keeps our own Rejuvenation / Regrowth HoT rolling on a tank that is being hit
    class TankHotTrigger : public Trigger
    {
    public:
        TankHotTrigger(PlayerbotAI* ai, std::string spell) : Trigger(ai, spell + " on tank", 1), spell(spell) {}

        Value<ObjectGuid>* GetTargetValue() override { return context->GetValue<ObjectGuid>("party tank without lifebloom", spell); }
        bool IsActive() override
        {
            Unit* target = GetTarget();
            return target
                && target->IsAlive()
                && !ai->HasAura(spell, target, false, true, -1, false, 1000)
                && ai->CanCastSpell(spell, target, 0)
                && !target->getAttackers().empty();
        }

    private:
        std::string spell;
    };

    class NaturesSwiftnessActiveTrigger : public HasAuraTrigger
    {
    public:
        NaturesSwiftnessActiveTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "nature's swiftness") {}
    };

    class ClearcastingTrigger : public HasAuraTrigger
    {
    public:
        ClearcastingTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "clearcasting") {}
    };
}
