-- MangosPro: service NPCs for the gear of the player (script npc_gear_service.cpp)
--   900100 npc_gear_enchanter: TBC enchants per slot, or the recommended enchants of the talent spec
--   900101 npc_gear_gemcutter: fills the sockets with the best gems for class and talent spec
-- Spawn ingame with: .npc add 900100  /  .npc add 900101

DELETE FROM creature_template WHERE Entry IN (900100, 900101);

DROP TEMPORARY TABLE IF EXISTS tmp_gear_service;
CREATE TEMPORARY TABLE tmp_gear_service SELECT * FROM creature_template WHERE Entry = 19540;

-- Enchanter (model of Asarnan, Master Enchanting Trainer)
UPDATE tmp_gear_service SET Entry = 900100, Name = 'Lyria Sternenglanz', SubName = 'Verzauberungen', IconName = NULL,
    MinLevel = 70, MaxLevel = 70, Faction = 35, NpcFlags = 1, GossipMenuId = 0, EquipmentTemplateId = 0,
    ScriptName = 'npc_gear_enchanter';
INSERT INTO creature_template SELECT * FROM tmp_gear_service;

-- Gemcutter (model of the World Jewelcrafting Trainer)
UPDATE tmp_gear_service SET Entry = 900101, Name = 'Doran Funkelstein', SubName = 'Edelsteine & Sockel',
    DisplayId1 = 2575, ScriptName = 'npc_gear_gemcutter';
INSERT INTO creature_template SELECT * FROM tmp_gear_service;

DROP TEMPORARY TABLE tmp_gear_service;
