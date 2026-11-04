-- MangosPro: well-known WoW player and guild names for the random bots.
-- Apply to the characters database (tbccharacters) while mangosd is stopped.
--  1. Adds the names to the bot name pools so newly created bots and guilds can use them.
--  2. Renames existing random bots (race/gender fitting) and about half of the bot-led
--     guilds, because new names alone would hardly ever be drawn from the 100000 pool names.
-- Safe to run again: names that are already taken are skipped.

-- Name pool categories (RandomPlayerbotFactory::NameRaceAndGender), +1 for female:
-- 0 human/undead, 2 gnome, 4 dwarf, 6 night elf, 8 draenei, 10 orc, 12 troll, 14 tauren, 16 blood elf
DROP TEMPORARY TABLE IF EXISTS `known_names`;
CREATE TEMPORARY TABLE `known_names` (`name` varchar(12) NOT NULL PRIMARY KEY, `cat` tinyint unsigned NOT NULL);
INSERT INTO `known_names` VALUES
('Leeroy',0),('Jenkins',0),('Chucknorris',0),('Kungen',0),('Ciderhelm',0),('Asmongold',0),
('Sodapoppin',0),('Esfand',0),('Preach',0),('Tigole',0),('Furor',0),('Vurtne',0),('Reckful',0),('Payo',0),
('Swifty',2),('Wizk',2),('Venruki',2),('Chanimal',2),
('Bajheera',4),('Laintime',4),('Snutz',4),
('Awake',6),('Pikaboo',6),('Xaryu',6),('Hamlinz',6),
('Kalgan',8),('Bashiok',8),('Zarhym',8),
('Mankrik',10),('Cdew',10),('Drysc',10),
('Ziqo',12),('Pshero',12),('Kollektiv',12),
('Hydra',14),('Guzu',14),('Rhykker',14),
('Kruf',16),('Treckie',16),('Sco',16),('Whaazz',16),('Absterge',16),
('Nethaera',1),('Alinity',1),('Kaceytron',1),
('Hafu',17),('Lylirra',17);

-- 1a. Bot name pool (ids above the shipped 0..99999 range)
DELETE FROM `ai_playerbot_names` WHERE `name_id` BETWEEN 100000 AND 100999;
INSERT INTO `ai_playerbot_names` (`name_id`, `name`, `gender`)
SELECT 100000 + ROW_NUMBER() OVER (ORDER BY k.`name`), k.`name`, k.`cat`
FROM `known_names` k
WHERE NOT EXISTS (SELECT 1 FROM `ai_playerbot_names` n WHERE n.`name` = k.`name`);

-- 1b. Guild name pool
DROP TEMPORARY TABLE IF EXISTS `known_guilds`;
CREATE TEMPORARY TABLE `known_guilds` (`name` varchar(24) NOT NULL PRIMARY KEY);
INSERT INTO `known_guilds` VALUES
('Nihilum'),('Death and Taxes'),('Ensidia'),('SK Gaming'),('Paragon'),('Exodus'),('Vodka'),
('Curse'),('Elitist Jerks'),('Blood Legion'),('Serenity'),('Midwinter'),('Limit'),('Echo'),
('Complexity Limit'),('Premonition'),('Pals for Life'),('Last Resort'),('Forte'),('Wraith'),
('Exorsus'),('Afterlife'),('Nightfall'),('Apocalypse Now'),('Method');
INSERT IGNORE INTO `ai_playerbot_guild_names` (`name`) SELECT `name` FROM `known_guilds`;

-- 2a. Rename random bots: one bot per free name, matched by race/gender, never a player account's character
DROP TEMPORARY TABLE IF EXISTS `rename_bots`;
CREATE TEMPORARY TABLE `rename_bots` AS
-- human and undead share category 0: MIN keeps one bot per name
SELECT MIN(b.`guid`) AS guid, k.`name`
FROM (
    SELECT `name`, `cat`, ROW_NUMBER() OVER (PARTITION BY `cat` ORDER BY RAND()) AS rn
    FROM `known_names`
    WHERE `name` NOT IN (SELECT `name` FROM `characters`)
) k
JOIN (
    SELECT c.`guid`,
           CASE c.`race` WHEN 1 THEN 0 WHEN 5 THEN 0 WHEN 7 THEN 2 WHEN 3 THEN 4 WHEN 4 THEN 6
                         WHEN 11 THEN 8 WHEN 2 THEN 10 WHEN 8 THEN 12 WHEN 6 THEN 14 WHEN 10 THEN 16 END + c.`gender` AS cat,
           ROW_NUMBER() OVER (PARTITION BY c.`race`, c.`gender` ORDER BY RAND()) AS rn
    FROM `characters` c
    JOIN `tbcrealmd`.`account` a ON a.`id` = c.`account`
    WHERE a.`username` LIKE 'RNDBOT%'
) b ON b.`cat` = k.`cat` AND b.`rn` = k.`rn`
GROUP BY k.`name`;
UPDATE `characters` c JOIN `rename_bots` r ON r.`guid` = c.`guid` SET c.`name` = r.`name`;

-- 2b. Rename about half of the guilds that only have random bot members
DROP TEMPORARY TABLE IF EXISTS `rename_guilds`;
CREATE TEMPORARY TABLE `rename_guilds` AS
SELECT g.`guildid`, k.`name`
FROM (
    SELECT `name`, ROW_NUMBER() OVER (ORDER BY RAND()) AS rn
    FROM `known_guilds`
    WHERE `name` NOT IN (SELECT `name` FROM `guild`)
) k
JOIN (
    SELECT g.`guildid`, ROW_NUMBER() OVER (ORDER BY RAND()) AS rn
    FROM `guild` g
    WHERE NOT EXISTS (
        SELECT 1 FROM `guild_member` m
        JOIN `characters` c ON c.`guid` = m.`guid`
        JOIN `tbcrealmd`.`account` a ON a.`id` = c.`account`
        WHERE m.`guildid` = g.`guildid` AND a.`username` NOT LIKE 'RNDBOT%')
) g ON g.`rn` = k.`rn`
WHERE g.`rn` <= (SELECT CEIL(COUNT(*) / 2) FROM `guild`);
UPDATE `guild` g JOIN `rename_guilds` r ON r.`guildid` = g.`guildid` SET g.`name` = r.`name`;

SELECT r.`name` AS bot_name, c.`race`, c.`gender`, c.`level` FROM `rename_bots` r JOIN `characters` c ON c.`guid` = r.`guid` ORDER BY r.`name`;
SELECT `guildid`, `name` AS guild_name FROM `rename_guilds` ORDER BY `name`;
