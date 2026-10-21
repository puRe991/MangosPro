# Arans Schemen: Logpruefung vom 18.10.2026

Nur untersucht, noch keine Aran-Korrektur implementiert oder eingespielt.
Zeitfenster: 16:56:22–17:00:24, Europe/Berlin.
Analyseauszug: `build/aran-window.json` (ignoriertes Build-Artefakt).
Boss 16524 startet bei 100 Prozent und steht am Ende des Ausschnitts bei
58,1721 Prozent; kein Kill im untersuchten Fenster.

## Bestaetigte Befunde

- Die Fluchtaktion gegen Arkane Explosion 29973 wird 32-mal ausgefuehrt.
  Das belegt Bewegungsauftraege, nicht das Erreichen einer sicheren Position.
- `KarazhanAranRunAwayFromArcaneExplosionAction` verwendet 21 Meter als Ziel
  und endet bereits ab diesem Abstand. Die lokale `spell_template` verwendet
  fuer 29973 Radiusindex 38; `SpellRadius.dbc` enthaelt dort exakt 21 Meter.
  Die Flucht hat somit keine Sicherheitsreserve gegen Trefferpruefung,
  Rundung oder noch nicht vollstaendig ausgefuehrte Bewegung.
- Erste erfasste Explosion: Cast ca. 16:57:39–16:57:48. Derick, Jovilenne,
  Lavenna und Onise werden um 16:57:49 als tot protokolliert. Zeitpunkt und
  Positionen sprechen stark fuer die Explosion; das Entscheidungslog ist
  kein vollstaendiges Schadensereignisprotokoll.
- Onise: Fluchtauftrag 16:57:40, erneuter Reaktionsversuch 16:57:43 FAILED;
  anschliessend Curse of Agony, 16:57:45 Shadow Bolt bei protokolliertem
  Bossabstand 11,02 Metern. Tod 16:57:49.
- Jovilenne: zwei Fluchtauftraege; 16:57:46 Steady Shot bei protokolliertem
  Abstand 17,92 Metern, anschliessend Tod.
- Lengocow: 16:57:42 `reach area trigger`, 16:57:43 `reach melee` waehrend
  des Explosionscasts; erneute Flucht 16:57:46. Die Bewegungssperren verhindern
  konkurrierende Wege nicht durchgaengig. Den genauen Umgehungsweg noch
  untersuchen (Strategieaktivierung, Casting-Erkennung, Aktionswiederaufnahme).
- Verlangsamung 30035 ist bei mehreren Betroffenen waehrend der Flucht aktiv.
  Einmaliger kurzer Bewegungsauftrag reicht daher nicht als Erfolgskriterium.
- Unterbrechungsaktionen werden verwendet: Ladina 17 Kick, Lengocow 13
  Shield Bash. EXECUTED allein beweist nicht den erfolgreichen Abbruch eines
  gegnerischen Zaubers.
- Flammenkranz-Stopp: 1 EXECUTED, 152 FAILED. FAILED ist hier kein Beweis
  fuer Fehlverhalten: die Aktion gibt absichtlich false zurueck, wenn der Bot
  bereits steht. Bewegung und Auraablauf anhand der Millisekunden vergleichen,
  nicht allein innerhalb derselben gerundeten Sekunde.
- Wiederholte Todesfaelle und Manaprobleme bleiben sichtbar. Diesen Kampf
  nicht allein als Rotations- oder Ausruestungsproblem behandeln.

## Naechste Korrekturen

1. Fuer Arkane Explosion einen Abstand oberhalb des wirklichen Radius mit
   Reserve verwenden, idealerweise aus den lokalen Zauberdaten ableiten.
2. Flucht bis zum bestaetigten sicheren Abstand verfolgen und nach abgebrochenem
   Weg neu planen. Kein langsamer Schadenscast darf das Entkommen vorzeitig
   beenden; dringende Ueberlebensaktionen sinnvoll erhalten.
3. Reach/Follow/Bereichswechsel sowie automatisch fortgesetzte Angriffe gegen
   die Bossmechanik absichern. Aktionsklassen und Casting-Erkennung pruefen.
4. Flammenkranz separat pruefen: freiwillige Bewegung verhindern, aber einen
   bereits stehenden Bot nicht anhand des FAILED-Status als fehlerhaft werten.
5. Tests fuer Schadensradius, Reserve, Slow, abgebrochene Navigation,
   konkurrierende Bewegung und Rueckkehr zur normalen Rotation nach dem Cast.

## Betriebszustand bei dieser Pruefung

Der Benutzer hatte den vorherigen Weltserver herunterfahren lassen. Bei dieser
spaeteren Pruefung lief wieder PID 10392 (Start 16:48:15), Port 8085 offen.
Die installierte Datei hat weiterhin SHA256
`E3D72459A6A266FCCC1813B644BD80E59A8B0A56A24AC1E74B7B5DD83D561364`.
Dieser Prueflauf hat den Server weder gestartet, gestoppt noch veraendert.
Keine neue Neustartfreigabe aus diesem Untersuchungsauftrag ableiten.
