# Playerbots – TBC Raid-Taktiken (MangosPro)

Die Strategie **`tbc raid`** wird automatisch aktiviert, sobald ein Bot einen TBC-Raid betritt
(über die Standard-Strategie `dungeon`), und beim Verlassen wieder entfernt.
Sie greift nur, wenn die Bots einen der unten aufgeführten Bosse (oder dessen Adds) bekämpfen.

Code:

| Datei | Inhalt |
|------|--------|
| `src/modules/PlayerBots/playerbot/strategy/generic/TbcRaidEncounters.cpp` | **Boss-Tabelle** – hier stehen alle Taktiken als Daten |
| `src/modules/PlayerBots/playerbot/strategy/generic/TbcRaidStrategy.*` | Strategie, Auswertung der Tabelle, Multiplikatoren |
| `src/modules/PlayerBots/playerbot/strategy/triggers/TbcRaidTriggers.*` | Auslöser (Feuer, Debuff, Abstand, Fokusziel …) |
| `src/modules/PlayerBots/playerbot/strategy/actions/TbcRaidActions.*` | Aktionen (rauslaufen, verteilen, spotten, Würfel, Stacheln …) |

## Allgemeine Mechaniken

| Mechanik | Verhalten der Bots |
|---|---|
| Gefahrenzonen | Bots verlassen Bodeneffekte (Death and Decay, Rain of Fire, Blizzard, Consecration …), gefährliche Objekte (Blaze) und Kreaturen (Wirbelwind, Infernals, Wasserkugeln, Doomfire …). Läuft auch während eines Zaubers (Reaktion). Tanks bleiben bei dem Gegner, den sie tanken. |
| Verteilen | Fernkämpfer/Heiler halten bei vielen Bossen Abstand zueinander; einige Bosse verlangen Verteilen für alle (z. B. Gruul nach Ground Slam). Heiler bleiben in Heilreichweite zum Tank, Fernkämpfer in Reichweite zu ihrem Ziel. |
| Debuff → raus aus dem Raid | Wer bestimmte Debuffs hat, läuft von allen weg (Static Charge, Wrath of the Astromancer, Doom, Fatal Attraction, Burn, Fire Bloom …). |
| Kill-Reihenfolge | DPS-Bots greifen das wichtigste Add zuerst an und wechseln nicht mehr auf das Ziel mit den wenigsten Lebenspunkten. Alle fokussieren dasselbe Ziel. |
| Frontal-Angriffe | Nicht-Tanks stellen sich aus dem Frontkegel des Bosses (Cleave, Breath, Arcing Smash …). |
| Tankwechsel | Off-Tank spottet bei X Stapeln eines Debuffs; der Tank mit den Stapeln spottet nicht zurück. Bei Formwechseln (Nalorakk) tankt jeder Tank „seine“ Form. |

## Bosse

### Karazhan (zusätzlich zu den vorhandenen Strategien für Netherspite, Aran, Wolf, Malchezaar)
- **Maiden of Virtue** – Caster bleiben außerhalb von Holy Ground, Caster verteilen sich.
- **The Curator** – Astral Flares sofort töten, Caster verteilen sich.
- **Terestian Illhoof** – Demon Chains (Opfer befreien) zuerst, danach Kil'rek.
- **Nightbane** – raus aus Charred Earth, Skelette töten, nicht vor dem Drachen stehen.
- **The Crone** – Zyklon meiden.

### Gruul's Lair
- **High King Maulgar** – Kill-Order Blindeye → Olm (+ Felhunter) → Kiggler → Krosh (nur Fernkampf) → Maulgar. Raus aus Maulgars Wirbelwind und Kroshs Blast Wave, nicht vor Maulgar stehen.
- **Gruul** – Fernkampf immer 10 m verteilt; mit Gronn Lord's Grasp/Stoned verteilen sich **alle** auf 15 m (Shatter), raus aus Cave In.

### Magtheridon's Lair
- **Magtheridon** – Phase 1: Hellfire Channeler nacheinander töten, dann Burning Abyssals. Phase 2: **Manticron-Würfel** – 5 Bots (Fernkampf vor Heilern vor Nahkampf, ohne Mind Exhaustion) stellen sich an je einen Würfel und klicken, sobald Magtheridon Blast Nova wirkt (oder ein anderer Spieler schon klickt). Den Kanal brechen sie nicht ab. Raus aus Blaze/Conflagration, nicht vor dem Boss stehen.

### Serpentshrine Cavern
- **Hydross** – Pure/Tainted Spawns zuerst.
- **Lurker Below** – Ambusher (Fernkampf) und Guardians (Nahkampf) töten, raus aus Whirl und Geysir.
- **Leotheras** – jeder tötet seinen **eigenen Inner Demon**, raus aus dem Wirbelwind, Nahkampf weg von der Dämonenform, Shadow of Leotheras töten.
- **Karathress** – Spitfire Totem sofort, dann Tidalvess → Sharkkis → Caribdis → Karathress; Zyklone meiden.
- **Morogrim** – Water Globules meiden, nicht vor dem Boss (Tidal Wave).
- **Lady Vashj** – Static Charge → raus; Tainted Elemental → Enchanted Elemental → Coilfang Elite/Sporebats; Striders (Fear) und Toxic Spores meiden.

### Tempest Keep: The Eye
- **Al'ar** – Embers töten, Flame Patches meiden, Tankwechsel bei Melt Armor, Caster verteilen sich.
- **Void Reaver** – Caster verteilen sich auf 12 m (Arcane Orb).
- **Solarian** – Wrath of the Astromancer → raus aus dem Raid; Priester vor Agenten.
- **Kael'thas** – Thaladred-Gaze-Ziel läuft weg, Nahkampf weg von Capernian; Waffen: Kolben → Stab → Dolch → Schwert → Axt → Bogen → Schild; Phoenix-Eier sofort; Flamestrike und Nether Vapor meiden; in Gravity Lapse verteilen.

### Hyjal Summit
- **Rage Winterchill** – raus aus Death and Decay.
- **Anetheron** – Infernals nur von Tanks, nicht vor dem Boss (Carrion Swarm).
- **Kaz'rogal** – Mark of Kaz'rogal + wenig Mana → raus aus dem Raid.
- **Azgalor** – raus aus Rain of Fire, Doom → raus aus dem Raid.
- **Archimonde** – Doomfire meiden, Caster verteilen sich (Air Burst).

### Black Temple
- **Naj'entus** – Caster verteilen sich; der nächste freie Bot zieht den Impaling Spine aus dem Opfer und wirft ihn bei Tidal Shield auf den Boss.
- **Supremus** – Gaze-Ziel kitet den Boss, Volcanos und Molten Flame meiden.
- **Shade of Akama** – Sorcerer → Channeler → Spiritbinder → Elementalist → Rogue → Defender.
- **Teron Gorefiend** – Shadowy Constructs töten.
- **Gurtogg Bloodboil** – nur Tanks vorne (Arcing Smash), Caster verteilen sich.
- **Reliquary of Souls** – Enslaved Souls töten, nicht in den Soul-Scream-Kegel.
- **Mother Shahraz** – Fatal Attraction → sofort auseinander.
- **Illidari Council** – raus aus Flamestrike/Blizzard/Consecration, Abstand zu Zerevor.
- **Illidan** – Shadow Demons und Parasitic Shadowfiends sofort; Flame Crash, Blaze, Eye Blast, Demon Fire meiden; nur Tanks an die Flames of Azzinoth; Parasit-Debuff → raus; in der Dämonenform verteilen.

### Zul'Aman
- **Nalorakk** – zwei Tanks: Tank 1 Trollform, Tank 2 spottet bei jeder Bärenform.
- **Akil'zon** – unter das Electrical-Storm-Ziel stellen, sonst verteilen.
- **Jan'alai** – Fire Bombs meiden, Hatcher sofort töten, nicht vor den Boss (Flame Breath).
- **Halazzi** – Corrupted Lightning Totem sofort.
- **Hex Lord Malacrass** – Alyson Antille (Heilerin) zuerst.
- **Zul'jin** – raus aus Wirbelwind, Feather Vortex und Column of Fire.

### Sunwell Plateau
- **Brutallus** – Burn → raus aus dem Raid.
- **Felmyst** – Demonic Vapor + Spur meiden, Unyielding Dead töten.
- **Eredar Twins** – Conflagration → raus, Blaze meiden, verteilen.
- **M'uru / Entropius** – Void Sentinels → Fury Mages → Berserker; Dark Fiends meiden/aus der Ferne töten; Darkness und Singularity meiden.
- **Kil'jaeden** – Shield Orbs (Fernkampf) und Sinister Reflections töten, Armageddon meiden, Fire Bloom → raus.

## Bekannte Grenzen

- **Lady Vashj Phase 2**: Die Tainted Cores werden nicht durch den Raid geworfen. Das müssen echte Spieler machen.
- **Lurker Spout**: Die Bots springen nicht ins Wasser.
- **Teron Gorefiend**: Als Geist steuern Bots ihre Fähigkeiten nicht.
- **Kalecgos**: Den Wechsel ins Dämonenreich machen Bots nicht.
- **Kil'jaeden**: Bots nutzen das Darkness-Schild der blauen Drachen nicht.
- **Hydross**: Bots ziehen keine Resistenz-Ausrüstung an und machen keinen Positionswechsel des Tanks.
- **Gurtogg Bloodboil**: Die Bloodboil-Rotation der Fernkämpfer fehlt.
- **Hurtful Strike, Saber Lash und Meteor Slash**: Bots ordnen die Bedrohung nicht gezielt an.
- **Nicht im Spiel getestet**: Die Abstände sind aus Raid-Guides übernommen und noch nicht im Spiel geprüft. Bei Bedarf passt du sie in `TbcRaidEncounters.cpp` an.

## Eigene Taktik hinzufügen

```cpp
Add("boss name", MAP_ID, { BOSS_ENTRY, ADD_ENTRY })
    .Kill(ADD_ENTRY, ROLE_DPS)               // Kill-Reihenfolge
    .Avoid(CREATURE_ENTRY, 8.0f)             // Abstand zu Kreatur
    .AvoidArea(SPELL_ID)                     // Bodeneffekt verlassen
    .RunWithDebuff(SPELL_ID, 15.0f)          // mit Debuff raus aus dem Raid
    .Spread(10.0f, ROLE_CASTERS)             // Caster verteilen
    .TankSwap(BOSS_ENTRY, SPELL_ID, STACKS)  // Tankwechsel
    .AvoidFront(BOSS_ENTRY, 15.0f);          // nicht vor dem Boss stehen
```
