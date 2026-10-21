# Raid-Korrekturen – Folgepaket 18.10.2026

Installiert am 18.10.2026 nach ausdr�cklicher Neustartfreigabe, mit f�nf Sekunden Vorwarnung. Vorbereitete Datei: `build/bin/x64_Release/mangosd-karazhan-followup.exe`; installiert als `mangosd.exe`. Build-Pr�fsumme und Prozess stehen in `build/karazhan-followup-manifest.json`; Sicherung und Neustartdaten in `build/raid-followup-deployment.json`. Serverstart abgeschlossen und World-Port 8085 f�r den neuen Prozess 2460 gepr�ft. SHA256: `0AB3398C3A1EDBD98A9F403CA7B4DB7786FFF045EBCD27895BFCDB9F24E9BB09`. Raid-Debug-Marker weiterhin aktiv.

## Terestian

Lebende, angreifbare Dämonenketten werden als passives Kampfziel anerkannt. DDs fokussieren zuerst Ketten, dann den nicht verbannten Kil'rek, dann Illhoof. Haupttank bleibt bei Illhoof, zweiter Tank übernimmt Kil'rek. Generische Zielwahl und Boss-Zielmarkierung folgen derselben Reihenfolge. Fremdziele und konkurrierende Flächen-/Nebenzielfähigkeiten werden während aktiver Ketten unterdrückt.

## Aran

Flammenkranz sperrt konkurrierende Laufwege auch direkt an den Bewegungsfunktionen. Bereits stehende Bots unterbrechen dafür nicht ständig ihre Zauber. Bei Arkane Explosion laufen Bots bis 26 statt 21 Yards Abstand und prüfen die Flucht nach 200 ms erneut; längere Zauber müssen während der Flucht warten. Entfernung und Bewegungsentscheidung werden als `[RaidEscape]` protokolliert.

## Netherspite

Portale und Gefahren bekommen eine direkte Suche als Rückfall, wenn der normale GUID-Cache kein Ergebnis liefert. Nur der zugeteilte Bot aktiviert die jeweilige Strahlaktion. Rote und grüne Nahkampfpositionen funktionieren auch nahe am Boss; Suchpunkte berücksichtigen Leerezonen. Angekommene blaue/grüne Strahlspieler unterbrechen ihre laufenden Zauber nicht mehr bei jedem Takt. Konkurrierende Laufwege werden während der Zuteilung unterdrückt. `[RaidBeam]` zeigt gefundene Portale, Zuteilungen, Phase und Stapel.

Die ursprüngliche Ursache der vollständig fehlenden Strahltrigger ist mit den alten Logs nicht abschließend bewiesen. Die neue Diagnose muss beim nächsten Raid bestätigen, dass Portalsuche, Trigger und tatsächlicher Strahlfang zusammen funktionieren.

## Prinz Malchezaar

Enfeeble-Flucht mit 36 Yards Zielabstand und kurzer Nachprüfung. Eine bereits betretene Infernalzone darf nach außen verlassen werden; der alte Pfadtest konnte jede Fluchtrichtung blockieren. Pfade durch weitere Gefahren werden abgelehnt. Tank kann den Boss weiterziehen, auch wenn er vorübergehend außerhalb der Nahkampfreichweite ist. Gefahrenbewegungen sind auch bei einem Stay-Befehl möglich. Fluchtentscheidungen werden protokolliert.

## Akil'zon

Siehe `Bot_Akilzon_Findings.md`: hochgehobenes Sturmziel vom Sturmschaden ausnehmen und Folge-Sturmfenster auf 54–60 Sekunden korrigieren.

## Prüfung und Grenzen

Das Paket wurde im Folgeauftrag um Magtheridon und Leotheras ergänzt. Details:
`Bot_Magtheridon_Findings.md` und `Bot_Leotheras_Findings.md`. Neue Szenarien
prüfen fünf eindeutige nahe Würfelzuordnungen, Tank-Ausschluss, Ersatz nach Tod
oder Erschöpfung, Vorbereitungsgrenze, Bewegungsschutz und Auswahl-Rückgabe bei
frühem Zauberabbruch. Der Neustart nach Abschluss ist inzwischen ausdrücklich
freigegeben; der tatsächliche Installationsstand steht im Build-Manifest.

Basis ist das zuletzt installierte Archiv `playerbots-known-fixes.lib`; dessen bisherige Korrekturen bleiben enthalten. Gezielte Szenarien prüfen Produktionsfunktionen für Ketten-Ziele, Portal-Rückfall, Gefahrenpfade, Fluchtgrenzen, Strahlrollen, Erschöpfung/Stapelwechsel, stationäre Zauber und Akil'zon-Schutz/Timer. Separate Übersetzung der geänderten Einheiten und Linkprüfung des Pakets. Diese Tests ersetzen keinen Raid-Test und garantieren keine fehlerfreien Rotationen oder sämtliche Bossmechaniken.
