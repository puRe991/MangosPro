-- MangosPro Website – Startinhalte: News, Patch-Notes (Updates) und Events.
-- Optional nach website.sql einspielen:  mysql -u mangos -p tbcrealmd < website/sql/website_content.sql
-- Mehrfaches Einspielen ist unschädlich (Beiträge werden über den Titel erkannt).
-- Texte und Daten danach gern im Admin-Bereich anpassen.

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.0.0 – Serverstart: CMaNGOS TBC mit Playerbots', 'update', 'Der Grundstein: MangosPro läuft auf **CMaNGOS mangos-tbc** (Client 2.4.3) mit fest eingebautem **Playerbots**-Modul.

## Neu
- Core: CMaNGOS mangos-tbc – alle TBC-Zonen, Dungeons, Schlachtfelder und Arena
- Playerbots liegen direkt im Server-Code statt beim Bauen aus dem Internet nachgeladen zu werden – eigene Verbesserungen bleiben dadurch erhalten
- Classic- und WotLK-Welt-SQL des Bot-Moduls entfernt, weil sie für TBC nicht gebraucht werden', 'ADMINISTRATOR', 1, 0, '2026-10-15 03:30:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.0.0 – Serverstart: CMaNGOS TBC mit Playerbots');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.1.0 – Raid-Taktiken für alle TBC-Raids', 'update', 'Die Bots bekommen eine eigene, datengetriebene Raid-Strategie `tbc raid`. Sie schaltet sich in jedem TBC-Raid automatisch ein.

## Neu
- Bodeneffekten, gefährlichen Objekten und Kreaturen ausweichen (Feuer, Wirbelwinde …)
- Bei gefährlichen Debuffs verteilen oder vom Raid weglaufen
- Kill-Reihenfolge für Adds, gemeinsames Fokusziel
- Frontalkegel meiden, Tankwechsel bei Debuff-Stapeln und Nalorakks Gestalten
- Abdeckung: Gruul, Magtheridon, SSC, Festung der Stürme, Hyjal, Black Temple, Zul''Aman, Sunwell und die restlichen Karazhan-Bosse

## Sondermechaniken
- **Lady Vashj:** Besudelte Kerne aufheben und an aktive Schildgeneratoren bringen oder durch den Raid zum nächsten Generator weiterwerfen
- **Lurker:** bei Spout ins Wasser springen
- **Teron Gorefiend:** Rachsüchtigen Geist steuern und die Schattenkonstrukte töten
- **Kil''jaeden:** Blaue Drachenkugeln nutzen, über dem Raid fliegen, Schild der Blauen gegen Dunkelheit; der Raid sammelt sich darunter
- **Hydross:** Frosttank hält ihn im Reinigungskreis, Tankwechsel bei 100 % Mal
- **Kalecgos:** Spieler im Spektralreich töten Sathrovarr, der Rest den Drachen
- Bloodboil-Rotation, Meteor Slash teilen, Offtanks bleiben bei Gruul und Shahraz zweite auf der Bedrohungsliste', 'ADMINISTRATOR', 1, 0, '2026-10-15 10:50:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.1.0 – Raid-Taktiken für alle TBC-Raids');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.2.0 – TBC-Rotationen, Raid-Buffs und Ausrüstung', 'update', 'Die Bots spielen ihre Klassen jetzt nach TBC-Regeln statt mit WotLK-Fähigkeiten.

## Rotationen
- Wiederherstellungsschamane: Welle der Heilung statt Riptide (WotLK)
- Jäger: Zuverlässiger Schuss zwischen den automatischen Schüssen, Schlangenbiss vor Vipernbiss
- Katze: Zerfleischen-Debuff bleibt oben, Zerfetzen erst ab 5 Combopunkten
- Meucheln-Schurke: neu Vergiften und Tückische Klinge
- Furor-Krieger: Rüstung zerreißen nur ohne Kriegertank
- Vergelter: Kreuzfahrerstoß vor Exorzismus; Schutzpaladin: Weihe
- Schattenpriester, Elementar- und Verstärkungsschamane, Feuermagier und Zerstörungshexer angepasst

## Raid-Buffs
- Chatbefehle `lust` (sofort / hold / auto / ?) und `auras` (? / auto / off)
- Kampfrausch/Heldentum mit Boss-Timing: Vashj, Kael''thas, Illidan, Kil''jaeden
- Paladin-Auren je Untergruppe, Widerstandsaura je Boss, große Segen nach der häufigsten Rolle der Klasse
- SQL: Kampfrausch/Heldentum wirkt auf den ganzen Raid

## Ausrüstung
- TBC-Werte-Gewichtung für alle Skillungen (Waffen-DPS, Tank-Werte, Sockel)
- Neue Ausrüstung wird mit den besten Edelsteinen der Skillung gesockelt, Meta-Edelstein nach Rolle
- Bedarf erst ab spürbarer Verbesserung, und nur der Bot mit dem größten Gewinn
- Tier-Token werden wie das passende Set-Teil bewertet und sofort getauscht
- Bots werden in TBC endlich verzaubert (Mindeststufe 81 → 60)

## Verbessert
- Tank-Bedrohung einstellbar (`AiPlayerbot.TankThreatMultiplier`, Standard 3,0)
- Fernkämpfer weichen nicht mehr vor Gegnern zurück, die jemand anderen angreifen
- Bots fliehen nicht mehr aus Bodeneffekten befreundeter Zauberer', 'ADMINISTRATOR', 1, 0, '2026-10-15 14:40:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.2.0 – TBC-Rotationen, Raid-Buffs und Ausrüstung');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.3.0 – Eigene Boss-Strategien: Gruul, Magtheridon, Karazhan', 'update', 'Neues Grundgerüst: Jeder Raid kann eine eigene Boss-Strategie haben, die beim Betreten automatisch aktiv wird und beim Verlassen wieder aus.

## Neu
- Gemeinsame Boss-Hilfsfunktionen (Port der EncounterHelpers aus mod-playerbots) und eine Tabelle Map → Strategie
- **Gruul''s Lair:** Maulgar-Rat mit Tank-Zuweisung, Kill-Reihenfolge, Wirbelwind, Krosh und Teufelsjäger; Gruul mit Tankposition, Verteilen, Einsturz und Zerschmettern
- **Magtheridon:** Kanalisierer tanken und in fester Reihenfolge töten, Höllenbestien verbannen bzw. fürchten, Würfel-Timing pro Druckwelle mit frischen Klickern nach der Erschöpfung, Trümmern ausweichen
- **Karazhan:** Attumen, Moroes, Maiden (Säulenplätze), Großer böser Wolf, Romulo & Julianne, Zauberer von Oz, Kurator, Illhoof, Arans Schemen, Netherspite-Strahlen, Prinz Malchezaar (Schwächung, Infernos) und Nightbane (Terrasse, Flugphase)', 'ADMINISTRATOR', 1, 0, '2026-10-16 02:20:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.3.0 – Eigene Boss-Strategien: Gruul, Magtheridon, Karazhan');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.4.0 – Boss-Strategien für SSC, TK, Hyjal, Black Temple und Zul''Aman', 'update', 'Damit hat jeder TBC-Raid seine eigene Boss-Strategie.

## Neu
- **Serpentshrine Cavern:** Hydross-Tankplätze und Formwechsel, Lurker-Wächter und Geysir, Leotheras-Dämonentank und innere Dämonen, Karathress-Tankplätze und Kill-Reihenfolge, Morogrim-Phasen, Vashj mit Schreitern, Statischer Aufladung und Kern-Weitergabe
- **Festung der Stürme:** Al''ar-Plattformrotation, Glut, Flammenkiele und Sturzflug, Leerhäscher-Position und Arkankugeln, Solarians Priester und Zorn, Kael''thas mit Beratern, Waffen (werden aufgehoben und benutzt), Phönixen, Gedankenkontrolle und Gravitationsverlust
- **Hyjal:** Winterchill, Anetheron (Höllenbestien-Tank), Kaz''rogal (Mana-Mal, Spalten), Azgalor (Verdammniswachen-Tank, Feuerregen), Archimonde (Erdstoß-Totem, Luftstoß, Verdammnisfeuer)
- **Black Temple:** Naj''entus-Stacheln, Supremus-Blick und Vulkane, Akamas Kanalisierer, Teron Gorefiend, Bloodboil-Gruppenrotation, Reliquiar der Seelen, Shahraz (Säule, Tödliche Anziehung), Illidari-Rat mit Tank-Zuweisungen, Illidan-Phasen
- **Zul''Aman:** Medizinmann-Totems, Akil''zons Sturm, Nalorakk-Tankwechsel, Jan''alai-Bomben und Brüter, Halazzi, Hex Lord Malacrass und Zul''jin-Phasen
- Boss-Strategien reagieren jetzt auch während eines laufenden Zaubers', 'ADMINISTRATOR', 1, 0, '2026-10-16 18:10:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.4.0 – Boss-Strategien für SSC, TK, Hyjal, Black Temple und Zul''Aman');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.4.1 – Hotfix: Server-Build für Al''ars Balkonsprung', 'update', '## Behoben
- `MotionMaster::MoveJump` war im Core nur deklariert, nicht umgesetzt – sobald die Festung-der-Stürme-Taktik (Sprung vom Al''ar-Balkon) ihn aufrief, ließ sich der Server nicht mehr bauen
- TBC kennt keine Parabelsprünge: Ein Sprung nach unten wird jetzt als Fall mit horizontaler Strecke gesendet, alles andere als gerade Bewegung', 'ADMINISTRATOR', 1, 0, '2026-10-16 23:50:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.4.1 – Hotfix: Server-Build für Al''ars Balkonsprung');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.5.0 – Nach dem ersten Testraid: Tank-Aggro, Heilung und Boss-Erkennung', 'update', 'Die ersten echten Testraids am 17.10. (Gruul, Black Temple, Festung der Stürme, Karazhan) haben gezeigt, wo es hakt. Dieser Build wurde in der Nacht eingespielt.

## Behoben
- **Boss-Taktiken fielen aus:** Gespeicherte Bot-Profile konnten den Instanz-Verteiler entfernen, der die Boss-Strategien einschaltet – er wird jetzt beim Laden wieder eingesetzt
- **Attumen:** Zorn der Gerechtigkeit wurde nur außerhalb des Kampfes erneuert (bei einem Paladin-Tank nur in 19 von 256 Kampfaufnahmen aktiv) – jetzt auch im Kampf, z. B. nach einem Tod
- **Schutzkrieger:** Schildschlag richtet sich nach der tatsächlichen Einsetzbarkeit statt fester 20 Wut; Verwüsten blockiert nicht mehr pauschal
- **Zielwahl:** Assist-Aktionen reißen ein korrekt gewähltes Ziel nicht mehr weg, Begleiter behalten ihr Ziel
- **Maulgar/Gruul:** normales Heranlaufen hat Schritte zur Tankposition nicht mehr rückgängig gemacht
- **Kiggler:** ein Druide in Heilerrolle wird nicht mehr als Moonkin-Tank eingeteilt

## Verbessert
- **Heilung (Moroes, Maiden):** Spieler unter 20 % haben Vorrang vor der Heilerverteilung, gesunde Tanks werden nicht mehr nur wegen ihres Platzes in der Verteilung geheilt
- **Jäger** halten den Mindestabstand für den automatischen Schuss
- Gruul, Magtheridon, SSC und Festung der Stürme reagieren auf Gefahren jetzt auch im Reaktionsmodus
- Abgelehnte Zauber werden mit Grund protokolliert (`CAST_REJECTED`); neue Chatbefehle `debug raid` zum Prüfen der Boss-Taktiken', 'ADMINISTRATOR', 1, 0, '2026-10-18 00:50:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.5.0 – Nach dem ersten Testraid: Tank-Aggro, Heilung und Boss-Erkennung');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.6.0 – Folgepaket: Karazhan, Magtheridon, Akil''zon und Leotheras', 'update', 'Ausgewertet wurden die Kämpfe vom 18.10.; das Paket wurde am selben Abend eingespielt.

## Karazhan
- **Terestian:** Dämonenketten werden als Ziel erkannt – zuerst die Ketten, dann Kil''rek, dann Illhoof; zweiter Tank übernimmt Kil''rek
- **Arans Schemen:** Bei Arkaner Explosion laufen Bots 26 statt 21 m weit (der Zauber trifft genau 21 m) und prüfen nach 200 ms erneut; der Flammenkranz sperrt alle konkurrierenden Laufwege
- **Netherspite:** zusätzliche direkte Portalsuche, nur der eingeteilte Bot fängt den jeweiligen Strahl
- **Prinz Malchezaar:** Flucht vor Schattennova auf 36 m; eine Infernozone darf nach außen verlassen werden (vorher war jede Fluchtrichtung blockiert)

## Raids
- **Magtheridon:** Würfel-Klicker nach Entfernung ausgewählt, Vorbereitung schon ab 30 statt 48 Sekunden, Ersatz nach Tod oder Erschöpfung; ein Klick zählt erst bei bestätigtem Kanal. DDs ignorieren verbannte Höllenbestien
- **Akil''zon:** Das hochgehobene Sturmziel wird vom Sturmschaden ausgenommen (Core), Folgestürme alle 54–60 Sekunden
- **Leotheras:** Der eigene innere Dämon hat Vorrang bei der Zielwahl, auch für Heiler und den Hexenmeister-Tank; Nahkampf-Tanks halten sich in der Dämonenphase zurück

## Bekannte Probleme
- Netherspite und Leotheras sind noch nicht durch einen Kill bestätigt', 'ADMINISTRATOR', 1, 0, '2026-10-18 23:30:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.6.0 – Folgepaket: Karazhan, Magtheridon, Akil''zon und Leotheras');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.6.1 – Maulgar und Gruul: weniger Tode, keine Cast-Abbrüche', 'update', 'Nach dem Testraid am Morgen des 19.10. (Maulgar mit 120, Gruul mit 61 Bot-Toden) neu gebaut und eingespielt.

## Behoben
- **Trash löste Boss-Taktik aus:** Schon die Oger vor Maulgar schalteten die Rat-Taktik ein – jetzt nur noch bei echten Bossen
- **Ständige Ausweichreaktionen:** Krosh-Ausweichen nur noch unter 17 m, Wirbelwind-Ausweichen nur bei aktiver Aura unter 12 m. Vorher wurden hunderte Zauber unnötig abgebrochen

## Verbessert
- **Gruul – Zerschmettern:** Bots bewerten den Abstand zu allen Nachbarn und wählen nur erreichbare, bessere Schritte; kein Hin-und-her-Laufen mehr
- **Gruul – Fernkampf:** Startposition 25–30 m auf der aktuellen Bossseite statt eines zufälligen Gruppenplatzes, Mindestabstand auch nach Rückstoß oder Wiederbelebung
- Verteilen unterbricht keine laufenden Zauber mehr, solange keine echte Gefahr besteht
- Playerbots verlieren keine Haltbarkeit mehr', 'ADMINISTRATOR', 1, 0, '2026-10-19 21:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.6.1 – Maulgar und Gruul: weniger Tode, keine Cast-Abbrüche');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.7.0 – Rotations-Prüfung, 35 Dungeon-Strategien und Hyjal-Option', 'update', 'Ergebnis einer kompletten Prüfung aller Klassenrotationen (20.10.) und der Auswertung aller 26 Kämpfe vom 19.10.

## Rotationen
- **Waffenkrieger:** Zerschmettern im richtigen Schwung-Fenster, Tödlicher Stoß höher priorisiert
- **Bär:** Zerfleischen sobald bereit (nicht erst ab 40 Wut), Aufschlitzen wird 3 Sekunden vor Ablauf erneuert, Zermalmen mit eigener Wutreserve
- **Kampfschurke:** Blutung vor Ausweiden auf langlebigen Zielen
- **Wiederherstellungsdruide:** Rasche Heilung vor Nachwachsen bei kritischen Zielen
- **Schamanen:** Lufttotem nach Gruppenzusammensetzung (Zorn der Luft für Zauberer, Windzorn für Nahkämpfer)
- **Zerstörungshexer:** Füllzauber je nach Build – Feuer-Skillung Verbrennen, sonst Schattenblitz
- Jäger, Paladin und weitere Ziel-, Heil- und Bewegungsverbesserungen

## Neu
- **Dungeon-Strategien:** alle 16 TBC- und 19 Classic-Dungeons (35 Maps, 110 Boss-Profile) plus 5 Classic-Raids mit 40 Profilen – Add-Prioritäten, Abstand, Frontalangriffe, Gefahrenzonen; Sonderregeln u. a. für Ikiss, Anzu und Murmur
- **Option `Hyjal.SkipTrashWaves`:** Jaina und Thrall rufen die Bosse direkt, ohne Trash-Wellen
- **Option `SSC.DisableCoilfangFrenzy`:** die Fische im Wasser von SSC lassen sich abschalten

## Behoben
- **Karazhan:** Die Boss-Suche arbeitete mit einem falschen Radius – verließ ein Boss die kleine Suchzone, fielen Flucht- und Gefahrenregeln aus (u. a. Aran, Wolf, Maiden, Netherspite, Malchezaar). Jetzt 150 m wie bei allen Raids

## Hinweis
- Neue automatische Tests für Rotations-Prioritäten und SSC/Hyjal-Regressionen', 'ADMINISTRATOR', 1, 0, '2026-10-21 08:30:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.7.0 – Rotations-Prüfung, 35 Dungeon-Strategien und Hyjal-Option');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.7.1 – Verbrauchsgüter, Al''ar und Ersatz-Tanks', 'update', '## Neu
- **Verbrauchsgüter im Raid:** Bots nutzen Fläschchen, Elixiere und Buff-Essen
- **Ausrüstungspflege:** Verzauberungen und Edelsteine werden auf der getragenen Ausrüstung aktuell gehalten
- **Ersatz-Tanks:** Fehlt bei Maulgar ein Tank, übernimmt ein geeigneter Bot Olm bzw. Blindeye
- Raid-Off-Spec wird berücksichtigt

## Core
- **Al''ar** fliegt gerade Bahnen, mit Bewegungs-Watchdog und sicherer Wiedergeburt

## Behoben
- Rückstoß schleudert Bots nicht mehr unter die Decke
- **Magtheridon:** Würfel-Reaktionen nur noch kurz vor der Druckwelle, ohne Zauber abzubrechen
- Encounter-Korrekturen für Black Temple, Gruul, Hyjal, SSC und Festung der Stürme
- Bot-Befehlsserver: Pufferüberlauf beim Empfang behoben
- Login-Regeln: kommagetrennte Kriterien werden richtig gelesen, gildenlose Charaktere gelten nicht mehr als gleiche Gilde, Fehlerpfad beim Login aufgeräumt
- Begleiter-Heilung prüft das richtige Ziel
- Raid-KI-Logs werden rotiert

## Bekannte Probleme
- **Netherspite:** Der Boss heilt sich nach der Verbannung, bisher kein Kill mit Bots
- **Leotheras:** Bots kommen nur bis ca. 92 %
- Rat, Karathress und Lurker-Wächter brauchen meist mehr als zwei Tanks', 'ADMINISTRATOR', 1, 0, '2026-10-21 13:45:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.7.1 – Verbrauchsgüter, Al''ar und Ersatz-Tanks');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.8.0 – Maulgar-Kill-Reihenfolge, Hydross und Service-NPCs', 'update', '## Neu
- **Service-NPCs** für Spieler-Ausrüstung: Verzauberer (NPC 900100) und Juwelenschleifer (NPC 900101), Spawn mit `.npc add`

## Playerbots
- **Maulgar:** Kill-Reihenfolge nach Guide – nach Blindeye und Olm geht der Nahkampf auf Maulgar (Totenkopf), der Fernkampf tötet Kiggler, dann Krosh (Kreuz). Hilfstanks spotten Maulgar nicht mehr weg
- **Hydross:** In Frostform verteilen sich alle außer den Tanks auf 9 m (Wassergrab); Nahkämpfer verteilen sich im Ring um ihr Ziel
- **Hyjal:** Tank- und Spott-Korrekturen
- **Leerhäscher:** feste Plätze für den Fernkampf
- **Al''ar:** sauberer Sprung vom Balkon, Rückstöße stapeln sich nicht mehr

## Beute & Buffs
- Würfeln wird zuverlässiger ausgelöst
- Beim zweiten Ring- bzw. Schmuckplatz wird gegen das schwächere Teil verglichen
- Raid-Buffs nur noch im Raid, eigener Timer für Verbrauchsgüter', 'ADMINISTRATOR', 1, 0, '2026-10-23 12:30:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.8.0 – Maulgar-Kill-Reihenfolge, Hydross und Service-NPCs');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.8.1 – Naj''entus, Supremus, Leotheras, Archimonde und Hexenmeister-Flüche', 'update', '## Playerbots
- **Naj''entus:** Fernkampf auf Ringen mit Abstand, Nahkampf in zwei Flankengruppen, ein Bot wirft den Stachel, sobald der Raid hochgeheilt ist; Frostwiderstandsaura
- **Supremus:** Das Blick-Ziel kitet außerhalb seiner Reichweite mit 10 m Puffer und zaubert nicht, solange er nah ist; Irreführung nur beim Pull und bei jeder Rückkehr zur Tankphase; DPS warten nach dem Bedrohungs-Reset 5 Sekunden
- **Leotheras:** Schadenspause und Irreführung nach jedem Wirbelwind, Verteilen in Menschengestalt, Hexenmeister-Tank nach Feuerwiderstand mit zwei festen Heilern, Cooldowns für die letzten 15 %, gedankenkontrollierte Spieler werden ausgeschaltet
- **Archimonde:** Langsamer Fall bei Luftstoß, Griff der Legion wird entflucht, Fernkampf bleibt im Erdstoß-Totem der eigenen Gruppe, Tank zieht den Boss aus dem Verdammnisfeuer, Defensiv-Cooldowns bei Seelenladung, zweiter Kampfrausch vor Hand des Todes
- **Hexenmeister im Raid:** einer hält Fluch der Elemente, einer Fluch der Tollkühnheit (bei genug Nahkämpfern), der Rest Verdammnis oder Pein; Lebensentzug nur mit Schattenverwundbarkeit; Teufelswachen-Builds behalten die Teufelswache, andere opfern den passenden Dämon
- Verteilen im Nahkampf-Ring berücksichtigt jetzt die ganze Gruppe

## Bekannte Probleme
- **Netherspite** ist weiterhin offen
- Die meisten Raid-Fixes sind gebaut, aber noch nicht durch einen Kill mit Bots bestätigt – Rückmeldungen aus euren Raids helfen sehr!', 'ADMINISTRATOR', 1, 0, '2026-10-23 14:40:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.8.1 – Naj''entus, Supremus, Leotheras, Archimonde und Hexenmeister-Flüche');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.9.0 – Neue Website mit Account-Verwaltung', 'update', 'Die Server-Website ist da: Account erstellen und verwalten, ohne einen GM fragen zu müssen.

## Website
- Registrierung direkt im Browser, gleiche Zugangsdaten wie im Spiel
- Account-Bereich mit allen Charakteren, Passwort und E-Mail ändern, „Passwort vergessen“ per Mail
- Live-Serverstatus mit Spielerzahl, Allianz/Horde-Verteilung und aktiven Bots
- Online-Liste, Ranglisten (Stufe, Ehrenhafte Siege, Arena, Spielzeit) und Arsenal mit Charakter- und Gildenseiten
- Verbindungsanleitung, Regeln, Impressum, Datenschutz und News mit Admin-Editor

## Hinweis
- Random-Bots tauchen in Listen und Ranglisten nicht auf, werden aber als „Bots unterwegs“ gezählt
- Kein Tracking, keine externen Dienste – nur ein technisch notwendiges Sitzungs-Cookie', 'ADMINISTRATOR', 1, 0, '2026-11-03 02:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.9.0 – Neue Website mit Account-Verwaltung');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Update 1.9.1 – Progress Race und Changelog', 'update', '## Website
- **Progress Race:** Boss-Kills der Gilden werden automatisch aus den Raid-IDs erkannt – Realm-Firsts je Boss, Gildenwertung nach Raidstufe und eine Chronik
- Ein Kill zählt als Gilden-Kill ab **10 Gildenmitgliedern in Karazhan und Zul''Aman** und **20 in allen anderen Raids** – Spieler oder Bots
- Gildenseiten zeigen den Raid-Fortschritt, die Startseite die Top 5
- Diese Changelog-Seite mit allen Versionen; News nach Kategorien filterbar', 'ADMINISTRATOR', 1, 0, '2026-11-03 18:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Update 1.9.1 – Progress Race und Changelog');

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
- Ein Kill zählt nur als **Gilden-Kill**, wenn genug Mitglieder eurer Gilde in der Raid-ID sind: **mindestens 10 in Karazhan und Zul''Aman, mindestens 20 in allen anderen Raids**
- Mitglieder können Spieler oder Playerbots sein – **eure eigenen Bots zählen mit**, wenn sie in der Gilde sind. Eine kleine Gilde mit gut ausgerüsteten Bots kann also ganz vorne mitspielen
- Gemischte Gruppen mit zu wenigen Gildenmitgliedern werden nicht gewertet
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
SELECT 'Testraid-Bericht: 26 Kämpfe ausgewertet', 'news', 'Am Morgen haben wir einen kompletten Testraid mit Bots aufgezeichnet – von Gruul über Magtheridon bis durch ganz Karazhan – und jeden Kampf ausgewertet.

## Was schon gut läuft
- **Attumen, Moroes und Terestian:** keine Bot-Tode, Gäste und Dämonenketten werden richtig fokussiert
- **Kurator:** nur ein Tod, Zielwechsel auf die Astralflammen funktioniert

## Wo es noch hakt
- **Maulgar** (120 Bot-Tode) und **Gruul** (61): ständige Ausweichreaktionen und Todesgruppen bei Zerschmettern
- **Magtheridon:** maximal 3–4 gleichzeitige Würfelklicker statt 5
- **Aran** und **Malchezaar:** Bots brechen ihre Flucht zu früh ab
- **Netherspite:** kein blauer Strahl wird gefangen

## Was wir daraus machen
- Maulgar und Gruul sind schon heute Abend neu eingespielt (Update 1.6.1)
- Eine Ursache für die Karazhan-Probleme ist gefunden: Die Boss-Suche hatte einen zu kleinen Radius. Die Korrektur kommt mit dem nächsten Update

Danke an alle, die mitgetestet haben!', 'ADMINISTRATOR', 1, 0, '2026-10-19 15:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Testraid-Bericht: 26 Kämpfe ausgewertet');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Stand der Bot-Raids: Wo es noch hakt', 'news', 'Ehrlich gesagt: Nicht jeder Boss liegt schon zuverlässig mit Bots. Hier ein Überblick, damit ihr eure Raidabende planen könnt.

## Läuft gut
- Die meisten Bosse in Karazhan, Gruul''s Lair und Magtheridon haben gezielte Korrekturen erhalten
- Al''ar und Akil''zon wurden auch serverseitig repariert

## In Arbeit
- **Netherspite:** Der Boss heilt sich nach der Verbannungsphase, bisher kein Kill mit Bots
- **Leotheras:** Bots kamen bislang nur bis ca. 92 % – wird im nächsten Update überarbeitet
- **Rat, Karathress, Lurker-Wächter:** brauchen oft mehr als zwei Tanks; Ersatz-Tanks gibt es bisher nur bei Maulgar

Wenn euch in einem Kampf etwas auffällt, meldet euch mit Boss, Gruppenaufstellung und Uhrzeit – das hilft beim Nachvollziehen in den Logs enorm.', 'ADMINISTRATOR', 1, 0, '2026-10-22 15:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Stand der Bot-Raids: Wo es noch hakt');

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `published`, `pinned`, `created_at`)
SELECT 'Wartung: Neustart für Update 1.8', 'maintenance', 'Der Server wird für das Einspielen von **Update 1.8.0 und 1.8.1** kurz neu gestartet. Rechnet mit etwa 15 Minuten Ausfallzeit.

Laufende Raid-IDs bleiben erhalten. Bitte loggt euch vorher an einem sicheren Ort aus.', 'ADMINISTRATOR', 1, 0, '2026-10-23 15:00:00'
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news` WHERE `title` = 'Wartung: Neustart für Update 1.8');

-- Ältere Fassung der Patch-Notes (1.0–1.4) durch die ausführlichen Versionen ersetzen
DELETE FROM `web_news` WHERE `author` = 'ADMINISTRATOR' AND `title` IN ('Update 1.0 – Serverstart: TBC-Core mit Playerbots', 'Update 1.1 – Eigene Boss-Taktiken für alle neun TBC-Raids', 'Update 1.2 – Rotationen, Verbrauchsgüter und viele Raid-Korrekturen', 'Update 1.3 – Maulgar, Hydross, Leotheras, Archimonde und Black Temple', 'Update 1.4 – Neue Website mit Account-Verwaltung und Progress Race', 'Wartung: Neustart für Update 1.3');

-- Bereits eingespielter Event-Beitrag: neue Mindestanzahl übernehmen
UPDATE `web_news` SET `body` = 'Das Rennen um die Scherbenwelt beginnt! Ab heute zeichnet die Website jeden ersten Boss-Kill einer Gilde automatisch auf.

## So funktioniert es
- Ein Kill zählt nur als **Gilden-Kill**, wenn genug Mitglieder eurer Gilde in der Raid-ID sind: **mindestens 10 in Karazhan und Zul''Aman, mindestens 20 in allen anderen Raids**
- Mitglieder können Spieler oder Playerbots sein – **eure eigenen Bots zählen mit**, wenn sie in der Gilde sind. Eine kleine Gilde mit gut ausgerüsteten Bots kann also ganz vorne mitspielen
- Gemischte Gruppen mit zu wenigen Gildenmitgliedern werden nicht gewertet
- Gilden, die von Random-Bots geführt werden, laufen außer Konkurrenz
- Die Rangfolge richtet sich nach der höchsten Raidstufe: Sunwell vor Tier 6, vor Tier 5, vor Zul''Aman, vor Tier 4. Bei Gleichstand gewinnt, wer zuerst dort war

## Die Stationen
- **Tier 4:** Karazhan, Gruul''s Lair, Magtheridon
- **Tier 5:** Serpentshrine Cavern, Festung der Stürme
- **Zul''Aman**
- **Tier 6:** Hyjal, Black Temple
- **Sunwell Plateau**

Den aktuellen Stand seht ihr jederzeit unter **Progress**. Viel Erfolg – und vergesst nicht, `lust` im richtigen Moment zu rufen!' WHERE `title` = 'Progress Race eröffnet: Wer holt den ersten Realm-First?' AND `body` LIKE '%mindestens 5 im 10er%';
