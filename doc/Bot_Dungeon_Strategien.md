# Classic- und TBC-Instanzstrategien

Alle 19 Classic- und 16 TBC-Dungeon-Maps haben einen registrierten
`boss ...`-Eintrag. Die gemeinsame Map-Liste liegt in
`strategy/raids/dungeons/DungeonBossStrategy.cpp`; dieselbe Liste wird fuer
die Context-Factories und die automatische Aktivierung verwendet.
Zusaetzlich sind die fuenf bisherigen Classic-Raid-Platzhalter (MC, BWL,
Onyxia, AQ20 und Naxxramas) registriert. Ihre gemeinsame Map-Liste ist
`GetClassicRaidBossMaps()`.

Die `dungeon`-Strategie aktiviert die passende Boss-Strategie beim Betreten
in Kampf, Ruhe und Reaktion und entfernt sie beim Map-Wechsel. Die gemeinsame
Encounter-Engine wird wiederverwendet; die zusaetzliche Strategie `tbc raid`
wird auf diesen Dungeon-/Classic-Raid-Maps nicht automatisch eingeschaltet. Die bisherigen
Dungeon-/Boss-Strategien bleiben verfuegbar.

## Umfang

- Classic: Ragefire Chasm, Wailing Caverns, Deadmines, Shadowfang Keep,
  Stockades, Blackfathom Deeps, Gnomeregan, Razorfen Kraul, Scarlet Monastery,
  Razorfen Downs, Uldaman, Zul'Farrak, Maraudon, Sunken Temple, Blackrock
  Depths, Blackrock Spire (LBRS und UBRS auf Map 229), Dire Maul (alle Fluegel
  auf Map 429), Scholomance und Stratholme (beide Seiten auf Map 329).
- TBC: Ramparts, Blood Furnace, Shattered Halls, Slave Pens, Underbog,
  Steamvault, Mana-Tombs, Auchenai Crypts, Sethekk Halls, Shadow Labyrinth,
  Old Hillsbrad, Black Morass, Mechanar, Botanica, Arcatraz und Magisters'
  Terrace. Normal-/Heroisch-Varianten verwenden dieselbe Map-Strategie.

`DungeonEncounters.cpp` enthaelt 110 Dungeon- und 40 Classic-Raid-Profile mit Add-Prioritaeten,
Abstand, Debuff-Auslaufen, frontalen Angriffen und Gefahren durch Kreaturen,
GameObjects, Flaechen, Auren und laufende Zauber. Bosskaempfe ohne eigene
Positions-/Zielwechselregel laufen weiter ueber die normale Klassen-KI.
Das ist Kampfunterstuetzung, keine Automatisierung von Instanzraetseln,
Escort-Dialogen, Schaltern oder der Auswahl einer Dungeon-Route.

Sonderregeln behandeln Ikiss' Arkane Explosion durch Sichtlinienbruch,
Schadenspausen bei Reflexion/Immunitaet, Anzus Zauberbombe und Murmurs
Tank-Rueckkehr nach Sonic Boom. Heilung bleibt bei Reflexionspausen erlaubt;
die Zauberbombe sperrt bis zum Ablauf auch Heilzauber. Kontrollierte Pets
werden beim Angriff auf einen reflektierenden Boss zurueckgerufen.
Die Classic-Raid-Strategien verwenden zusaetzlich die vorhandene Magmadar-,
BWL-Unterdrueckungsraum- und Four-Horsemen-Logik. Neu sind unter anderem
Tankwechsel bei Kurinnaxx, Ebonroc, Firemaw und Gluth, Eier-Zielwahl und
Kiten bei Buru, Kristallbenutzung bei Ossirian, Deckung bei Chromaggus und
Sapphiron sowie Zombie-Prioritaeten nach Gluths Dezimieren.
Die Raidleitung uebernimmt weiterhin Rollen-/Gruppeneinteilung und
Mechaniken ohne implementierte Sonderaktion, etwa Razorgores Kontrollkugel,
Faerlinas Worshipper-Zuweisung und die Koordination des Heigan-Tanzes.
Gefahrenregeln bleiben nach dem letzten Angreifer zehn Sekunden erhalten.
Der Encounter-Cache wird bei Map- und Instanzwechsel zurueckgesetzt.

Die IDs stammen aus den lokalen ScriptDevAI-Skripten, der TBC-Datenbank und
den Spell-DBC-SQL-Daten. Beispielsweise erzeugt Nazans Feuer GameObjects
180125/182533; der Beschwoerungszauber selbst ist keine DynamicObject-Flaeche.

## Pruefung im Spiel

Nach Neubau und Neustart des Servers einen Bot anfluestern:

```text
debug raid dungeons
debug raid classic
debug raid
debug raid on
debug raid off
```

`debug raid dungeons` erzeugt alle 35 Strategien ueber den echten
AI-Context und prueft ihre Namen, Encounter-Eintraege, Trigger und Aktionen
in Kampf, Ruhe und Reaktion. Erwartet werden `Maps=35`, `Encounter=110` und
`Fehlend=0`. Die Pruefung funktioniert auch ausserhalb einer Instanz.
`debug raid classic` prueft entsprechend die fuenf Classic-Raids mit
`Maps=5`, `Encounter=40` und erwartet ebenfalls `Fehlend=0`.

`debug raid` zeigt auf einer Dungeon-Map die Registrierung, aktive
Bot-Zustaende und den erkannten Encounter. `debug raid on` protokolliert
ausgefuehrte/fehlgeschlagene Strategieaktionen mit dem vorhandenen
`[RaidTactics]`-Praefix in der Server-Logdatei.

Fuer den Funktionstest: Eintritt und Verlassen pruefen, dann jeweils einen
Tank, Nahkaempfer, Fernkaempfer und Heiler an Mechaniken wie Herods
Wirbelwind, Keli'dans Nova, Ikiss' Explosion, Pandemonius' Schild und Murmurs
Sonic Boom testen. Sichtlinienbruch und sichere Bewegung benoetigen die
passenden VMAPs/MMAPs. Kompilierung und Datenpruefung ersetzen diese
Bosskaempfe im Spiel nicht.

## Build

Die neuen Dateien werden vom vorhandenen rekursiven CMake-Glob erfasst:

```text
cmake -S . -B build
cmake --build build --config Release --target mangosd
```

Ein laufender Server kann das Kopieren/Ersetzen seiner DLLs und EXE
verhindern. Gezielte Compilerpruefungen des Playerbots-Projekts koennen mit
`BuildProjectReferences=false` erfolgen; sie ersetzen keinen vollstaendigen
Server-Neubau und keinen Neustart.

Fuer diese Aenderung wurden die betroffenen C++-Dateien kompiliert, das
Playerbots-Archiv aus allen 528 Projektobjekten zusammengesetzt und eine
separate `build/bin/x64_Release/mangosd-dungeons.exe` erfolgreich gelinkt.
Der Starttest mit `--help` war erfolgreich. Der laufende Server wurde nicht
neu gestartet; die neue EXE muss bei einem geplanten Server-Neustart
verwendet werden. Die bisherige `mangosd.exe` wurde nicht ersetzt.

Die statische Pruefung bestaetigt 48 registrierbare Map-Strategien
(35 Dungeons, 5 Classic- und 8 vorhandene TBC-Raids), 150 Profile ohne
mehrdeutige NPC-Zuordnung pro Map sowie gueltige IDs fuer 329 Kreaturen,
4 GameObjects und 39 konfigurierte Mechanik-Zauber. Die Ingame-Pruefung
mit den beiden Debug-Befehlen und die Bosskaempfe stehen noch aus.
