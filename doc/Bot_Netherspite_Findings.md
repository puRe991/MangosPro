# Netherspite: Logpruefung vom 18.10.2026

Nur untersucht. Keine Korrektur, DB-Aenderung oder Neustart durchgefuehrt.
Fenster: 17:04:33–17:07:02, Europe/Berlin.
Analyseauszug: `build/netherspite-window.json` (ignoriertes Build-Artefakt).
Weltserver bei der Pruefung: PID 10392, gestartet 16:48:15.

## Gesicherte Beobachtungen

- Boss-HP: Minimum im Ausschnitt 92,9815 Prozent, zuletzt 99,9881 Prozent.
  Nach der Verbannungsphase steigt der Boss ab ca. 17:06:13 wieder nahezu
  auf volle HP. Kein Kill im Ausschnitt.
- Keine protokollierte Aktivierung oder Ausfuehrung von
  `netherspite block red beam`, `... blue beam` oder `... green beam`.
  Auch keine entsprechenden START/FAILED/BLOCKED-Eintraege. Die drei
  Beam-Trigger werden im Gegensatz zu anderen Netherspite-Triggern nicht
  als TRIGGERED protokolliert.
- Boss erhaelt Strahlbuffs: Serenity 30467 bis 32 Stapel, Dominance 30468
  bis 51, Perseverence 30466 bis 27. Diese Zauber belegen, dass Portale/
  Strahlwirkungen vorhanden sind. Ungeblocktes Gruen heilt den Boss.
- Die lokale DB bestaetigt fuer 30467 einen periodischen Heilungseffekt
  (EffectApplyAuraName2=8, EffectBasePoints2=3999).
- Spielerbuffs sind nicht voellig abwesend: Sailan erhaelt im Verlauf Rot,
  Gruen und Blau sowie rote Erschoepfung; Sangon Rot und Gruen, Jovilenne
  kurz Rot. Andere Bots haben keine dieser Auren in den erfassten Zustaenden.
  Das belegt einzelne Strahlkontakte, keine korrekte Rollenverteilung.
- Verbannungsvisual 38542 ist 17:05:34–17:06:05 in den Boss-Snapshots
  vorhanden; Dauer passt zum lokalen 60s-Strahl-/30s-Verbannungsablauf.
  Grundlegende Phasenwechsel funktionieren also. Nicht daraus folgern,
  dass jede einzelne Bossfaehigkeit korrekt ausgefuehrt wurde.
- Wiederholte Todesfaelle, besonders bei Heilern: Derick fuenf erkannte
  Alive->Dead-Uebergaenge, Sosho drei. Zustandslogs ersetzen kein genaues
  Schadensereignisprotokoll.

## Code-/DB-Pruefung

- Boss 15689 ist an `boss_netherspite` gebunden; Portale 17367/17368/17369
  korrekt an `npc_netherspite_portal`. Keine fehlenden ScriptName-Bindungen
  fuer diese vier Templates gefunden.
- Beam-Trigger und Aktionen sind in KarazhanContext registriert und in
  KarazhanStrategy verdrahtet.
- `IsBeamActive` verlangt einen gefundenen Boss, keine Verbannung und ein
  per `GetKaraUnit` gefundenes farbiges Portal.
- Suchkette: `KarazhanNearbyCreaturesValue` -> `GetKaraUnits` ->
  `GetKaraUnit` -> `IsBeamActive`. Die Portal-IDs sind in IsTrackedEntry
  enthalten. Warum diese Such-/Triggerkette im Versuch keine Blockieraktion
  ausloest, ist noch nicht abschliessend bewiesen.
- Blocker-Zuordnung bevorzugt Tanks fuer Rot, Mana-DDs fuer Blau und
  Nicht-Mana-DDs bzw. Heiler fuer Gruen. Sie prueft farbige Erschoepfung,
  Blue/Healer-Green-Stacks und speichert Zuordnungen pro Instanz.
- FindBeamPosition sucht zwischen Boss und Portal, 18–30 Meter vom Boss.
  Zu kurze Portalabstaende sowie Boden-/Sichtlinienpruefung sind bei der
  weiteren Untersuchung ebenfalls zu kontrollieren; bislang keine Ursache
  fuer die fehlenden Trigger nachgewiesen.

## Naechste Arbeit

1. Portal-Suche und IsBeamActive instrumentieren: Portal-GUID, Entry,
   lebend/in Welt/Instanz/Entfernung, Suchcache-Inhalt, Phasenentscheidung.
2. Pro Farbe zugewiesenen Bot und Ausschlussgrund protokollieren, ebenso
   Zielpunkt, Abstand zur Strahllinie, aktuelle Strahlaura/Stacks/Erschoepfung.
3. Erst danach gezielt Erkennung/Zuordnung korrigieren; sichere Uebergabe
   beim Wechsel von Blockern und nach Tod/Verbannung pruefen.
4. Im Live-Versuch bestaetigen, dass Gruen den Boss nicht dauerhaft heilt,
   Blau nicht den Boss hochstapelt und Rot vom vorgesehenen Tank kontrolliert
   wird. Normales Nachlaufen darf Positionen nicht ueberschreiben.

Nachtrag: Portal-Rückfallsuche, exklusive Strahlrollen, Nahkampfpositionen,
stationäres Zaubern und Bewegungsschutz sind im Folgepaket vorbereitet.
`[RaidBeam]` protokolliert Portalfunde, Zuteilung und Stapel; die ursprüngliche
Ursache der fehlenden Trigger bleibt bis zur Live-Prüfung offen. Details in
`Bot_Raid_Followup_Fixes.md`.

Der Untersuchungsauftrag enthaelt keine neue Freigabe zum Stoppen oder
Neustarten des Servers.
