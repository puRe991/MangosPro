# Heilzielwahl, Jaegerdistanz und Maiden

Vorbereitet nach den Kaempfen am 18.10.2026 (Moroes ca. 00:19–00:23,
Maiden ca. 00:26–00:30). Diese Aenderungen werden separat gebaut und erst
nach ausdruecklicher Freigabe installiert. Kein automatischer Neustart.

## Heilzielwahl

Ein Gruppenmitglied unter `AiPlayerbot.CriticalHealth` (Standard 20 Prozent)
hat Vorrang vor der Verteilung der Heiler auf unterschiedliche Ziele. Unter
`AiPlayerbot.LowHealth` (Standard 50 Prozent) verteilt sich die Heilung nur
auf ebenfalls niedrig verletzte Ziele. Gesunde Tanks werden dann nicht
allein wegen ihres Platzes in der Heilerverteilung ausgewaehlt.

Kritische und niedrig verletzte Spieler bleiben auch dann Kandidaten, wenn
bereits eine Heilung auf sie unterwegs ist. Das kann in Notfaellen zu
Ueberheilung fuehren, vermeidet aber das pauschale Ignorieren bedrohter Ziele.
Bestehende Reichweiten-, Fokusziel- und Heilimmunitaetsregeln gelten weiter.
Laufende Zauber werden durch diese Zielauswahl nicht automatisch abgebrochen.

Die Vorheilungsrechnung begrenzt erwarteten Schaden auf die aktuellen HP,
damit vorzeichenlose Subtraktion keine extrem hohen Lebenswerte vortaeuscht.
Gesunde Pets zaehlen nicht mehr allein wegen einer fehlenden ankommenden
Heilung als Heilziel. Tote oder unerreichbare externe Ziele und Pets werden
ausgeschlossen. Doppelte Spielereintraege durch manuelle Zielwahl werden
vermieden. Die Aenderung erhoeht keine Manareserven und setzt keine Cooldowns
zurueck.

## Jaeger

Die TBC-Kampfstrategie erhaelt `hunter recover ranged distance`. Sie prueft
die Mindestreichweite von Auto Shot einschliesslich der tatsaechlichen
Kampfreichweite des Gegners. Bei zu kurzem Abstand zu einem anderweitig
getankten Ziel bewegt sich der Jaeger mit Sicherheitsabstand nach aussen.
Der normale Navigations- und Gefahrenpfad bleibt aktiv.

Bei eigenem Aggro, fehlendem Opfer des Gegners, Nahkampfstrategie, Stay oder
Bewegungsunfaehigkeit greift diese Korrektur nicht ein. Der Wechsel in den
Nahkampf, etwa bei fehlender Munition, hat weiterhin hoehere Prioritaet.
Bossbezogene Bewegungssperren gelten auch fuer die neue Bewegungsaktion.

## Maiden

Der Aggrohalter zieht Maiden zum naechsten lebenden, durch Busse betaeubten
Heiler. Der Zielpunkt liegt drei Meter hinter diesem Heiler in Zugrichtung,
unabhaengig von dessen Blickrichtung. Boden und Sichtlinie werden geprueft.
Normales Nahkampf-Nachlaufen und Follow unterbrechen die Tankpositionierung
nicht mehr; Offtanks und Heilreichweitenbewegungen behalten ihre Funktion.
Die Rueckkehr zur Tankposition funktioniert auch ausserhalb der Nahkampfreichweite.

Bei aktivierter Raid-Diagnose erscheint hoechstens alle drei Sekunden:
`[RaidPosition] boss=Maiden ... wakeHealer=... healerHp=... bossToHealer=...
tankToHealer=... destination=(...) moved=...`.
`moved=1` bestaetigt einen erteilten Bewegungsauftrag, nicht das erfolgreiche
Aufwecken. Dafuer muessen anschliessend Busse-Auren und Heilaktionen geprueft werden.

## Opera und Kurator

- Die Engine behandelt einen Multiplikator null sofort als Veto. Eine solche
  Aktion wird weder erneut eingereiht noch duerfen ihre Voraussetzungen mit
  einer kleinen positiven Prioritaet starten. Das schliesst einen Weg, auf dem
  ein weiterer Warteschlangeneintrag derselben Aktion taktische Sperren umgehen
  konnte. Der Test `build/engine-veto-test.cpp` prueft doppelte Aktionen mit
  Null-, Normal- und reduzierter Prioritaet aus dem Produktionscode.
- Soulshatter 29858 behaelt das korrekte Selbstziel fuer seinen Flaecheneffekt.
  Die vorzeitige Freund/Feind-Sperre der Bot-Pruefung laesst diesen speziellen
  Fall zum normalen `CheckCast` durch. Ressourcen, Reagenzien und Cooldowns
  prueft weiterhin der Core.
- Finte verwendet eine Nahkampfaktion mit feindlichem aktuellem Ziel statt
  einer Selbstbuff-Aktion.
- Die Feuerkontrolle gegen Strohmann unterstuetzt neben Magiern (Scorch)
  auch Hexenmeister (Searing Pain). Der bestehende Taktiktrigger und die
  normalen Zauberregeln bleiben aktiv.
- Kurators Aggrohalter kann den Boss auch zwischen Nahkampfschritten ziehen.
  Reach/Follow/Charge unterbrechen seine Tankpositionierung nicht; der Offtank
  kann nach Aggroverlust weiterhin normal nachlaufen.
- Kampfrausch/Heldentum pruefen die Kurator-Phasenregel auch in der
  Zauberaktion, bevor die Kartenstrategie beim Pull vollstaendig hinzugefuegt
  sein muss. Das aktuelle Bossziel dient bei noch leerem Suchcache als Rueckfall.
  Hervorrufung und die Schlussphase unter 15 Prozent geben den Cooldown frei.
- Die generische Zauberaktion laesst Mindestreichweite und Sichtlinie vom
  `CheckCast` des Cores pruefen. Die vorherige Rechnung vermischte einen um
  Kampfreichweite gekuerzten Abstand mit einer Mindestreichweite, die bereits
  Kampfreichweite enthaelt. Dadurch wurden gueltige Jaegerschuesse abgewiesen.

## Manaverbrauch

Direkte Standardheilungen werden unmittelbar vor der Ausfuehrbarkeit erneut
auf ein inzwischen gesundes Ziel geprueft. Veraltete Trigger sollen dadurch
keine unnoetigen Heilungen mehr starten. Explizites Preheal bleibt erhalten.
Ein gesunder Heiler priorisiert ausserdem einen kritischen Gruppenpatienten
gegenueber seiner eigenen direkten Heilung. Heilungen auf verwundete Ziele,
HoTs, Schilde und Gruppenheilungen behalten ihre bisherige Funktion.

Das beseitigt unnoetigen Verbrauch und Fehlpriorisierung, garantiert aber
keine ausreichenden Manareserven bei zu hohem eingehendem Schaden.

## Pruefung und Artefakte

`build/test_known_fixes.py` aktualisiert einen C++-Test mit den tatsaechlichen
Produktionsfunktionskoerpern. Die Tests decken Notfallheilung neben gesunden
Tanks, die niedrige HP-Zielgruppe, Vorheilungs-Unterlauf, Bossgroesse bei
Schussdistanz, Aggro-/Stay-/Bewegungssperren, Bewegungsziel, naechsten lebenden
Heiler, ungueltigen Boden und die Zustaendigkeit fuer Tankbewegung ab.
`build/test_extra_fixes.py` prueft zusaetzlich die Kurator-Phasenregel mit
leerem Suchcache, die Schlussphase, Tankbewegung, ueberholte Heilaktionen,
Selbstheilung bei Gruppen-Notfaellen, die begrenzte Soulshatter-Ausnahme,
Fintes feindliches Ziel und Feuerkontrolle fuer beide Zauberklassen.
Die Engine-, Navigations- und Kampfinteraktion muss im Live-Kampf bestaetigt
werden.

Der separate Build verwendet `playerbots-attumen.lib` als Basis und ersetzt
nur die fuenfzehn betroffenen Objektdateien einschliesslich der Engine und der erforderlichen
Schurken-Fabriken. Ziel ist `build/bin/x64_Release/mangosd-known-fixes.exe`.
Der Benutzer hat nach diesen Aenderungen einen Neustart mit fuenf Sekunden
Vorwarnung ausdruecklich freigegeben. Installation folgt erst nach allen
Build- und Pruefschritten; die vorherige Serverdatei wird gesichert.
