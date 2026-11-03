# MangosPro

[English](README.md) · **Deutsch**

TBC-Server (2.4.3) auf Basis von **CMaNGOS mangos-tbc** mit fest integriertem **Playerbots**-Modul.
Ziel des Projekts: die Playerbots so zu verbessern, dass sie Raid-Bosse sinnvoll spielen.

## Herkunft der Quellen

| Teil | Upstream | Commit (Import) | Pfad |
|------|----------|-----------------|------|
| Core | https://github.com/cmangos/mangos-tbc | `4e329ff72a4f9c88bb754a10cf57c2a50ffaff2e` | `/` |
| Playerbots | https://github.com/cmangos/playerbots | `2f5ace0061f2c94dcc92f6af5dd2c427dfa932d6` | `src/modules/PlayerBots/` |

Beide sind GPL-2.0 (siehe `LICENSE`). Aus dem Playerbots-Modul wurden nur die
Welt-SQL-Dateien für Classic und WotLK entfernt (`sql/world/classic`, `sql/world/wotlk`), da sie für TBC nicht gebraucht werden.

### Unterschied zum Upstream
Upstream lädt das Playerbots-Modul beim CMake-Lauf per `FetchContent` von GitHub nach
(und würde lokale Änderungen dabei überschreiben). In MangosPro liegt der Code direkt im Repo;
`src/CMakeLists.txt` nutzt `src/modules/PlayerBots` direkt, wenn das Verzeichnis existiert.
Änderungen an den Bots werden also einfach in `src/modules/PlayerBots/` gemacht und committet.

## Bauen (Linux)

```bash
sudo apt install build-essential cmake git libboost-all-dev default-libmysqlclient-dev libssl-dev zlib1g-dev libbz2-dev
mkdir build && cd build
cmake .. -DBUILD_PLAYERBOTS=ON -DPCH=ON -DCMAKE_INSTALL_PREFIX=$HOME/mangos-tbc
make -j$(nproc) && make install
```

Windows: CMake-GUI, Haken bei `BUILD_PLAYERBOTS`, dann Visual Studio-Projekt erzeugen.
Ausführliche Anleitung: https://github.com/cmangos/issues/wiki/Installation-Instructions

## Datenbank
- Welt-DB: https://github.com/cmangos/tbc-db (passend zum Core-Stand halten)
- Playerbot-Tabellen: `src/modules/PlayerBots/sql/characters/*.sql` in die Characters-DB,
  `src/modules/PlayerBots/sql/world/*.sql` und `sql/world/tbc/*.sql` in die World-DB.
- Bot-Konfiguration: `aiplayerbot.conf` (wird beim Install aus `aiplayerbot.conf.dist` erzeugt).

> **Hinweis zum Repack:** Ein vorhandenes Repack lässt sich nur mit diesem Core weiterbetreiben,
> wenn es ebenfalls auf CMaNGOS-TBC basiert und die DB-Revision passt. Sonst die DBs aus
> `tbc-db` neu aufsetzen (Charakter-DB vorher sichern).

## Stand der Raid-Strategien (Playerbots)
Alle TBC-Raids haben Boss-Taktiken: Karazhan, Gruul, Magtheridon, SSC, Festung der Stürme,
Hyjal, Black Temple, Zul'Aman, Sunwell (Strategie `tbc raid`, wird im Raid automatisch aktiv).
Details, Kill-Reihenfolgen und bekannte Grenzen: **[doc/TBC_Raid_Taktiken.md](doc/TBC_Raid_Taktiken.md)**.

Rotationen, Raid-Buffs (Befehle `lust` / `auras`), Edelsteine, Würfelregeln und Tank-Bedrohung:
**[doc/Bot_Raidbuffs_und_Ausruestung.md](doc/Bot_Raidbuffs_und_Ausruestung.md)**.
Zusätzliche SQL-Änderungen: **[sql/mangospro/README.md](sql/mangospro/README.md)**.
Plan für Bekanntmachung und Verteilung: **[doc/Verbreitungsplan.md](doc/Verbreitungsplan.md)**.

Relevanter Code:
- `src/modules/PlayerBots/playerbot/strategy/generic/TbcRaidEncounters.cpp` – Boss-Tabelle der TBC-Raids
- `src/modules/PlayerBots/playerbot/strategy/generic/*DungeonStrategies.*` – ältere Boss-Strategien
- `src/modules/PlayerBots/playerbot/strategy/actions/*DungeonActions.*` – Boss-spezifische Aktionen
- `src/modules/PlayerBots/playerbot/strategy/generic/DungeonStrategy.*` – Aktivierung je Instanz/Boss

## Website
Unter `website/` liegt eine Server-Website für Apache/PHP mit Registrierung (SRP6), Account-Verwaltung,
Realmstatus, Online-Liste, Ranglisten, Arsenal, News-Admin sowie Impressum und Datenschutz.
Installation: **[website/README.md](website/README.md)**.

Die ursprüngliche CMaNGOS-README (Projektziele, Lizenz, Links) steht am Ende der [englischen README](README.md#continued-mangos----readme).
