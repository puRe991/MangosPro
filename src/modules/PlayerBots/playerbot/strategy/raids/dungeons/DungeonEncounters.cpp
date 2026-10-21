#include "playerbot/playerbot.h"
#include "playerbot/strategy/generic/TbcRaidEncounters.h"

using namespace ai::tbcraid;

// Creature/spell IDs are those used by this core's ScriptDevAI scripts and
// creature spell lists. Normal and heroic spell variants are listed together.
// Encounters without a positioning/target-selection mechanic use the class AI.
void EncounterRegistry::RegisterDungeons()
{
    // Classic ----------------------------------------------------------------
    Add("taragaman the hungerer", 389, { 11520 }).AvoidFront(11520, 8.0f);
    Add("bazzalan", 389, { 11519 }).AvoidFront(11519, 8.0f);

    Add("lord cobrahn", 43, { 3669, 5048 }).Kill(5048).AvoidFront(3669, 10.0f);
    Add("lord pythas", 43, { 3670 }).AvoidFront(3670, 8.0f);
    Add("mutanus the devourer", 43, { 3654 }).AvoidFront(3654, 10.0f);

    Add("sneed", 36, { 642, 643 }).AvoidFront(642, 8.0f);
    Add("mr smite", 36, { 646, 657 }).Kill(657).AvoidFront(646, 8.0f);
    Add("edwin vancleef", 36, { 639, 636 }).Kill(636).AvoidFront(639, 8.0f);

    Add("commander springvale", 33, { 4278 }).AvoidFront(4278, 8.0f);
    Add("archmage arugal", 33, { 4275 }).Spread(8.0f, ROLE_CASTERS);
    Add("bazil thredd", 34, { 1716 }).AvoidFront(1716, 8.0f);
    Add("dextren ward", 34, { 1663 }).Spread(8.0f, ROLE_CASTERS);
    Add("aku'mai", 48, { 4829 }).AvoidFront(4829, 12.0f);
    Add("twilight lord kelris", 48, { 4832 }).Spread(8.0f, ROLE_CASTERS);

    Add("mekgineer thermaplugg", 90, { 7800, 7915 }).Kill(7915).Avoid(7915, 8.0f);
    Add("charlga razorflank", 47, { 4421, 4625 }).Kill(4625);
    Add("overlord ramtusk", 47, { 4420, 4437 }).Kill(4437).AvoidFront(4420, 8.0f);

    Add("herod", 189, { 3975, 6575 })
        .AvoidWhileAura(3975, 8989, 12.0f, ROLE_ALL).AvoidFront(3975, 8.0f).Kill(6575);
    Add("arcanist doan", 189, { 6487 })
        .AvoidWhileCasting(6487, 9435, 22.0f, ROLE_ALL)
        .AvoidWhileAura(6487, 9438, 22.0f, ROLE_ALL);
    Add("mograine and whitemane", 189, { 3976, 3977 }).Kill(3977).AvoidFront(3976, 8.0f);

    Add("tuten'kash", 129, { 7355 }).AvoidFront(7355, 10.0f);
    Add("amnennar the coldbringer", 129, { 7358, 8585 }).Kill(8585).Spread(8.0f, ROLE_CASTERS);
    Add("archaedas", 70, { 2748, 7076, 10120, 7309 }).Kill(7076).Kill(10120).Kill(7309);
    Add("grimlok", 70, { 4854, 4851 }).Kill(4851).AvoidFront(4854, 8.0f);
    Add("antusul", 209, { 8127, 8156 }).Kill(8156);
    Add("witch doctor zum'rah", 209, { 7271, 7286 }).Kill(7286);
    Add("nekrum and sezz'ziz", 209, { 7796, 7275 }).Kill(7275);
    Add("gahz'rilla", 209, { 7273 }).AvoidFront(7273, 12.0f);

    Add("noxxion", 349, { 13282, 13456 }).Kill(13456);
    Add("princess theradras", 349, { 12201 }).AvoidFront(12201, 10.0f);
    Add("celebras the cursed", 349, { 12225, 12224 }).Kill(12224);
    Add("jammal'an and ogom", 109, { 5710, 5711 }).Kill(5710);
    Add("shade of eranikus", 109, { 5709 }).AvoidFront(5709, 18.0f);
    Add("avatar of hakkar", 109, { 8443, 8437, 8438 }).Kill(8438).Kill(8437).AvoidFront(8443, 12.0f);

    Add("general angerforge", 230, { 9033, 8894, 8893 }).Kill(8894).Kill(8893).AvoidFront(9033, 8.0f);
    Add("ambassador flamelash", 230, { 9156, 9178 }).Kill(9178);
    // Leave Moira alive for the princess quest. She is deliberately not a priority target.
    Add("emperor dagran thaurissan", 230, { 9019, 8929 }).Kill(9019).AvoidFront(9019, 8.0f);
    Add("magmus", 230, { 9938 }).AvoidFront(9938, 15.0f);

    Add("quartermaster zigris", 229, { 9736, 9696 }).Kill(9696);
    Add("shadow hunter vosh'gajin", 229, { 9236, 9239 }).Kill(9239);
    Add("overlord wyrmthalak", 229, { 9568, 9268, 9269 }).Kill(9269).Kill(9268).AvoidFront(9568, 12.0f);
    Add("gyth and rend", 229, { 10339, 10429 }).AvoidFront(10339, 18.0f).AvoidFront(10429, 10.0f);
    Add("the beast", 229, { 10430 }).AvoidFront(10430, 15.0f);
    Add("general drakkisath", 229, { 10363, 10319 }).Kill(10319).AvoidFront(10363, 18.0f);

    Add("alzzin the wildshaper", 429, { 11492, 11460 }).Kill(11460);
    Add("immol'thar", 429, { 11496, 14396 }).Kill(14396).AvoidFront(11496, 12.0f);
    // Tribute: focus Gordok and preserve Cho'Rush; never pull skipped guards.
    Add("king gordok", 429, { 11501, 14324 }).Kill(11501).AvoidFront(11501, 10.0f);
    Add("prince tortheldrin", 429, { 11486 }).AvoidFront(11486, 8.0f);

    Add("rattlegore", 289, { 11622 }).AvoidFront(11622, 10.0f);
    Add("ras frostwhisper", 289, { 10508 }).Spread(8.0f, ROLE_CASTERS);
    Add("darkmaster gandling", 289, { 1853, 11598 }).Kill(11598);
    Add("alexei barov", 289, { 10504, 10488 }).Kill(10488);
    Add("nerub'enkan", 329, { 10437, 10876 }).Kill(10876);
    Add("ramstein the gorger", 329, { 10439, 11030 }).Kill(11030).AvoidFront(10439, 10.0f);
    Add("baron rivendare", 329, { 10440, 11197 }).Kill(11197).AvoidFront(10440, 10.0f);
    Add("balnazzar", 329, { 10813 }).Spread(8.0f, ROLE_CASTERS);

    // TBC --------------------------------------------------------------------
    Add("watchkeeper gargolmar", 543, { 17306, 17309 }).Kill(17309).AvoidFront(17306, 10.0f);
    Add("omor the unscarred", 543, { 17308, 17540 })
        .RunWithDebuff(30695, 16.0f).RunWithDebuff(37566, 16.0f).Kill(17540);
    Add("nazan and vazruden", 543, { 17536, 17537 })
        .AvoidObject(180125, 5.0f).AvoidObject(182533, 5.0f).AvoidFront(17536, 20.0f).Kill(17537);

    Add("the maker", 542, { 17381 }).AvoidFront(17381, 12.0f);
    Add("broggok", 542, { 17380, 17662 }).Avoid(17662, 7.0f).AvoidFront(17380, 12.0f);
    Add("kelidan the breaker", 542, { 17377, 17653 })
        .Kill(17653).AvoidWhileCasting(17377, 30940, 22.0f, ROLE_ALL)
        .AvoidWhileCasting(17377, 33775, 22.0f, ROLE_ALL).AvoidWhileCasting(17377, 37371, 22.0f, ROLE_ALL)
        .AvoidWhileAura(17377, 30940, 22.0f, ROLE_ALL);

    Add("grand warlock nethekurse", 540, { 16807 })
        .Avoid(17471, 7.0f).AvoidFront(16807, 15.0f);
    Add("warbringer omrogg", 540, { 16809 }).Spread(8.0f, ROLE_CASTERS).AvoidFront(16809, 12.0f);
    Add("warchief kargath bladefist", 540, { 16808, 17621, 17622, 17623 })
        .Kill(17623).Kill(17622).Kill(17621).Spread(8.0f, ROLE_CASTERS);

    Add("mennu the betrayer", 547, { 17941, 20208, 18176, 18177, 18179 })
        .Kill(20208).Kill(18176).Kill(18177).Avoid(18179, 12.0f);
    Add("rokmar the crackler", 547, { 17991 }).AvoidFront(17991, 12.0f);
    Add("quagmirran", 547, { 17942 }).AvoidFront(17942, 20.0f);
    Add("hungarfen", 546, { 17770 })
        .Avoid(17990, 8.0f).AvoidWhileAura(17770, 31673, 20.0f, ROLE_ALL)
        .AvoidWhileCasting(17770, 31673, 20.0f, ROLE_ALL);
    Add("ghazan", 546, { 18105 }).AvoidFront(18105, 18.0f);
    Add("swamplord muselek", 546, { 17826, 17827 }).Kill(17827).Spread(8.0f, ROLE_CASTERS);
    Add("the black stalker", 546, { 17882, 22299 })
        .Kill(22299).RunWithDebuff(31715, 12.0f).Spread(8.0f, ROLE_CASTERS);
    Add("hydromancer thespia", 545, { 17797, 17917 })
        .AvoidArea(25033).Kill(17917).Spread(8.0f, ROLE_CASTERS);
    Add("mekgineer steamrigger", 545, { 17796, 17951 }).Kill(17951).AvoidFront(17796, 10.0f);
    Add("warlord kalithresh", 545, { 17798, 17954 }).Kill(17954).AvoidFront(17798, 10.0f);

    Add("pandemonius", 557, { 18341 }).Spread(8.0f, ROLE_CASTERS);
    Add("tavarok", 557, { 18343 }).AvoidFront(18343, 12.0f);
    Add("nexus prince shaffar", 557, { 18344, 18431, 18430 }).Kill(18431).Kill(18430);
    Add("shirrak the dead watcher", 558, { 18371 }).Avoid(18374, 12.0f).Avoid(18371, 25.0f, ROLE_CASTERS);
    Add("exarch maladaar", 558, { 18373, 18441, 18478 }).Kill(18441).Kill(18478);
    Add("darkweaver syth", 556, { 18472, 19203, 19204, 19205, 19206 })
        .Kill(19203).Kill(19204).Kill(19205).Kill(19206).Spread(8.0f, ROLE_CASTERS);
    // Arcane Explosion is avoided by breaking line of sight, see DungeonBossSpecialAction.
    Add("talon king ikiss", 556, { 18473 }).Spread(8.0f, ROLE_CASTERS);
    Add("anzu", 556, { 23035, 23132 }).Kill(23132);
    Add("ambassador hellmaw", 555, { 18731 }).AvoidFront(18731, 18.0f);
    Add("blackheart the inciter", 555, { 18667 }).Spread(10.0f, ROLE_CASTERS);
    Add("grandmaster vorpil", 555, { 18732, 19226 })
        .Kill(19226).AvoidArea(33617).AvoidArea(39363);
    Add("murmur", 555, { 18708 })
        .AvoidWhileCasting(18708, 33923, 28.0f, ROLE_ALL)
        .AvoidWhileCasting(18708, 38796, 28.0f, ROLE_ALL)
        .RunWithDebuff(33711, 20.0f).RunWithDebuff(38794, 20.0f);

    Add("lieutenant drake", 560, { 17848 }).AvoidWhileAura(17848, 31909, 12.0f, ROLE_ALL).AvoidFront(17848, 10.0f);
    Add("captain skarloc", 560, { 17862, 17833, 17860 }).Kill(17833).Kill(17860).AvoidFront(17862, 10.0f);
    Add("epoch hunter", 560, { 18096, 18170, 18171, 18172 })
        .Kill(18171).Kill(18172).Kill(18170).AvoidFront(18096, 20.0f);
    Add("chrono lord deja", 269, { 17879 }).Spread(8.0f, ROLE_CASTERS);
    Add("temporus", 269, { 17880 }).AvoidFront(17880, 15.0f);
    Add("aeonus", 269, { 17881 }).AvoidFront(17881, 20.0f);
    // Adds must not reach Medivh; this also covers the portals between boss waves.
    Add("black morass rift", 269, { 17839, 21104, 21140, 21148, 17835, 21137, 17892, 21136, 18994, 21138, 18995, 21139, 21818 })
        .Kill(17835).Kill(21137).Kill(17892).Kill(21136).Kill(18994).Kill(21138).Kill(18995).Kill(21139).Kill(21818);

    Add("mechano lord capacitus", 554, { 19219 })
        .Avoid(20405, 8.0f).Spread(12.0f, ROLE_ALL);
    // Same entry, chase condition and distance as the existing Sepethrea strategy.
    Add("nethermancer sepethrea", 554, { 19221 }).Kite(20481, 21.0f, ROLE_ALL);
    Add("pathaleon the calculator", 554, { 19220, 21062 }).Kill(21062).Spread(8.0f, ROLE_CASTERS);
    Add("commander sarannis", 553, { 17976, 20078, 20083 }).Kill(20083).Kill(20078);
    Add("high botanist freywinn", 553, { 17975, 19953 }).Kill(19953);
    Add("thorngrin the tender", 553, { 17978 })
        .AvoidWhileCasting(17978, 34659, 18.0f, ROLE_ALL).AvoidWhileCasting(17978, 39131, 18.0f, ROLE_ALL)
        .AvoidWhileAura(17978, 34659, 18.0f, ROLE_ALL).AvoidWhileAura(17978, 39131, 18.0f, ROLE_ALL);
    Add("laj", 553, { 17980, 19919, 19920 }).Kill(19920).Kill(19919);
    Add("warp splinter", 553, { 17977, 19949 }).Kill(19949);

    Add("zereketh the unbound", 552, { 20870 })
        .Avoid(21101, 8.0f).RunWithDebuff(36123, 12.0f).Spread(8.0f, ROLE_CASTERS);
    Add("dalliah the doomsayer", 552, { 20885 }).AvoidWhileAura(20885, 36142, 12.0f, ROLE_ALL);
    Add("wrath scryer soccothrates", 552, { 20886 })
        .Avoid(20978, 7.0f).AvoidFront(20886, 15.0f).Spread(8.0f, ROLE_CASTERS);
    Add("harbinger skyriss", 552, { 20912, 21466, 21467 }).Kill(21466).Kill(21467).Spread(8.0f, ROLE_CASTERS);

    Add("selin fireheart", 585, { 24723, 24722 }).Kill(24722);
    Add("vexallus", 585, { 24744, 24745 }).Kill(24745).Spread(10.0f, ROLE_CASTERS);
    Add("priestess delrissa", 585, { 24560, 24557, 24558, 24554, 24561, 24559, 24555, 24553, 24556 })
        .Kill(24560).Kill(24553).Kill(24558).Kill(24561).AvoidArea(44178).AvoidArea(46195);
    Add("kaelthas sunstrider", 585, { 24664, 24675, 24674 })
        .Kill(24675).Kite(24674, 12.0f).Avoid(24666, 8.0f).Avoid(24708, 8.0f)
        .Spread(8.0f, ROLE_CASTERS);

    // Classic raids ----------------------------------------------------------
    Add("lucifron", 409, { 12118, 12119 }).Kill(12119).AvoidFront(12118, 10.0f);
    Add("magmadar", 409, { 11982 }).AvoidObject(177704, 8.0f).AvoidFront(11982, 20.0f);
    Add("gehennas", 409, { 12259, 11661 }).Kill(11661).AvoidArea(19717);
    Add("garr", 409, { 12057, 12099 }).Kill(12099).Spread(10.0f, ROLE_CASTERS);
    Add("baron geddon", 409, { 12056 })
        .RunWithDebuff(20475, 25.0f).AvoidWhileAura(12056, 19695, 25.0f, ROLE_ALL);
    Add("shazzrah", 409, { 12264 }).Avoid(12264, 22.0f, ROLE_CASTERS).Spread(10.0f, ROLE_CASTERS);
    Add("sulfuron harbinger", 409, { 12098, 11662 }).Kill(11662).AvoidFront(12098, 10.0f);
    Add("golemagg the incinerator", 409, { 11988, 11672 }).Kill(11988).AvoidFront(11988, 15.0f);
    Add("majordomo executus", 409, { 12018, 11663, 11664 }).Kill(11663).Kill(11664);
    Add("ragnaros", 409, { 11502, 12143 }).Kill(12143).Spread(10.0f, ROLE_CASTERS).AvoidFront(11502, 20.0f);

    Add("razorgore", 469, { 12435, 12557, 14456, 12420, 12416, 12422 })
        .Kill(12557).Kill(12420).Kill(12416).Kill(12422).AvoidFront(12435, 15.0f);
    Add("vaelastrasz", 469, { 13020 }).RunWithDebuff(23620, 25.0f).AvoidFront(13020, 25.0f);
    Add("broodlord lashlayer", 469, { 12017 }).AvoidFront(12017, 15.0f);
    Add("firemaw", 469, { 11983 }).AvoidFront(11983, 25.0f).TankSwap(11983, 23341, 5);
    Add("ebonroc", 469, { 14601 }).AvoidFront(14601, 25.0f).TankSwap(14601, 23340, 1);
    Add("flamegor", 469, { 11981 }).AvoidFront(11981, 25.0f);
    Add("chromaggus", 469, { 14020 }).AvoidFront(14020, 30.0f);
    Add("nefarian", 469, { 11583, 10162, 14261, 14262, 14263, 14264, 14265, 14302, 14605 })
        .Kill(14302).Kill(14261).Kill(14262).Kill(14263).Kill(14264).Kill(14265).Kill(14605)
        .AvoidFront(11583, 30.0f);

    Add("onyxia", 249, { 10184, 11262 }).Kill(11262).AvoidFront(10184, 30.0f).Spread(10.0f, ROLE_CASTERS);

    Add("kurinnaxx", 509, { 15348 }).AvoidObject(180647, 7.0f).TankSwap(15348, 25646, 3).AvoidFront(15348, 12.0f);
    Add("general rajaxx", 509, { 15341, 15385, 15388, 15386, 15390, 15389, 15392, 15391, 15387, 15344 })
        .Kill(15344).Kill(15387).AvoidFront(15341, 15.0f);
    Add("moam", 509, { 15340, 15527 }).Kill(15527).Spread(10.0f, ROLE_CASTERS);
    Add("buru the gorger", 509, { 15370, 15514, 15521 }).Kill(15521).AvoidFront(15370, 10.0f);
    Add("ayamiss the hunter", 509, { 15369, 15555, 15546 }).Kill(15555, ROLE_ALL).Kill(15546).Spread(8.0f, ROLE_CASTERS);
    Add("ossirian the unscarred", 509, { 15339 }).Avoid(15428, 12.0f).AvoidFront(15339, 15.0f);

    Add("anub'rekhan", 533, { 15956, 16573, 16698 })
        .Kill(16573).Kill(16698).AvoidWhileAura(15956, 28785, 30.0f, ROLE_ALL).Spread(8.0f, ROLE_CASTERS);
    // Preserve Worshippers for Widow's Embrace; only Followers are a kill priority.
    Add("grand widow faerlina", 533, { 15953, 16505, 16506 }).Kill(16505).AvoidArea(28794);
    Add("maexxna", 533, { 15952, 16486, 17055 }).Kill(16486, ROLE_ALL).Kill(17055).AvoidFront(15952, 15.0f);
    Add("noth the plaguebringer", 533, { 15954, 16984, 16983, 16981 }).Kill(16984).Kill(16983).Kill(16981);
    Add("heigan the unclean", 533, { 15936 }).AvoidWhileAura(15936, 29350, 25.0f, ROLE_ALL);
    // Spores are assigned by the raid leader; do not automatically kill them near tanks.
    Add("loatheb", 533, { 16011 }).Spread(8.0f, ROLE_CASTERS);
    Add("instructor razuvious", 533, { 16061, 16803 }).AvoidFront(16061, 15.0f);
    Add("gothik the harvester", 533, { 16060, 16124, 16125, 16126, 16127, 16148, 16150, 16149 })
        .Kill(16126).Kill(16150).Kill(16125).Kill(16148).Kill(16124).Kill(16127).Kill(16149);
    Add("the four horsemen", 533, { 16062, 16063, 16064, 16065 })
        .AvoidArea(28863).Spread(8.0f, ROLE_CASTERS);
    Add("patchwerk", 533, { 16028 }).AvoidFront(16028, 10.0f, ROLE_CASTERS);
    Add("grobbulus", 533, { 15931, 16290 })
        .RunWithDebuff(28169, 20.0f).AvoidArea(28240).Kill(16290).AvoidFront(15931, 20.0f);
    Add("gluth", 533, { 15932, 16360 }).TankSwap(15932, 25646, 3)
        .Kite(16360, 20.0f, ROLE_NON_TANK).AvoidFront(15932, 15.0f);
    Add("thaddius", 533, { 15928, 15929, 15930 }).Spread(12.0f, ROLE_NON_TANK);
    Add("sapphiron", 533, { 15989 }).AvoidArea(28547).AvoidFront(15989, 30.0f);
    Add("kel'thuzad", 533, { 15990, 16427, 16428, 16429 })
        .Kill(16429, ROLE_RANGED).Kill(16427, ROLE_RANGED).Kill(16428).Avoid(16129, 7.0f)
        .RunWithDebuff(27819, 20.0f).Spread(12.0f, ROLE_NON_TANK);
}
