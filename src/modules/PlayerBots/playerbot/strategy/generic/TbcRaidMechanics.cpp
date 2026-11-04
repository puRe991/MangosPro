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

        // Kalecgos
        NPC_KALECGOS_DRAGON         = 24850,
        NPC_SATHROVARR              = 24892,
        GO_SPECTRAL_RIFT            = 187055,
        SPELL_SPECTRAL_REALM_AURA   = 46021,
        SPELL_SPECTRAL_EXHAUSTION   = 44867,
        SPELL_SATHROVARR_BANISH     = 44836,
    };

    // Kalecgos: players wanted in the Spectral Realm at the same time. Only 7 left most of the raid on the platform
    // stacking Arcane Buffet: 57 deaths at 7-15 stacks on 2026-10-31.
    const uint32 REALM_TANKS = 1;
    const uint32 REALM_HEALERS = 3;
    const uint32 REALM_DAMAGE = 8;
    const uint32 REALM_DAMAGE_CATCH_UP = 10;  // while the dragon is ahead of Sathrovarr
    const uint32 REALM_DAMAGE_SLOW = 5;       // while Sathrovarr is more than 10% ahead of the dragon
    const float SATHROVARR_HOLD_PCT = 12.0f;  // Crazed Rage starts below 10%
    const uint32 SPELL_ARCANE_BUFFET = 45018;

    // Kalecgos dragon and Sathrovarr (128 yd below the platform), alive and in combat
    bool GetKalecgosPair(Player* bot, Unit*& dragon, Unit*& sathrovarr)
    {
        dragon = nullptr;
        sathrovarr = nullptr;
        for (Unit* unit : TbcRaidHelper::FindCreatures(bot, NPC_KALECGOS_DRAGON, 160.0f))
        {
            if (unit->IsAlive() && unit->IsInCombat())
                dragon = unit;
        }

        // Sathrovarr is not killed but banished at 0 HP; afterwards the dragon has to be finished (on 2026-10-31
        // the hold rule kept the raid off the dragon at 8.7% while the banished Sathrovarr still counted as alive)
        for (Unit* unit : TbcRaidHelper::FindCreatures(bot, NPC_SATHROVARR, 160.0f))
        {
            if (unit->IsAlive() && unit->IsInCombat() && !unit->HasAura(SPELL_SATHROVARR_BANISH))
                sathrovarr = unit;
        }

        return dragon && sathrovarr;
    }

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

GameObject* TbcRaidHelper::GetSpectralRiftToUse(PlayerbotAI* ai)
{
    Unit* boss = nullptr;
    const Encounter* encounter = GetEncounter(ai, &boss);
    if (!encounter || !(encounter->special & SPECIAL_SPECTRAL_REALM))
        return nullptr;

    Player* bot = ai->GetBot();
    if (bot->HasAura(SPELL_SPECTRAL_REALM_AURA) || bot->HasAura(SPELL_SPECTRAL_EXHAUSTION))
        return nullptr;

    GameObject* rift = nullptr;
    for (GameObject* candidate : FindGameObjects(bot, GO_SPECTRAL_RIFT, 60.0f))
    {
        if (candidate->IsSpawned() && (!rift || bot->GetDistance(candidate) < bot->GetDistance(rift)))
            rift = candidate;
    }

    if (!rift)
        return nullptr;

    // The tank holding the dragon stays; the returning tank takes it over on Arcane Buffet stacks (TankSwap)
    Unit* dragonVictim = nullptr;
    for (Unit* dragon : FindCreatures(bot, NPC_KALECGOS_DRAGON, 100.0f))
    {
        if (dragon->IsAlive())
            dragonVictim = dragon->GetVictim();
    }

    if (dragonVictim == bot)
        return nullptr;

    const uint8 role = tbcraid::GetRole(ai, bot);
    const bool damage = (role & ROLE_DPS) != 0;
    Unit* dragon = nullptr;
    Unit* sathrovarr = nullptr;
    uint32 realmDamage = REALM_DAMAGE;
    if (GetKalecgosPair(bot, dragon, sathrovarr))
    {
        const float lead = dragon->GetHealthPercent() - sathrovarr->GetHealthPercent();
        if (lead < 0.0f)
            realmDamage = REALM_DAMAGE_CATCH_UP;
        else if (lead > 10.0f)
            realmDamage = REALM_DAMAGE_SLOW;
    }

    const uint32 wanted = role == ROLE_TANK ? REALM_TANKS : (role == ROLE_HEAL ? REALM_HEALERS : realmDamage);

    // The realm is 128 yd below the platform: count the whole raid
    uint32 inRealm = 0;
    std::vector<Player*> candidates;
    for (Player* member : GetRaidMembers(bot, 300.0f, true))
    {
        const uint8 memberRole = tbcraid::GetRole(ai, member);
        if (damage ? !(memberRole & ROLE_DPS) : memberRole != role)
            continue;

        if (member->HasAura(SPELL_SPECTRAL_REALM_AURA))
        {
            ++inRealm;
            continue;
        }

        if (!member->GetPlayerbotAI() || member == dragonVictim || member->HasAura(SPELL_SPECTRAL_EXHAUSTION) ||
            !member->IsWithinDistInMap(rift, 60.0f))
            continue;

        candidates.push_back(member);
    }

    if (inRealm >= wanted)
        return nullptr;

    // The bots with the most Arcane Buffet stacks go first (the realm clears them), then the closest
    std::sort(candidates.begin(), candidates.end(), [rift](Player* a, Player* b)
    {
        const uint32 stacksA = a->GetAuraCount(SPELL_ARCANE_BUFFET);
        const uint32 stacksB = b->GetAuraCount(SPELL_ARCANE_BUFFET);
        if (stacksA != stacksB)
            return stacksA > stacksB;

        return a->GetDistance(rift) < b->GetDistance(rift);
    });
    const size_t needed = wanted - inRealm;
    for (size_t i = 0; i < candidates.size() && i < needed; ++i)
    {
        if (candidates[i] == bot)
            return rift;
    }

    return nullptr;
}

Unit* TbcRaidHelper::GetKalecgosToHoldDamage(PlayerbotAI* ai)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || !(encounter->special & SPECIAL_SPECTRAL_REALM))
        return nullptr;

    Player* bot = ai->GetBot();
    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return nullptr;

    // Both have to fall nearly together. Sathrovarr has to die first: at 1% the dragon banishes itself and opens
    // no more rifts (2026-10-31: dragon 1% with Sathrovarr at 40%, fail). But Sathrovarr below 10% gives the dragon
    // Crazed Rage until its own end (2026-10-31: Sathrovarr banished with the dragon at 36%, the enraged dragon
    // wiped the raid). So the platform waits for Sathrovarr, and the realm holds Sathrovarr at 12% for the dragon.
    // Asked by the multiplier for every action: search at most every 500 ms per bot
    static thread_local std::unordered_map<uint32, std::pair<uint32, ObjectGuid>> cache;
    std::pair<uint32, ObjectGuid>& cached = cache[bot->GetGUIDLow()];
    const uint32 now = WorldTimer::getMSTime();
    if (cached.first && WorldTimer::getMSTimeDiff(cached.first, now) < 500)
        return cached.second ? ai->GetUnit(cached.second) : nullptr;

    cached.first = now;
    cached.second = ObjectGuid();

    Unit* dragon = nullptr;
    Unit* sathrovarr = nullptr;
    if (!GetKalecgosPair(bot, dragon, sathrovarr))
        return nullptr;

    const float dragonPct = dragon->GetHealthPercent();
    const float sathrovarrPct = sathrovarr->GetHealthPercent();
    Unit* hold = nullptr;
    if (bot->HasAura(SPELL_SPECTRAL_REALM_AURA))
    {
        if (sathrovarrPct < SATHROVARR_HOLD_PCT && dragonPct > sathrovarrPct + 2.0f)
            hold = sathrovarr;
    }
    else if (dragonPct < 10.0f || dragonPct < sathrovarrPct - 3.0f)
        hold = dragon;

    if (hold)
        cached.second = hold->GetObjectGuid();

    return hold;
}

Unit* TbcRaidHelper::GetSathrovarrToTaunt(PlayerbotAI* ai)
{
    const Encounter* encounter = GetEncounter(ai);
    if (!encounter || !(encounter->special & SPECIAL_SPECTRAL_REALM))
        return nullptr;

    Player* bot = ai->GetBot();
    if (!bot->HasAura(SPELL_SPECTRAL_REALM_AURA) || GetRole(ai) != ROLE_TANK)
        return nullptr;

    // Kalec's Heroic Strikes out-threaten a paladin tank: Sathrovarr hit Kalec most of the 2026-10-31 attempt
    for (Unit* sathrovarr : FindCreatures(bot, NPC_SATHROVARR, 40.0f))
    {
        if (!sathrovarr->IsAlive() || !sathrovarr->IsInCombat())
            continue;

        Unit* victim = sathrovarr->GetVictim();
        if (!victim || victim == bot)
            continue;

        if (victim->GetTypeId() == TYPEID_PLAYER && PlayerbotAI::IsTank((Player*)victim))
            continue;

        // Righteous Defense on Kalec only started and never landed (2026-10-31): paladins taunt off players only
        if (bot->getClass() == CLASS_PALADIN && victim->GetTypeId() != TYPEID_PLAYER)
            continue;

        return sathrovarr;
    }

    return nullptr;
}
