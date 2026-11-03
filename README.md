# MangosPro

**English** · [Deutsch](README.de.md)

**A World of Warcraft: The Burning Crusade (2.4.3) server where playerbots handle raid boss mechanics.**

MangosPro is based on [CMaNGOS mangos-tbc](https://github.com/cmangos/mangos-tbc) and ships with the
[Playerbots](https://github.com/cmangos/playerbots) module built in. The goal is that you can take a raid of bots
into every TBC raid, and the bots handle the boss mechanics instead of just standing there and fighting.

> **Status: work in progress.** Every TBC raid has boss tactics in the code, but most of them have not yet been
> confirmed by a full kill with bots. The [per-boss status table](doc/Bot_Boss_Status.md) shows what is fixed,
> what is tested and what is still open.

## Features

- **Boss tactics for every TBC raid:** Karazhan, Gruul's Lair, Magtheridon, Serpentshrine Cavern, Tempest Keep,
  Mount Hyjal, Black Temple, Zul'Aman and Sunwell Plateau. The `tbc raid` strategy activates automatically inside a raid.
- **Raid coordination:** kill orders, positioning, spreading, tank swaps, kiting and threat resets per boss.
- **Raid buffs on command:** Bloodlust/Heroism (`lust`), paladin aura assignment per subgroup (`auras auto`),
  resistance auras on matching bosses, warlock curse coordination.
- **Gear and loot:** TBC rotations, gems, tier tokens and loot rolls that explain themselves (`roll feedback`).
- **Server website** (Apache/PHP) with account registration (SRP6), realm status, armory, rankings and a guild progress race.
- **Plain source code in the repo:** the bot module lives in `src/modules/PlayerBots/`, so changes to the bots are
  normal commits instead of being overwritten by a download during the build.

## What is (and isn't) included

MangosPro contains **only the server source code**. It does not include a game client or any data extracted from
one (maps, vmaps, mmaps, DBC). You extract these yourself from your own 2.4.3 client with the extractors that come
with the core (`-DBUILD_EXTRACTORS=ON`).

## Quick start

### Build on Linux

```bash
sudo apt install build-essential cmake git libboost-all-dev default-libmysqlclient-dev libssl-dev zlib1g-dev libbz2-dev
git clone https://github.com/puRe991/MangosPro.git && cd MangosPro
mkdir build && cd build
cmake .. -DBUILD_PLAYERBOTS=ON -DPCH=ON -DCMAKE_INSTALL_PREFIX=$HOME/mangos-tbc
make -j$(nproc) && make install
```

### Build on Windows

Open the project in the CMake GUI, tick `BUILD_PLAYERBOTS`, generate a Visual Studio solution and build it.
Detailed steps for both platforms: [CMaNGOS installation instructions](https://github.com/cmangos/issues/wiki/Installation-Instructions).

### Databases

1. **World DB:** [cmangos/tbc-db](https://github.com/cmangos/tbc-db). Keep its revision in step with the core.
2. **Playerbot tables:**
   - `src/modules/PlayerBots/sql/characters/*.sql` go into the characters DB.
   - `src/modules/PlayerBots/sql/world/*.sql` and `src/modules/PlayerBots/sql/world/tbc/*.sql` go into the world DB.
3. **MangosPro changes:** see [sql/mangospro/README.md](sql/mangospro/README.md) (for example raid-wide Bloodlust/Heroism).
4. **Bot configuration:** `aiplayerbot.conf` is created from `aiplayerbot.conf.dist` on install.

> **Using an existing repack:** you can only keep running a repack with this core if it is also based on CMaNGOS TBC
> and its DB revision matches. Otherwise set up fresh DBs from `tbc-db` (back up your characters DB first).

## Useful bot commands

Whisper a bot, or use party/raid chat to address all bots.

| Command | Effect |
|---|---|
| `lust` | One ready shaman casts Bloodlust/Heroism right away; the others keep theirs |
| `lust hold` / `lust auto` | Save Bloodlust/Heroism for the `lust` command / let shamans time it per boss again |
| `auras` | Every paladin reports its aura and the blessings it keeps up |
| `auras auto` | The paladins of each subgroup share out the auras between them |
| `roll feedback` | Bots explain their loot rolls in chat |

Full list, rotations, gems and tank threat: [doc/Bot_Raidbuffs_und_Ausruestung.md](doc/Bot_Raidbuffs_und_Ausruestung.md).
The original Playerbots command reference is in [doc/PlayerBot/commands.txt](doc/PlayerBot/commands.txt).

## Documentation

Most project docs are in German for now; English translations will follow.

| Topic | File |
|---|---|
| Status per boss | [doc/Bot_Boss_Status.md](doc/Bot_Boss_Status.md) |
| Raid tactics, kill orders, known limits | [doc/TBC_Raid_Taktiken.md](doc/TBC_Raid_Taktiken.md) |
| Rotations, raid buffs, gear, threat | [doc/Bot_Raidbuffs_und_Ausruestung.md](doc/Bot_Raidbuffs_und_Ausruestung.md) |
| Extra SQL changes | [sql/mangospro/README.md](sql/mangospro/README.md) |
| Server website | [website/README.md](website/README.md) |

Relevant code:
- `src/modules/PlayerBots/playerbot/strategy/generic/TbcRaidEncounters.cpp`: boss table for the TBC raids
- `src/modules/PlayerBots/playerbot/strategy/generic/*DungeonStrategies.*`: older boss strategies
- `src/modules/PlayerBots/playerbot/strategy/actions/*DungeonActions.*`: boss-specific actions
- `src/modules/PlayerBots/playerbot/strategy/generic/DungeonStrategy.*`: activation per instance/boss

## Reporting bugs

Found a boss where the bots fail? Please open an issue with the **Boss bug** template. It asks for the boss, your raid
setup and a log, which makes the problem much faster to fix. There are also templates for build problems and feature requests.
Contributions are welcome, see [CONTRIBUTING.md](CONTRIBUTING.md).

## Credits and license

| Part | Upstream | Imported commit | Path |
|------|----------|-----------------|------|
| Core | [cmangos/mangos-tbc](https://github.com/cmangos/mangos-tbc) | `4e329ff72a4f9c88bb754a10cf57c2a50ffaff2e` | `/` |
| Playerbots | [cmangos/playerbots](https://github.com/cmangos/playerbots) (bot AI by ike3) | `2f5ace0061f2c94dcc92f6af5dd2c427dfa932d6` | `src/modules/PlayerBots/` |

Both are licensed under GPL-2.0 (see `LICENSE`), and so is MangosPro. Only the Classic and WotLK world SQL files
(`sql/world/classic`, `sql/world/wotlk`) were removed from the Playerbots module, because TBC does not need them.

Upstream downloads the Playerbots module with CMake `FetchContent` at configure time, which would overwrite local
changes. In MangosPro the code is part of the repo, and `src/CMakeLists.txt` uses `src/modules/PlayerBots` directly when it exists.

World of Warcraft® is a trademark of Blizzard Entertainment, Inc. MangosPro is not affiliated with or endorsed by Blizzard.

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
