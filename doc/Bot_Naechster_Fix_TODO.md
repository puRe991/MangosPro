# Playerbots – To-do für den nächsten Fix

Stand: 18.10.2026. Grundlage: statische Codeprüfung. Noch nicht behoben und nicht im laufenden Server reproduziert. Dieser Auftrag erlaubt Dokumentation und weitere Prüfung, keine Codeänderungen oder Servereingriffe.

## Hohe Priorität

- [ ] **Magtheridon: unnötige Würfel-Reaktionen verhindern.** `MagtheridonIncomingBlastNovaTrigger::IsActiveInEncounter()` ist für zugeteilte Würfelspieler während der gesamten aktiven Bossphase wahr. Die Reaktionssteuerung kann dadurch laufende Zauber abbrechen, bevor `Execute()` feststellt, dass noch keine Vorbereitung notwendig ist. Trigger/Nützlichkeit und Unterbrechungsregeln auf tatsächlichen Handlungsbedarf begrenzen. Prüfen: Heilzauber außerhalb des Vorbereitungsfensters, rechtzeitiges Anlaufen, Klicken während Nova und korrektes Loslassen. Stellen: `strategy/raids/magtheridon/MagtheridonTriggers.cpp:95`, `MagtheridonActions.h`, `PlayerbotAI.cpp:654`.
- [ ] **Befehlsserver: Puffergrenze korrigieren.** In `PlayerbotCommandServer.cpp:25–34` kann `read_some` alle 1025 Bytes lesen; `buf[n] = 0` schreibt dann außerhalb des Arrays. Empfang längenbasiert verarbeiten oder Platz für Terminierung reservieren. Prüfen: volle Puffer, über mehrere Lesevorgänge verteilte Zeilen und Verbindungsende. Betrifft aktivierten `AiPlayerbot.CommandServerPort`, standardmäßig deaktiviert.

## Mittlere Priorität

- [ ] **Standard-Loginregeln korrekt aufteilen.** `PlayerbotAIConfig.cpp:308–310` speichert kommaseparierte Kriterien als jeweils einen String; `PlayerbotLoginMgr::GetLoginCriteria()` erwartet einzelne Namen. Fallback-Regeln in einzelne Einträge aufteilen. Prüfen: asynchroner Login ohne explizite `LoginCriteria`, Klassen-/Rassen-/Levelverteilung und zeitgesteuerter Logout.
- [ ] **Gilden-ID 0 ausschließen.** `PlayerbotLoginMgr.cpp:113–126`, `IsInPlayerGuild()`: Zwei gildenlose Charaktere gelten wegen `0 == 0` als gemeinsame Gilde. Prüfen: beide ohne Gilde, unterschiedliche Gilden und tatsächlich gleiche Gilde.
- [ ] **Login-Fehlerpfad zurücksetzen.** `PlayerbotLoginMgr.cpp:174–177`: Nach fehlgeschlagenem `Initialize()` bleibt `holderState` auf `HOLDER_SENT` und `holder` zeigt auf gelöschten Speicher. Zeiger und Status konsistent zurücksetzen. Prüfen: künstlicher Initialisierungsfehler und anschließender erfolgreicher Wiederholungsversuch; keine Anfrage als ausstehend markieren, wenn kein Callback folgen kann.
- [ ] **Begleiterheilung auf das richtige Ziel prüfen.** `strategy/values/PartyMemberToHeal.cpp:122` prüft laufende Heilung auf `player` statt auf `pet`. Gemeinsamen Helfer passend für Begleiter erweitern; dessen aktuelle Schnittstelle erwartet `Player*`. Prüfen: Besitzer wird geheilt/Begleiter verletzt, Begleiter wird bereits geheilt sowie beide verletzt.

Alle genannten Quellpfade sind relativ zu `src/modules/PlayerBots/playerbot/`, sofern nicht anders angegeben. Zeilennummern beziehen sich auf den Prüfstand und können sich bei späteren Änderungen verschieben.

## Weitere Prüfung

- [ ] Bossstrategien erneut zusammenhängend auf Trigger, Multiplikatoren, Reaktionsaktionen, Zielwahl und Bewegungsregeln prüfen; bestätigte zusätzliche Befunde hier ergänzen.
