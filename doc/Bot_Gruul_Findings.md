# Gruul: Todesfaelle und Positionskorrekturen

Stand: 19.10.2026. Quelle: `build/bin/x64_Release/logs/raid-ai-1792392027-397650-*.log`, Map 565, Instanz 9. Gruul-Zielbilder von 09:01:59 bis 09:08:36. Auswertung: `build/analyze_gruul.py`, Ergebnisfenster `build/gruul-window.json`.

## Befunde

- Einzelne Todesfaelle bei stabiler Boss-Aggro auf Sailan: Gienni 09:02:27, Achma 09:03:05, Peshe 09:03:13, Rakaejin 09:03:22, Sangon 09:03:30, Hovujmu 09:03:39, Gienni 09:03:48. Viele letzte Zustandsbilder zeigen volle HP. Der ungefaehr 8-sekuendige Rhythmus passt zum lokalen Hurtful-Strike-Timer. Das Entscheidungslog enthaelt keinen toedlichen Schadensdatensatz; die Zuordnung zu Hurtful Strike bleibt eine gut begruendete Vermutung.
- Ausschliesslich Sailan fuehrt die Bot-Tankaktion gegen Gruul aus. Ein menschlicher Offtank ist daraus nicht ausgeschlossen. Hurtful Strike waehlt laut Boss-Skript den Spieler mit der hoechsten Bedrohung in Nahkampfreichweite ausser dem aktuellen Tank. Ohne geeigneten zweiten Tank trifft dies DPS oder nah stehende Heiler.
- Shatter: drei gleichzeitige Todesfaelle 09:04:06 (Domenelin, Rohander, Achma), fuenf 09:05:20 (Olistra, Galexandro, Rohander, Gienni, Thugg), drei 09:06:31 (Galexandro, Peshe, Onise). Letzte Bilder enthalten Grasp 33572 mit fuenf Stapeln und Stoned 33652.
- Die Shatter-Bewegungen wechseln mehrfach die Richtung: Rohander bewegt sich zwischen 09:04:01 und 09:04:05 von (263.971,393.059) nach (264.996,395.890), zurueck nach (263.835,395.663), dann nach (264.356,392.661). Es wird nur der naechste Spieler beruecksichtigt; Ausweichen kann in den naechsten Nachbarn fuehren.
- Fernkampf-Verteilung wird 822-mal als erfolgreich protokolliert. Der bisherige feste Gruppenplatz konnte auf der anderen Bossseite liegen. Die einmalige Ankunftsmarkierung verhindert ausserdem keine zu nahe Position nach Ground Slam/Wiederbelebung. Achma steht nach dem ersten Slam nur wenige Yards vom Boss entfernt.

## Aenderungen

- Shatter-Kandidaten bewerten den Abstand zu allen nahen lebenden Gruppenmitgliedern. Nur erreichbare Schritte mit geringerer gesamter Naehebelastung werden gewaehlt; bekannte Einsturzflaechen werden ausgeschlossen. Dies ist eine lokale Ausweichheuristik, keine Garantie fuer 22 yd Abstand in jeder Raidsituation.
- Ground-Slam-Phase umfasst nun auch Grasp und Stoned, damit die normale Zielverfolgung vor dem eigentlichen Shatter nicht wieder freigegeben wird.
- Shatter- und Einsturzreaktionen pruefen ihren aktuellen Nutzen vor der Cast-Unterbrechung; keine Shatterbewegung bei Stoned, ohne Geschwindigkeit oder ohne verbesserten erreichbaren Schritt.
- Fernkaempfer/Heiler sichern erneut den Mindestabstand zum tatsaechlichen Boss, auch nach Knockback oder Wiederbelebung. Startposition auf der aktuellen Bossseite mit 25–30 yd statt zufaelligem Gruppenplatz mit bis zu 40 yd. Normale Nachbarverteilung darf nicht zurueck in Bossnaehe oder ueber 35 yd hinaus fuehren.
- Nicht dringende Verteilung unterbricht keine laufenden Zauber mehr. Mindestabstand und konkrete Gefahren behalten Vorrang.

## Pruefung und verbleibende Arbeit

- `build/test_gruul_survival.py`: C++-Tests mit extrahierten Produktionsfunktionen fuer Phasenabdeckung, mehrere Nachbarn, unbewegliche/sichere Situationen, blockierte Wege, Einsturz, Nahbereich nach Knockback, Cast-Erhaltung und Startposition ohne Bossquerung bestanden.
- Maulgar-Regressionstest erneut bestanden.
- Vier geaenderte Gruul-Translation-Units mit vorhandenen Release-Compileroptionen kompiliert; kein vollstaendiger Link/Deployment und kein Live-Kampf mit den Aenderungen.
- Vor erneuter Bewertung muss ein zweiter geeigneter Tank fuer Hurtful Strike in Nahkampfreichweite ausreichend Bedrohung halten. Die Aenderungen setzen keine DPS-Spezialisierung automatisch auf Tank um.
- Kein Serverneustart oder Austausch der laufenden EXE. Maulgar- und Haltbarkeitskorrektur sind ebenfalls weiterhin Quellcodeaenderungen.

Deployment-Nachtrag 19.10.2026: Vollstaendig neu gebaut und nach Benutzerfreigabe installiert. Weltserver meldet 'World initialized', Spielport 8085, Login 3724 und Datenbank 3306 sind aktiv. Backup und SHA256: build/raid-oct19-deployment.json. Neuer Live-Raid noch nicht verifiziert.
