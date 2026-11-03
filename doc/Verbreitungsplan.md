# MangosPro – Plan für Bekanntmachung und Verteilung

Stand: 03.11.2026 · Zeitraum: November 2026 bis Februar 2027

## 1. Ausgangslage

**Was wir haben**
- TBC-2.4.3-Core (CMaNGOS) mit fest integrierten Playerbots, die alle TBC-Raids mit Boss-Taktiken spielen sollen.
- Ausführliche Doku zu Raid-Taktiken, Rotationen, Raid-Buffs und Befunden je Boss (`doc/`).
- Eine fertige Server-Website mit Registrierung, Arsenal und Progress Race (`website/`).
- Windows-Build-Workflow (`.github/workflows/windows-release.yml`), aus dem Upstream übernommen.

**Was fehlt, bevor wir laut werden**
- **Nachweis.** Laut `doc/Bot_Boss_Status.md` stehen fast alle Bosse auf „Fix im Code“, kaum einer auf „Kill“. Eine Ankündigung „Bots raiden Black Temple“ ohne Video wird in der Szene sofort zerlegt.
- **Englische Darstellung.** README und Doku sind deutsch; der Großteil der Zielgruppe liest Englisch.
- **Ein Release.** Es gibt keinen Git-Tag und keine Release-Seite mit fertigen Binaries.
- **Eine Anlaufstelle.** Es gibt keinen Discord und keine Issue-Vorlagen für Bug-Reports aus dem Raid.

## 2. Positionierung

**Ein Satz:** *MangosPro ist ein TBC-2.4.3-Server, auf dem man jeden TBC-Raid allein mit Bots spielen kann.*

**Zielgruppen, nach Priorität**
1. **Solo- und Kleingruppen-Spieler**, die TBC-Raids erleben wollen, ohne 25 Leute zu finden. Größte Gruppe, verbreitet Videos.
2. **Self-Hoster / Repack-Nutzer**, die schon CMaNGOS oder AzerothCore mit Bots betreiben und bessere Raid-KI suchen.
3. **Entwickler** aus der CMaNGOS- und Playerbots-Szene, die Code beisteuern oder unsere Fixes übernehmen.

**Abgrenzung** (das, was uns von anderen Projekten unterscheidet)
- Boss-spezifische Taktiken für *alle* TBC-Raids, nicht nur generisches „Bot folgt und haut drauf“.
- Bots, die Raid-Buffs, Kampfrausch, Edelsteine, Würfelregeln und Tank-Bedrohung beherrschen.
- Ehrlicher Status pro Boss, öffentlich gepflegt.

**Tonfall:** sachlich, ehrlich, mit Belegen. Lieber „12 von 37 Bossen mit Bots gelegt, Video anbei“ als „die besten Bots aller Zeiten“.

## 3. Rechtliche Leitplanken (gelten für alle Phasen)

- **Nur Server-Code verteilen.** Keine Spiel-Clients, keine extrahierten Daten (maps, vmaps, mmaps, dbc). Die erzeugt jeder Nutzer selbst mit den Extraktoren aus seinem eigenen Client.
- **GPL-2.0 einhalten:** Quellcode bleibt öffentlich, Upstream-Herkunft (CMaNGOS, Playerbots/ike3) wird in README und Release-Notes genannt.
- **Keine Blizzard-Logos oder -Grafiken** in Website, Banner oder Thumbnails. „World of Warcraft“ nur beschreibend nennen. Spielszenen-Screenshots für Videos sind in der Szene üblich, aber nicht als Logo/Werbegrafik verwenden.
- **Kein Geld mit dem Spiel verdienen.** Keine Item-Shops, keine bezahlten Vorteile. Sonst steigt das Abmahn- bzw. Takedown-Risiko deutlich. Spenden höchstens für Serverkosten, transparent.
- **Öffentlicher Spieler-Server ist optional** (siehe Phase 4) und ein eigenes Risiko. Das Open-Source-Projekt zum Selbsthosten ist der Kern der Verbreitung.

## 4. Phasen

### Phase 0 – Vorbereitung (03.11. – 23.11.2026)

| # | Aufgabe | Wer |
|---|---|---|
| 0.1 | Englische README (`README.md` englisch, deutsche Fassung als `README.de.md`), mit Feature-Liste, Screenshot, Schnellstart, Status-Tabelle | Claude |
| 0.2 | Upstream-Badges in der README auf eigene Workflows umstellen | Claude |
| 0.3 | Issue-Vorlagen: „Boss-Bug“ (Boss, Raid-Zusammensetzung, Log, Video-Link), „Build-Problem“, „Feature-Wunsch“ | Claude |
| 0.4 | Release-Workflow anpassen: bei Git-Tag `v*` Windows-Zip (Server + Playerbots + AHBot) und Linux-Build bauen und als GitHub-Release anhängen | Claude |
| 0.5 | Docker-Compose für Linux (Core + MariaDB + optional Website), damit Self-Hoster in 15 Minuten laufen | Claude |
| 0.6 | GitHub-Repo: Beschreibung, Topics (`cmangos`, `playerbots`, `tbc`, `wow-emulator`, `burning-crusade`, `bots`, `raid-ai`), Social-Preview-Bild | Du (Einstellungen), Bild: Claude |
| 0.7 | **Nachweis-Videos aufnehmen:** mindestens 3 Kills mit Bots (Vorschlag: Attumen, Gruul, Magtheridon), je 5–10 Minuten, ungeschnitten + ein 60-s-Zusammenschnitt | Du (Aufnahme), Schnittliste: Claude |
| 0.8 | `doc/Bot_Boss_Status.md` nach den Tests aktualisieren, „Kill“-Einträge mit Video-Link | gemeinsam |
| 0.9 | Discord-Server anlegen: #ankündigungen, #installation-hilfe, #boss-bugs, #videos, #entwicklung (zweisprachig) | Du |

**Abschluss:** Release `v1.0.0` getaggt, Binaries hängen dran, README englisch, mindestens 3 Kill-Videos online.

### Phase 1 – Soft-Launch in der Fachszene (24.11. – 14.12.2026)

Ziel: Feedback von Leuten, die solche Server schon betreiben, bevor das breite Publikum kommt.

| Kanal | Inhalt | Wer |
|---|---|---|
| Playerbots-Discord (Link in `src/modules/PlayerBots/README.md`) | Kurzvorstellung, Link auf Status-Tabelle und Videos, Frage nach Testern | Du postest, Text: Claude (Abschnitt 6) |
| CMaNGOS-Discord | Gleiche Vorstellung, Fokus Technik | Du, Text: Claude |
| **Upstream-Beiträge** | 2–3 allgemein nützliche Fixes (z. B. Bosssuche, Raid-Buff-Logik) als Pull Requests an `cmangos/playerbots`. Bringt Glaubwürdigkeit und Sichtbarkeit bei den Entwicklern | Claude bereitet vor, Du reichst ein |

**Abschluss:** mindestens 5 externe Tester, erste Bug-Reports über die Issue-Vorlagen, gröbste Installationsprobleme behoben (Release `v1.0.x`).

### Phase 2 – Öffentlicher Launch (15.12.2026 – 11.01.2027)

Weihnachtsferien = viel Spielzeit, gutes Zeitfenster.

| Kanal | Inhalt | Wer |
|---|---|---|
| YouTube | Launch-Video „Clearing TBC raids solo with 24 bots“ (3–6 min): Kills, kurzer Blick auf Bot-Befehle, Link auf GitHub | Du nimmst auf, Skript/Titel/Beschreibung/Kapitel: Claude |
| YouTube Shorts / TikTok | 3 Clips à 30–60 s aus dem Material (z. B. „Bots dodge Gruul's Shatter“) | Du, Schnittvorschläge: Claude |
| Reddit r/wowservers | Vorstellungspost (Text in Abschnitt 6), Self-Hosting-Projekt, nicht als Server-Werbung | Du |
| Deutschsprachige Foren / Communities (z. B. Emulator-Foren, Discord-Server deutscher Repack-Szene) | Deutscher Vorstellungstext | Du, Text: Claude |
| GitHub | Release-Notes `v1.1.0` mit „Was ist neu seit Launch“ | Claude |

**Abschluss:** Launch-Video online, Reddit-Post live, Discord wächst.

### Phase 3 – Dranbleiben (ab 12.01.2027, laufend)

- **Alle 2–4 Wochen ein Release** mit Changelog. Jedes Release = Anlass für einen Discord-Post und einen kurzen Clip.
- **„Boss der Woche“:** ein Boss wird gezielt verbessert, Vorher/Nachher-Clip.
- **Status-Tabelle** bleibt die zentrale Wahrheit; Fortschritt dort ist die beste Werbung.
- **Community-Beiträge** aktiv einladen: Issues mit Label `good first issue` (z. B. Boss-Taktik dokumentieren, Log auswerten).
- **Monatlicher Rückblick** (Discord + Website-News): was geschafft, was als Nächstes.

### Phase 4 – Optional: öffentlicher Spieler-Server

Nur wenn Phase 2 genug Interesse zeigt **und** du Hosting, Moderation und das rechtliche Risiko tragen willst. Die Website (`website/`) ist dafür fertig. Empfehlung: zuerst als kleiner Test-Realm „Bots-Raid-Demo“ ohne Spenden, ohne Shop, mit klaren Regeln. Gelistet würde er dann auf den üblichen Server-Listen – das entscheiden wir getrennt.

## 5. Erfolgskennzahlen

| Kennzahl | Ende Phase 1 | Ende Phase 2 | März 2027 |
|---|---|---|---|
| GitHub-Sterne | 25 | 150 | 300 |
| Release-Downloads (Summe) | 30 | 300 | 1.000 |
| Discord-Mitglieder | 20 | 150 | 400 |
| Externe Issues / PRs | 5 / 0 | 30 / 2 | 60 / 8 |
| Bosse mit Status „Kill“ | 3 | 8 | 20 |
| Launch-Video Aufrufe | – | 5.000 | 15.000 |

Die Zahlen sind Schätzwerte für eine Nische; wir prüfen sie am Ende jeder Phase und passen sie an.

## 6. Fertige Texte

### 6.1 Reddit r/wowservers (Englisch)

> **[Open Source] MangosPro – TBC 2.4.3 core where playerbots actually do raid mechanics**
>
> I've been working on a fork of CMaNGOS TBC with the Playerbots module built in. The goal: you can run every TBC raid with a bot raid and the bots handle the boss mechanics instead of just tank-and-spank.
>
> What's in it:
> - Boss tactics for Karazhan, Gruul, Magtheridon, SSC, TK, Hyjal, BT, ZA and Sunwell
> - Raid buffs and Bloodlust/Heroism on command, gems, loot rules, tank threat handling
> - Per-boss status table that's honest about what works and what doesn't: [link]
>
> Videos: [Attumen] · [Gruul] · [Magtheridon]
>
> It's GPL-2.0, self-hosted, no client or game data included – you extract your own. Windows builds are on the Releases page, Linux via Docker.
>
> GitHub: https://github.com/puRe991/MangosPro · Discord: [link]
>
> Feedback and boss bug reports (ideally with a log) are very welcome.

### 6.2 Discord-Kurzvorstellung (Englisch, für Playerbots-/CMaNGOS-Discord)

> Hi! I maintain **MangosPro**, a CMaNGOS TBC fork with Playerbots built in, focused on making bots handle TBC raid bosses. Boss table + status per boss: [link]. Kill videos: [link]. If any of the fixes are useful upstream I'm happy to send PRs. Looking for a few testers who already run TBC bots. https://github.com/puRe991/MangosPro

### 6.3 Deutscher Vorstellungstext (Foren, Discord)

> **MangosPro – TBC 2.4.3 mit Bots, die raiden können**
>
> MangosPro ist ein Open-Source-Server auf Basis von CMaNGOS TBC mit fest eingebauten Playerbots. Ziel: Jeden TBC-Raid allein mit Bots spielen – und die Bots beherrschen die Boss-Mechaniken.
>
> - Taktiken für alle TBC-Raids von Karazhan bis Sonnenbrunnen
> - Raid-Buffs, Kampfrausch per Befehl, Edelsteine, Würfelregeln, Tank-Bedrohung
> - Ehrliche Status-Tabelle pro Boss: [Link]
> - Fertige Server-Website mit Registrierung, Arsenal und Progress Race
>
> Videos: [Links] · Download und Anleitung: https://github.com/puRe991/MangosPro · Discord: [Link]
>
> Kein Client, keine Spieldaten, kein Shop – nur der Server-Code unter GPL-2.0.

## 7. Aufgabenteilung

**Übernehme ich (Claude) im Repo:** englische README, Issue-Vorlagen, Release- und Docker-Setup, Release-Notes, Social-Preview-Grafik (eigene Gestaltung, keine Blizzard-Grafiken), Upstream-PRs vorbereiten, alle Texte, Video-Skripte, Titel und Beschreibungen, Aktualisierung von Status-Tabelle und Changelog.

**Brauche ich von dir:** Accounts und das eigentliche Posten (Reddit, Discord, YouTube – ich poste nicht unter deinem Namen), die Gameplay-Aufnahmen, die GitHub-Repo-Einstellungen (Beschreibung, Topics, Social Preview), die Entscheidung zu Phase 4.

**Nächster Schritt:** Ich beginne mit 0.1 – 0.5. Sobald die ersten Kill-Videos (0.7) da sind, wird `v1.0.0` getaggt und Phase 1 startet.
