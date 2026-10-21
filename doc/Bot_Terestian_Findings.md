# Terestian: offene Befunde fuer die naechste Arbeit

Auf ausdruecklichen Benutzerwunsch am 18.10.2026 festgehalten.
Noch keine Terestian-Korrektur implementiert oder eingespielt.

## Stand und Aufzeichnung

- Untersucht: 18.10.2026 ca. 01:02:58 bis 01:07:11, Europe/Berlin.
- Server nach dem vorher freigegebenen Neustart: PID 11196, Welt-Port 8085.
- Installierter Build SHA256:
  `E3D72459A6A266FCCC1813B644BD80E59A8B0A56A24AC1E74B7B5DD83D561364`.
- Vorherige Korrekturen sind in `Bot_Moroes_Maiden_Fix.md` dokumentiert;
  Build-/Installationsdaten: `build/known-fixes-manifest.json`.
- Lokaler Analyseauszug: `build/illhoof-window.json` (ignoriertes Build-Artefakt,
  keine dauerhafte Loggarantie).

## Beobachtungen

1. Terestian (15688) steht um 01:06:28 noch bei 83,2591 Prozent HP.
   Um 01:06:29 springt der erfasste Wert auf null. Dieser Sprung belegt keinen
   normalen durch Bot-Schaden erreichten Kill; Ursache aus diesen Logs unklar.
2. Beide Tanks wechseln oft auf einzelne Fiendish Imps (17267):
   Lengocow 74 ausgefuehrte `tank assist`, Sailan 63. Lengocow fuehrt im
   untersuchten Abschnitt keinen Shield Slam/Devastate aus, Sailan drei
   Consecrations. Beide Tanks verlieren dadurch viel Zeit fuer Zielwahl und
   Nachlaufen. Das ist eine Beobachtung der KI-Aktionen, keine Schadensmessung.
3. Auf Demon Chains (17248) folgen wiederholt `invalid target`, `dps assist`
   und `select new target`. Die Ketten werden im Bossskript passiv gesetzt.
   `PossibleAttackTargetsValue::IsValid` verwendet standardmaessig
   `AttackersValue::IsValid` mit Kampf-/Bedrohungspruefung und verlangt danach
   unter anderem ein Opfer, UNIT_FIELD_TARGET oder eine andere Ausnahme.
   Die generische Raid-Prioritaet prueft dagegen mit deaktivierter
   Kampfbedingung. Diese widerspruechlichen Regeln sind ein konkreter Ansatz
   fuer die erneute Zielwahl; einzelne Ablehnungszweige sind nicht geloggt.
4. Ketten werden angegriffen, aber nicht konsequent mit direkten Aktionen:
   Beispiel 01:04:24: Lavenna hat Ketten als aktuelles Ziel, setzt aber
   `seed of corruption on attacker` auf Terestian. Um 01:04:28 folgt Saat auf
   einem weiteren Gegner. Die Aktionsziele sind separat protokolliert.
5. Beobachtete Zeit vom ersten Ketteneintrag bis zum ersten HP-null-Eintrag:
   - GUID 5860544: 01:03:33–01:03:39, etwa 6 Sekunden.
   - GUID 5860566: 01:04:18–01:04:31, etwa 13 Sekunden.
   - GUID 5860587: 01:05:06–01:05:10, etwa 4 Sekunden.
   - GUID 5860606: 01:05:51–01:06:05, etwa 14 Sekunden; beim ersten Eintrag
     bereits ca. 86,9 Prozent HP, also keine exakte Lebensdauer.
   Nachfolgende Eintraege toter Ketten nicht als Zerstoerungsdauer zaehlen.
6. Mehrere Tank-/DD-/Heilertode. `NO_POWER`-Ablehnungen: Sosho 68,
   Derick 8; dies sind gedrosselte Diagnoseeintraege, keine exakte Zahl aller
   versuchten Zauber. Finte wird jetzt ausgefuehrt (10 Aktionen). Seelenbrechen
   wird hier mit NOT_READY abgelehnt, nicht mehr TARGET_FRIENDLY; kein Beleg,
   dass Cooldowns falsch sind.

## Naechste Korrekturen und sinnvolle Pruefung

- Haupttank an Terestian binden; Offtank/Wichtelkontrolle sinnvoll trennen,
  damit beide Tanks nicht staendig einzelnen Fernkampf-Wichteln nachlaufen.
- Passive Mechanikziele wie Demon Chains konsistent als gueltige Ziele
  behandeln, ohne normale Schutzpruefungen fuer tote, unantastbare, fremde
  oder unerreichbare Ziele pauschal auszuschalten.
- Waehrend lebender Ketten direkte Schadensaktionen darauf priorisieren;
  konkurrierende Mehrziel-DoTs und normale Assist-Wechsel gezielt begrenzen.
  Wichtel-AoE ausserhalb dieser Notfallphase erhalten.
- Taktische Markierungen und generische TBC-Prioritaeten auf dieselbe
  Zielentscheidung abstimmen (Ketten, Kil'rek, Boss).
- Regressionstests fuer passive Ketten, sterbende/verschwundene Ketten,
  Rueckkehr zum Boss, Offtank-Aggrohilfe und erlaubte AoE einplanen.
- Im naechsten Live-Versuch Ketten-Zerstoerungszeit, Tank-Zielwechsel,
  direkte Schadensaktionen, Boss-HP-Verlauf und Heilermana vergleichen.

## Betriebsgrenze

Nachtrag: Die Korrekturen an Ketten-Gültigkeit, Zielreihenfolge und konkurrierenden
Fähigkeiten sind inzwischen im Folgepaket vorbereitet. Siehe
`Bot_Raid_Followup_Fixes.md`. Noch nicht installiert; Live-Prüfung ausstehend.

Der Benutzer wollte diese Befunde fuer das naechste Mal merken.
Die letzte Neustartfreigabe galt dem bereits erledigten vorherigen Deployment.
Fuer diese offenen Terestian-Korrekturen liegt keine weitere ausdrueckliche
Neustartfreigabe vor. Den laufenden Server nicht eigenmaechtig stoppen,
ersetzen oder neu starten.
