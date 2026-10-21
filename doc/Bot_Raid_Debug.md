# Boss-Taktiken pruefen

Einem Bot in der Instanz zufluestern:

```text
debug raid
debug raid on
debug raid off
debug raid all
debug raid full
```

`debug raid` zeigt Map und Instanz, Registrierung der Boss-Strategie sowie deren
Aktivierung in Kampf-, Ruhe- und Reaktionsmodus. Die Verdrahtungspruefung losest
die Trigger und Aktionen der Strategie auf, ohne die Trigger auszufuehren.
`Fehlend=0` bestaetigt die Registrierung; es beweist noch keine erfolgreiche
Ausfuehrung einer Mechanik im Kampf.

In Gruuls Unterschlupf werden ausserdem die erkannten Tanks fuer Maulgar, Olm,
Blindeye, Krosh und Kiggler sowie Heilerzahl und Boss-Sichtbarkeit angezeigt.
`FEHLT` bedeutet, dass die aktuelle Auswahl keinen lebenden geeigneten Spieler
auf dieser Map gefunden hat. Krosh braucht fuer diese Taktik einen Magier,
Kiggler einen Balance-Druiden; die drei Nahkampf-Zuweisungen brauchen drei
erkannte Tanks. Ein Wipe allein zeigt nicht, ob die Taktik deaktiviert war.

Beim automatischen Aktivieren bzw. Entfernen einer Boss-Strategie erscheint
eine Zeile mit `[RaidTactics]` in `logs/Server.log` und in der Serverausgabe.
Bots mit menschlichem Master bestaetigen die Aktivierung auch per Fluestern.
Die Synchronisierung prueft jede Sekunde alle KI-Modi, auch nach einem Reset
einzelner Modi. Die normale KI-Aktualisierungszeit kommt dazu.

`debug raid on` schaltet fuer diesen Bot zusaetzlich Aktionsdiagnosen in
`logs/Server.log` ein, zum Beispiel (nur ein Formatbeispiel):

```text
[RaidTactics] bot=Name map=565 instance=1 strategy="boss gruul" state=0 trigger="high king maulgar krosh needs mage tank" action="high king maulgar mage tank attack krosh" result=EXECUTED
```

`EXECUTED` bzw. `FAILED` sind der Rueckgabewert der ausgefuehrten Aktion.
Unnuetze, unmoegliche oder durch Prioritaeten/Multiplikatoren verworfene Aktionen
erreichen diese Ausgabe nicht. Wiederholungen derselben Aktion und desselben
Ergebnisses werden pro KI-Modus auf einmal in fuenf Sekunden begrenzt.
Modi: `0` Kampf, `1` Ruhe, `2` tot, `3` Reaktion.
Mit `debug raid off` abschalten; die Einstellung wird nicht gespeichert.

`debug raid all` prueft alle hinterlegten Mapstrategien samt Triggern und Aktionen.
Details zu Korrekturen, Logbefunden und Grenzen der Pruefung stehen in
[Bot_Raid_Audit.md](Bot_Raid_Audit.md).

## Rotation und Reaktionen auswerten

`debug raid full` aktiviert fuer den Bot die umfassende Instanzdiagnose. Liegt
`raid-ai-debug.enabled` neben `mangosd.exe`, startet sie automatisch fuer Bots
in derselben Gruppe wie ihr menschlicher Master, sobald sie in einer Instanz
sind. Das gilt auch nach einem Neustart. `debug raid off` deaktiviert sie fuer
diesen Bot bis zum naechsten Neustart; das Entfernen der Markerdatei beendet
die automatische Aktivierung nach spaetestens zehn Sekunden.

Die ausfuehrlichen Dateien liegen unter `logs/raid-ai-*.log`. Neue Segmente
beginnen nach 64 MiB. Bei weniger als 2 GiB freiem Speicher pausiert diese
Detaildiagnose mit einem Eintrag im Serverlog; vorhandene Logs werden nicht
geloescht. Identische Start-/Erfolgsmeldungen und Zustandsbilder werden auf
einmal pro Sekunde begrenzt, identische verworfene Aktionen auf einmal in
fuenf Sekunden. Die Aufzeichnung beeinflusst die Aktionsauswahl nicht.

`[BotDecision]` unterscheidet `START`, `EXECUTED`, `FAILED`, `USELESS`,
`IMPOSSIBLE`, `BLOCKED` (ggf. mit Multiplikatorname), `STUNNED`, `UNKNOWN`,
`PREREQUISITE`, aktive `TRIGGERED`-Ereignisse und periodische `STATE`-Bilder.
Es werden Zustand, Prioritaet, Aktionsdauer, HP, Ressourcen, Position,
Bewegung, laufender Zauber/Kanal, Kampfziel, dessen HP/Aggrohalter/Entfernung
sowie das von der Aktion aufgeloeste Ziel aufgezeichnet. `STATE` fuehrt
ausserdem Bot-/Ziel-Auren als `SpellId:Stacks:RestdauerMillisekunden` und
den aktuellen Zielzauber auf. Eine Ringhistorie mit 512 Aktionen pro Bot
steht fuer `debug history 128` zur Verfuegung.

`IMPOSSIBLE` ist die gemeinsame Rueckgabe von `isPossible`; die Diagnose
zerlegt sie nicht automatisch in jeden einzelnen Klassencheck. Der
Aktionszielwert wird zur Aufnahmezeit aufgeloest. Die Auswertung muss
Rolle, Ressourcen, Auren und Bossphase mit einbeziehen; aus einem einzelnen
verworfenem Zauber folgt keine falsche Rotation.
