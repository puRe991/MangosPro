// Ported from mod-playerbots (AzerothCore, GPL v2+), src/Ai/Raid/ZA/ZAMultipliers.cpp, and adapted for this core.
#include "playerbot/playerbot.h"
#include "ZaMultipliers.h"
#include "ZaActions.h"
#include "ZaHelpers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/actions/ChooseTargetActions.h"
#include "playerbot/strategy/actions/GenericSpellActions.h"

using namespace ai;
using namespace ai::za;

namespace
{
    bool NameIs(Action* action, std::initializer_list<const char*> names)
    {
        if (!action)
            return false;

        const std::string name = action->getName();
        for (const char* n : names)
        {
            if (name == n)
                return true;
        }

        return false;
    }

    // Only engaged bosses: the attackers list is cheap, a grid search for every action is not
    Unit* EngagedBoss(PlayerbotAI* ai, uint32 entry)
    {
        for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("attackers")->Get())
        {
            Unit* unit = ai->GetUnit(guid);
            if (unit && unit->IsAlive() && !unit->IsPlayer() && unit->GetEntry() == entry)
                return unit;
        }

        return nullptr;
    }

    bool IsNonCombat(PlayerbotAI* ai)
    {
        return ai->GetState() == BotState::BOT_STATE_NON_COMBAT;
    }
}

float ZaEncounterMultiplier::GetValue(Action* action)
{
    if (!action || bot->GetMapId() != ZA_MAP_ID || !IsEncounterInProgress(bot))
        return 1.0f;

    return GetValueInEncounter(action);
}

// General

float ZaDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (IsNonCombat(ai) || !raidboss::IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* boss = nullptr;
    for (uint32 entry : { NPC_ZULJIN, NPC_HEX_LORD_MALACRASS, NPC_JANALAI, NPC_NALORAKK, NPC_AKILZON })
    {
        if ((boss = EngagedBoss(ai, entry)))
            break;
    }

    if (!boss)
        return 1.0f;

    if (boss->GetHealthPercent() > raidboss::BOSS_ENGAGED_HEALTH_PCT)
        return 0.0f;

    // Further restrictions on Bloodlust for Zul'jin and Jan'alai
    if (bot->getClass() != CLASS_SHAMAN || !NameIs(action, { "bloodlust", "heroism" }))
        return 1.0f;

    // Zul'jin: hold until Phase 3 (or later)
    if (boss->GetEntry() == NPC_ZULJIN)
    {
        return boss->HasAura(SPELL_SHAPE_OF_THE_EAGLE) || boss->HasAura(SPELL_SHAPE_OF_THE_LYNX) ||
               boss->HasAura(SPELL_SHAPE_OF_THE_DRAGONHAWK) ? 1.0f : 0.0f;
    }

    // Jan'alai: hold until it is time to burn the Hatchlings
    if (boss->GetEntry() == NPC_JANALAI)
    {
        if (boss->GetHealthPercent() <= JANALAI_HATCH_ALL_HEALTH_PCT)
            return 1.0f;

        return CountJanalaiHatchlings(ai) >= JANALAI_BLOODLUST_HATCHLING_COUNT ? 1.0f : 0.0f;
    }

    return 1.0f;
}

// Malacrass siphoning a Warrior soul and Zul'jin have very similar Whirlwinds
float ZaAvoidWhirlwindMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsApproachMovement(action) && !NameIs(action, { "killing spree" }))
        return 1.0f;

    Unit* boss = EngagedBoss(ai, NPC_ZULJIN);
    uint32 whirlwind = SPELL_ZULJIN_WHIRLWIND;
    if (!boss)
    {
        boss = EngagedBoss(ai, NPC_HEX_LORD_MALACRASS);
        whirlwind = SPELL_HEX_LORD_WHIRLWIND;
    }

    if (!boss || boss->GetVictim() == bot || !boss->HasAura(whirlwind))
        return 1.0f;

    return raidboss::ExactDist2d(bot, boss) <= ZA_WHIRLWIND_HOLD_DISTANCE ? 0.0f : 1.0f;
}

// This core has no "tank face" action, so only tank assist and taunts are controlled here
float ZaDisableTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (IsNonCombat(ai) || !PlayerbotAI::IsTank(bot))
        return 1.0f;

    const bool isTankAssist = dynamic_cast<TankAssistAction*>(action) != nullptr;
    const bool isTaunt = raidboss::IsTauntAction(bot, action);
    if (!isTankAssist && !isTaunt)
        return 1.0f;

    // Nalorakk: tank swap by form, only the tank assigned to the current form may taunt. Same as
    // ZaNalorakkTanksPositionBossAction: main tank gets bear, assist tank gets troll.
    if (Unit* nalorakk = EngagedBoss(ai, NPC_NALORAKK))
    {
        if (!isTaunt)
            return 0.0f;

        const bool isFormTank = IsNalorakkInBearForm(nalorakk) ? raidboss::IsMainTank(bot) : raidboss::IsAssistTank(bot, 0);
        return isFormTank ? 1.0f : 0.0f;
    }

    // Halazzi: the assist tank picks up the lynx, so the main tank must not taunt it
    if (EngagedBoss(ai, NPC_HALAZZI))
    {
        if (!isTaunt)
            return 0.0f;

        if (!raidboss::IsMainTank(bot))
            return 1.0f;

        Unit* target = action->GetTarget();
        return target && target->GetEntry() == NPC_SPIRIT_OF_THE_LYNX ? 0.0f : 1.0f;
    }

    if (isTaunt)
        return 1.0f;

    // Jan'alai: tank assist only for the assist tanks to pick up the Hatchlings
    if (EngagedBoss(ai, NPC_JANALAI))
        return raidboss::IsMainTank(bot) ? 0.0f : 1.0f;

    return 1.0f;
}

// Nalorakk's troll form and Halazzi's lynx add are handled by the assist tank
float ZaControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (IsNonCombat(ai) || bot->getClass() != CLASS_HUNTER || !NameIs(action, { "misdirection on party tank" }))
        return 1.0f;

    return EngagedBoss(ai, NPC_NALORAKK) || EngagedBoss(ai, NPC_HALAZZI) ? 0.0f : 1.0f;
}

// mod-playerbots: CombatFormationMoveAction ("disperse"), here the formation moves that keep
// distance to the other players or the enemies
float ZaDisableCombatFormationMoveMultiplier::GetValueInEncounter(Action* action)
{
    if (IsNonCombat(ai) || !NameIs(action, { "move out of enemy contact", "tbc raid spread" }))
        return 1.0f;

    Unit* zuljin = EngagedBoss(ai, NPC_ZULJIN);
    if (zuljin && zuljin->HasAura(SPELL_SHAPE_OF_THE_EAGLE))
        return 0.0f;

    return EngagedBoss(ai, NPC_JANALAI) || EngagedBoss(ai, NPC_AKILZON) ? 0.0f : 1.0f;
}

// Not in mod-playerbots: the "tbc raid" table rules for these bosses that contradict this port
float ZaTbcRaidOverlapMultiplier::GetValueInEncounter(Action* action)
{
    if (IsNonCombat(ai) || !NameIs(action, { "tbc raid spread", "tbc raid gather", "tbc raid taunt",
                                              "tbc raid attack priority target", "tbc raid leave front" }))
    {
        return 1.0f;
    }

    const std::string name = action->getName();

    // Own spread / stack / tank swap logic
    if (EngagedBoss(ai, NPC_AKILZON) || EngagedBoss(ai, NPC_NALORAKK) || EngagedBoss(ai, NPC_ZULJIN))
        return name == "tbc raid attack priority target" ? 1.0f : 0.0f;

    // Ranged circle around the tank position replaces leaving the Flame Breath front
    if (EngagedBoss(ai, NPC_JANALAI))
        return name == "tbc raid attack priority target" ? 1.0f : 0.0f;

    // Own kill order (Lord Raadan first)
    if (EngagedBoss(ai, NPC_HEX_LORD_MALACRASS))
        return name == "tbc raid attack priority target" ? 0.0f : 1.0f;

    return 1.0f;
}

// Akil'zon <Eagle Avatar>

float ZaAkilzonStayInEyeOfTheStormMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsHazardousMovement(action) || dynamic_cast<ZaAkilzonMoveToEyeOfTheStormAction*>(action))
        return 1.0f;

    if (!EngagedBoss(ai, NPC_AKILZON))
        return 1.0f;

    return IsInStormWindow(ai) ? 0.0f : 1.0f;
}

// Jan'alai <Dragonhawk Avatar>

float ZaJanalaiStayAwayFromFireBombsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsHazardousMovement(action) || dynamic_cast<ZaJanalaiAvoidFireBombsAction*>(action))
        return 1.0f;

    return IsJanalaiBombing(EngagedBoss(ai, NPC_JANALAI)) ? 0.0f : 1.0f;
}

float ZaJanalaiDoNotCrowdControlHatchersMultiplier::GetValueInEncounter(Action* action)
{
    if (IsNonCombat(ai) || !dynamic_cast<CastCrowdControlSpellAction*>(action))
        return 1.0f;

    Unit* target = action->GetTarget();
    return target && (target->GetEntry() == NPC_AMANISHI_HATCHER_1 || target->GetEntry() == NPC_AMANISHI_HATCHER_2) ? 0.0f : 1.0f;
}

// Halazzi <Lynx Avatar>

float ZaHalazziDisableAutoDpsTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (IsNonCombat(ai) || !raidboss::IsDps(bot))
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<CastRangedDebuffSpellOnAttackerAction*>(action) &&
        !dynamic_cast<CastMeleeDebuffSpellOnAttackerAction*>(action))
    {
        return 1.0f;
    }

    return EngagedBoss(ai, NPC_HALAZZI) ? 0.0f : 1.0f;
}

// Hex Lord Malacrass

// Dispelling Unstable Affliction silences the dispeller (it is a magic effect, not a curse)
float ZaHexLordMalacrassUnstableAfflictionMultiplier::GetValueInEncounter(Action* action)
{
    if (IsNonCombat(ai))
        return 1.0f;

    if (bot->getClass() == CLASS_PRIEST)
    {
        if (!NameIs(action, { "dispel magic on party", "dispel magic", "mass dispel" }))
            return 1.0f;
    }
    else if (bot->getClass() == CLASS_PALADIN)
    {
        if (!NameIs(action, { "cleanse magic on party", "cleanse magic" }))
            return 1.0f;
    }
    else
    {
        return 1.0f;
    }

    if (!EngagedBoss(ai, NPC_HEX_LORD_MALACRASS))
        return 1.0f;

    Unit* target = ai->GetUnit(AI_VALUE2(ObjectGuid, "party member to dispel", DISPEL_MAGIC));
    if (HasUnstableAffliction(target))
        return 0.0f;

    // Self dispels
    return NameIs(action, { "dispel magic", "cleanse magic" }) && HasUnstableAffliction(bot) ? 0.0f : 1.0f;
}

float ZaHexLordMalacrassSpellReflectionMultiplier::GetValueInEncounter(Action* action)
{
    if (!raidboss::IsRangedPlayer(bot) || bot->getClass() == CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action) || dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* malacrass = EngagedBoss(ai, NPC_HEX_LORD_MALACRASS);
    if (!malacrass || !malacrass->HasAura(SPELL_HEX_LORD_SPELL_REFLECTION))
        return 1.0f;

    // Spells on the adds are fine
    return action->GetTarget() == malacrass ? 0.0f : 1.0f;
}

float ZaHexLordMalacrassStayAwayFromFreezingTrapMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsApproachMovement(action))
        return 1.0f;

    return EngagedBoss(ai, NPC_HEX_LORD_MALACRASS) && GetNearbyFreezingTrap(ai) ? 0.0f : 1.0f;
}

// Zul'jin

float ZaZuljinStopAttackingDuringPhaseChangeMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    // Above 80% is Phase 1
    Unit* zuljin = EngagedBoss(ai, NPC_ZULJIN);
    if (!zuljin || zuljin->GetHealthPercent() > 80.0f)
        return 1.0f;

    // After 80%, Zul'jin is in troll form only during the transformation sequences
    return IsZuljinInAnimalForm(zuljin) ? 1.0f : 0.0f;
}

// The Feather Vortexes chase players and can't be dodged: avoiding them only scatters the raid
float ZaZuljinEagleDisableAvoidAoeMultiplier::GetValueInEncounter(Action* action)
{
    if (IsNonCombat(ai) || !NameIs(action, { "flee", "tbc raid leave hazard" }))
        return 1.0f;

    Unit* zuljin = EngagedBoss(ai, NPC_ZULJIN);
    return zuljin && zuljin->HasAura(SPELL_SHAPE_OF_THE_EAGLE) ? 0.0f : 1.0f;
}
