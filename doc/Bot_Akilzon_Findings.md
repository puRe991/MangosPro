# Akil'zon – Untersuchung 18.10.2026

Logfenster: 17:40:08–17:43:30, Zul'Aman, Instanz 7. Auszug: `build/akilzon-window.json`.

Die dedizierte Taktik läuft: 62 ausgeführte Sturmbewegungen, 103 Verteilbewegungen und 76 Tankpositionsaktionen. Das bedeutet noch nicht, dass alle Bewegungen rechtzeitig abgeschlossen wurden.

Erster Sturm ab 17:40:58: Ziel Lavenna, Position 367.876/1428.54/85.707. Die anderen Bots sammeln sich bis etwa 17:41:02 auf 0.3–1.3 Yards horizontal unter ihr und erhalten Schutz-Aura 44007. Ladina und Sangon kommen bereits mit wenig Leben an. Lavenna selbst hat die Sturm-Aura 43648, aber keinen Schutz 44007. Ihr Leben fällt 97.7 → 91.7 → 79.6 → 61.5 → 37.4 → 0 bis 17:41:04. Danach verliert die Gruppe ihr Sturmziel; weitere Todesfälle folgen.

Der Core prüft bei Schadenszauber 43657 ausschließlich Schutz 44007. Das hochgehobene Sturmziel kann außerhalb seiner Schutzfläche liegen. Korrektur: Auch Träger von 43648 werden vom Sturmschaden ausgenommen. Die Auswertung belegt fehlenden Schutz und ein zum Sturmschaden passendes Schadensmuster; die AI-Snapshots sind kein vollständiges Schadensereignisprotokoll.

Weitere Stürme im Log: Lengocow ab 17:41:59 (mit Schutz), Ladina ab 17:42:55 (ab nächstem Snapshot geschützt). Das Skript wiederholt den Wetter-Timer alle 54–60 Sekunden; die zusätzliche Verzögerung von 8.5 Sekunden gilt jeweils und addiert sich deshalb nicht zum Abstand aufeinanderfolgender Stürme. Bot-Vorwarnung bisher auf 62.5–68.5 Sekunden eingestellt. Auf 54–60 Sekunden mit bestehendem Vorlauf von 5 Sekunden korrigiert.

Gezielte Tests verwenden die Produktionsbedingungen für Schutzprüfung und Sturmfenster. Ein Raid-Test nach bewusster Installation bleibt erforderlich. Adler-Zielwahl und sämtliche Todesursachen sind damit noch nicht vollständig nachgewiesen.

Status: Korrekturen im vorbereiteten Folge-Build, nicht auf den laufenden Server übertragen.
