-- MangosPro: smaller aggro radius for the TBC raid bosses and their encounter adds (world database).
-- creature_template.Detection is the aggro range in yards (default 18, some bosses 25-55, Gruul and
-- Lady Vashj 45). Bosses with more than 15 yards are lowered to 15 so the raid can prepare (tactic,
-- buffs, positions) without pulling by accident. Trash is not touched.
-- Entries: the creatures listed in TbcRaidEncounters.cpp, except bosses whose fight starts by an event
-- or script and keep their range: Archimonde, Kael'thas, Brutallus, Kalecgos, M'uru, Leotheras.
-- Revert: re-run InstallFullDB (or restore creature_template.Detection from a backup).

UPDATE `creature_template`
SET `Detection` = 15
WHERE `Detection` > 15 AND `Entry` IN (15688, 15691, 16457, 17096, 17225, 17229, 17248, 17256, 17257, 17261, 17454, 17767, 17808, 17818, 17842, 17888, 18168, 18412, 18805, 18806, 18831, 18832, 18834, 18835, 18836, 18925, 19044, 19514, 19516, 19551, 20060, 20062, 20063, 20064, 21212, 21213, 21214, 21216, 21217, 21268, 21269, 21270, 21271, 21272, 21273, 21274, 21362, 21364, 21857, 21865, 21873, 21875, 21958, 21964, 21965, 21966, 22009, 22035, 22036, 22055, 22056, 22140, 22841, 22856, 22871, 22887, 22898, 22917, 22947, 22948, 22949, 22950, 22951, 22952, 22997, 23111, 23215, 23216, 23375, 23418, 23419, 23420, 23421, 23426, 23469, 23498, 23574, 23576, 23577, 23578, 23818, 23863, 24143, 24239, 24504, 24892, 25038, 25165, 25166, 25268, 25315, 25502, 25708, 25744, 25772, 25798, 25799, 25840);