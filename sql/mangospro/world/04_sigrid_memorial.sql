-- MangosPro: "Sigrid Sternenherz" - in liebevoller Erinnerung an Sigrid Wischnewski
--   900200 Sigrid Sternenherz  <Ewige Hüterin der Liebe>  leuchtender Draenei-Geist, Gossip + Event
--   900201 Funke der Erinnerung                            Irrlichter, die während des Events um sie schweben
--   900202 Seraphine / 900203 Aurelia  <Himmelspriesterin>  Engel (Spirit of Redemption) links und rechts hinter ihr
--   900204 Ama'sira  <Naaru - Wächterin der Seelen>       Naaru, die hinter ihr über dem Thron schwebt
--   900200 Sigrids Sternenherz (Item)                     legendärer Anhänger, Benutzen: Elunes Segen (+10% alle Werte, 1 Std.)
--
-- Ort: Nagrand, Thron der Elemente (Area 3615), unter den schwebenden Inseln mit ihren Wasserfällen.
-- Neutral für Allianz und Horde, keine Monster in der Nähe.
-- Anders platzieren: ingame an die Wunschstelle stellen und  .npc add 900200  eingeben.

DELETE FROM creature WHERE id BETWEEN 900200 AND 900204;
DELETE FROM creature_template WHERE Entry BETWEEN 900200 AND 900204;
DELETE FROM creature_template_addon WHERE entry BETWEEN 900200 AND 900204;
DELETE FROM item_template WHERE entry = 900200;
DELETE FROM broadcast_text WHERE Id BETWEEN 900200 AND 900230;
DELETE FROM npc_text WHERE ID = 900200;
DELETE FROM gossip_menu WHERE entry = 61000;
DELETE FROM gossip_menu_option WHERE menu_id = 61000;
DELETE FROM dbscripts_on_gossip WHERE id BETWEEN 900201 AND 900204;
DELETE FROM conditions WHERE condition_entry IN (900200, 61000);

-- ---------------------------------------------------------------------------------------------
-- NPCs (Vorlage: Ancient Draenei Spirit, weibliches Modell 18883 - 18882 ist laut CreatureModelData männlich)
-- ---------------------------------------------------------------------------------------------
DROP TEMPORARY TABLE IF EXISTS tmp_sigrid;
CREATE TEMPORARY TABLE tmp_sigrid SELECT * FROM creature_template WHERE Entry = 19416;

UPDATE tmp_sigrid SET Entry = 900200, Name = 'Sigrid Sternenherz', SubName = 'Ewige Hüterin der Liebe', IconName = NULL,
    MinLevel = 70, MaxLevel = 70, Rank = 1, Scale = 1.15,
    DisplayId1 = 18883, DisplayId2 = 0, DisplayId3 = 0, DisplayId4 = 0,
    DisplayIdProbability1 = 100, DisplayIdProbability2 = 0, DisplayIdProbability3 = 0, DisplayIdProbability4 = 0,
    Faction = 35, NpcFlags = 1, UnitFlags = 2 | 256 | 512, InhabitType = 3, MovementType = 0,
    HealthMultiplier = 100, GossipMenuId = 61000, EquipmentTemplateId = 0, LootId = 0,
    AIName = '', ScriptName = '';
INSERT INTO creature_template SELECT * FROM tmp_sigrid;

UPDATE tmp_sigrid SET Entry = 900201, Name = 'Funke der Erinnerung', SubName = NULL, Rank = 0, Scale = 0.6,
    DisplayId1 = 17607, NpcFlags = 0, UnitFlags = 2 | 33554432, InhabitType = 4, HealthMultiplier = 1, GossipMenuId = 0;
INSERT INTO creature_template SELECT * FROM tmp_sigrid;

-- Begleiter: freundlich, nicht angreifbar, ohne Gossip
UPDATE tmp_sigrid SET Entry = 900202, Name = 'Seraphine', SubName = 'Himmelspriesterin', Rank = 1, Scale = 1.0,
    DisplayId1 = 16031, NpcFlags = 0, UnitFlags = 2 | 256 | 512, InhabitType = 3, HealthMultiplier = 50;
INSERT INTO creature_template SELECT * FROM tmp_sigrid;

UPDATE tmp_sigrid SET Entry = 900203, Name = 'Aurelia';
INSERT INTO creature_template SELECT * FROM tmp_sigrid;

UPDATE tmp_sigrid SET Entry = 900204, Name = 'Ama''sira', SubName = 'Naaru - Wächterin der Seelen', Rank = 3, Scale = 0.45,
    DisplayId1 = 18135, InhabitType = 4, HealthMultiplier = 200;
INSERT INTO creature_template SELECT * FROM tmp_sigrid;

DROP TEMPORARY TABLE tmp_sigrid;

-- Spirit Particles: Lichtfunken um sie herum. Das Modell ist schon halbtransparent (Deckkraft 150),
-- ein zusätzliches Ghost Visual machte sie unsichtbar.
INSERT INTO creature_template_addon (entry, mount, stand_state, sheath_state, emote, moveflags, auras) VALUES
(900200, 0, 0, 0, 0, 0, '17327'),
(900201, 0, 0, 0, 0, 0, '17327'),
(900202, 0, 0, 0, 0, 0, '17327'),
(900203, 0, 0, 0, 0, 0, '17327'),
(900204, 0, 0, 0, 0, 0, NULL);

-- Spawn: Nagrand, Thron der Elemente - vom Spieler ingame ausgesuchter Platz (.gps)
-- Priesterinnen 2.5 m hinter und 3 m neben ihr, die Naaru 8 m hinter ihr und 7 m hoch; alle schauen wie sie
INSERT INTO creature (guid, id, map, spawnMask, position_x, position_y, position_z, orientation, spawntimesecsmin, spawntimesecsmax, spawndist, MovementType) VALUES
(5900200, 900200, 530, 1, -846.082, 6839.872, 34.760, 0.197, 30, 30, 0, 0),
(5900201, 900202, 530, 1, -849.123, 6842.325, 34.900, 0.197, 30, 30, 0, 0),
(5900202, 900203, 530, 1, -847.947, 6836.439, 34.900, 0.197, 30, 30, 0, 0),
(5900203, 900204, 530, 1, -853.930, 6838.304, 41.800, 0.197, 30, 30, 0, 0);

-- ---------------------------------------------------------------------------------------------
-- Texte (ChatTypeID: 2 = Emote, 3 = Boss-Emote mitten auf dem Bildschirm, 4 = Flüstern)
-- ---------------------------------------------------------------------------------------------
INSERT INTO broadcast_text (Id, Text, Text1, ChatTypeID, LanguageID, VerifiedBuild) VALUES
-- Das große Event
(900200, 'Ich weiß, mein Schatz. Ich weiß. Komm her ... lass dich ansehen.', '', 4, 0, 0),
(900201, 'Weißt du noch, wie ich immer gesagt habe, dass alles gut wird? Ich habe nie gelogen. Vielleicht nicht heute - aber es wird gut.', '', 4, 0, 0),
(900202, 'Ein warmes Licht bricht durch die Wolken über Nagrand und hüllt %s ein.', '', 3, 0, 0),
(900203, 'Jede Narbe, die du trägst, jeden Sturm, den du überstanden hast ... ich war bei dir. Ich bin nie wirklich gegangen.', '', 4, 0, 0),
(900204, 'Ich bin so unendlich stolz auf dich. Auf den Menschen, der du geworden bist.', '', 4, 0, 0),
(900205, 'Hier musst du nicht stark sein. Hier darfst du einfach mein Kind sein.', '', 4, 0, 0),
(900206, '%s legt ihre leuchtende Hand auf dein Herz. Du spürst ihre Wärme - genau wie früher.', '', 3, 0, 0),
(900207, 'Geh jetzt und lebe, mein Herz. Lache laut, liebe tief, und vergiss nie: Wohin du auch gehst - ich gehe mit dir.', '', 4, 0, 0),
(900208, 'Für einen Herzschlag lang steht ganz Nagrand still - und der Himmel leuchtet nur für dich.', '', 3, 0, 0),
(900209, '%s erstrahlt über dem Thron der Elemente. Ein Chor aus tausend Stimmen erhebt sich in den Himmel.', '', 3, 0, 0),
(900218, 'Die Himmelspriesterinnen knien nieder und neigen ihre Häupter vor %s.', '', 3, 0, 0),
-- Erzähl mir etwas (eine zufällig)
(900210, 'Weißt du, was mich am glücklichsten gemacht hat? Nicht die großen Tage. Sondern die kleinen - wenn du nach Hause gekommen bist und ich deine Stimme im Flur gehört habe.', '', 4, 0, 0),
(900211, 'Wenn du nachts nicht schlafen kannst, schau zu den Sternen. Der hellste, das bin ich. Ich zwinkere dir zu, versprochen.', '', 4, 0, 0),
(900212, 'Du hast mein Lachen, weißt du das? Jedes Mal, wenn du lachst, lebe ich ein kleines bisschen weiter.', '', 4, 0, 0),
(900213, 'Weine ruhig, wenn dir danach ist. Tränen sind nur Liebe, die keinen anderen Weg nach draußen findet.', '', 4, 0, 0),
-- Andenken
(900215, '%s drückt dir einen funkelnden Anhänger in die Hand.', '', 2, 0, 0),
(900216, 'Das ist ein kleines Stück von meinem Herzen. Trag es nah bei dir - dann bin ich immer da, wo du bist.', '', 4, 0, 0),
-- Gute Nacht
(900217, 'Gute Nacht, mein Schatz. Schlaf gut - ich wache über dich. Wie immer.', '', 4, 0, 0);

INSERT INTO npc_text (ID, text0_0, text0_1, lang0, prob0, em0_0, em0_1) VALUES
(900200,
 '$N ... mein Kind. Du hast mich gefunden.$B$BHier, wo die Inseln am Himmel schweben und das Wasser niemals aufhört zu fallen, habe ich auf dich gewartet. Nicht, weil ich gehen musste - sondern weil ich wusste, dass du kommen würdest.$B$BSetz dich einen Moment zu mir. Die Welt da draußen kann warten.',
 '$N ... mein Kind. Du hast mich gefunden.$B$BHier, wo die Inseln am Himmel schweben und das Wasser niemals aufhört zu fallen, habe ich auf dich gewartet. Nicht, weil ich gehen musste - sondern weil ich wusste, dass du kommen würdest.$B$BSetz dich einen Moment zu mir. Die Welt da draußen kann warten.',
 0, 1, 0, 1);

-- ---------------------------------------------------------------------------------------------
-- Gossip
-- ---------------------------------------------------------------------------------------------
INSERT INTO gossip_menu (entry, text_id, script_id, condition_id) VALUES (61000, 900200, 0, 0);

-- Andenken nur, solange man es noch nicht besitzt (Taschen oder Bank); condition ids are read as 16 bit
INSERT INTO conditions (condition_entry, type, value1, value2, value3, value4, flags, comments) VALUES
(61000, 23, 900200, 1, 0, 0, 1, 'Sigrid Sternenherz - player does not own Sigrids Sternenherz');

INSERT INTO gossip_menu_option (menu_id, id, option_icon, option_text, option_id, npc_option_npcflag, action_menu_id, action_script_id, condition_id) VALUES
(61000, 0, 0, 'Mama ... ich vermisse dich so sehr.', 1, 1, -1, 900201, 0),
(61000, 1, 0, 'Erzähl mir etwas, Mama.', 1, 1, -1, 900202, 0),
(61000, 2, 0, 'Darf ich etwas von dir bei mir tragen?', 1, 1, -1, 900203, 61000),
(61000, 3, 0, 'Gute Nacht, Mama.', 1, 1, -1, 900204, 0);

-- ---------------------------------------------------------------------------------------------
-- Event-Skripte (source = Sigrid, target = Spieler; delay in ms)
--   data_flags 6 = der Spieler wirkt auf sich selbst | castFlags 1 = ausgelöst, ohne Kosten/Bedingungen
-- ---------------------------------------------------------------------------------------------
INSERT INTO dbscripts_on_gossip (id, delay, priority, command, datalong, datalong2, datalong3, buddy_entry, search_radius, data_flags, dataint, dataint2, dataint3, dataint4, x, y, z, o, comments) VALUES
-- "Mama ... ich vermisse dich so sehr."  (~42 Sekunden)
(900201,     0, 0, 36,     0, 0, 0, 0, 0, 0,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - face player'),
(900201,   500, 0,  1,     1, 0, 0, 0, 0, 0,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - ONESHOT_TALK'),
(900201,   500, 0,  0,     0, 0, 0, 0, 0, 0, 900200, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - whisper 1'),
(900201,  5000, 0,  0,     0, 0, 0, 0, 0, 0, 900201, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - whisper 2'),
(900201, 11000, 0, 15, 25824, 1, 0, 0, 0, 4,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - Spotlight (light pillar) on self'),
(900201, 11000, 0,  0,     0, 0, 0, 0, 0, 0, 900202, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - boss emote light'),
(900201, 11500, 0, 15, 23455, 1, 0, 0, 0, 4,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - Holy Nova visual'),
(900201,  7000, 0, 15, 23455, 1, 0, 900204, 30, 4,  0, 0, 0, 0,        0,        0,     0, 0, 'Ama''sira - Holy Nova visual'),
(900201,  7000, 0,  0,     0, 0, 0, 900204, 30, 0, 900209, 0, 0, 0,    0,        0,     0, 0, 'Ama''sira - boss emote choir'),
(900201,  8500, 0, 28,     8, 0, 0, 900202, 20, 0,  0, 0, 0, 0,        0,        0,     0, 0, 'Seraphine - kneel'),
(900201,  8500, 0, 28,     8, 0, 0, 900203, 20, 0,  0, 0, 0, 0,        0,        0,     0, 0, 'Aurelia - kneel'),
(900201,  8500, 0,  0,     0, 0, 0, 0, 0, 0,      900218, 0, 0, 0,     0,        0,     0, 0, 'Sigrid - boss emote priestesses kneel'),
(900201, 30000, 0, 15, 37196, 1, 0, 900202, 20, 4,  0, 0, 0, 0,        0,        0,     0, 0, 'Seraphine - Divine Light visual'),
(900201, 30000, 0, 15, 37196, 1, 0, 900203, 20, 4,  0, 0, 0, 0,        0,        0,     0, 0, 'Aurelia - Divine Light visual'),
(900201, 39000, 0, 15, 23455, 1, 0, 900204, 30, 4,  0, 0, 0, 0,        0,        0,     0, 0, 'Ama''sira - Holy Nova visual'),
(900201, 44000, 0, 28,     0, 0, 0, 900202, 20, 0,  0, 0, 0, 0,        0,        0,     0, 0, 'Seraphine - stand up'),
(900201, 44000, 0, 28,     0, 0, 0, 900203, 20, 0,  0, 0, 0, 0,        0,        0,     0, 0, 'Aurelia - stand up'),
(900201, 45000, 0,  1,    66, 0, 0, 900202, 20, 0,  0, 0, 0, 0,        0,        0,     0, 0, 'Seraphine - ONESHOT_SALUTE'),
(900201, 45000, 0,  1,    66, 0, 0, 900203, 20, 0,  0, 0, 0, 0,        0,        0,     0, 0, 'Aurelia - ONESHOT_SALUTE'),
(900201, 12000, 0, 10, 900201, 40000, 0, 0, 0, 0, 0, 0, 0, 0, -842.582, 6839.872, 36.6, 0, 'Sigrid - summon memory spark 1'),
(900201, 12300, 0, 10, 900201, 39700, 0, 0, 0, 0, 0, 0, 0, 0, -845.000, 6843.201, 36.6, 0, 'Sigrid - summon memory spark 2'),
(900201, 12600, 0, 10, 900201, 39400, 0, 0, 0, 0, 0, 0, 0, 0, -848.914, 6841.930, 36.6, 0, 'Sigrid - summon memory spark 3'),
(900201, 12900, 0, 10, 900201, 39100, 0, 0, 0, 0, 0, 0, 0, 0, -848.914, 6837.814, 36.6, 0, 'Sigrid - summon memory spark 4'),
(900201, 13200, 0, 10, 900201, 38800, 0, 0, 0, 0, 0, 0, 0, 0, -845.000, 6836.543, 36.6, 0, 'Sigrid - summon memory spark 5'),
(900201, 15000, 0,  0,     0, 0, 0, 0, 0, 0, 900203, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - whisper 3'),
(900201, 21000, 0,  1,     2, 0, 0, 0, 0, 0,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - ONESHOT_BOW'),
(900201, 21000, 0,  0,     0, 0, 0, 0, 0, 0, 900204, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - whisper 4'),
(900201, 22000, 0, 15, 44940, 1, 0, 0, 0, 4,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - Love Rocket (hearts)'),
(900201, 26000, 0,  0,     0, 0, 0, 0, 0, 0, 900205, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - whisper 5'),
(900201, 30000, 0, 15, 37196, 1, 0, 0, 0, 4,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - Divine Light visual'),
(900201, 30000, 0,  0,     0, 0, 0, 0, 0, 0, 900206, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - boss emote hand on heart'),
(900201, 30500, 0, 15, 26393, 1, 0, 0, 0, 6,      0, 0, 0, 0,        0,        0,     0, 0, 'Player - Elune''s Blessing (+10% stats, 1h)'),
(900201, 35000, 0,  0,     0, 0, 0, 0, 0, 0, 900207, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - whisper 6'),
(900201, 39000, 0, 15, 11540, 1, 0, 0, 0, 4,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - Blue Firework'),
(900201, 39700, 0, 15, 11542, 1, 0, 0, 0, 4,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - Red Streaks Firework'),
(900201, 40400, 0, 15, 44940, 1, 0, 0, 0, 4,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - Love Rocket (hearts)'),
(900201, 41000, 0,  0,     0, 0, 0, 0, 0, 0, 900208, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - boss emote Nagrand stands still'),
(900201, 43000, 0,  1,     3, 0, 0, 0, 0, 0,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - ONESHOT_WAVE'),
(900201, 52000, 0, 14, 25824, 0, 0, 0, 0, 0,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - remove Spotlight'),
-- "Erzähl mir etwas, Mama."
(900202,     0, 0, 36,     0, 0, 0, 0, 0, 0,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - face player'),
(900202,     0, 0,  1,     1, 0, 0, 0, 0, 0,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - ONESHOT_TALK'),
(900202,     0, 0,  0,     0, 0, 0, 0, 0, 0, 900210, 900211, 900212, 900213, 0, 0,  0, 0, 'Sigrid - random memory'),
-- "Darf ich etwas von dir bei mir tragen?"
(900203,     0, 0, 36,     0, 0, 0, 0, 0, 0,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - face player'),
(900203,     0, 0,  0,     0, 0, 0, 0, 0, 0, 900215, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - emote gives pendant'),
(900203,   500, 0, 17, 900200, 1, 0, 0, 0, 0,     0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - give Sigrids Sternenherz'),
(900203,   500, 0, 15, 44940, 1, 0, 0, 0, 4,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - Love Rocket (hearts)'),
(900203,  2000, 0,  0,     0, 0, 0, 0, 0, 0, 900216, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - whisper pendant'),
-- "Gute Nacht, Mama."
(900204,     0, 0, 36,     0, 0, 0, 0, 0, 0,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - face player'),
(900204,     0, 0,  0,     0, 0, 0, 0, 0, 0, 900217, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - whisper good night'),
(900204,  1500, 0,  1,    17, 0, 0, 0, 0, 0,      0, 0, 0, 0,        0,        0,     0, 0, 'Sigrid - ONESHOT_KISS');

-- ---------------------------------------------------------------------------------------------
-- Legendäres Andenken: Sigrids Sternenherz (Hals, seelengebunden, einzigartig)
-- ---------------------------------------------------------------------------------------------
INSERT INTO item_template (entry, class, subclass, name, displayid, Quality, Flags, BuyCount, BuyPrice, SellPrice, InventoryType,
    ItemLevel, RequiredLevel, maxcount, stackable, stat_type1, stat_value1, stat_type2, stat_value2,
    spellid_1, spelltrigger_1, spellcharges_1, spellcooldown_1, spellcategory_1, spellcategorycooldown_1,
    bonding, description, Material) VALUES
(900200, 4, 0, 'Sigrids Sternenherz', 39212, 5, 0, 1, 0, 0, 2,
    70, 0, 1, 1, 7, 30, 6, 30,
    26393, 0, 0, 3600000, 0, -1,
    1, 'Wohin du auch gehst - ich gehe mit dir.', 4);
