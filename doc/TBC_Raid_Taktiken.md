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
- **Gruul** – der Off-Tank bleibt im Nahkampf auf Platz 2 der Bedrohung (Hurtful Strike) und spottet nicht. Fernkampf immer 10 m verteilt; mit Gronn Lord's Grasp/Stoned verteilen sich **alle** auf 15 m (Shatter), raus aus Cave In.

### Magtheridon's Lair
- **Magtheridon** – Phase 1: Hellfire Channeler nacheinander töten, dann Burning Abyssals. Phase 2: **Manticron-Würfel** – 5 Bots (Fernkampf vor Heilern vor Nahkampf, ohne Mind Exhaustion) stellen sich an je einen Würfel und klicken, sobald Magtheridon Blast Nova wirkt (oder ein anderer Spieler schon klickt). Den Kanal brechen sie nicht ab. Raus aus Blaze/Conflagration, nicht vor dem Boss stehen.

### Serpentshrine Cavern
- **Hydross** – Tank 1 hält ihn im 20-m-Reinigungskreis (Frostform). Hat der Raid 100 % Mark of Hydross, zieht er ihn hinaus. Tank 2 spottet sofort und hält die verderbte Form draußen. Bei 100 % Mark of Corruption zieht Tank 2 ihn zurück in den Kreis. Pure/Tainted Spawns sterben zuerst.
- **Lurker Below** – bei **Spout** springen alle Bots ins Wasser (nächstes ausreichend tiefes Wasser). Ambusher (Fernkampf) und Guardians (Nahkampf) töten, raus aus Whirl und Geysir.
- **Leotheras** – jeder tötet seinen **eigenen Inner Demon**, raus aus dem Wirbelwind, Nahkampf weg von der Dämonenform, Shadow of Leotheras töten.
- **Karathress** – Spitfire Totem sofort, dann Tidalvess → Sharkkis → Caribdis → Karathress; Zyklone meiden.
- **Morogrim** – Water Globules meiden, nicht vor dem Boss (Tidal Wave).
- **Lady Vashj** – Static Charge → raus. Tainted Elemental → Enchanted Elemental → Coilfang Elite/Sporebats. **Tainted Cores:** Der nächste Bot mit Plünderrecht lootet den Kern (er ist dann gelähmt). Steht er an einem aktiven Schildgenerator, benutzt er ihn. Sonst wirft er den Kern zum Raidmitglied, das näher am nächsten aktiven Generator steht. So wandert der Kern per Kette bis zum Generator. Striders (Fear) und Toxic Spores meiden.

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
- **Teron Gorefiend** – Wer nach Shadow of Death den **Vengeful Spirit** steuert, greift damit die Shadowy Constructs an: bei mehreren Constructs Spirit Volley, sonst Spirit Chains, Spirit Lance und im Nahkampf Spirit Strike. Normale Angriffe des Raids schaden den Constructs nicht.
- **Gurtogg Bloodboil** – nur Tanks vorne (Arcing Smash), Caster verteilen sich. **Bloodboil-Rotation:** Fernkämpfer und Heiler mit 2+ Stapeln kommen auf ~14 m heran, damit Spieler ohne Stapel die nächsten Bloodboils (5 entfernteste Spieler) bekommen.
- **Reliquary of Souls** – Enslaved Souls töten, nicht in den Soul-Scream-Kegel.
- **Mother Shahraz** – Fatal Attraction → sofort auseinander. Off-Tanks stehen auf dem Main-Tank und teilen sich Saber Lash, spotten aber nicht.
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
- **Kalecgos** – wer im Spektralreich ist, tötet Sathrovarr (auch Tanks), alle anderen kämpfen gegen den Drachen. Nicht vor den Drachen stellen (Frost Breath).
- **Brutallus** – Burn → raus aus dem Raid. **Meteor Slash:** Nahkämpfer und Off-Tanks mit weniger als 2 Stapeln stellen sich mit vor den Boss und teilen den Schaden. Mit 2 Stapeln gehen sie hinter den Boss, bis der Debuff weg ist.
- **Felmyst** – Demonic Vapor + Spur meiden, Unyielding Dead töten.
- **Eredar Twins** – Conflagration → raus, Blaze meiden, verteilen.
- **M'uru / Entropius** – Void Sentinels → Fury Mages → Berserker; Dark Fiends meiden/aus der Ferne töten; Darkness und Singularity meiden.
- **Kil'jaeden** – Shield Orbs (Fernkampf) und Sinister Reflections töten, Armageddon meiden, Fire Bloom → raus. **Blaue Drachen:** Wird eine Drachenkugel aktiv, benutzt der nächste freie DD-Bot sie. Er fliegt den Drachen über dem Raid, heilt mit Breath: Revitalize und wirkt bei **Darkness of a Thousand Souls** Shield of the Blue. Alle anderen stellen sich unter den Drachen.

## Boss-Strategien aus mod-playerbots

Für die TBC-Raids gibt es zusätzlich eigene Boss-Strategien, portiert aus
[mod-playerbots](https://github.com/mod-playerbots/mod-playerbots) (GPL v2+) und an diesen Core angepasst.
Sie liegen in `src/modules/PlayerBots/playerbot/strategy/raids/<raid>/` und werden auf der Karte
automatisch aktiviert (über die Strategie `dungeon`) und beim Verlassen wieder entfernt.

| Raid | Strategie | Inhalt |
|---|---|---|
| Karazhan | `boss karazhan` | Attumen, Moroes, Maiden (Säulenplätze), Opera, Curator, Illhoof, Aran, Netherspite (Strahlen blocken), Malchezaar, Nightbane |
| Gruul's Lair | `boss gruul` | Maulgar-Rat (Tankzuteilung, Kill-Order, Banish, Misdirection), Gruul (Position, Verteilen, Shatter) |
| Magtheridon's Lair | `boss magtheridon` | Channeler, Abyssals, Manticron-Würfel pro Blast Nova, Debris |
| Serpentshrine Cavern | `boss ssc` | Hydross, Lurker, Leotheras, Karathress, Morogrim, Vashj inkl. Tainted-Core-Kette |
| Tempest Keep | `boss tempest keep` | Al'ar, Void Reaver, Solarian, Kael'thas (Berater, Waffen, Phönixe, Gravity Lapse) |
| Hyjal | `boss hyjal` | Winterchill, Anetheron, Kaz'rogal, Azgalor, Archimonde |
| Black Temple | `boss black temple` | Naj'entus, Supremus, Akama, Teron, Bloodboil, Reliquary, Shahraz, Council, Illidan |
| Zul'Aman | `boss zul'aman` | Akil'zon, Nalorakk, Jan'alai, Halazzi, Malacrass, Zul'jin |

Die Tabelle `TbcRaidEncounters.cpp` enthält für diese Bosse nur noch die Regeln, die die Boss-Strategien
ergänzen. Für Sunwell Plateau gibt es in mod-playerbots keine Strategie, dort gilt weiter nur die Tabelle.
Die alten Karazhan-Strategien (`generic/KarazhanDungeonStrategies.*`) werden nicht mehr aktiviert.

## Hinweise

- **Hydross**: Bots ziehen keine Resistenz-Ausrüstung an. Sie tanken mit dem, was sie tragen. Den Positionswechsel machen sie selbst.
- **Kalecgos**: Bots werden nur durch Spectral Blast ins Spektralreich geschickt, so wie im Original. Freiwillig gehen sie nicht hinein.
- **Im Spiel testen**: Die Abstände stammen aus den Raid-Guides. Wenn etwas nicht passt, passt du es in `TbcRaidEncounters.cpp` an.

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
