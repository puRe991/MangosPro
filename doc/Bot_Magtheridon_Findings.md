# Magtheridon – Untersuchung 18.10.2026

Ausgewertetes Logfenster: 17:55:00–18:01:27, Map 544. Auszug `build/magtheridon-window.json` enthält 76.647 BotDecision-Einträge. Die Gruppe umfasst inzwischen andere bzw. mehr Bots als der vorherige Karazhan-Test; Ergebnisse beziehen sich auf diesen Kampf.

## Würfel und Drucknova

Die dedizierte Taktik ist aktiv: 107 ausgeführte und 198 fehlgeschlagene Würfelaktionen. Das sind Aktionsentscheidungen, keine bestätigten Klicks. Shadow Grasp 30410 und Kanalisierung 30410 belegen tatsächliche Benutzung.

Erste beobachtete Nova ab 17:59:16: Nur Thugg kanalisiert nachweisbar, von 17:59:16 bis 17:59:21. Sein Leben fällt dabei von 100 auf 1.18%; um 17:59:22 ist er tot. Andere zugeteilte Klicker sind noch unterwegs. Rohander läuft 17:59:13–20 über mehrere weit auseinanderliegende Punkte und stirbt um 17:59:23. Domenelin läuft bis 17:59:23 ebenfalls noch. Shohkak bleibt von 17:59:12–23 auf -56.793/-25.6348/14.5347 stehen, obwohl mehrfach bewegte/ausgeführte Würfelaktionen gemeldet werden. Das belegt fehlenden tatsächlichen Fortschritt, aber noch nicht die genaue Ursache (z.B. Bewegungs-/Höheneffekt oder Pfadproblem).

Zweite beobachtete Nova um 18:00:12: Shohkak beginnt zu kanalisieren, Domenelin und Galexandro sind ab 18:00:13 gleichzeitig erfasst. Maximal drei gleichzeitige Bot-Kanalisierungen in den Snapshots. Fünf gleichzeitige Klicker sind nicht belegt. Am Boss ist keine Shadow-Cage-Aura 30168 erfasst, während die Nova-Aura 30616 wiederholt bestehen bleibt.

Die lokale Würfel-Implementierung benötigt fünf Shadow-Grasp-Auren 30166 am Boss, um Shadow Cage auszulösen und seinen Nova-Kanal abzubrechen. Der Klick wird also grundsätzlich ausgeführt, die gemeinsame Bereitschaft/Ankunft funktioniert in diesem Versuch nicht.

Im Code werden Klicker nach Gruppenreihenfolge und Würfel-GUID zugeteilt; Entfernung und benötigte Laufzeit gehen nicht in die Auswahl ein. Vorbereitung beginnt anhand eines festen Timers, wartet zunächst auf einem 10-Yard-Ring um den Würfel, und nähert sich erst während der Nova bis zur Interaktionsdistanz. Die sichtbaren weiten Laufwege und zu späte Ankunft sprechen für eine notwendige Verbesserung dieser Vorbereitung. Eine vollständige Zuordnungs-/Distanzdiagnose fehlt noch.

## Weitere Auffälligkeiten

- Schon bei den Kanalisierern gibt es viele Todesfälle, z.B. Shahke/Olistra ab 17:57:33 sowie mehrere Heiler um 17:57:38–47. Das liegt vor der ersten beobachteten Nova und darf ihr nicht zugeschrieben werden.
- Bei den Höllenbestien sind nur zwei dedizierte CC-Aktionen ausgeführt, 32 fehlgeschlagen. Das genügt ohne Ziel-/Ablehnungsdetails nicht zum Nachweis einer funktionierenden Verbannungsrotation.
- Zwischen 17:58:19 und 18:00:16 werden 617 Verteilaktionen, 557 DPS-Assist- und 549 Neuzielaktionen ausgeführt. Insgesamt entstehen im Fenster 611 Neuzielaktionen mit Trigger `invalid target`. Das ist ein deutlicher Hinweis auf konkurrierende Bewegung/Zielverlust; die genaue Ursache ist noch offen.
- Boss verliert vor dem letzten Einbruch lediglich etwa 2.3% Leben (100 auf 97.6873%). Der anschließende Sprung auf 0 belegt keinen normalen Kill.
- Tankpositionsaktion wird 39-mal ausgeführt, 33-mal fehlgeschlagen. Der Code verlangt weiterhin Nahkampfreichweite für weiteres Ziehen; derselbe problematische Abbruch wurde bei anderen Bossen bereits korrigiert. Wie stark das hier die Todesfälle beeinflusst, bleibt offen.

## Datenbankprüfung und nächste Korrekturen

`boss_magtheridon`, `mob_hellfire_channeler` und `go_manticron_cube` sind an den korrekten Vorlagen gebunden. Fehlende Würfelskripte sind damit nicht die Erklärung. Ein leeres ScriptName beim Abyssal allein belegt keinen Defekt.

Priorität: fünf tatsächlich bereite Klicker sicherstellen, frühzeitig anhand von Entfernung/Laufzeit vorbereiten, festhängende Bewegung erkennen und neu planen, Ersatz nach Tod bereitstellen und jeden Würfel mit Zuordnung/Distanz/Kanalstatus protokollieren. Danach Zielverlustschleifen, Verteilbewegung und Add-CC untersuchen.

Nachtrag zum Folgeauftrag: Korrekturen werden in das gemeinsame Paket integriert.
Die DD-Zielwahl ignoriert verbannte Höllenbestien und hält nach den Kanalisierern
Magtheridon als Ziel. Die Verteilbewegung verwendet stabile Positionen statt
ständigem gegenseitigem Weglaufen. Klicker werden unter geeigneten Kandidaten
nach Entfernung ausgewählt, ab 30 statt 48 Sekunden vorbereitet und direkt nahe
der Interaktionsdistanz geparkt. Wartepunkte verwenden Bodenhöhe am Würfel statt
der möglicherweise angehobenen Bot-Höhe. Laufentscheidungen werden nach 200 ms
erneut bewertet. Andere Bewegungen und normale Zauber können den Würfelkanal
nicht mehr einfach überschreiben. Ein Klick gilt erst bei bestätigtem Kanal/Aura
als erfolgreich. `[RaidCube]` enthält Würfel, Entfernung, Höhenabweichung,
Interaktionsreichweite, Nova, Vorbereitung, Kanal und Erschöpfung.

Der Benutzer hat den Neustart nach Fertigstellung aller Korrekturen freigegeben.
Tests und gemeinsamer Build müssen vor dem Austausch abgeschlossen sein.

Installationsnachtrag: Gemeinsames Paket am 18.10.2026 nach bestandenen Tests und Build-Pr�fung mit ausdr�cklicher Neustartfreigabe installiert; siehe `build/karazhan-followup-manifest.json`. Die beschriebenen Wirkungen m�ssen im n�chsten Raid best�tigt werden.

## Folgekorrektur vom 19.10.2026

Neues Fenster: `build/magtheridon-current-window.json`, 47.001 Entscheidungen, 09:16:13–09:21:17. Erfasst sind ein Kick und ein Gegenzauber. Lavenna, Darlina und Thugg beginnen 09:18:21–22 Verbannung auf dieselbe GUID 5860266. Das beweist konkurrierende Zielauswahl; aus EXECUTED folgt noch kein erfolgreich abgeschlossener CC-Cast. Die dedizierte CC-Aktion hat drei protokollierte FAILED-Ergebnisse und kein EXECUTED. Die Kanalisierer sind von 09:17:06 bis 09:18:34 als Ziele sichtbar; das ist nicht identisch mit ihrer aktiven Kampfzeit.

Korrekturen:

- Neue Unterbrechungsreaktion sucht bei allen fuenf Kanalisierern nach Dunkler Heilung, Schattenblitzsalve und Furcht. Sie waehlt nur bekannte, verfuegbare und in Reichweite nutzbare Unterbrechungen der jeweiligen Klasse; laufende Heilung hat Vorrang. Vor Ausfuehrung wird neu geprueft. Unterstuetzt sind Kick, Schildhieb/Zuschlagen, Gegenzauber, Erdschock, Hammer der Gerechtigkeit und Stille, sofern gelernt und nutzbar.
- Dedizierte Hexenmeister-Reaktion mit getrennten Add-Zuweisungen. Normale Verbannung/Furcht wird bei vorhandenen Abyssals unterdrueckt, damit nicht mehrere Bots dasselbe markierte Ziel bedienen. Bereits kontrollierte Adds und bereits laufende eigene CC-Casts werden ausgelassen. Zu entfernte zugewiesene Adds loesen Annaeherung aus; Casting-Dauer wird an die Reaktion weitergegeben. Gruppenmitglieder anderer Instanzen werden ausgeschlossen.
- Wuerfelaktion ist nur bei konkretem Vorbereitungsbedarf, Nova oder gehaltenem Kanal nuetzlich. Ihre Reaktion darf Cast/Bewegung nicht mehr pauschal vorher abbrechen. Beim Halten bleibt der Kanal erhalten; Loslassen erfolgt ausdruecklich nach der Nova. Ein bestehender Kanal kann auch nach geaenderter Zuteilung freigegeben werden.

Tests: `build/test_magtheridon_control.py` prueft Produktionsfunktionen fuer Unterbrechungsprioritaet/Reichweite/Verfuegbarkeit, getrennte CC-Ziele, ueberschuessige Adds, laufendes CC und Wuerfelzustandswechsel. Bestanden. Maulgar-/Gruul-Regressionstests ebenfalls bestanden. Vollstaendige game/playerbots-Bibliotheken gebaut und mangosd erfolgreich gelinkt; siehe `build/raid-oct19-full-build.log` und `build/raid-oct19-link.log`. Deployment-Hashes und Sicherung stehen in `build/raid-oct19-deployment.json`. Ein echter Raid mit dem neuen Build ist weiterhin erforderlich; die Tests beweisen keinen Boss-Kill.

Deployment-Nachtrag 19.10.2026: Vollstaendig neu gebaut und nach Benutzerfreigabe installiert. Weltserver meldet 'World initialized', Spielport 8085, Login 3724 und Datenbank 3306 sind aktiv. Backup und SHA256: build/raid-oct19-deployment.json. Neuer Live-Raid noch nicht verifiziert.
