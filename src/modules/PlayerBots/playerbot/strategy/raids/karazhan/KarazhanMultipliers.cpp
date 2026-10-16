// Ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara/KaraMultipliers) and adapted for this core.
#include "playerbot/playerbot.h"
#include "KarazhanMultipliers.h"
#include "KarazhanActions.h"
#include "KarazhanHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/actions/GenericSpellActions.h"
#include <initializer_list>

using namespace ai;
using namespace ai::karazhan;

namespace
{
    bool NameIs(Action* action, std::initializer_list<const char*> names)
    {
        const std::string& name = action->getName();
        for (const char* n : names)
        {
            if (name == n)
                return true;
        }

        return false;
    }

    bool IsAttack(Action* action)
    {
        return dynamic_cast<AttackAction*>(action) != nullptr;
    }

    // Movement incl. movement spells (Blink)
    bool IsMove(Action* action)
    {
        return raidboss::IsMovementAction(action) || NameIs(action, { "blink" });
    }

    bool IsReach(Action* action)
    {
        return NameIs(action, { "reach melee", "reach spell", "reach pull" });
    }

    // Movement of the combat formation (spreading, keeping the range) of this core
    bool IsFormationMove(Action* action)
    {
        return NameIs(action, { "flee", "move out of enemy contact" });
    }

    bool IsDamageAction(Action* action)
    {
        if (dynamic_cast<CastHealingSpellAction*>(action))
            return false;

        return IsAttack(action) || dynamic_cast<CastSpellAction*>(action) != nullptr;
    }

    bool IsLust(Action* action)
    {
        return NameIs(action, { "bloodlust", "heroism" });
    }

    // Seconds since a shared timer started, true while the dps has to wait
    bool IsWaitingForTimer(Player* bot, KaraTimer timer, int64 seconds)
    {
        const int64 elapsed = GetTimerElapsed(bot->GetInstanceId(), timer);
        return elapsed < 0 || elapsed < seconds;
    }
}

// General

float KarazhanSetTremorTotemMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!NameIs(action, { "strength of earth totem", "stoneskin totem", "stoneclaw totem", "earthbind totem" }))
        return 1.0f;

    return AreFearCastersPresent(ai) ? 0.0f : 1.0f;
}

// Attumen the Huntsman

float KarazhanAttumenDisableAutomaticTargetingMultiplier::GetValue(Action* action)
{
    if (!NameIs(action, { "tank assist", "dps assist" }))
        return 1.0f;

    return IsAttumenEncounterActive(ai) ? 0.0f : 1.0f;
}

float KarazhanAttumenStayStackedMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot) || IsAttack(action) || !IsMove(action))
        return 1.0f;

    return GetAttumenMounted(ai) ? 0.0f : 1.0f;
}

// Give the main tank 5 seconds to grab aggro when Attumen mounts Midnight
float KarazhanAttumenWaitForDpsMultiplier::GetValue(Action* action)
{
    if (!IsDamageAction(action))
        return 1.0f;

    if (!GetAttumenMounted(ai) || raidboss::IsMainTank(bot))
        return 1.0f;

    return IsWaitingForTimer(bot, KaraTimer::ATTUMEN_DPS_WAIT, 5) ? 0.0f : 1.0f;
}

// Maiden of Virtue

float KarazhanMaidenDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!IsFormationMove(action))
        return 1.0f;

    return GetKaraUnit(ai, NPC_MAIDEN_OF_VIRTUE) ? 0.0f : 1.0f;
}

float KarazhanMaidenSetGroundingTotemMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!NameIs(action, { "windfury totem", "grace of air totem", "wrath of air totem", "nature resistance totem",
                          "tranquil air totem", "windwall totem" }))
        return 1.0f;

    return GetKaraUnit(ai, NPC_MAIDEN_OF_VIRTUE) ? 0.0f : 1.0f;
}

// The Curator

float KarazhanCuratorDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (!NameIs(action, { "tank assist" }))
        return 1.0f;

    return GetKaraUnit(ai, NPC_CURATOR) ? 0.0f : 1.0f;
}

float KarazhanCuratorDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!IsFormationMove(action))
        return 1.0f;

    return GetKaraUnit(ai, NPC_CURATOR) ? 0.0f : 1.0f;
}

// Save Bloodlust / Heroism for Evocation (100% increased damage)
float KarazhanCuratorDelayBloodlustMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN || !IsLust(action))
        return 1.0f;

    Unit* curator = GetKaraUnit(ai, NPC_CURATOR);
    return curator && !curator->HasAura(SPELL_CURATOR_EVOCATION) ? 0.0f : 1.0f;
}

// Terestian Illhoof

float KarazhanIllhoofDontDotFiendishImpsMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CastRangedDebuffSpellOnAttackerAction*>(action) && !dynamic_cast<CastMeleeDebuffSpellOnAttackerAction*>(action))
        return 1.0f;

    Unit* current = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
    return current && current->GetEntry() == NPC_FIENDISH_IMP ? 0.0f : 1.0f;
}

// Shade of Aran

float KarazhanAranArcaneExplosionRunAwayMultiplier::GetValue(Action* action)
{
    if (IsAttack(action) || dynamic_cast<KarazhanAranRunAwayFromArcaneExplosionAction*>(action) || !IsMove(action))
        return 1.0f;

    return IsAranCastingArcaneExplosion(GetKaraUnit(ai, NPC_SHADE_OF_ARAN)) ? 0.0f : 1.0f;
}

// I will not move when Flame Wreath is cast or the raid blows up
float KarazhanAranFlameWreathDisableMovementMultiplier::GetValue(Action* action)
{
    if (IsAttack(action) || dynamic_cast<KarazhanAranStopMovingDuringFlameWreathAction*>(action) || !IsMove(action))
        return 1.0f;

    return IsFlameWreathActive(ai) ? 0.0f : 1.0f;
}

// Netherspite

float KarazhanNetherspiteKeepBlockingBeamMultiplier::GetValue(Action* action)
{
    const bool isReach = IsReach(action);
    const bool isBlocked = isReach || IsFormationMove(action) ||
        NameIs(action, { "follow", "avoid aoe", "move away from hazard", "move away from specific creatures", "move away from void zone" }) ||
        action->getName().rfind("tbc raid", 0) == 0 && dynamic_cast<MovementAction*>(action);
    if (!isBlocked)
        return 1.0f;

    Unit* netherspite = GetKaraUnit(ai, NPC_NETHERSPITE);
    if (!netherspite || IsBanishPhase(netherspite))
        return 1.0f;

    const BeamBlockers blockers = GetCurrentBeamBlockers(bot);
    if (bot == blockers.green || bot == blockers.blue)
        return 0.0f;

    // The red blocker tanks Netherspite and may still reach him
    if (bot == blockers.red && !isReach)
        return 0.0f;

    return 1.0f;
}

// Give tanks 5 seconds to get aggro during phase transitions
float KarazhanNetherspiteWaitForDpsMultiplier::GetValue(Action* action)
{
    if (!IsDamageAction(action) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* netherspite = GetKaraUnit(ai, NPC_NETHERSPITE);
    if (!netherspite || IsBanishPhase(netherspite))
        return 1.0f;

    return IsWaitingForTimer(bot, KaraTimer::NETHERSPITE_DPS_WAIT, 5) ? 0.0f : 1.0f;
}

// Prince Malchezaar

float KarazhanMalchezaarEnfeebleMultiplier::GetValue(Action* action)
{
    if (IsAttack(action) || dynamic_cast<KarazhanMalchezaarEnfeebledAvoidHazardAction*>(action))
        return 1.0f;

    if (!bot->HasAura(SPELL_ENFEEBLE))
        return 1.0f;

    // Disable movement other than escaping Shadow Nova range
    if (IsMove(action))
        return 0.0f;

    // Disable cooldowns and potions used on low health: Enfeeble restores the health when it expires
    if (NameIs(action, { "ice block", "mana shield", "divine shield", "lay on hands", "cloak of shadows", "evasion",
                         "shamanistic rage", "binding heal", "desperate prayer", "pain suppression", "last stand",
                         "shield wall", "deterrence", "frenzied regeneration", "healthstone", "healing potion",
                         "use healing potion", "use healthstone" }))
        return 0.0f;

    return 1.0f;
}

// Wait until phase 3 to use Bloodlust / Heroism
float KarazhanMalchezaarDelayBloodlustMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN || !IsLust(action))
        return 1.0f;

    Unit* malchezaar = GetKaraUnit(ai, NPC_PRINCE_MALCHEZAAR);
    return malchezaar && malchezaar->GetHealthPercent() > 30.0f ? 0.0f : 1.0f;
}

// Nightbane

// Temporary pets run out of bounds and cause skeletons to spawn off the map
// (hunter and warlock pets are handled by the control pet aggression action)
float KarazhanNightbaneDisablePetsMultiplier::GetValue(Action* action)
{
    if (!NameIs(action, { "summon water elemental", "shadowfiend", "fire elemental totem", "earth elemental totem", "force of nature" }))
        return 1.0f;

    return GetKaraUnit(ai, NPC_NIGHTBANE) ? 0.0f : 1.0f;
}

// Give the main tank 8 seconds to get aggro during phase transitions
float KarazhanNightbaneWaitForDpsMultiplier::GetValue(Action* action)
{
    if (!IsDamageAction(action))
        return 1.0f;

    Unit* nightbane = GetKaraUnit(ai, NPC_NIGHTBANE);
    if (!nightbane || IsNightbaneFlying(nightbane) || raidboss::IsMainTank(bot))
        return 1.0f;

    // The flight phase action switches to the skeletons
    if (dynamic_cast<KarazhanNightbaneFlightPhaseStackAction*>(action))
        return 1.0f;

    return IsWaitingForTimer(bot, KaraTimer::NIGHTBANE_DPS_WAIT, 8) ? 0.0f : 1.0f;
}

float KarazhanNightbaneDisableAvoidAoeMultiplier::GetValue(Action* action)
{
    if (!NameIs(action, { "flee", "avoid aoe", "move away from hazard" }))
        return 1.0f;

    Unit* nightbane = GetKaraUnit(ai, NPC_NIGHTBANE);
    if (!nightbane || IsNightbaneFlying(nightbane))
        return 1.0f;

    return raidboss::IsRangedPlayer(bot) || raidboss::IsMainTank(bot) ? 0.0f : 1.0f;
}

float KarazhanNightbaneDisableMovementMultiplier::GetValue(Action* action)
{
    const bool isFollow = NameIs(action, { "follow" });
    const bool isReach = IsReach(action);
    const bool isBlocked = IsFormationMove(action) || NameIs(action, { "blink" });
    const bool isAssist = NameIs(action, { "dps assist", "tank assist", "dps aoe", "attack anything", "attack least hp target" });
    if (!isFollow && !isReach && !isBlocked && !isAssist)
        return 1.0f;

    Unit* nightbane = GetKaraUnit(ai, NPC_NIGHTBANE);
    if (!nightbane)
        return 1.0f;

    if (isBlocked)
        return 0.0f;

    if (!IsNightbaneFlying(nightbane))
        return 1.0f;

    // Nightbane is in the air: don't pick or chase him (replaces the target exclusion of mod-playerbots)
    if (isReach || isAssist)
        return 0.0f;

    // After 35s Nightbane goes to land, and bots freely follow their master
    return IsNightbaneFlightPhaseActive(nightbane) ? 0.0f : 1.0f;
}
