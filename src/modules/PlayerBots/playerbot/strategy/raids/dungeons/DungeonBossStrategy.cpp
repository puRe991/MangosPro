#include "playerbot/playerbot.h"
#include "DungeonBossStrategy.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"
#include "playerbot/strategy/actions/GenericSpellActions.h"
#include "Entities/Pet.h"
#include "Spells/Spell.h"
#include "Spells/SpellAuras.h"
#include "playerbot/strategy/generic/MoltenCoreDungeonStrategies.h"
#include "playerbot/strategy/generic/BlackwingLairDungeonStrategies.h"
#include "playerbot/strategy/generic/NaxxramasDungeonStrategies.h"

using namespace ai;
using namespace ai::tbcraid;

const std::vector<std::pair<uint32, std::string>>& ai::GetDungeonBossMaps()
{
    static const std::vector<std::pair<uint32, std::string>> maps = {
        { 389, "boss ragefire chasm" }, { 43, "boss wailing caverns" },
        { 36, "boss deadmines" }, { 33, "boss shadowfang keep" },
        { 34, "boss stockades" }, { 48, "boss blackfathom deeps" },
        { 90, "boss gnomeregan" }, { 47, "boss razorfen kraul" },
        { 189, "boss scarlet monastery" }, { 129, "boss razorfen downs" },
        { 70, "boss uldaman" }, { 209, "boss zulfarrak" },
        { 349, "boss maraudon" }, { 109, "boss sunken temple" },
        { 230, "boss blackrock depths" }, { 229, "boss blackrock spire" },
        { 429, "boss dire maul" }, { 289, "boss scholomance" },
        { 329, "boss stratholme" },
        { 543, "boss ramparts" }, { 542, "boss blood furnace" },
        { 540, "boss shattered halls" }, { 547, "boss slave pens" },
        { 546, "boss underbog" }, { 545, "boss steamvault" },
        { 557, "boss mana tombs" }, { 558, "boss auchenai crypts" },
        { 556, "boss sethekk halls" }, { 555, "boss shadow labyrinth" },
        { 560, "boss old hillsbrad" }, { 269, "boss black morass" },
        { 554, "boss mechanar" }, { 553, "boss botanica" },
        { 552, "boss arcatraz" }, { 585, "boss magisters terrace" }
    };
    return maps;
}

bool ai::IsDungeonBossMap(uint32 mapId)
{
    for (const auto& entry : GetDungeonBossMaps())
        if (entry.first == mapId)
            return true;
    return false;
}

const std::vector<std::pair<uint32, std::string>>& ai::GetClassicRaidBossMaps()
{
    static const std::vector<std::pair<uint32, std::string>> maps = {
        { 409, "boss molten core" }, { 469, "boss blackwing lair" },
        { 249, "boss onyxia" }, { 509, "boss aq20" }, { 533, "boss naxxramas" }
    };
    return maps;
}

bool ai::IsClassicRaidBossMap(uint32 mapId)
{
    for (const auto& entry : GetClassicRaidBossMaps())
        if (entry.first == mapId)
            return true;
    return false;
}

void ClassicRaidBossStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DungeonBossStrategy::InitCombatTriggers(triggers);
    // Reuse the existing Magmadar, suppression-room and Four Horsemen logic.
    if (getName() == "boss molten core")
        MoltenCoreDungeonStrategy(ai).InitTriggers(triggers, BotState::BOT_STATE_COMBAT);
    else if (getName() == "boss blackwing lair")
        BlackwingLairDungeonStrategy(ai).InitTriggers(triggers, BotState::BOT_STATE_COMBAT);
    else if (getName() == "boss naxxramas")
        NaxxramasDungeonStrategy(ai).InitTriggers(triggers, BotState::BOT_STATE_COMBAT);
}

void ClassicRaidBossStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    DungeonBossStrategy::InitNonCombatTriggers(triggers);
    if (getName() == "boss molten core")
        MoltenCoreDungeonStrategy(ai).InitTriggers(triggers, BotState::BOT_STATE_NON_COMBAT);
    else if (getName() == "boss blackwing lair")
        BlackwingLairDungeonStrategy(ai).InitTriggers(triggers, BotState::BOT_STATE_NON_COMBAT);
}

namespace
{
    void AddTrigger(std::list<TriggerNode*>& triggers, const char* trigger, const char* action, float relevance)
    {
        triggers.push_back(new TriggerNode(trigger, NextAction::array(0, new NextAction(action, relevance), nullptr)));
    }

    // Reflective/immune phases: stop damage, but allow healing and defensive spells.
    bool HoldDamage(PlayerbotAI* ai, Unit* boss)
    {
        if (!boss)
            return false;
        switch (boss->GetEntry())
        {
        case 18341: return boss->HasAura(32358) || boss->HasAura(38759); // Pandemonius
        case 19219: return boss->HasAura(35158) || boss->HasAura(35159); // Capacitus
        case 17975: return boss->HasAura(34551);                       // Freywinn: Tree Form
        case 23035: return boss->HasAura(42354);                       // Anzu: Banish
        case 6487:  return boss->HasAura(9438);                        // Doan: Arcane Bubble
        case 17798: return boss->HasAura(31534);                       // Kalithresh: Spell Reflection
        case 15370: return boss->GetHealthPercent() > 20.0f;           // Buru: break the carapace with eggs
        default: return false;
        }
    }

    bool NeedsCover(Unit* boss)
    {
        if (!boss)
            return false;
        if (boss->GetEntry() == 18473)
            return TbcRaidHelper::IsCreatureCasting(boss, 38197) || TbcRaidHelper::IsCreatureCasting(boss, 40425);
        if (boss->GetEntry() == 14020)
            for (uint32 spell : { 23308u, 23309u, 23310u, 23312u, 23313u })
                if (TbcRaidHelper::IsCreatureCasting(boss, spell))
                    return true;
        return false;
    }

    bool HostileAction(Action* action, Player* bot)
    {
        if (dynamic_cast<AttackAction*>(action))
            return true;
        auto* spell = dynamic_cast<CastSpellAction*>(action);
        if (!spell || dynamic_cast<CastHealingSpellAction*>(action))
            return false;
        Unit* target = action->GetTarget();
        return (target && target->IsEnemy(bot)) ||
            action->getThreatType() == ActionThreatType::ACTION_THREAT_AOE;
    }

    bool CastingAtBoss(Player* bot, Unit* boss)
    {
        if (!boss)
            return false;
        for (CurrentSpellTypes slot : { CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL })
            if (Spell* spell = bot->GetCurrentSpell(slot))
                if (spell->m_targets.getUnitTarget() == boss)
                    return true;
        return false;
    }

    Unit* FindBuruEgg(PlayerbotAI* ai, Unit* boss, float maxBossDistance)
    {
        Unit* best = nullptr;
        for (Unit* egg : raidboss::FindUnits(ai->GetBot(), 15514, 100.0f))
            if (egg->IsAlive() && boss->GetDistance(egg) <= maxBossDistance &&
                (!best || boss->GetDistance(egg) < boss->GetDistance(best)))
                best = egg;
        return best;
    }

    bool OssirianNeedsCrystal(Unit* boss)
    {
        if (!boss || boss->GetEntry() != 15339)
            return false;
        for (uint32 spell : { 25177u, 25178u, 25180u, 25181u, 25183u })
            if (SpellAuraHolder* aura = boss->GetSpellAuraHolder(spell))
                if (aura->GetAuraDuration() > 10000)
                    return false;
        return true;
    }

    GameObject* FindOssirianCrystal(PlayerbotAI* ai, Unit* boss)
    {
        if (!OssirianNeedsCrystal(boss))
            return nullptr;
        GameObject* best = nullptr;
        for (GameObject* crystal : raidboss::FindGameObjects(ai->GetBot(), 180619, 200.0f))
            if (!best || boss->GetDistance(crystal) < boss->GetDistance(best))
                best = crystal;
        return best;
    }

    bool IsCrystalHandler(Player* bot)
    {
        Player* handler = nullptr;
        for (Player* member : raidboss::GetGroupMembers(bot, 200.0f))
            if (member->GetPlayerbotAI() && !PlayerbotAI::IsTank(member) &&
                (!handler || member->GetObjectGuid() < handler->GetObjectGuid()))
                handler = member;
        return handler ? handler == bot : raidboss::IsMechanicTrackerBot(bot, 509);
    }

    // Positions imposed by an encounter must survive ordinary follow/reach actions.
    bool ClassicPosition(PlayerbotAI* ai, Unit* boss, WorldPosition& point)
    {
        Player* bot = ai->GetBot();
        if (!boss || !boss->IsAlive())
            return false;
        if (boss->GetEntry() == 15370 && boss->GetHealthPercent() > 20.0f && boss->GetVictim() == bot)
        {
            if (Unit* egg = FindBuruEgg(ai, boss, 100.0f))
            {
                const float angle = boss->GetAngle(egg);
                point = WorldPosition(bot->GetMapId(), egg->GetPositionX() + cos(angle) * 10.0f,
                    egg->GetPositionY() + sin(angle) * 10.0f, egg->GetPositionZ());
                return true;
            }
        }
        if (boss->GetEntry() == 15989 && !bot->HasAura(28522) &&
            (boss->HasAura(18430) || TbcRaidHelper::IsCreatureCasting(boss, 28524)))
        {
            float nearest = 10000.0f;
            for (Player* member : raidboss::GetGroupMembers(bot, 100.0f))
            {
                if (!member->HasAura(28522))
                    continue;
                const float angle = boss->GetAngle(member);
                WorldPosition candidate(bot->GetMapId(), member->GetPositionX() + cos(angle) * 5.0f,
                    member->GetPositionY() + sin(angle) * 5.0f, member->GetPositionZ());
                const float distance = WorldPosition(bot).sqDistance2d(candidate);
                if (distance < nearest)
                {
                    nearest = distance;
                    point = candidate;
                }
            }
            return nearest < 10000.0f;
        }
        if (GameObject* crystal = FindOssirianCrystal(ai, boss))
        {
            if ((boss->GetVictim() == bot && boss->GetDistance(crystal) > 10.0f) ||
                (IsCrystalHandler(bot) && boss->GetDistance(crystal) <= 20.0f && bot->GetDistance(crystal) > INTERACTION_DISTANCE))
            {
                point = WorldPosition(crystal);
                return true;
            }
        }
        return false;
    }
}

Unit* ai::GetClassicRaidPriorityTarget(PlayerbotAI* ai)
{
    Unit* boss = nullptr;
    TbcRaidHelper::GetEncounter(ai, &boss);
    if (boss && boss->GetEntry() == 15370 && boss->GetHealthPercent() > 20.0f &&
        boss->GetVictim() != ai->GetBot() && raidboss::IsDps(ai->GetBot()))
        return FindBuruEgg(ai, boss, 6.0f);
    if (boss && boss->GetEntry() == 15932 && raidboss::IsDps(ai->GetBot()))
    {
        Unit* closest = nullptr;
        for (Unit* zombie : raidboss::FindUnits(ai->GetBot(), 16360, 70.0f))
            if (zombie->IsAlive() && zombie->GetHealthPercent() <= 10.0f &&
                (!closest || boss->GetDistance(zombie) < boss->GetDistance(closest)))
                closest = zombie;
        return closest;
    }
    return nullptr;
}

void DungeonBossStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AddTrigger(triggers, "tbc raid in hazard", "tbc raid leave hazard", ACTION_EMERGENCY + 8);
    AddTrigger(triggers, "tbc raid debuff run", "tbc raid spread", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "tbc raid taunt", "tbc raid taunt", ACTION_EMERGENCY + 2);
    AddTrigger(triggers, "dungeon boss special", "dungeon boss special", ACTION_EMERGENCY + 5);
    AddTrigger(triggers, "tbc raid spread", "tbc raid spread", ACTION_MEDIUM_HEAL + 2);
    AddTrigger(triggers, "tbc raid in front of boss", "tbc raid leave front", ACTION_MEDIUM_HEAL + 1);
    AddTrigger(triggers, "tbc raid priority target", "tbc raid attack priority target", ACTION_LIGHT_HEAL + 5);
}

void DungeonBossStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    AddTrigger(triggers, "tbc raid in hazard", "tbc raid leave hazard", ACTION_EMERGENCY + 8);
}

void DungeonBossStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
    AddTrigger(triggers, "tbc raid debuff run", "tbc raid spread", ACTION_EMERGENCY + 6);
    AddTrigger(triggers, "dungeon boss special", "dungeon boss special", ACTION_EMERGENCY + 5);
}

void DungeonBossStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new TbcRaidMultiplier(ai));
    multipliers.push_back(new DungeonBossMultiplier(ai));
}

void DungeonBossStrategy::InitReactionMultipliers(std::list<Multiplier*>& multipliers)
{
    InitCombatMultipliers(multipliers);
}

bool DungeonBossSpecialTrigger::IsActive()
{
    Unit* boss = nullptr;
    if (!bot->IsAlive() || !TbcRaidHelper::GetEncounter(ai, &boss) || !boss)
        return false;
    if (bot->GetMapId() == 556 && bot->HasAura(40303))
        return bot->IsNonMeleeSpellCasted(false);
    WorldPosition position;
    if (ClassicPosition(ai, boss, position))
        return WorldPosition(bot).sqDistance2d(position) > 4.0f;
    if (GameObject* crystal = FindOssirianCrystal(ai, boss))
        if (IsCrystalHandler(bot) && boss->GetDistance(crystal) <= 20.0f && bot->GetDistance(crystal) <= INTERACTION_DISTANCE)
            return true;
    if (NeedsCover(boss))
        return boss->IsWithinLOSInMap(bot);
    if (HoldDamage(ai, boss))
        return bot->GetVictim() == boss || CastingAtBoss(bot, boss) ||
            (bot->GetPet() && bot->GetPet()->GetVictim() == boss);
    // Murmur's Resonance punishes groups with nobody in melee. Keep the tank close,
    // but let the hazard reaction move everyone out during Sonic Boom.
    return boss->GetEntry() == 18708 && PlayerbotAI::IsTank(bot) &&
        !TbcRaidHelper::IsCreatureCasting(boss, 33923) && !TbcRaidHelper::IsCreatureCasting(boss, 38796) &&
        bot->GetDistance(boss) > 5.0f;
}

bool DungeonBossSpecialAction::Execute(Event& event)
{
    Unit* boss = nullptr;
    if (!TbcRaidHelper::GetEncounter(ai, &boss) || !boss)
        return false;
    if (bot->GetMapId() == 556 && bot->HasAura(40303) && bot->IsNonMeleeSpellCasted(false))
    {
        bot->InterruptNonMeleeSpells(false);
        return true;
    }
    WorldPosition position;
    if (ClassicPosition(ai, boss, position))
        return MoveToPoint(position);
    if (GameObject* crystal = FindOssirianCrystal(ai, boss))
    {
        if (IsCrystalHandler(bot) && boss->GetDistance(crystal) <= 20.0f && bot->GetDistance(crystal) <= INTERACTION_DISTANCE)
        {
            bot->StopMoving();
            crystal->Use(bot);
            return true;
        }
    }
    if (NeedsCover(boss))
    {
        return MoveToBestPoint(3.0f, 45.0f, [&](const WorldPosition& point) -> float
        {
            if (boss->IsWithinLOS(point.getX(), point.getY(), point.getZ() + bot->GetCollisionHeight()) ||
                TbcRaidHelper::IsInHazard(point, TbcRaidHelper::GetHazards(ai), 1.0f))
                return -1.0f;
            return sqrt(WorldPosition(bot).sqDistance2d(point));
        });
    }
    if (HoldDamage(ai, boss))
    {
        bool changed = false;
        if (bot->GetVictim() == boss)
        {
            bot->AttackStop();
            changed = true;
        }
        if (CastingAtBoss(bot, boss))
        {
            bot->InterruptNonMeleeSpells(false);
            changed = true;
        }
        if (Pet* pet = bot->GetPet())
        {
            if (pet->GetVictim() == boss)
            {
                pet->AttackStop();
                pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
                changed = true;
            }
        }
        return changed;
    }
    if (boss->GetEntry() == 18708)
        return MoveToBestPoint(3.0f, 20.0f, [&](const WorldPosition& point) -> float
        {
            if (TbcRaidHelper::IsInHazard(point, TbcRaidHelper::GetHazards(ai), 1.0f))
                return -1.0f;
            return sqrt(point.sqDistance2d(WorldPosition(boss)));
        });
    return false;
}

bool DungeonBossSpecialAction::ShouldReactionInterruptCast() const
{
    Unit* boss = nullptr;
    TbcRaidHelper::GetEncounter(ai, &boss);
    WorldPosition position;
    return NeedsCover(boss) || ClassicPosition(ai, boss, position) || CastingAtBoss(bot, boss) ||
        (bot->GetMapId() == 556 && bot->HasAura(40303));
}

float DungeonBossMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() == "dungeon boss special")
        return 1.0f;
    Unit* boss = nullptr;
    if (!TbcRaidHelper::GetEncounter(ai, &boss) || !boss)
        return 1.0f;
    if (bot->GetMapId() == 556 && bot->HasAura(40303) && dynamic_cast<CastSpellAction*>(action))
        return 0.0f;
    const std::string name = action->getName();
    WorldPosition position;
    if (ClassicPosition(ai, boss, position) && (raidboss::IsMovementAction(action) || HostileAction(action, bot)))
        return name == "tbc raid leave hazard" ? 1.0f : 0.0f;
    // Ordinary following/reaching must not undo a mechanic's movement.
    bool debuffRun = false;
    TbcRaidHelper::GetRequiredSpread(ai, &debuffRun);
    if (raidboss::IsMovementAction(action) && name.rfind("tbc raid", 0) != 0 &&
        (debuffRun || TbcRaidHelper::IsInHazard(WorldPosition(bot), TbcRaidHelper::GetHazards(ai))))
        return 0.0f;
    if (NeedsCover(boss) && (raidboss::IsMovementAction(action) || HostileAction(action, bot)))
        return name == "tbc raid leave hazard" ? 1.0f : 0.0f;
    if (HoldDamage(ai, boss) && HostileAction(action, bot) &&
        (action->GetTarget() == boss || action->getThreatType() == ActionThreatType::ACTION_THREAT_AOE))
        return 0.0f;
    return 1.0f;
}
