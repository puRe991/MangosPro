// Ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara/KaraStrategy) and adapted for this core.
#include "playerbot/playerbot.h"
#include "KarazhanStrategy.h"
#include "KarazhanMultipliers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;

namespace
{
    void AddTrigger(std::list<TriggerNode*>& triggers, const char* trigger, const char* action, float relevance)
    {
        triggers.push_back(new TriggerNode(trigger, NextAction::array(0, new NextAction(action, relevance), NULL)));
    }
}

void KarazhanBossStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    const float RAID = raidboss::ACTION_RAID;

    // General
    AddTrigger(triggers, "karazhan enemies cast fear", "karazhan set tremor totem", RAID);

    // Trash
    AddTrigger(triggers, "mana warp is about to explode", "mana warp stun creature before warp breach", ACTION_EMERGENCY + 6);

    // Attumen the Huntsman
    AddTrigger(triggers, "attumen the huntsman phase one active", "attumen the huntsman handle phase one", RAID);
    AddTrigger(triggers, "attumen the huntsman phase two active", "attumen the huntsman handle phase two", RAID);
    AddTrigger(triggers, "attumen the huntsman phase transition", "attumen the huntsman set dps timer", ACTION_EMERGENCY + 9);

    // Moroes
    AddTrigger(triggers, "moroes should prioritize adds", "moroes mark target", RAID);

    // Maiden of Virtue
    AddTrigger(triggers, "maiden of virtue boss engaged by tanks", "maiden of virtue tank position boss", RAID);
    AddTrigger(triggers, "maiden of virtue holy wrath deals chain damage", "maiden of virtue position ranged between pillars", RAID + 1);
    AddTrigger(triggers, "maiden of virtue grounding totem consumes holy fire", "maiden of virtue set grounding totem", RAID);

    // The Big Bad Wolf
    AddTrigger(triggers, "big bad wolf boss is chasing little red riding hood", "big bad wolf little red riding hood run away", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "big bad wolf boss engaged by tank", "big bad wolf position boss", RAID);

    // Romulo and Julianne
    AddTrigger(triggers, "romulo and julianne both bosses revived", "romulo and julianne mark target", RAID);

    // The Wizard of Oz
    AddTrigger(triggers, "wizard of oz need target priority", "wizard of oz mark target", RAID);
    AddTrigger(triggers, "wizard of oz strawman is vulnerable to fire", "wizard of oz scorch strawman", RAID + 1);

    // The Curator
    AddTrigger(triggers, "the curator astral flare spawned", "the curator mark astral flare", RAID + 1);
    AddTrigger(triggers, "the curator boss engaged by tanks", "the curator position boss", RAID);
    AddTrigger(triggers, "the curator boss engaged by ranged", "the curator spread ranged", RAID);

    // Terestian Illhoof
    AddTrigger(triggers, "terestian illhoof should prioritize chains", "terestian illhoof mark target", RAID);

    // Shade of Aran
    AddTrigger(triggers, "shade of aran arcane explosion is casting", "shade of aran run away from arcane explosion", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "shade of aran flame wreath is active", "shade of aran stop moving during flame wreath", ACTION_EMERGENCY + 7);
    AddTrigger(triggers, "shade of aran conjured elementals summoned", "shade of aran mark conjured elemental", RAID);
    AddTrigger(triggers, "shade of aran boss casts counterspell nearby", "shade of aran ranged maintain distance", RAID + 1);

    // Netherspite
    AddTrigger(triggers, "netherspite red beam is active", "netherspite block red beam", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "netherspite blue beam is active", "netherspite block blue beam", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "netherspite green beam is active", "netherspite block green beam", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "netherspite bot is not beam blocker", "netherspite avoid beam and void zone", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "netherspite boss is banished", "netherspite banish phase avoid void zone", ACTION_EMERGENCY + 1);
    AddTrigger(triggers, "netherspite should manage timers and trackers", "netherspite manage timers and trackers", ACTION_EMERGENCY + 9);

    // Prince Malchezaar
    AddTrigger(triggers, "prince malchezaar bot is enfeebled", "prince malchezaar enfeebled bot avoid hazard", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "prince malchezaar engaged by non-tanks", "prince malchezaar non-tank avoid infernal", ACTION_EMERGENCY + 1);
    AddTrigger(triggers, "prince malchezaar boss engaged by tanks", "prince malchezaar tanks position boss", ACTION_EMERGENCY + 6);

    // Nightbane
    AddTrigger(triggers, "nightbane boss engaged by tanks", "nightbane ground phase tanks position boss", RAID);
    AddTrigger(triggers, "nightbane ground phase engaged by ranged", "nightbane ground phase coordinate ranged movement", ACTION_EMERGENCY + 1);
    AddTrigger(triggers, "nightbane pets ignore collision to chase flying boss", "nightbane control pet aggression", RAID + 1);
    AddTrigger(triggers, "nightbane boss is flying", "nightbane flight phase stack and move", RAID);
    AddTrigger(triggers, "nightbane bot went out of bounds", "nightbane teleport back to terrace", ACTION_EMERGENCY + 9);
    AddTrigger(triggers, "nightbane should manage timers and trackers", "nightbane manage timers and trackers", ACTION_EMERGENCY + 9);
}

void KarazhanBossStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AddTrigger(triggers, "karazhan bot is not in combat", "karazhan reset encounter states", ACTION_EMERGENCY + 9);
}

void KarazhanBossStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    // Reactions are checked while casting: stop the cast to get out
    AddTrigger(triggers, "shade of aran arcane explosion is casting", "shade of aran run away from arcane explosion", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "shade of aran flame wreath is active", "shade of aran stop moving during flame wreath", ACTION_EMERGENCY + 7);
    AddTrigger(triggers, "big bad wolf boss is chasing little red riding hood", "big bad wolf little red riding hood run away", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "prince malchezaar bot is enfeebled", "prince malchezaar enfeebled bot avoid hazard", ACTION_EMERGENCY + 6);
}

void KarazhanBossStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new KarazhanSetTremorTotemMultiplier(ai));
    multipliers.push_back(new KarazhanAttumenDisableAutomaticTargetingMultiplier(ai));
    multipliers.push_back(new KarazhanAttumenStayStackedMultiplier(ai));
    multipliers.push_back(new KarazhanAttumenWaitForDpsMultiplier(ai));
    multipliers.push_back(new KarazhanMaidenDisableCombatFormationMoveMultiplier(ai));
    multipliers.push_back(new KarazhanMaidenSetGroundingTotemMultiplier(ai));
    multipliers.push_back(new KarazhanCuratorDisableTankAssistMultiplier(ai));
    multipliers.push_back(new KarazhanCuratorDisableCombatFormationMoveMultiplier(ai));
    multipliers.push_back(new KarazhanCuratorDelayBloodlustMultiplier(ai));
    multipliers.push_back(new KarazhanIllhoofDontDotFiendishImpsMultiplier(ai));
    multipliers.push_back(new KarazhanAranArcaneExplosionRunAwayMultiplier(ai));
    multipliers.push_back(new KarazhanAranFlameWreathDisableMovementMultiplier(ai));
    multipliers.push_back(new KarazhanNetherspiteKeepBlockingBeamMultiplier(ai));
    multipliers.push_back(new KarazhanNetherspiteWaitForDpsMultiplier(ai));
    multipliers.push_back(new KarazhanMalchezaarEnfeebleMultiplier(ai));
    multipliers.push_back(new KarazhanMalchezaarDelayBloodlustMultiplier(ai));
    multipliers.push_back(new KarazhanNightbaneDisablePetsMultiplier(ai));
    multipliers.push_back(new KarazhanNightbaneWaitForDpsMultiplier(ai));
    multipliers.push_back(new KarazhanNightbaneDisableAvoidAoeMultiplier(ai));
    multipliers.push_back(new KarazhanNightbaneDisableMovementMultiplier(ai));
}

void KarazhanBossStrategy::InitReactionMultipliers(std::list<Multiplier*>& multipliers)
{
    // Movement reactions (avoid aoe, flee) must respect the same rules
    multipliers.push_back(new KarazhanAttumenStayStackedMultiplier(ai));
    multipliers.push_back(new KarazhanMaidenDisableCombatFormationMoveMultiplier(ai));
    multipliers.push_back(new KarazhanCuratorDisableCombatFormationMoveMultiplier(ai));
    multipliers.push_back(new KarazhanAranArcaneExplosionRunAwayMultiplier(ai));
    multipliers.push_back(new KarazhanAranFlameWreathDisableMovementMultiplier(ai));
    multipliers.push_back(new KarazhanNetherspiteKeepBlockingBeamMultiplier(ai));
    multipliers.push_back(new KarazhanMalchezaarEnfeebleMultiplier(ai));
    multipliers.push_back(new KarazhanNightbaneDisableAvoidAoeMultiplier(ai));
    multipliers.push_back(new KarazhanNightbaneDisableMovementMultiplier(ai));
}
