# MangosPro

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

Relevanter Code:
- `src/modules/PlayerBots/playerbot/strategy/generic/TbcRaidEncounters.cpp` – Boss-Tabelle der TBC-Raids
- `src/modules/PlayerBots/playerbot/strategy/generic/*DungeonStrategies.*` – ältere Boss-Strategien
- `src/modules/PlayerBots/playerbot/strategy/actions/*DungeonActions.*` – Boss-spezifische Aktionen
- `src/modules/PlayerBots/playerbot/strategy/generic/DungeonStrategy.*` – Aktivierung je Instanz/Boss

---

# C(ontinued)-MaNGOS -- README
[![Windows](../../actions/workflows/windows.yml/badge.svg)](../../actions/workflows/windows.yml) [![Ubuntu](../../actions/workflows/ubuntu.yml/badge.svg)](../../actions/workflows/ubuntu.yml) [![MacOS](../../actions/workflows/macos.yml/badge.svg)](../../actions/workflows/macos.yml)

This file is part of the CMaNGOS Project. See [AUTHORS](AUTHORS.md) and [COPYRIGHT](COPYRIGHT.md) files for Copyright information

## Welcome to C(ontinued)-MaNGOS

CMaNGOS is a free project with the following goal:

  **Doing Emulation Right!**

This means, we want to focus on:

* Doing
  * This project is focused on developing software!
  * Also there are many other aspects that need to be done and are
    considered equally important.
  * Anyone who wants to do stuff is very welcome to do so!

* Emulation
  * This project is about developing a server software that is able to
    emulate a well known MMORPG service.

* Right
  * Our goal must always be to provide the best code that we can.
  * Being 'right' is defined by the behaviour of the system
    we want to emulate.
  * Developing things right also includes documenting and discussing
    _how_ to do things better, hence...
  * Learning and teaching are very important in our view, and must
    always be a part of what we do.

To be able to accomplish these goals, we support and promote:

* Freedom
  * of our work: Our work - including our code - is released under the GPL.
    So everybody is free to use and contribute to this open source project.
  * for our developers and contributors on things that interest them.
    No one here is telling anybody _what_ to do.
    If you want somebody to do something for you, pay them,
    but we are here to enjoy.
  * to have FUN with developing.

* A friendly environment
  * We try to leave personal issues behind us.
  * We only argue about content and not about thin air!
  * We follow the [Netiquette](http://tools.ietf.org/html/rfc1855).

-- The C(ontinued)-MaNGOS Team!

## Further information

  You can find further information about CMaNGOS at the following places:
  * [CMaNGOS Discord](https://discord.gg/Dgzerzb)
  * [GitHub repositories](https://github.com/cmangos/)
  * [Issue tracker](https://github.com/cmangos/issues/issues)
  * [Pull Requests](https://github.com/cmangos/mangos-tbc/pulls)
  * [Wiki](https://github.com/cmangos/issues/wiki) with additional information on installation
  * [Contributing Guidelines](CONTRIBUTING.md)
  * Documentation can be found in the doc/ subdirectory and on the GitHub wiki

## License

  CMaNGOS is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


  You can find the full license text in the file [COPYING](COPYING) delivered with this package.

### Exceptions to GPL

  World of Warcraft® ©2004 Blizzard Entertainment, Inc. All rights reserved.
  World of Warcraft® content and materials mentioned or referenced are copyrighted by
  Blizzard Entertainment, Inc. or its licensors.
  World of Warcraft, WoW, Warcraft, The Frozen Throne, The Burning Crusade, Wrath of the Lich King,
  Cataclysm, Mists of Pandaria, Ashbringer, Dark Portal, Darkmoon Faire, Frostmourne, Onyxia's Lair,
  Diablo, Hearthstone, Heroes of Azeroth, Reaper of Souls, Starcraft, Battle Net, Blizzcon, Glider,
  Blizzard and Blizzard Entertainment are trademarks or registered trademarks of
  Blizzard Entertainment, Inc. in the U.S. and/or other countries.

  Any World of Warcraft® content and materials mentioned or referenced are copyrighted by
  Blizzard Entertainment, Inc. or its licensors.
  CMaNGOS project is not affiliated with Blizzard Entertainment, Inc. or its licensors.

  Some third-party libraries CMaNGOS uses have other licenses, that must be
  upheld.  These libraries are located within the dep/ directory

  In addition, as a special exception, the CMaNGOS project
  gives permission to link the code of its release of MaNGOS with the
  OpenSSL project's "OpenSSL" library (or with modified versions of it
  that use the same license as the "OpenSSL" library), and distribute
  the linked executables.  You must obey the GNU General Public License
  in all respects for all of the code used other than "OpenSSL".  If you
  modify this file, you may extend this exception to your version of the
  file, but you are not obligated to do so.  If you do not wish to do
  so, delete this exception statement from your version.
