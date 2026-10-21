# Boss strategies ported from mod-playerbots

The boss tactics in this folder are ported from mod-playerbots (AzerothCore, GPL v2+,
https://github.com/mod-playerbots/mod-playerbots, `src/Ai/Raid` and `src/Ai/Dungeon`) and rewritten
for this core (CMaNGOS TBC + ike3/celguar playerbots). Each raid has its own folder.

## Layout

| Path | Content |
|---|---|
| `RaidBossHelpers.*` | Shared helpers (port of `EncounterHelpers`): raid icons, tank order, steps, dynamic objects, action classification |
| `RaidBossStrategies.*` | Map id -> strategy name, trigger `raid boss map strategy` + action `update raid boss map strategy` (added to the `dungeon` strategy) |
| `RaidBossContexts.*` | Registers the contexts of every raid folder in `AiObjectContext` |
| `<raid>/<Prefix>Context.h` | `<Prefix>StrategyContext`, `<Prefix>ActionContext`, `<Prefix>TriggerContext`, `<Prefix>ValueContext` |

Strategy names (one per map, added automatically on the map and removed when leaving):

| Map | Strategy | Folder |
|---|---|---|
| 409 Molten Core | `boss molten core` | `mc` |
| 469 Blackwing Lair | `boss blackwing lair` | `bwl` |
| 249 Onyxia's Lair | `boss onyxia` | `onyxia` |
| 509 Ruins of Ahn'Qiraj | `boss aq20` | `aq20` |
| 533 Naxxramas | `boss naxxramas` | `naxx` |
| 532 Karazhan | `boss karazhan` | `karazhan` |
| 565 Gruul's Lair | `boss gruul` | `gruul` |
| 544 Magtheridon's Lair | `boss magtheridon` | `magtheridon` |
| 548 Serpentshrine Cavern | `boss ssc` | `ssc` |
| 550 Tempest Keep | `boss tempest keep` | `tk` |
| 534 Hyjal Summit | `boss hyjal` | `hyjal` |
| 564 Black Temple | `boss black temple` | `bt` |
| 568 Zul'Aman | `boss zul'aman` | `za` |
| 19 Classic / 16 TBC dungeon maps | `boss ...` names from `GetDungeonBossMaps()` | `dungeons` |

The dungeon contexts now register map-specific strategies backed by the existing
encounter engine, with 110 dungeon and 40 Classic raid profiles and additional line-of-sight and
damage-hold mechanics. See `doc/Bot_Dungeon_Strategien.md` for coverage and
`debug raid dungeons` for a runtime check of every dungeon context and handler.
The Classic raid contexts (`mc`, `bwl`, `onyxia`, `aq20`, `naxx`) now register
`ClassicRaidBossStrategy`, including the existing Magmadar, suppression-room and
Four Horsemen strategies. `debug raid classic` checks their wiring separately.

## API differences (mod-playerbots -> this core)

| mod-playerbots | here |
|---|---|
| `botAI` | `ai` (`PlayerbotAI*`), `bot` is the same |
| `Strategy::InitTriggers(std::vector<TriggerNode*>&)` | `InitCombatTriggers(std::list<TriggerNode*>&)` (+ `InitNonCombatTriggers`, `InitReactionTriggers`) |
| `new TriggerNode("t", { NextAction("a", 60.0f) })` | `new TriggerNode("t", NextAction::array(0, new NextAction("a", 60.0f), NULL))` |
| `InitMultipliers(std::vector<Multiplier*>&)` | `InitCombatMultipliers(std::list<Multiplier*>&)` |
| `ACTION_RAID` (60, above heals) | `raidboss::ACTION_RAID` (85, above heals 60-80) |
| `ACTION_LIGHT/MEDIUM/CRITICAL_HEAL` (10/20/30) | 60/70/80 here, `ACTION_EMERGENCY` is 90 in both. Keep every relevance below 100 |
| `bool Execute(Event event)` | `bool Execute(Event& event) override` |
| `bool isUseful()` | same |
| `AI_VALUE`, `AI_VALUE2` | same macros |
| `AI_VALUE2(Unit*, "find target", "name")` | `raidboss::FindUnitByName(ai, "name")` or better `raidboss::FindUnit(ai, ENTRY)` |
| `GuidVector` | `std::list<ObjectGuid>` (`"attackers"`, `"possible targets no los"`, `"nearest npcs"`) |
| `Position` | `WorldPosition(mapId, x, y, z)` (`getX()`, `getY()`, `getZ()`) |
| `unit->GetExactDist2d(x)` | `raidboss::ExactDist2d(unit, other)` (CMaNGOS `GetDistance` subtracts the bounding radius) |
| `GetGUID()` | `GetObjectGuid()` |
| `ref->GetSource()` | `ref->getSource()` |
| `MoveTo(mapId, x, y, z, false, false, false, false, MovementPriority::..., true, backwards)` | `MoveTo(mapId, x, y, z, false /*idle*/, false /*react*/, false /*noPath*/, true /*ignoreEnemyTargets*/)` from `MovementAction` |
| `MoveAway(unit, dist)` | see `TbcRaidMovementAction::MoveToBestPoint` or compute a point and `MoveTo` |
| `Attack(target)` (`AttackAction`) | derive from `AttackAction`, `Attack(GetMaster(), target)` |
| `botAI->CastSpell(id, target)` / `CanCastSpell` | `ai->CanCastSpell(id, target, 0)` / `ai->CastSpell(id, target)`, or `raidboss::TryCast` |
| `botAI->HasAura(id, unit)` | `unit->HasAura(id)` |
| `GetInstanceScript()->IsEncounterInProgress()` | `raidboss::IsEncounterInProgress(bot, mapId, {bossEntries})` |
| `PlayerbotAI::IsTank/IsHeal(player)` | same (static) |
| `PlayerbotAI::IsRanged/IsMelee(player)` | `raidboss::IsRangedPlayer / IsMeleePlayer` |
| `PlayerbotAI::IsDps(player)` | `raidboss::IsDps` |
| group main tank / assist tank | `raidboss::GetGroupMainTank / GetGroupAssistTank(bot, index)` |
| `IsDpsCooldownAction`, `IsTauntAction` (dynamic_cast) | same names in `raidboss`, based on the action name |
| `instance script data (GetData/GetBossState)` | `bot->GetInstanceData()` + the ScriptDev2 script in `src/game/AI/ScriptDevAI/scripts/...` |

Spell, creature and game object ids are the Blizzard ids and normally identical. Always check
them against the ScriptDev2 boss script of this core (`src/game/AI/ScriptDevAI/scripts`), because
the boss scripts (timers, phases, summoned npcs, auras) can differ from AzerothCore.

## Coexistence with `tbc raid`

The table driven `tbc raid` strategy (`generic/TbcRaidEncounters.cpp`) still runs for every TBC raid.
Where a dedicated strategy handles a mechanic itself, the matching rule of the table is removed so
the bots don't get two different orders.

## Building

The folder is picked up by `file(GLOB_RECURSE ...)` in `src/modules/PlayerBots/CMakeLists.txt`.
Re-run CMake after adding new files.
