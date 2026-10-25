# Playerbots – To-do für den nächsten Fix

Stand: 21.10.2026. Die Punkte unter "Hohe" und "Mittlere Priorität" sind im Code behoben (statisch geprüft, noch nicht im laufenden Server reproduziert). Offene Punkte stehen unter "Offen: Bot-DPS zu niedrig".

## Hohe Priorität

- [x] **Magtheridon: unnötige Würfel-Reaktionen verhindern.** *(Erledigt: `NeedsCubeAction` greift nur noch bei bevorstehender Nova ausserhalb der Würfelposition, und die Aktion unterbricht keine Zauber per Reaktion.)* `MagtheridonIncomingBlastNovaTrigger::IsActiveInEncounter()` ist für zugeteilte Würfelspieler während der gesamten aktiven Bossphase wahr. Die Reaktionssteuerung kann dadurch laufende Zauber abbrechen, bevor `Execute()` feststellt, dass noch keine Vorbereitung notwendig ist. Trigger/Nützlichkeit und Unterbrechungsregeln auf tatsächlichen Handlungsbedarf begrenzen. Prüfen: Heilzauber außerhalb des Vorbereitungsfensters, rechtzeitiges Anlaufen, Klicken während Nova und korrektes Loslassen. Stellen: `strategy/raids/magtheridon/MagtheridonTriggers.cpp:95`, `MagtheridonActions.h`, `PlayerbotAI.cpp:654`.
- [x] **Befehlsserver: Puffergrenze korrigieren.** In `PlayerbotCommandServer.cpp:25–34` kann `read_some` alle 1025 Bytes lesen; `buf[n] = 0` schreibt dann außerhalb des Arrays. Empfang längenbasiert verarbeiten oder Platz für Terminierung reservieren. Prüfen: volle Puffer, über mehrere Lesevorgänge verteilte Zeilen und Verbindungsende. Betrifft aktivierten `AiPlayerbot.CommandServerPort`, standardmäßig deaktiviert.

## Mittlere Priorität

- [x] **Standard-Loginregeln korrekt aufteilen.** `PlayerbotAIConfig.cpp:308–310` speichert kommaseparierte Kriterien als jeweils einen String; `PlayerbotLoginMgr::GetLoginCriteria()` erwartet einzelne Namen. Fallback-Regeln in einzelne Einträge aufteilen. Prüfen: asynchroner Login ohne explizite `LoginCriteria`, Klassen-/Rassen-/Levelverteilung und zeitgesteuerter Logout.
- [x] **Gilden-ID 0 ausschließen.** `PlayerbotLoginMgr.cpp:113–126`, `IsInPlayerGuild()`: Zwei gildenlose Charaktere gelten wegen `0 == 0` als gemeinsame Gilde. Prüfen: beide ohne Gilde, unterschiedliche Gilden und tatsächlich gleiche Gilde.
- [x] **Login-Fehlerpfad zurücksetzen.** `PlayerbotLoginMgr.cpp:174–177`: Nach fehlgeschlagenem `Initialize()` bleibt `holderState` auf `HOLDER_SENT` und `holder` zeigt auf gelöschten Speicher. Zeiger und Status konsistent zurücksetzen. Prüfen: künstlicher Initialisierungsfehler und anschließender erfolgreicher Wiederholungsversuch; keine Anfrage als ausstehend markieren, wenn kein Callback folgen kann.
- [x] **Begleiterheilung auf das richtige Ziel prüfen.** `strategy/values/PartyMemberToHeal.cpp:122` prüft laufende Heilung auf `player` statt auf `pet`. Gemeinsamen Helfer passend für Begleiter erweitern; dessen aktuelle Schnittstelle erwartet `Player*`. Prüfen: Besitzer wird geheilt/Begleiter verletzt, Begleiter wird bereits geheilt sowie beide verletzt.

## Offen: Bot-DPS zu niedrig (25.10.2026)

Anlass: Recount in Gruul's Lair zeigt bei Bots mit T5/T6 nur etwa 680–1170 DPS (eher T4-Niveau, erwartet wären etwa 1300–2000+). Befund nur aus dem Code, noch nicht im Spiel gemessen.

- [ ] **Wartezeit nach jedem Zauber verkürzen.** `PlayerbotAI::GetSpellCastDuration()` (`PlayerbotAI.cpp:4349–4386`) gibt `max(Castzeit, GCD) + reactDelay` zurück, und die TBC-Config setzt `AiPlayerbot.ReactDelay = 500` (`aiplayerbot.conf.dist.in.tbc:881`; Code-Standard und Classic-Config: 100). Auch während eines Casts wird `reactDelay` plus `sWorld.GetAverageDiff()` aufgeschlagen (`PlayerbotAI.cpp:626`). Folge: etwa −25 % Casts bei Sofortzaubern (1,5 s → 2,0 s), −17 % bei 2,5-s-Zaubern, −33 % bei Fähigkeiten mit 1,0-s-GCD (Rogue/Katze). Caster sind am stärksten betroffen. Vorschlag: nach Zaubern nur eine kleine feste Reaktionszeit (etwa 50–100 ms) statt des vollen `reactDelay` aufschlagen, damit die CPU-Last ausserhalb des Kampfs gleich bleibt. Schnelltest ohne Code: `AiPlayerbot.ReactDelay = 100` in der Server-Config und Recount am selben Boss vergleichen. Prüfen: DPS vorher/nachher, keine abgebrochenen Casts durch zu frühe Folgeaktion, Server-CPU mit vielen Bots.
- [ ] **Warrior-Talentlinks in der TBC-Config korrigieren.** In `aiplayerbot.conf.dist.in.tbc:562–570` schreiben die Specs 1.6 bis 1.9 ihre Links alle auf `AiPlayerbot.PremadeSpecLink.1.5.70`. Dadurch wird "pvp prot" (1.5) durch "arms swords" überschrieben, und "fury slam", "arms poleaxes", "arms maces" und "arms swords" haben keinen eigenen Level-70-Link. Schlüssel auf `1.6.70` bis `1.9.70` ändern. Prüfen: wie der Code einen fehlenden Link behandelt, und Talentverteilung von Warrior-Bots nach dem Fix.
- [ ] **Ausrüstung der Bots stichprobenartig prüfen.** Gems (`InitGems`), Verzauberungen (`minEnchantingBotLevel = 60`) und Flasks/Elixiere/Tränke sind im Code vorhanden. Per Inspect prüfen, ob die Bots wirklich volles T5/T6 mit Gems und Verzauberungen tragen und im Kampf Consumables nutzen.

Alle genannten Quellpfade sind relativ zu `src/modules/PlayerBots/playerbot/`, sofern nicht anders angegeben. Zeilennummern beziehen sich auf den Prüfstand und können sich bei späteren Änderungen verschieben.

## Weitere Prüfung

- [ ] Bossstrategien erneut zusammenhängend auf Trigger, Multiplikatoren, Reaktionsaktionen, Zielwahl und Bewegungsregeln prüfen; bestätigte zusätzliche Befunde hier ergänzen.
