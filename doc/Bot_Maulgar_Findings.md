# Maulgar und Gruul-Trash: Cast-Abbrueche

Stand: 19.10.2026. Ausgewertet: `build/bin/x64_Release/logs/raid-ai-1792392027-397650-{0,1}.log`, Map 565, Instanz 9, 08:40:27 bis 08:51:11.

## Bestaetigte Ursachen

1. Gruul-Trigger und -Multiplikatoren riefen `raidboss::IsEncounterInProgress` ohne Boss-Entries auf. Dessen Fallback akzeptiert dann jeden lebenden NPC in `attackers`. Schon Trash Entry 21350 aktiviert damit die Bosstaktik. `FindUnit` findet ausserdem nahe, noch unbeteiligte Ratsmitglieder. Dadurch laufen falsche Zielzuweisungen und Krosh-Ausweichaktionen; DPS-Assist und Cooldowns koennen gesperrt werden.
2. Kroshs Ausweichtrigger pruefte nur, ob Krosh existiert. Der Wirbelwindtrigger pruefte die Aura, aber keine Entfernung. Beide Bewegungsaktionen erbten eine Nuetzlichkeitspruefung ohne Gefahrenabstand. Die Reaktionssteuerung unterbricht Cast und Bewegung bereits bei der Auswahl, bevor `Execute` den unnoetigen Ausweichversuch mit false beendet. Das betrifft Schaden und Heilung, beim Wirbelwind auch nachdem die Adds tot sind.

## Logbelege

- 775 protokollierte FAILED-Reaktionen fuer Krosh, davon 631 mit aktuellem Ziel Entry 21350; 14 EXECUTED-Reaktionen.
- 571 FAILED-Reaktionen fuer Wirbelwind; 225 EXECUTED-Reaktionen.
- Gezählt ist ausschliesslich `state=3`; Diagnosemeldungen werden gedrosselt. Diese Zahlen sind keine vollstaendige Anzahl abgebrochener Zauber.
- Galexandro, 08:40:51, ms=421438: Arkanschlag 30451 startet gegen Trash 21350. Bei ms=421981 startet Krosh-Ausweichen, cast=0, unveraenderte Position (543 ms spaeter).
- Achma, 08:40:55, ms=425154: Heilwelle 25396 startet. Bei ms=425385 startet Krosh-Ausweichen, cast=0 (231 ms spaeter).
- Maulgars Enrage/Flurry 33232 startet laut Boss-Skript unter 50 % HP. Dies ist Phase 2, kein Zeitlimit fuer den DPS.

## Korrektur

- Gemeinsamer Gruul-Helfer mit expliziten Boss-Entries fuer Trigger, Multiplikatoren und Reset-Erkennung. Trash alleine aktiviert die Bosstaktik nicht mehr; der Instanzstatus und echte Boss-Angreifer bleiben gueltig.
- Krosh-Ausweichen nur unter 17 yd; Wirbelwind-Ausweichen nur bei aktiver Aura und unter 12 yd.
- Dieselben Gefahrenbedingungen werden in `isUseful` vor Auswahl einer Reaktion und erneut vor Ausfuehrung geprueft. Damit werden auch bereits eingereihte, inzwischen unnoetige Ausweichaktionen verworfen.
- Bestehende Tank-Ausnahmen und die Stay-Strategie bleiben erhalten.

## Validierung und Grenzen

`build/test_maulgar_cast_fix.py` extrahiert die Produktionsfunktionen in einen C++-Regressionstest: Trash mit nahem Rat, Boss-Fallback, Instanzstatus, Distanzgrenzen, abgelaufener Wirbelwind, Tank-Ausnahmen, Stay und toter Boss. Test bestanden.

`build/compile_maulgar_cast_fix.py` kompiliert die vier geaenderten Translation Units separat mit den vorhandenen Release-Compileroptionen. Kein Serverneustart, kein Austausch der laufenden EXE. Ein vollstaendiger Build mit anschliessender Aktivierung und ein erneuter Trash-/Maulgar-Kampf sind fuer die Live-Bestaetigung erforderlich. Aus dem Entscheidungslog wird kein numerischer Raid-DPS abgeleitet.

Deployment-Nachtrag 19.10.2026: Vollstaendig neu gebaut und nach Benutzerfreigabe installiert. Weltserver meldet 'World initialized', Spielport 8085, Login 3724 und Datenbank 3306 sind aktiv. Backup und SHA256: build/raid-oct19-deployment.json. Neuer Live-Raid noch nicht verifiziert.
