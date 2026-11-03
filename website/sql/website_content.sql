-- MangosPro Website – Startinhalte: News, Patch-Notes (Updates) und Events.
-- Optional nach website.sql einspielen:  mysql -u mangos -p tbcrealmd < website/sql/website_content.sql
-- Mehrfaches Einspielen ist unschädlich (Beiträge werden über den Titel erkannt).
-- Texte und Daten danach gern im Admin-Bereich anpassen.

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.0 – Serverstart: TBC-Core mit Playerbots', 'update', 'Der Grundstein ist gelegt: MangosPro läuft auf **CMaNGOS mangos-tbc** (Client 2.4.3) mit fest eingebautem **Playerbots**-Modul.

## Neu
- Core: CMaNGOS mangos-tbc, alle TBC-Zonen, Dungeons, Schlachtfelder und Arena
- Playerbots direkt im Server-Code statt nachgeladen – eigene Verbesserungen bleiben erhalten
- Erste Boss-Taktiken für die Bots in allen TBC-Raids (Strategie `tbc raid`, im Raid automatisch aktiv)
- Rotationen, Raid-Buffs und Ausrüstungsbewertung für TBC statt WotLK-Werten

## Playerbots
- Tank-Bedrohung: Spieler in Verteidigungshaltung, Bärengestalt oder mit Zorn der Gerechtigkeit erzeugen dreifache Bedrohung (`TankThreatMultiplier`)
- Tier-Token (T4–T6) werden von Bots wie das passende Set-Teil bewertet und nach dem Gewinn sofort getauscht
- Chatbefehle `lust` (Kampfrausch/Heldentum auf Kommando) und `auras auto` (Auren-Verteilung der Paladine)

## Hinweis
- Für Kampfrausch/Heldentum auf den ganzen Raid muss `sql/mangospro/world/01_raidwide_bloodlust_heroism.sql` eingespielt sein.', 'ADMINISTRATOR', 1, 0, '2026-10-15 12:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.0 – Serverstart: TBC-Core mit Playerbots');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.1 – Eigene Boss-Taktiken für alle neun TBC-Raids', 'update', 'Jeder TBC-Raid hat jetzt eine eigene Boss-Strategie für die Bots. Sie wird beim Betreten der Instanz automatisch aktiviert.

## Neu
- **Gruul''s Lair:** Maulgar-Rat mit Tank-Zuweisung und Kill-Reihenfolge, Gruul mit Positionierung, Verteilen bei Shatter, Cave In
- **Magtheridon:** Kanalisierer, Banish/Fear für Abyssals, Würfel-Timing pro Druckwelle mit frischen Klickern
- **Karazhan:** Attumen bis Nightbane – u. a. Maiden-Säulen, Oper (Wolf, Romulo & Julianne, Zauberer von Oz), Aran, Netherspite-Strahlen, Malchezaars Infernos
- **Serpentshrine Cavern:** Hydross-Formwechsel, Lurker-Geysir, Leotheras-Dämonentank, Karathress, Morogrim, Vashj mit Kern-Weitergabe zu den Generatoren
- **Festung der Stürme:** Al''ar-Plattformen und Sturzflug, Leerhäscher-Kugeln, Solarian, Kael''thas mit Beratern, Waffen und Gravitationsverlust
- **Hyjal:** Winterchill, Anetheron, Kaz''rogal, Azgalor und Archimonde (Erdstoß-Totem, Luftstoß, Verdammnisfeuer)
- **Black Temple:** Naj''entus bis Illidan inklusive Teron Gorefiend, Reliquiar, Shahraz und Illidari-Rat
- **Zul''Aman:** Akil''zon-Sturm, Nalorakk-Tankwechsel, Jan''alai-Bomben, Halazzi, Hex Lord und Zul''jin-Phasen

## Core
- `MotionMaster::MoveJump` implementiert (wird für den Sprung vom Al''ar-Balkon gebraucht)', 'ADMINISTRATOR', 1, 0, '2026-10-16 18:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.1 – Eigene Boss-Taktiken für alle neun TBC-Raids');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.2 – Rotationen, Verbrauchsgüter und viele Raid-Korrekturen', 'update', 'Ein großes Paket nach den ersten Testraids: Die Bots spielen ihre Klassen besser und überleben mehr Mechaniken.

## Playerbots
- Neue TBC-Rotationsregeln für Druide, Jäger, Paladin, Schurke, Schamane, Hexenmeister und Krieger
- Bots benutzen im Raid Fläschchen, Elixiere und Nahrung und halten Verzauberungen und Edelsteine auf ihrer Ausrüstung aktuell
- Ersatz-Tanks: fehlt bei Maulgar ein Tank, übernimmt ein geeigneter Bot Olm bzw. Blindeye
- Raid-Off-Spec wird berücksichtigt; bessere Zielwahl, Heilung und Bewegung
- Dungeon-Bossstrategien ergänzt
- Encounter-Korrekturen für Gruul, Magtheridon, Karazhan, SSC, TK, ZA, Hyjal und Black Temple

## Core
- **Al''ar** fliegt jetzt gerade Bahnen, mit Bewegungs-Watchdog und sicherer Wiedergeburt
- **Akil''zon:** Elektrischer Sturm trifft das hochgehobene Sturmziel nicht mehr
- Neue Option `Hyjal.SkipTrashWaves`: Jaina/Thrall rufen die Bosse direkt, ohne Trash-Wellen
- Neue Option `SSC.DisableCoilfangFrenzy`: die Fische im Wasser von SSC lassen sich abschalten
- Playerbots verlieren keine Haltbarkeit mehr

## Behoben
- Rückstoß-Effekte schleudern Bots nicht mehr unter die Decke
- Überlauf im Befehlsserver der Bots, Fehler bei Login-Kriterien und Gildenzuordnung

## Bekannte Probleme
- **Netherspite:** noch kein Kill mit Bots, der Boss heilt sich nach der Verbannungsphase
- **Leotheras:** Bots kommen bisher nur bis ca. 92 % – wird im nächsten Update angegangen', 'ADMINISTRATOR', 1, 0, '2026-10-21 20:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.2 – Rotationen, Verbrauchsgüter und viele Raid-Korrekturen');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.3 – Maulgar, Hydross, Leotheras, Archimonde und Black Temple', 'update', 'Gezielte Korrekturen für Bosse, an denen Bot-Raids bisher gescheitert sind, dazu Service-NPCs für echte Spieler.

## Neu
- **Service-NPCs** für Spieler-Ausrüstung: Verzauberer (NPC 900100) und Juwelenschleifer (NPC 900101)
- Hexenmeister-Bots stimmen ihre Flüche im Raid ab: einer hält Fluch der Elemente, einer Fluch der Tollkühnheit (bei genug Nahkämpfern), der Rest Verdammnis oder Pein; Dämonenwahl je nach Skillung

## Playerbots
- **Maulgar:** Kill-Reihenfolge nach Guide – nach Blindeye und Olm geht der Nahkampf auf Maulgar, Fernkampf tötet Kiggler und danach Krosh; Hilfstanks spotten Maulgar nicht mehr weg
- **Hydross:** in Frostform verteilen sich alle außer den Tanks auf 9 m (Wassergrab)
- **Leotheras:** Schadenspause nach jedem Wirbelwind, Verteilen in Menschengestalt, Hexenmeister-Tank nach Feuerwiderstand mit zwei festen Heilern, Cooldowns für die letzten 15 %
- **Archimonde:** Langsamer Fall bei Luftstoß, Griff der Legion wird entflucht, Fernkampf bleibt im Erdstoß-Totem der eigenen Gruppe, Tank zieht den Boss aus dem Verdammnisfeuer, zweiter Kampfrausch vor Hand des Todes
- **Naj''entus:** Fernkampf auf Ringen, Nahkampf in zwei Flankengruppen, ein Bot wirft den Stachel
- **Supremus:** Blick-Ziel kitet mit Sicherheitsabstand, DPS warten nach dem Bedrohungs-Reset 5 Sekunden
- **Hyjal:** Tank- und Spott-Korrekturen; **Leerhäscher:** feste Plätze für den Fernkampf
- Würfeln: Bots würfeln zuverlässiger, bewerten den schwächeren Ring- bzw. Schmuckplatz richtig und nutzen eigene Verbrauchsgüter mit eigenem Timer

## Bekannte Probleme
- **Netherspite** ist weiterhin offen
- Die meisten Raid-Fixes sind gebaut, aber noch nicht durch einen Kill mit Bots bestätigt – Rückmeldungen aus euren Raids helfen sehr!', 'ADMINISTRATOR', 1, 0, '2026-10-23 21:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.3 – Maulgar, Hydross, Leotheras, Archimonde und Black Temple');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.4 – Neue Website mit Account-Verwaltung und Progress Race', 'update', 'Die Server-Website ist da: Account erstellen und verwalten, ohne GM fragen zu müssen.

## Website
- Registrierung direkt im Browser, gleiche Zugangsdaten wie im Spiel
- Account-Bereich mit allen Charakteren, Passwort und E-Mail ändern, „Passwort vergessen“ per Mail
- Live-Serverstatus mit Spielerzahl, Allianz/Horde-Verteilung und aktiven Bots
- Online-Liste, Ranglisten (Stufe, Ehrenhafte Siege, Arena, Spielzeit) und Arsenal mit Charakter- und Gildenseiten
- **Progress Race:** Boss-Kills der Gilden werden automatisch aus den Raid-IDs erkannt, mit Realm-Firsts und Gildenwertung
- Diese Changelog-Seite

## Hinweis
- Random-Bots tauchen in Listen und Ranglisten nicht auf, werden aber als „Bots unterwegs“ gezählt
- Kein Tracking, keine externen Dienste – nur ein technisch notwendiges Sitzungs-Cookie', 'ADMINISTRATOR', 1, 0, '2026-11-03 18:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.4 – Neue Website mit Account-Verwaltung und Progress Race');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Die neue Website ist online', 'news', 'Ab sofort könnt ihr euren Account selbst erstellen und verwalten.

Unter **Account erstellen** registriert ihr euch in einer halben Minute. Im Bereich **Mein Account** seht ihr alle eure Charaktere, könnt Passwort und E-Mail ändern und findet die Verbindungsanleitung.

## Was es sonst noch gibt
- **Online:** Wer ist gerade unterwegs und wo?
- **Rangliste:** Stufe, Ehrenhafte Siege, Arena-Teams und Spielzeit
- **Arsenal:** Charaktere und Gilden suchen
- **Progress Race:** Wer legt die Raidbosse zuerst?
- **Updates:** Alle Patch-Notes an einem Ort

Fehler gefunden oder Wünsche? Schreibt uns – die Seite wird weiter ausgebaut.', 'ADMINISTRATOR', 1, 0, '2026-11-03 18:30:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Die neue Website ist online');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Progress Race eröffnet: Wer holt den ersten Realm-First?', 'event', 'Das Rennen um die Scherbenwelt beginnt! Ab heute zeichnet die Website jeden ersten Boss-Kill einer Gilde automatisch auf.

## So funktioniert es
- Gewertet wird die Gilde, die in einer Raid-ID die meisten Mitglieder stellt – mindestens 5 im 10er- und 10 im 25er-Raid
- **Eure eigenen Playerbots zählen mit**, wenn sie in der Gilde sind. Eine kleine Gilde mit gut ausgerüsteten Bots kann also ganz vorne mitspielen
- Gilden, die von Random-Bots geführt werden, laufen außer Konkurrenz
- Die Rangfolge richtet sich nach der höchsten Raidstufe: Sunwell vor Tier 6, vor Tier 5, vor Zul''Aman, vor Tier 4. Bei Gleichstand gewinnt, wer zuerst dort war

## Die Stationen
- **Tier 4:** Karazhan, Gruul''s Lair, Magtheridon
- **Tier 5:** Serpentshrine Cavern, Festung der Stürme
- **Zul''Aman**
- **Tier 6:** Hyjal, Black Temple
- **Sunwell Plateau**

Den aktuellen Stand seht ihr jederzeit unter **Progress**. Viel Erfolg – und vergesst nicht, `lust` im richtigen Moment zu rufen!', 'ADMINISTRATOR', 1, 1, '2026-11-03 19:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Progress Race eröffnet: Wer holt den ersten Realm-First?');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Guide: So spielst du mit Playerbots im Raid', 'news', 'Die Bots spielen die Boss-Taktiken selbstständig – mit ein paar Befehlen holt ihr aber deutlich mehr heraus. Alle Befehle funktionieren per Flüstern an einen Bot oder im Gruppen-/Schlachtzugschat an alle.

## Gruppe zusammenstellen
- `.bot add Name` loggt einen eigenen Charakter (gleicher Account) als Bot ein
- Random-Bots ladet ihr einfach mit `/invite Name` ein
- `follow`, `stay`, `attack` und `summon` steuern die Gruppe

## Kampfrausch / Heldentum
- `lust` – ein bereiter Schamane zündet sofort, die anderen halten ihren Cooldown zurück
- `lust hold` – Schamanen warten auf euer Kommando
- `lust auto` – zurück zum automatischen Timing (z. B. Vashj unter 49 %, Kael''thas unter 50 %, Illidan unter 30 %, Kil''jaeden unter 25 %)

## Paladin-Auren
- `auras auto` – die Paladine jeder Untergruppe verteilen die Auren sinnvoll (Tank-, Nahkampf-, Caster-Gruppe)
- `auras` – jeder Paladin nennt Aura und Segen
- Bei Bossen mit Widerstandsaura stellt der erste Paladin der Gruppe automatisch um

## Beute
- Bots würfeln Bedarf erst ab 5 % Verbesserung; mit echten Spielern in der Gruppe würfelt nur der Bot mit dem größten Gewinn Bedarf
- `roll feedback` – der Bot begründet seine Würfe im Chat

## Tipps
- Nehmt drei Tanks mit, wenn ihr könnt – viele Begegnungen (Rat, Karathress, Lurker-Wächter) laufen damit deutlich stabiler
- Eure Ausrüstung lasst ihr beim Verzauberer (NPC 900100) und Juwelenschleifer (NPC 900101) aufwerten', 'ADMINISTRATOR', 1, 0, '2026-10-24 16:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Guide: So spielst du mit Playerbots im Raid');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Stand der Bot-Raids: Wo es noch hakt', 'news', 'Ehrlich gesagt: Nicht jeder Boss liegt schon zuverlässig mit Bots. Hier ein Überblick, damit ihr eure Raidabende planen könnt.

## Läuft gut
- Die meisten Bosse in Karazhan, Gruul''s Lair und Magtheridon haben gezielte Korrekturen erhalten
- Al''ar und Akil''zon wurden auch serverseitig repariert

## In Arbeit
- **Netherspite:** Der Boss heilt sich nach der Verbannungsphase, bisher kein Kill mit Bots
- **Leotheras:** Bots kamen bislang nur bis ca. 92 % (mit Update 1.3 überarbeitet – bitte testen!)
- **Rat, Karathress, Lurker-Wächter:** brauchen oft mehr als zwei Tanks; Ersatz-Tanks gibt es bisher nur bei Maulgar

Wenn euch in einem Kampf etwas auffällt, meldet euch mit Boss, Gruppenaufstellung und Uhrzeit – das hilft beim Nachvollziehen in den Logs enorm.', 'ADMINISTRATOR', 1, 0, '2026-10-22 15:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Stand der Bot-Raids: Wo es noch hakt');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Wartung: Neustart für Update 1.3', 'maintenance', 'Der Server wird für das Einspielen von **Update 1.3** kurz neu gestartet. Rechnet mit etwa 15 Minuten Ausfallzeit.

Laufende Raid-IDs bleiben erhalten. Bitte loggt euch vorher an einem sicheren Ort aus.', 'ADMINISTRATOR', 1, 0, '2026-10-23 20:30:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Wartung: Neustart für Update 1.3');
