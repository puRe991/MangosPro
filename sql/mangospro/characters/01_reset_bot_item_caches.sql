-- MangosPro: run after changing the bot stat weights (world: 02_tbc_bot_stat_weights.sql).
-- The bots cache the stat weight of every item per spec. Emptying the caches makes mangosd
-- rebuild them with the new weights on the next start (that start takes longer once).
-- Apply to the characters database (tbccharacters).

TRUNCATE TABLE `ai_playerbot_item_info_cache`;
TRUNCATE TABLE `ai_playerbot_equip_cache`;
