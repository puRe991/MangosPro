# Leotheras – Untersuchung und Folgekorrekturen 18.10.2026

Kampffenster im ausgewerteten SSC-Log: 18:14:30–18:16:54. Quelle: `build/ssc-window.json`.

Die dedizierte Taktik ist aktiv: 225 ausgeführte Wirbelwind-Fluchten, 262 Fernkampf-Positionsaktionen, 30 Dämonen-Tankaktionen und 20 Aktionen gegen innere Dämonen. 104 Aktionen stoppen die Nahkampftanks während der Dämonenphase. Diese Zahlen bestätigen Ausführung, aber keine erfolgreiche Mechanik.

Innere Dämonen/Insidious Whisper 37676 sind bei Rakaejin, Melidoran, Jovilenne und Olistra jeweils ungefähr 30 Sekunden sichtbar. Drei DDs haben Dämonen 21857 als aktuelles Ziel; die Heileraktion konnte vorher direkt auf den Dämon zaubern, ohne das aktuelle Kampfziel zu setzen. Später werden viele `attack enemy player`-Aktionen gegen Raidmitglieder ausgeführt. Das passt zu nicht rechtzeitig getöteten Dämonen und anschließender Kontrolle; eine vollständige Zuordnung jedes PvP-Angriffs erfordert die folgenden Live-Logs.

Boss-Leben fällt im normal beobachteten Verlauf nur von 100 auf etwa 92%. Es gibt viele Todesfälle und konkurrierende Bewegung/Zielwahl. Die Logs allein belegen keine fehlerfreie Hexertank- oder Wirbelwindreaktion.

Vorbereitete Korrekturen:

- Eigener innerer Dämon hat Vorrang in DD-, Tank- und generischer Raid-Zielwahl. Die Ziele anderer Spieler werden nicht übernommen.
- Eigener lebender Dämon wird vor einem reinen Auswahlwechsel als gültig behandelt; grundsätzliche Angreifbarkeit wird weiter geprüft.
- Heileraktion erwirbt ihr Kampfziel ausdrücklich und nähert sich bei fehlender Reichweite/Sichtlinie an. Paladin bekommt nutzbare Siegel-/Richturteil-Fallbacks.
- Der Hexertank darf ebenfalls seinen eigenen Dämon bekämpfen. Angriffe auf andere Ziele und konkurrierende Laufwege werden während der eigenen Dämonenaufgabe unterdrückt.
- Nahkampftanks wählen während der Dämonenphase mit vorhandenem Hexertank kein normales Assist-Ziel; bereits gestoppte Tanks wiederholen nicht sinnlos die Stoppaktion.
- Wirbelwind-Flucht schützt ihren Laufweg auch direkt vor anderen Bewegungsaufrufen.
- `[RaidLeotheras]` protokolliert Phase, Hexertank und Bossziel; `[RaidInnerDemon]` protokolliert Besitzer, Ziel-HP, Entfernung, Sichtlinie und Zielwahl.

Zusätzlich stellt eine lokale Scope-Wache in der allgemeinen Zauberfunktion die vorherige Auswahl auch bei frühen Abbrüchen wieder her. Vorher blieb nach bestimmten fehlgeschlagenen Hilfszaubern ein anderes Auswahlziel zurück; die Invalid-Target-Prüfung konnte daraufhin den Kampf neu beginnen. Dieser Fehler ist im Code belegt, sein Anteil an den einzelnen Log-Schleifen ist ohne Auswahl-GUID-Diagnose nicht exakt messbar.

Gezielte Tests prüfen Produktionsfunktionen und Integration. Live-Bestätigung nach Installation bleibt erforderlich; keine Zusage vollständig korrekter Rotationen oder aller Bossmechaniken.

Installationsnachtrag: Am 18.10.2026 mit dem freigegebenen gemeinsamen Folgepaket installiert. Live-Raid-Best�tigung ausstehend.
