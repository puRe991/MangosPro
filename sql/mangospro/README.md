# MangosPro SQL

Changes on top of the CMaNGOS / TBC-DB databases. Apply them in order after installing or
re-installing the databases (`InstallFullDB.sh` overwrites them), then restart `mangosd`.

| File | Database | Change |
|------|----------|--------|
| `world/01_raidwide_bloodlust_heroism.sql` | `tbcmangos` | Bloodlust and Heroism hit the whole raid in range, not only the party |
| `world/02_tbc_bot_stat_weights.sql` | `tbcmangos` | TBC stat weights for the playerbot item rating (gear, loot rolls, gems) |
| `characters/01_reset_bot_item_caches.sql` | `tbccharacters` | Empties the bot item caches so they are rebuilt with the new weights |

```bash
mysql -u root tbcmangos < world/01_raidwide_bloodlust_heroism.sql
mysql -u root tbcmangos < world/02_tbc_bot_stat_weights.sql
mysql -u root tbccharacters < characters/01_reset_bot_item_caches.sql
```
