<?php
declare(strict_types=1);

/** Statische Spieldaten (TBC 2.4.3): Völker, Klassen, Zonen. */
final class GameData
{
    public const RACES = [
        1 => 'Mensch', 2 => 'Orc', 3 => 'Zwerg', 4 => 'Nachtelf', 5 => 'Untoter',
        6 => 'Tauren', 7 => 'Gnom', 8 => 'Troll', 10 => 'Blutelf', 11 => 'Draenei',
    ];

    public const ALLIANCE = [1, 3, 4, 7, 11];

    public const CLASSES = [
        1 => ['Krieger', 'warrior'], 2 => ['Paladin', 'paladin'], 3 => ['Jäger', 'hunter'],
        4 => ['Schurke', 'rogue'], 5 => ['Priester', 'priest'], 7 => ['Schamane', 'shaman'],
        8 => ['Magier', 'mage'], 9 => ['Hexenmeister', 'warlock'], 11 => ['Druide', 'druid'],
    ];

    public const ZONES = [
        // Östliche Königreiche
        1 => 'Dun Morogh', 3 => 'Badlands', 4 => 'Blasted Lands', 8 => 'Swamp of Sorrows', 10 => 'Duskwood',
        11 => 'Wetlands', 12 => 'Elwynn Forest', 28 => 'Western Plaguelands', 33 => 'Stranglethorn Vale',
        36 => 'Alterac Mountains', 38 => 'Loch Modan', 40 => 'Westfall', 41 => 'Deadwind Pass',
        44 => 'Redridge Mountains', 45 => 'Arathi Highlands', 46 => 'Burning Steppes', 47 => 'The Hinterlands',
        51 => 'Searing Gorge', 85 => 'Tirisfal Glades', 130 => 'Silverpine Forest', 139 => 'Eastern Plaguelands',
        267 => 'Hillsbrad Foothills', 1497 => 'Undercity', 1519 => 'Stormwind City', 1537 => 'Ironforge',
        3430 => 'Eversong Woods', 3433 => 'Ghostlands', 3487 => 'Silvermoon City', 4080 => "Isle of Quel'Danas",
        // Kalimdor
        14 => 'Durotar', 15 => 'Dustwallow Marsh', 16 => 'Azshara', 17 => 'The Barrens', 141 => 'Teldrassil',
        148 => 'Darkshore', 215 => 'Mulgore', 331 => 'Ashenvale', 357 => 'Feralas', 361 => 'Felwood',
        400 => 'Thousand Needles', 405 => 'Desolace', 406 => 'Stonetalon Mountains', 440 => 'Tanaris',
        490 => "Un'Goro Crater", 493 => 'Moonglade', 618 => 'Winterspring', 1377 => 'Silithus',
        1637 => 'Orgrimmar', 1638 => 'Thunder Bluff', 1657 => 'Darnassus', 3524 => 'Azuremyst Isle',
        3525 => 'Bloodmyst Isle', 3557 => 'The Exodar',
        // Scherbenwelt
        3483 => 'Hellfire Peninsula', 3518 => 'Nagrand', 3519 => 'Terokkar Forest', 3520 => 'Shadowmoon Valley',
        3521 => 'Zangarmarsh', 3522 => "Blade's Edge Mountains", 3523 => 'Netherstorm', 3703 => 'Shattrath City',
        // Raids TBC
        3457 => 'Karazhan', 3923 => "Gruul's Lair", 3836 => "Magtheridon's Lair", 3607 => 'Serpentshrine Cavern',
        3845 => 'Tempest Keep', 3606 => 'Hyjal Summit', 3959 => 'Black Temple', 3805 => "Zul'Aman",
        4075 => 'Sunwell Plateau',
        // Dungeons TBC
        3562 => 'Hellfire Ramparts', 3713 => 'The Blood Furnace', 3714 => 'The Shattered Halls',
        3717 => 'The Slave Pens', 3716 => 'The Underbog', 3715 => 'The Steamvault', 3792 => 'Mana-Tombs',
        3790 => 'Auchenai Crypts', 3791 => 'Sethekk Halls', 3789 => 'Shadow Labyrinth',
        2367 => 'Old Hillsbrad Foothills', 2366 => 'The Black Morass', 3849 => 'The Mechanar',
        3847 => 'The Botanica', 3848 => 'The Arcatraz', 4131 => "Magisters' Terrace",
        // Classic-Instanzen
        2717 => 'Molten Core', 2677 => 'Blackwing Lair', 1977 => "Zul'Gurub", 3428 => "Ahn'Qiraj",
        2017 => 'Stratholme', 2057 => 'Scholomance', 1583 => 'Blackrock Spire', 1584 => 'Blackrock Depths',
        2557 => 'Dire Maul', 2100 => 'Maraudon', 1176 => "Zul'Farrak", 1581 => 'The Deadmines',
        718 => 'Wailing Caverns', 209 => 'Shadowfang Keep', 796 => 'Scarlet Monastery', 2437 => 'Ragefire Chasm',
        // Schlachtfelder / Arenen
        2597 => 'Alterac Valley', 3277 => 'Warsong Gulch', 3358 => 'Arathi Basin', 3820 => 'Eye of the Storm',
        3698 => 'Nagrand Arena', 3702 => "Blade's Edge Arena", 3968 => 'Ruins of Lordaeron',
    ];

    public static function race(int $id): string
    {
        return self::RACES[$id] ?? 'Unbekannt';
    }

    public static function className(int $id): string
    {
        return self::CLASSES[$id][0] ?? 'Unbekannt';
    }

    public static function classSlug(int $id): string
    {
        return self::CLASSES[$id][1] ?? 'unknown';
    }

    public static function faction(int $race): string
    {
        return in_array($race, self::ALLIANCE, true) ? 'alliance' : 'horde';
    }

    public static function factionName(int $race): string
    {
        return self::faction($race) === 'alliance' ? 'Allianz' : 'Horde';
    }

    public static function zone(int $id): string
    {
        return self::ZONES[$id] ?? 'Unbekannte Zone';
    }

    /** Kleines Badge mit Fraktion + farbigem Klassennamen. */
    public static function charBadge(array $c): string
    {
        return '<span class="faction-dot ' . self::faction((int)$c['race']) . '" title="' . e(self::factionName((int)$c['race'])) . '"></span>'
            . '<span class="cls-' . self::classSlug((int)$c['class']) . '">' . e($c['name']) . '</span>';
    }
}
