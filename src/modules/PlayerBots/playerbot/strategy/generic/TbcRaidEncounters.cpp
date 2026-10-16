#include "playerbot/playerbot.h"
#include "TbcRaidEncounters.h"

using namespace ai;
using namespace ai::tbcraid;

namespace
{
    enum TbcRaidMaps : uint32
    {
        MAP_KARAZHAN            = 532,
        MAP_GRUULS_LAIR         = 565,
        MAP_MAGTHERIDONS_LAIR   = 544,
        MAP_SERPENTSHRINE       = 548,
        MAP_THE_EYE             = 550,
        MAP_HYJAL               = 534,
        MAP_BLACK_TEMPLE        = 564,
        MAP_ZULAMAN             = 568,
        MAP_SUNWELL_PLATEAU     = 580,
    };
}

EncounterRegistry& EncounterRegistry::Instance()
{
    static EncounterRegistry instance;
    return instance;
}

Encounter& EncounterRegistry::Add(const std::string& name, uint32 mapId, const std::vector<uint32>& entries)
{
    Encounter* encounter = new Encounter();
    encounter->name = name;
    encounter->mapId = mapId;
    encounter->entries = entries;
    encounters.push_back(encounter);

    for (uint32 entry : entries)
        byCreature[entry].push_back(encounter);

    if (std::find(raidMaps.begin(), raidMaps.end(), mapId) == raidMaps.end())
        raidMaps.push_back(mapId);

    return *encounter;
}

const Encounter* EncounterRegistry::FindByCreature(uint32 mapId, uint32 entry) const
{
    auto it = byCreature.find(entry);
    if (it == byCreature.end())
        return nullptr;

    for (const Encounter* encounter : it->second)
    {
        if (encounter->mapId == mapId)
            return encounter;
    }

    return nullptr;
}

const Encounter* EncounterRegistry::FindNearby(PlayerbotAI* ai) const
{
    Player* bot = ai->GetBot();
    if (!IsRaidMap(bot->GetMapId()))
        return nullptr;

    for (ObjectGuid guid : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get())
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit || unit->IsPlayer())
            continue;

        if (const Encounter* encounter = FindByCreature(bot->GetMapId(), unit->GetEntry()))
            return encounter;
    }

    return nullptr;
}

bool EncounterRegistry::IsRaidMap(uint32 mapId) const
{
    return std::find(raidMaps.begin(), raidMaps.end(), mapId) != raidMaps.end();
}

uint8 tbcraid::GetRole(PlayerbotAI* ai, Player* player)
{
    if (PlayerbotAI::IsTank(player))
        return ROLE_TANK;

    if (PlayerbotAI::IsHeal(player))
        return ROLE_HEAL;

    return ai->IsRanged(player) ? ROLE_RANGED : ROLE_MELEE;
}

// The tactics below follow the common raid guides for each boss (wowtbc.gg, Icy Veins,
// Warcraft Tavern, Wowhead) and the boss scripts of this core for the exact ids.
EncounterRegistry::EncounterRegistry()
{
    // ---------------------------------------------------------------------
    // Karazhan (bosses without a dedicated strategy)
    // ---------------------------------------------------------------------

    // The Curator: kill every Astral Flare, ranged spread for Arcing Sear.
    Add("the curator", MAP_KARAZHAN, { 15691, 17096 })
        .Kill(17096, ROLE_DPS);

    // Terestian Illhoof: free the sacrificed player (Demon Chains), kill Kil'rek.
    Add("terestian illhoof", MAP_KARAZHAN, { 15688, 17248, 17229 })
        .ResistanceAura("shadow resistance aura")
        .Kill(17248, ROLE_DPS)
        .Kill(17229, ROLE_RANGED);

    // Nightbane: get out of Charred Earth, kill the skeletons in the air phase.
    Add("nightbane", MAP_KARAZHAN, { 17225, 17261 })
        .ResistanceAura("fire resistance aura")
        .AvoidArea(30129)
        .Kill(17261, ROLE_DPS)
        .AvoidFront(17225, 25.0f, ROLE_MELEE);      // Cleave / Smoldering Breath (ranged stack, see "boss karazhan")

    // Opera - The Crone: never touch the Cyclone.
    Add("the crone", MAP_KARAZHAN, { 18168, 18412 })
        .Avoid(18412, 8.0f);

    // ---------------------------------------------------------------------
    // Gruul's Lair
    // ---------------------------------------------------------------------

    // High King Maulgar / Gruul / Magtheridon: kill order, positions, spreading and the Manticron
    // Cubes are handled by the "boss gruul" and "boss magtheridon" strategies (strategy/raids).
    Add("high king maulgar", MAP_GRUULS_LAIR, { 18831, 18832, 18834, 18835, 18836 })
        .AvoidFront(18831, 10.0f)                   // Arcing Smash
        .Spread(6.0f, ROLE_CASTERS);

    // Gruul: ranged spread at all times, after Ground Slam everybody spreads 15 yards
    // before Shatter, leave Cave In, off tank stays in melee for Hurtful Strike.
    Add("gruul", MAP_GRUULS_LAIR, { 19044 })
        .Special(SPECIAL_OFFTANK_NO_TAUNT);         // off tank takes Hurtful Strike         // off tank takes Hurtful Strike

    // ---------------------------------------------------------------------
    // Magtheridon's Lair
    // ---------------------------------------------------------------------

    // Magtheridon: kill the Hellfire Channelers one by one (interrupt Shadow Bolt Volley / Dark Mending),
    // kill the Burning Abyssals, click the 5 Manticron Cubes on Blast Nova, leave the Blaze fires.
    Add("magtheridon", MAP_MAGTHERIDONS_LAIR, { 17257, 17256, 17454 })
        .AvoidObject(181832, 5.0f)                  // Blaze
        .AvoidArea(30757)                           // Conflagration
        .AvoidFront(17257, 12.0f);                  // Cleave

    // ---------------------------------------------------------------------
    // Serpentshrine Cavern
    // ---------------------------------------------------------------------

    // Serpentshrine Cavern: tank spots, kill orders, the Tainted Core chain and the other mechanics are
    // handled by the "boss ssc" strategy (strategy/raids/ssc). Only the complementing rules stay here.

    // Hydross the Unstable: the elementals spawned on every phase change die first.
    Add("hydross the unstable", MAP_SERPENTSHRINE, { 21216, 22035, 22036 })
        .ResistanceAura("frost resistance aura")
        .Kill(22035, ROLE_DPS)                      // Pure Spawn of Hydross
        .Kill(22036, ROLE_DPS);                     // Tainted Spawn of Hydross

    // The Lurker Below: stay out of Whirl, kill the Ambushers (ranged) during submerge.
    Add("the lurker below", MAP_SERPENTSHRINE, { 21217, 21865, 21873 })
        .Kill(21865, ROLE_RANGED)                   // Coilfang Ambusher
        .Kill(21873, ROLE_MELEE)                    // Coilfang Guardian
        .AvoidWhileCasting(21217, 37660, 14.0f, ROLE_NON_TANK) // Whirl
        .AvoidArea(37478)                           // Geyser
        .Special(SPECIAL_LURKER_SPOUT);

    // Leotheras the Blind: everybody kills their own Inner Demon, nobody stands in the Whirlwind,
    // melee leave the demon form (Chaos Blast) to the warlock tank.
    Add("leotheras the blind", MAP_SERPENTSHRINE, { 21215, 21857, 21875 })
        .KillMine(21857)                            // Inner Demon
        .AvoidWhileAura(21215, 37640, 12.0f, ROLE_ALL)   // Whirlwind
        .AvoidWhileAura(21215, 37673, 12.0f, ROLE_MELEE) // demon form
        .Spread(6.0f, ROLE_CASTERS);

    // Fathom-Lord Karathress: Spitfire Totem on sight, then Tidalvess -> Sharkkis -> Caribdis -> Karathress.
    Add("fathom-lord karathress", MAP_SERPENTSHRINE, { 21214, 21964, 21965, 21966 })
        .Avoid(22104, 8.0f, ROLE_NON_TANK)          // Cyclone
        .Spread(6.0f, ROLE_CASTERS);

    // Morogrim Tidewalker: stay away from the Water Globules, murlocs are AoE tanked.
    Add("morogrim tidewalker", MAP_SERPENTSHRINE, { 21213 })
        .Avoid(21913, 10.0f)                        // Water Globule
        .AvoidFront(21213, 12.0f);                  // Tidal Wave

    // Lady Vashj: Static Charge runs out, phase 2 Tainted Elementals and Enchanted Elementals
    // die first (before they reach Vashj), stay out of the Striders fear and the Toxic Spores.
    Add("lady vashj", MAP_SERPENTSHRINE, { 21212, 22009, 21958, 22056, 22055, 22140 })
        .LustBelow(49.0f)
        .Avoid(22056, 14.0f, ROLE_NON_TANK);        // Coilfang Strider (Panic)

    // ---------------------------------------------------------------------
    // Tempest Keep: The Eye
    // ---------------------------------------------------------------------

    // Tempest Keep: platforms, embers, Arcane Orbs, priests, advisors, weapons, phoenixes and Gravity Lapse
    // are handled by the "boss tempest keep" strategy (strategy/raids/tk). Only the complementing rules stay here.

    // Al'ar
    Add("al'ar", MAP_THE_EYE, { 19514, 19551 })
        .ResistanceAura("fire resistance aura");

    // Void Reaver (registered to identify the encounter)
    Add("void reaver", MAP_THE_EYE, { 19516 });

    // High Astromancer Solarian: Wrath of the Astromancer runs out of the raid, kill the agents.
    Add("high astromancer solarian", MAP_THE_EYE, { 18805, 18806, 18925 })
        .RunWithDebuff(33044, 22.0f)
        .Kill(18925, ROLE_DPS)                      // Solarium Agent
        .Spread(8.0f, ROLE_CASTERS);

    // Kael'thas Sunstrider: stay out of Nether Vapor.
    Add("kael'thas sunstrider", MAP_THE_EYE, { 19622, 20060, 20062, 20063, 20064,
        21268, 21269, 21270, 21271, 21272, 21273, 21274, 21362, 21364 })
        .Avoid(21002, 7.0f);                        // Nether Vapor

    // ---------------------------------------------------------------------
    // Hyjal Summit
    // ---------------------------------------------------------------------

    // Hyjal Summit: positions, Death and Decay, Infernals, Mark of Kaz'rogal, Doom, Doomfire and Air Burst
    // are handled by the "boss hyjal" strategy (strategy/raids/hyjal). Only the complementing rules stay here.

    // Rage Winterchill (registered to identify the encounter)
    Add("rage winterchill", MAP_HYJAL, { 17767 });

    // Anetheron: nobody but the tank in front of him (Carrion Swarm).
    Add("anetheron", MAP_HYJAL, { 17808, 17818 })
        .AvoidFront(17808, 30.0f);                  // Carrion Swarm

    // Kaz'rogal (registered to identify the encounter)
    Add("kaz'rogal", MAP_HYJAL, { 17888 });

    // Azgalor (registered to identify the encounter)
    Add("azgalor", MAP_HYJAL, { 17842 });

    // Archimonde: fire resistance aura.
    Add("archimonde", MAP_HYJAL, { 17968 })
        .ResistanceAura("fire resistance aura");

    // ---------------------------------------------------------------------
    // Black Temple
    // ---------------------------------------------------------------------

    // High Warlord Naj'entus: spread for Needle Spine, free impaled players, break Tidal Shield with a spine.
    Add("high warlord naj'entus", MAP_BLACK_TEMPLE, { 22887 })
        .Spread(10.0f, ROLE_CASTERS)
        .Special(SPECIAL_NAJENTUS_SPINES);

    // Supremus: Gaze target kites the boss, nobody stands in Volcanoes or the Molten Flame trail.
    Add("supremus", MAP_BLACK_TEMPLE, { 22898 })
        .ResistanceAura("fire resistance aura")
        .Kite(22898, 22.0f, ROLE_NON_TANK)
        .Avoid(23085, 12.0f)                        // Volcano
        .Avoid(23095, 5.0f)                         // Molten Flame
        .Spread(6.0f, ROLE_CASTERS);

    // Shade of Akama: Sorcerers and Channelers die first, then the other Ashtongue adds.
    Add("shade of akama", MAP_BLACK_TEMPLE, { 22841, 23421, 23215, 23216 })
        .Kill(23215, ROLE_DPS)                      // Ashtongue Sorcerer
        .Kill(23421, ROLE_DPS)                      // Ashtongue Channeler
        .Kill(23524, ROLE_DPS)                      // Ashtongue Spiritbinder
        .Kill(23523, ROLE_DPS)                      // Ashtongue Elementalist
        .Kill(23318, ROLE_DPS)                      // Ashtongue Rogue
        .Kill(23216, ROLE_DPS);                     // Ashtongue Defender

    // Teron Gorefiend: the Shadowy Constructs can only be damaged by the Vengeful Spirits,
    // a player with Shadow of Death controls his spirit and kills them.
    Add("teron gorefiend", MAP_BLACK_TEMPLE, { 22871, 23111 })
        .ResistanceAura("shadow resistance aura")
        .Spread(6.0f, ROLE_CASTERS)
        .Special(SPECIAL_TERON_SPIRITS);

    // Gurtogg Bloodboil: nobody but the tanks in front (Arcing Smash), ranged spread for Fel Geyser.
    // Bloodboil hits the 5 players furthest away: players with 2+ stacks move in, fresh players take the next ones.
    Add("gurtogg bloodboil", MAP_BLACK_TEMPLE, { 22948 })
        .AvoidFront(22948, 15.0f)
        .ApproachWithDebuff(42005, 2, 14.0f)
        .Spread(6.0f, ROLE_CASTERS);

    // Reliquary of Souls: kill the Enslaved Souls, stay out of the Soul Scream cone.
    Add("reliquary of souls", MAP_BLACK_TEMPLE, { 22856, 23418, 23419, 23420, 23469 })
        .Kill(23469, ROLE_DPS)
        .AvoidFront(23420, 20.0f)                   // Essence of Anger - Soul Scream
        .AvoidFront(23418, 10.0f)                   // Essence of Suffering
        .Spread(6.0f, ROLE_CASTERS);

    // Mother Shahraz: Fatal Attraction targets run away from each other immediately.
    Add("mother shahraz", MAP_BLACK_TEMPLE, { 22947 })
        .ResistanceAura("shadow resistance aura")
        .RunWithDebuff(41001, 18.0f)
        .AvoidFront(22947, 10.0f)
        .Special(SPECIAL_OFFTANK_NO_TAUNT | SPECIAL_STACK_OFFTANKS); // Saber Lash is split between the tanks

    // Illidari Council: leave Consecration, Flamestrike and Blizzard, stay away from Zerevor's Arcane Explosion.
    Add("illidari council", MAP_BLACK_TEMPLE, { 23426, 22949, 22950, 22951, 22952 })
        .AvoidArea(41481)                           // Flamestrike
        .AvoidArea(41482)                           // Blizzard
        .AvoidArea(41541, 0.0f, ROLE_NON_TANK)      // Consecration
        .Avoid(22950, 12.0f, ROLE_MELEE | ROLE_HEAL)// High Nethermancer Zerevor
        .Spread(8.0f, ROLE_CASTERS);

    // Illidan Stormrage: leave Flame Crash and Blaze, dodge the Eye Blast, only tanks near the Flames
    // of Azzinoth, kill Parasitic Shadowfiends and Shadow Demons on sight, spread in demon form.
    Add("illidan stormrage", MAP_BLACK_TEMPLE, { 22917, 22997, 23498, 23375 })
        .LustBelow(30.0f)
        .Kill(23375, ROLE_DPS)                      // Shadow Demon
        .Kill(23498, ROLE_DPS)                      // Parasitic Shadowfiend
        .Avoid(23336, 9.0f)                         // Flame Crash
        .Avoid(23259, 7.0f)                         // Blaze
        .Avoid(23070, 12.0f)                        // Eye Blast target
        .Avoid(23069, 5.0f)                         // Demon Fire
        .Avoid(22997, 15.0f, ROLE_NON_TANK)         // Flame of Azzinoth
        .RunWithDebuff(41917, 12.0f)                // Parasitic Shadowfiend
        .RunWithDebuff(41914, 12.0f)
        .SpreadWhileBossAura(8.0f, ROLE_NON_TANK, 40506) // demon form - Flame Burst
        .Spread(6.0f, ROLE_CASTERS);

    // ---------------------------------------------------------------------
    // Zul'Aman
    // ---------------------------------------------------------------------

    // Nalorakk: one tank for the troll form, the other one taunts on every bear form change.
    Add("nalorakk", MAP_ZULAMAN, { 23576 })
        .FormTank(23576, 42377)
        .Spread(6.0f, ROLE_CASTERS);

    // Akil'zon: stack under the Electrical Storm target, otherwise spread for Static Disruption.
    Add("akil'zon", MAP_ZULAMAN, { 23574 })
        .GatherOn(43648, 5.0f)
        .Spread(8.0f, ROLE_CASTERS);

    // Jan'alai: get out of the Fire Bombs, kill the Amani'shi Hatchers before they reach the eggs.
    Add("jan'alai", MAP_ZULAMAN, { 23578, 23818, 24504 })
        .Kill(23818, ROLE_DPS)
        .Kill(24504, ROLE_DPS)
        .Avoid(23920, 6.0f)                         // Fire Bomb
        .AvoidFront(23578, 25.0f)                   // Flame Breath
        .Spread(6.0f, ROLE_CASTERS);

    // Halazzi: kill the Corrupted Lightning Totem immediately.
    Add("halazzi", MAP_ZULAMAN, { 23577, 24143 })
        .Kill(24224, ROLE_DPS)
        .Spread(6.0f, ROLE_CASTERS);

    // Hex Lord Malacrass: Alyson Antille (healer) dies first, spread vs. Spirit Bolts splash.
    Add("hex lord malacrass", MAP_ZULAMAN, { 24239 })
        .Kill(24240, ROLE_DPS)
        .Spread(6.0f, ROLE_CASTERS);

    // Zul'jin: out of Whirlwind, Feather Vortexes and Columns of Fire.
    Add("zul'jin", MAP_ZULAMAN, { 23863 })
        .AvoidWhileAura(23863, 17207, 10.0f, ROLE_NON_TANK)
        .Avoid(24136, 7.0f)                         // Feather Vortex
        .Avoid(24187, 7.0f)                         // Column of Fire
        .Spread(8.0f, ROLE_CASTERS);

    // ---------------------------------------------------------------------
    // Sunwell Plateau
    // ---------------------------------------------------------------------

    // Kalecgos: players sent to the Spectral Realm kill Sathrovarr, the others fight the dragon.
    Add("kalecgos", MAP_SUNWELL_PLATEAU, { 24850, 24892 })
        .KillWithAura(24892, ROLE_ALL, 46021)       // Sathrovarr the Corruptor (Spectral Realm)
        .KillWithoutAura(24850, ROLE_DPS, 46021)    // Kalecgos dragon (normal realm)
        .AvoidFront(24850, 25.0f)                   // Frost Breath
        .Spread(8.0f, ROLE_CASTERS);

    // Brutallus: Burn runs away from everybody so it does not spread, the melee groups rotate soaking Meteor Slash.
    Add("brutallus", MAP_SUNWELL_PLATEAU, { 24882 })
        .ResistanceAura("fire resistance aura")
        .RunWithDebuff(45141, 12.0f)
        .SoakFront(24882, 45150, 2)
        .Spread(8.0f, ROLE_CASTERS);

    // Felmyst: stay away from Demonic Vapor and its trail, kill the Unyielding Dead.
    Add("felmyst", MAP_SUNWELL_PLATEAU, { 25038, 25268 })
        .Kill(25268, ROLE_DPS)
        .Avoid(25265, 8.0f)                         // Demonic Vapor
        .Avoid(25267, 5.0f)                         // Demonic Vapor trail
        .Spread(10.0f, ROLE_CASTERS);               // Encapsulate

    // Eredar Twins: Conflagration target runs out, leave Blaze, ranged spread.
    Add("eredar twins", MAP_SUNWELL_PLATEAU, { 25165, 25166 })
        .ResistanceAura("shadow resistance aura")
        .RunWithDebuff(45342, 12.0f)                // Conflagration
        .AvoidObject(187366, 5.0f)                  // Blaze
        .Spread(10.0f, ROLE_CASTERS);

    // M'uru / Entropius: Void Sentinels and Fury Mages first, avoid Dark Fiends, Darkness and Singularities.
    Add("m'uru", MAP_SUNWELL_PLATEAU, { 25741, 25840, 25772, 25798, 25799, 25744 })
        .ResistanceAura("shadow resistance aura")
        .Kill(25772, ROLE_DPS)                      // Void Sentinel
        .Kill(25799, ROLE_DPS)                      // Shadowsword Fury Mage
        .Kill(25798, ROLE_DPS)                      // Shadowsword Berserker
        .Kill(25744, ROLE_RANGED)                   // Dark Fiend
        .Kill(25824, ROLE_DPS)                      // Void Spawn
        .Avoid(25744, 6.0f, ROLE_NON_TANK)          // Dark Fiend
        .Avoid(25879, 9.0f)                         // Darkness
        .Avoid(25855, 12.0f)                        // Singularity
        .Spread(8.0f, ROLE_CASTERS);

    // Kil'jaeden: kill Shield Orbs and Sinister Reflections, Fire Bloom runs out, avoid Armageddon.
    Add("kil'jaeden", MAP_SUNWELL_PLATEAU, { 25315, 25502, 25708 })
        .LustBelow(25.0f).ResistanceAura("shadow resistance aura")
        .Kill(25502, ROLE_RANGED)                   // Shield Orb
        .Kill(25708, ROLE_DPS)                      // Sinister Reflection
        .Avoid(25735, 9.0f)                         // Armageddon
        .RunWithDebuff(45641, 10.0f)                // Fire Bloom
        .Spread(8.0f, ROLE_CASTERS)
        .Special(SPECIAL_BLUE_DRAGON);
}
