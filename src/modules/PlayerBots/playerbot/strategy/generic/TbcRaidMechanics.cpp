#include "playerbot/playerbot.h"
#include "TbcRaidStrategy.h"
#include "Loot/LootMgr.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

// Special boss mechanics of the Burning Crusade raids, used by the "tbc raid" strategy

using namespace ai;
using namespace ai::tbcraid;

namespace
{
    enum TbcRaidMechanicIds : uint32
    {
        // Hydross the Unstable
        NPC_HYDROSS                 = 21216,
        SPELL_HYDROSS_CORRUPTION    = 37961,

        // Lady Vashj
        NPC_TAINTED_ELEMENTAL       = 22009,
        NPC_TINY_TRIGGER            = 21987,
        ITEM_TAINTED_CORE           = 31088,
        SPELL_PARALYZE              = 38132,

        // The Lurker Below
        NPC_LURKER_BELOW            = 21217,
        SPELL_SPOUT                 = 37431,
        SPELL_SPOUT_LEFT            = 37429,
        SPELL_SPOUT_RIGHT           = 37430,

        // Teron Gorefiend
        SPELL_SPIRITUAL_VENGEANCE   = 40268,

        // Kil'jaeden
        NPC_KILJAEDEN               = 25315,
        SPELL_DARKNESS_OF_SOULS     = 46605,
        SPELL_VENGEANCE_BLUE_FLIGHT = 45839,
    };

    const float HYDROSS_SWITCH_RADIUS = 20.0f;   // purification circle around the combat start position

    // Mark of Hydross / Mark of Corruption at 100% (4th mark) and above
    const uint32 aHighMarkHydross[] = { 38218, 38231, 40584 };
    const uint32 aHighMarkCorruption[] = { 38222, 38230, 40583 };

    const uint32 aShieldGenerators[] = { 185051, 185052, 185053, 185054 };
    const uint32 aBlueDragonOrbs[] = { 188115, 188116, 187869, 188114 };

    bool HasAnyAura(Unit* unit, const uint32* auras, size_t count)
    {
        for (size_t i = 0; i < count; ++i)
        {
            if (unit->HasAura(auras[i]))
                return true;
        }

        return false;
    }

    // Lower is better: ranged damage dealers do the extra jobs first
    uint8 HelperRoleOrder(uint8 role)
    {
        switch (role)
        {
        case ROLE_RANGED: return 0;
        case ROLE_MELEE:  return 1;
        case ROLE_HEAL:   return 2;
        default:          return 3;
        }
    }
}

std::list<Unit*> TbcRaidHelper::FindCreaturesAround(WorldObject* center, uint32 entry, float range)
{
    std::list<Unit*> creatures;
    MaNGOS::AllCreaturesOfEntryInRangeCheck check(center, entry, range);
    MaNGOS::UnitListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(creatures, check);
    Cell::VisitAllObjects(center, searcher, range);
    return creatures;
}

// ---------------------------------------------------------------------------
// Tanks
// ---------------------------------------------------------------------------

Player* TbcRaidHelper::GetMainTankToStackOn(PlayerbotAI* ai)
{
    Unit* boss = nullptr;
    const Encounter* encounter = GetEncounter(ai, &boss);
    if (!encounter || !(encounter->special & SPECIAL_STACK_OFFTANKS) || !boss)
        return nullptr;

    Player* bot = ai->GetBot();
    if (GetRole(ai) != ROLE_TANK)
        return nullptr;

    Unit* victim = boss->GetVictim();
    if (!victim || victim == bot || victim->GetTypeId() != TYPEID_PLAYER || !victim->IsAlive())
        return nullptr;

    return (Player*)victim;
}

Unit* TbcRaidHelper::GetApproachTarget(PlayerbotAI* ai, float& distance)
{
    Unit* boss = nullptr;
    const Encounter* encounter = GetEncounter(ai, &boss);
    if (!encounter || encounter->debuffApproach.empty() || !boss || !boss->IsAlive())
        return nullptr;

    if (!HasRole(ROLE_CASTERS, GetRole(ai)))
        return nullptr;

    Player* bot = ai->GetBot();
    for (const DebuffApproachRule& rule : encounter->debuffApproach)
    {
        if (bot->GetAuraCount(rule.spellId) >= rule.minStacks)
        {
            distance = rule.distance;
            return boss;
        }
    }

    return nullptr;
}

bool TbcRaidHelper::ShouldSoakFront(PlayerbotAI* ai, Unit* boss)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || encounter->frontSoak.empty() || !boss)
        return false;

    const uint8 role = GetRole(ai);
    if (role != ROLE_MELEE && role != ROLE_TANK)
        return false;

    Player* bot = ai->GetBot();
    for (const FrontSoakRule& rule : encounter->frontSoak)
    {
        if (boss->GetEntry() == rule.bossEntry && bot->GetAuraCount(rule.spellId) < rule.maxStacks)
            return true;
    }

    return false;
}

Unit* TbcRaidHelper::GetSoakBoss(PlayerbotAI* ai)
{
    Unit* boss = nullptr;
    const Encounter* encounter = GetEncounter(ai, &boss);
    if (!encounter || encounter->frontSoak.empty() || !boss || !boss->IsAlive() || !boss->GetVictim())
        return nullptr;

    Player* bot = ai->GetBot();
    if (boss->GetVictim() == bot || !ShouldSoakFront(ai, boss))
        return nullptr;

    // Already standing in front of the boss, in melee range
    if (boss->HasInArc(bot, M_PI_F / 2.0f) && bot->IsWithinDistInMap(boss, boss->GetCombatReach() + 4.0f))
        return nullptr;

    return boss;
}

bool TbcRaidHelper::GetHydrossTankPosition(PlayerbotAI* ai, WorldPosition& position)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || !(encounter->special & SPECIAL_HYDROSS_TANKING))
        return false;

    Player* bot = ai->GetBot();
    Creature* hydross = nullptr;
    for (Unit* unit : FindCreatures(bot, NPC_HYDROSS, 80.0f))
    {
        if (unit->IsAlive() && unit->GetVictim() == bot)
        {
            hydross = (Creature*)unit;
            break;
        }
    }

    if (!hydross)
        return false;

    Position start;
    hydross->GetCombatStartPosition(start);
    if (start.GetPositionX() == 0.0f && start.GetPositionY() == 0.0f)
        return false;

    const bool corrupted = hydross->HasAura(SPELL_HYDROSS_CORRUPTION);

    // Time to switch: the raid reached 100% Mark of Hydross / Mark of Corruption
    bool switchPhase = false;
    for (Player* member : GetRaidMembers(bot, 80.0f, true))
    {
        if (corrupted ? HasAnyAura(member, aHighMarkCorruption, 3) : HasAnyAura(member, aHighMarkHydross, 3))
        {
            switchPhase = true;
            break;
        }
    }

    const float dx = hydross->GetPositionX() - start.GetPositionX();
    const float dy = hydross->GetPositionY() - start.GetPositionY();
    const float bossDistance = sqrt(dx * dx + dy * dy);

    // Clean form stays inside the circle, corrupted form outside
    const bool wantInside = corrupted ? switchPhase : !switchPhase;
    float wantedDistance = 0.0f;
    if (wantInside && bossDistance > HYDROSS_SWITCH_RADIUS - 6.0f)
        wantedDistance = HYDROSS_SWITCH_RADIUS - 12.0f;
    else if (!wantInside && bossDistance < HYDROSS_SWITCH_RADIUS + 4.0f)
        wantedDistance = HYDROSS_SWITCH_RADIUS + 9.0f;
    else
        return false;

    // Walk along the line from the circle center through the tank
    float angle = atan2(bot->GetPositionY() - start.GetPositionY(), bot->GetPositionX() - start.GetPositionX());
    if (bossDistance < 1.0f && fabs(bot->GetPositionX() - start.GetPositionX()) < 1.0f && fabs(bot->GetPositionY() - start.GetPositionY()) < 1.0f)
        angle = hydross->GetOrientation();

    const float x = start.GetPositionX() + wantedDistance * cos(angle);
    const float y = start.GetPositionY() + wantedDistance * sin(angle);
    const float z = bot->GetMap()->GetHeight(x, y, bot->GetPositionZ() + 5.0f);
    if (z <= INVALID_HEIGHT)
        return false;

    position = WorldPosition(bot->GetMapId(), x, y, z);
    return true;
}

// ---------------------------------------------------------------------------
// Lady Vashj - Tainted Cores
// ---------------------------------------------------------------------------

Creature* TbcRaidHelper::GetTaintedCoreCorpse(PlayerbotAI* ai)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || !(encounter->special & SPECIAL_TAINTED_CORES))
        return nullptr;

    Player* bot = ai->GetBot();
    if (bot->HasItemCount(ITEM_TAINTED_CORE, 1) || PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return nullptr;

    const std::vector<Player*> members = GetRaidMembers(bot, 100.0f, true);
    for (Unit* unit : FindCreatures(bot, NPC_TAINTED_ELEMENTAL, 80.0f))
    {
        if (unit->IsAlive())
            continue;

        Creature* corpse = (Creature*)unit;
        Loot* loot = sLootMgr.GetLoot(bot, corpse->GetObjectGuid());
        if (!loot || !loot->IsItemAlreadyIn(ITEM_TAINTED_CORE))
            continue;

        // The closest bot which is allowed to loot takes the core
        Player* looter = nullptr;
        uint8 looterOrder = 255;
        for (Player* member : members)
        {
            if (!member->GetPlayerbotAI() || PlayerbotAI::IsTank(member) || PlayerbotAI::IsHeal(member))
                continue;

            if (member->HasItemCount(ITEM_TAINTED_CORE, 1) || !loot->CanLoot(member))
                continue;

            const uint8 order = HelperRoleOrder(tbcraid::GetRole(ai, member));
            if (!looter || order < looterOrder || (order == looterOrder && member->GetDistance(corpse) < looter->GetDistance(corpse)))
            {
                looter = member;
                looterOrder = order;
            }
        }

        if (looter == bot)
            return corpse;
    }

    return nullptr;
}

GameObject* TbcRaidHelper::GetNearestActiveShieldGenerator(PlayerbotAI* ai, WorldObject* from)
{
    Player* bot = ai->GetBot();
    GameObject* nearest = nullptr;
    for (uint32 entry : aShieldGenerators)
    {
        for (GameObject* generator : FindGameObjects(bot, entry, 150.0f))
        {
            if (generator->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT))
                continue;

            // The generator is active while its trigger channels the Magic Barrier
            bool active = false;
            for (Unit* trigger : FindCreaturesAround(generator, NPC_TINY_TRIGGER, 6.0f))
            {
                if (trigger->IsNonMeleeSpellCasted(false))
                {
                    active = true;
                    break;
                }
            }

            if (!active)
                continue;

            if (!nearest || from->GetDistance(generator) < from->GetDistance(nearest))
                nearest = generator;
        }
    }

    return nearest;
}

Player* TbcRaidHelper::GetTaintedCoreReceiver(PlayerbotAI* ai, GameObject* generator)
{
    Player* bot = ai->GetBot();
    if (!generator)
        return nullptr;

    const float myDistance = bot->GetDistance(generator);
    Player* receiver = nullptr;
    for (Player* member : GetRaidMembers(bot, 35.0f))
    {
        if (member->HasAura(SPELL_PARALYZE) || member->HasItemCount(ITEM_TAINTED_CORE, 1))
            continue;

        const float distance = member->GetDistance(generator);
        if (distance >= myDistance - 5.0f)
            continue;

        if (!receiver || distance < receiver->GetDistance(generator))
            receiver = member;
    }

    return receiver;
}

// ---------------------------------------------------------------------------
// The Lurker Below - Spout
// ---------------------------------------------------------------------------

Unit* TbcRaidHelper::GetSpoutingLurker(PlayerbotAI* ai)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || !(encounter->special & SPECIAL_LURKER_SPOUT))
        return nullptr;

    for (Unit* lurker : FindCreatures(ai->GetBot(), NPC_LURKER_BELOW, 120.0f))
    {
        if (!lurker->IsAlive())
            continue;

        if (lurker->HasAura(SPELL_SPOUT_LEFT) || lurker->HasAura(SPELL_SPOUT_RIGHT) || IsCreatureCasting(lurker, SPELL_SPOUT))
            return lurker;
    }

    return nullptr;
}

// ---------------------------------------------------------------------------
// Possessed creatures (Vengeful Spirit, Blue Dragon)
// ---------------------------------------------------------------------------

bool TbcRaidHelper::IsPossessingBossCreature(Player* bot)
{
    return bot->HasAura(SPELL_SPIRITUAL_VENGEANCE) || bot->HasAura(SPELL_VENGEANCE_BLUE_FLIGHT);
}

Creature* TbcRaidHelper::GetPossessedCreature(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    if (!IsPossessingBossCreature(bot))
        return nullptr;

    Unit* charm = bot->GetCharm();
    if (!charm || charm->GetTypeId() != TYPEID_UNIT || !charm->IsAlive())
        return nullptr;

    return (Creature*)charm;
}

uint32 TbcRaidHelper::FindCharmSpell(Creature* charm, const std::string& namePart)
{
    if (!charm)
        return 0;

    for (uint32 spellId : charm->GetCharmSpells())
    {
        if (!spellId)
            continue;

        const SpellEntry* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
        if (!spellInfo || !spellInfo->SpellName[0])
            continue;

        const std::string name = spellInfo->SpellName[0];
        if (name.find(namePart) != std::string::npos)
            return spellId;
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Kil'jaeden - Blue Dragons
// ---------------------------------------------------------------------------

Unit* TbcRaidHelper::GetKiljaedenCastingDarkness(PlayerbotAI* ai)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || !(encounter->special & SPECIAL_BLUE_DRAGON))
        return nullptr;

    for (Unit* kiljaeden : FindCreatures(ai->GetBot(), NPC_KILJAEDEN, 150.0f))
    {
        if (kiljaeden->IsAlive() && IsCreatureCasting(kiljaeden, SPELL_DARKNESS_OF_SOULS))
            return kiljaeden;
    }

    return nullptr;
}

Unit* TbcRaidHelper::GetRaidBlueDragon(PlayerbotAI* ai)
{
    Player* bot = ai->GetBot();
    Unit* best = nullptr;
    for (Player* member : GetRaidMembers(bot, 150.0f))
    {
        if (!member->HasAura(SPELL_VENGEANCE_BLUE_FLIGHT))
            continue;

        Unit* dragon = member->GetCharm();
        if (dragon && dragon->IsAlive() && (!best || bot->GetDistance(dragon) < bot->GetDistance(best)))
            best = dragon;
    }

    return best;
}

GameObject* TbcRaidHelper::GetBlueOrbToUse(PlayerbotAI* ai)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || !(encounter->special & SPECIAL_BLUE_DRAGON))
        return nullptr;

    Player* bot = ai->GetBot();
    if (IsPossessingBossCreature(bot) || PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return nullptr;

    const std::vector<Player*> members = GetRaidMembers(bot, 150.0f, true);
    for (uint32 entry : aBlueDragonOrbs)
    {
        for (GameObject* orb : FindGameObjects(bot, entry, 150.0f))
        {
            if (orb->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT))
                continue;

            // The closest free damage dealer bot takes the orb
            Player* user = nullptr;
            uint8 userOrder = 255;
            for (Player* member : members)
            {
                if (!member->GetPlayerbotAI() || PlayerbotAI::IsTank(member) || PlayerbotAI::IsHeal(member) || IsPossessingBossCreature(member))
                    continue;

                const uint8 order = HelperRoleOrder(tbcraid::GetRole(ai, member));
                if (!user || order < userOrder || (order == userOrder && member->GetDistance(orb) < user->GetDistance(orb)))
                {
                    user = member;
                    userOrder = order;
                }
            }

            if (user == bot)
                return orb;
        }
    }

    return nullptr;
}
