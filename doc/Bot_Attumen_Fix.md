# Attumen: Befund und Korrekturen

Die Aufzeichnung vom 17.10.2026 zwischen 23:38:50 und 23:43:57 zeigt wiederholte
kritische Tankzustaende und Aggro auf Nicht-Tanks. Lengocow hat im untersuchten
Abschnitt acht ausgefuehrte Shield Slams und fuenf Devastates. Das sind Summen
ueber die aufgezeichnete Versuchsfolge, keine DPS- oder lueckenlose Castmessung.
Um 23:39:20 faellt er auf 15 % HP, um 23:39:25 auf 8 %, um 23:39:32 ist er tot.
Derick startet dabei Heilungen; Schildwall wird als unmoeglich verworfen.

Bei Sailan ist Zorn der Gerechtigkeit nur in 19 von 256 erfassten lebenden
Kampfzustandsbildern vorhanden. Die TBC-Strategie setzte diesen Aggro-Buff nur
ausserhalb des Kampfes nach. Die neuen Kampftrigger stellen ihn auch nach
einem Tod oder Verlust waehrend eines laufenden Kampfes wieder her.

## Zielwahl und Rotation

- DPS Assist und Tank Assist beanspruchen keine Entscheidung mehr fuer ein
  bereits korrekt gewaehltes und angegriffenes Ziel. Ein neues Ziel, ein
  zurueckgesetzter aktueller Zielwert oder ein gestoppter Angriff bleiben
  moegliche Gruende zum Assistieren. Die DPS-Flaggentraegerregel bleibt erhalten.
- Ein aktives Pet behaelt sein Ziel. Hat es unabhaengig vom Besitzer das Ziel
  verloren, kann Assist den Pet-Angriff wieder aufnehmen. Phasenverschobene
  oder kontrollierte Pets loesen keine staendigen Wiederholungen aus.
- Shield Slam wird in der TBC-Tankstrategie ueber seine tatsaechliche
  Einsetzbarkeit aktiviert. Die bisherige feste Schwelle von 20 Wut konnte
  trotz durch Talente reduzierter Kosten blockieren.
- Devastate wartet nur auf einen bereiten **und einsetzbaren** Shield Slam.
  Ein freier Cooldown bei fehlender Ressource oder einer anderen
  Einsatzbeschraenkung blockiert die Fuellaktion nicht mehr pauschal.
- Abgelehnte Zauber enthalten jetzt zusaetzlich `CAST_REJECTED` mit Spell-ID
  und dem vom Core gemeldeten Fehlernamen. Reichweitenfehler werden ebenfalls
  gekennzeichnet. Normale Cooldowns und Spielregeln bleiben wirksam.

## Zauberrang und Validierung

Die untersuchten Hauptzauber verwenden die hoechsten TBC-Raenge, unter anderem
Shield Slam 30356, Devastate 30022, Holy Shield 27179, Greater Heal 25213 und
Shadow Bolt 27209. Die gespeicherten Mana-Sparwerte der neun untersuchten Bots
sind 1. Eine Rang-1-Anzeige ist allein kein Fehler: Steady Shot 34120 hat in
TBC nur diesen Rang. Diese Stichprobe prueft nicht jede erlernbare Faehigkeit.

`build/test_attumen_fixes.py` extrahiert die geaenderten Produktionsfunktionen
in einen C++-Verhaltenstest. Er prueft stabile Ziele, Zielwechsel, gestoppte
Angriffe, Fernkaempfer, Flaggentraeger, Pet-Wiederaufnahme und deren Ausnahmen,
die Shield-Slam/Devastate-Entscheidung sowie den Kampftrigger fuer Zorn der
Gerechtigkeit. Das ersetzt keinen erneuten Attumen-Test mit echtem Schaden,
Heilung, Navigation und Bossphasen.
