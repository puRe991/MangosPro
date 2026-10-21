# Raid-Pruefung vom 17.10.2026

## Nachgewiesene Fehler und Korrekturen

- Gespeicherte Botprofile konnten den `dungeon`-Dispatcher entfernen. Beim Laden wird er fuer Kampf und Ruhe wieder eingesetzt; das betrifft die automatische Boss-Aktivierung aller unterstuetzten Maps.
- Maulgar und Gruul: Normales Heranlaufen konnte kurze Schritte zur Tankposition rueckgaengig machen. Nur der zugewiesene Maulgar-Tank bzw. Gruuls aktueller Aggrohalter unterdrueckt konkurrierendes Heranlaufen, Charge, Follow und Set Behind. Der Boss darf zwischen den Schritten nachlaufen. Bei Aggroverlust bleiben Heranlaufen und Taunt verfuegbar. Gruuls Off-Tank wird nicht am Erreichen der Nahkampfreichweite gehindert.
- Gruul, Magtheridon, SSC und Tempest Keep hatten keine eigenen Reaktionstrigger. Ausgewaehlte bestehende Gefahrenaktionen laufen jetzt auch im Reaktionsmodus; die jeweiligen Multiplikatoren gelten ebenfalls dort. Das betrifft Cave In/Shatter, Debris/Wuerfel, Spout/Wirbel/Static Charge sowie Flame Quills/Rebirth/Flame Patches/Orb/Wrath/Thaladred/Flame Strike. Zauber werden beim notwendigen Ausweichen unterbrochen.
- Kiggler: Ein Druide mit aktiver Heilerrolle wird nicht mehr allein aufgrund des Talent-Tabs zum Moonkin-Tank ausgewaehlt. Fehlende geeignete Rollen werden als fehlend angezeigt, nicht durch eine Umskillung repariert.

## Umfang der Codepruefung

Die acht spezialisierten TBC-Raidstrategien (Karazhan, Gruul, Magtheridon, SSC, TK, Hyjal, BT, ZA) wurden auf ihre benannten Trigger/Aktionen und auf vorhandene Reaktionstrigger geprueft. Der statische Abgleich ihrer Strategie- und Context-Dateien findet keine fehlenden benannten Verknuepfungen. Er prueft keine Boss-Kills und keine Wegfindung.

`debug raid all` prueft im laufenden Botkontext alle hinterlegten Mapstrategien einschliesslich Classic und Dungeons. `Fehlend=0` bedeutet, dass die Strategieobjekte und deren Trigger/Aktionen aufloesbar sind. Generische Regeln (z.B. Sunwell ueber `tbc raid`) sind davon getrennt. Ein leerer Reaktionsmodus wird bei diesem Befehl nicht pauschal als Registrierungsfehler ausgegeben.

## Neue Versuche im vorhandenen Log

| Strategie | Zeitraum am 17.10. | Sichtbare Mechaniken |
|---|---|---|
| Gruul | 00:49:36–01:23:15 | Oger-Zuweisung, Whirlwind, Tankposition, Ranged Spread, Cave In, Shatter |
| Black Temple | 01:23:18–01:40:45 | Naj'entus Tankposition und Ranged Spread |
| Tempest Keep | 01:42:28–02:00:21 | Al'ar Plattformen/Ember/Flame Quills, Solarian/Priester/Wrath, Void Reaver Position/Orb |

Die Aufzeichnung enthaelt ausgefuehrte Aktionen in allen drei Strategien. `FAILED` allein ist kein Fehlernachweis: auch bereits erreichte Positionen und unnoetige Reset-Aktionen liefern false. Aus den Eintraegen folgt nicht, dass ein Boss korrekt stand oder ein Ausweichweg sicher war. Fuer Naj'entus fehlen im untersuchten Aktionslog Spine-/Shield-Aktionen; ohne Aufzeichnung der entsprechenden Bossphase laesst sich daraus kein fehlender Trigger ableiten.

## Weitere Positionierungs-Kandidaten

Bei der Quellcodepruefung finden sich Nahkampfreichweiten-Bedingungen auch an Positionierungsaktionen in Karazhan, Magtheridon, SSC, TK, Hyjal, BT und ZA. Das sind Kandidaten fuer denselben Konflikt; die Bedingungen allein beweisen kein Pendeln. Insbesondere Phasenwechsel, Tankwechsel, niedriges Leben, Adds und feststehende Gegner brauchen eigene Regeln. Hier wurde keine globale Unterdrueckung aller Tankbewegungen eingebaut.

## Validierung und Diagnose

`build/verify_raid_positioning.py` extrahiert die echten Produktionsfunktionen fuer den Positionsschritt, beide neuen Tank-Multiplikatoren und Gruuls Tankaktion in eine kleine C++-Simulation. Sie prueft wechselnde Nahkampfreichweite, unterdruecktes Zuruecklaufen, Ankunft/Halten, Aggroverlust, erlaubtes Heranlaufen des Off-Tanks und weiterhin erlaubten Taunt. Die Simulation ersetzt keinen Instanztest mit Navigation.

`debug raid on` aktiviert fuer den angesprochenen Bot `[RaidTactics]`. Fuer Maulgar und Gruul kommen gedrosselte `[RaidPosition]`-Zeilen mit tatsaechlicher Tank-/Bossposition, Sollposition, Entfernung, Aggrohalter und Nahkampfreichweite dazu. `tank_at_spot` bestaetigt nur den Tankstandort; die Bosskoordinaten stehen separat daneben. Nach einem Neustart muss das Aktionslog erneut eingeschaltet werden.
