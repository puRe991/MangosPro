# Prinz Malchezaar – Untersuchung 18.10.2026

Kampffenster 17:15:13–17:18:40. Um 17:15:50 sterben Derick, Lavenna, Sailan und Sangon mit Enfeeble (30843). Vor Shadow Nova (30852) stehen Derick/Lavenna etwa 18.1/15.3 Yards entfernt; Sailan/Sangon stoppen bei etwa 27.8/23.8 Yards.

Fluchtaktionen: 14 ausgeführt, 22 fehlgeschlagen. Infernal-Ausweichen der Nichttanks: 125 fehlgeschlagen, keine ausgeführte Aktion. Tankposition: 61 fehlgeschlagen, keine ausgeführte Aktion. Eine fehlgeschlagene Aktion bedeutet nicht immer einen Fehler (z.B. Position bereits sicher); die Häufung zusammen mit den zu kurzen Fluchtwegen rechtfertigt die Prüfung der Bewegungslogik.

Gefundene Fehler: Ein Pfad mit Start innerhalb einer Infernalzone wurde grundsätzlich abgelehnt, auch wenn er aus der Zone hinausführt. Die Tankbewegung verlangte zusätzlich Nahkampfreichweite und konnte damit weiteres Ziehen abbrechen. Stay konnte normales Ausweichen verhindern. Die Flucht hatte keine kurze Nachprüfung des erreichten Abstands. Hinzu kommen viele Sichtlinien- und Zielwechselprobleme, deren Anteil am Gesamtschaden ohne Live-Prüfung offenbleibt.

Korrekturen und Debug-Ausgaben siehe `Bot_Raid_Followup_Fixes.md`. Status: vorbereitet, nicht installiert. Der letzte HP-Sprung von 69.5% auf 0 ist kein Beleg für einen regulären Kill.
