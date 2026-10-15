# Bots: Rotationen, Raid-Buffs, Ausrüstung und Bedrohung

Ergänzungen von MangosPro am Playerbots-Modul (nur TBC, `MANGOSBOT_ONE`).

## Chat-Befehle

Per Flüstern an einen Bot oder im Gruppen-/Schlachtzugschat an alle Bots.

| Befehl | Wirkung |
|--------|---------|
| `lust` | Ein bereiter Schamane zündet sofort Kampfrausch / Heldentum (nur einer, die anderen heben ihren Cooldown auf) |
| `lust hold` | Schamanen heben Kampfrausch / Heldentum für den Befehl `lust` auf |
| `lust auto` | Schamanen nutzen Kampfrausch / Heldentum wieder selbst (Boss-Timing, siehe unten) |
| `lust ?` | Schamane nennt Modus und Cooldown |
| `auras` | Jeder Paladin nennt seine Aura und welche Segen er wie oft aufrechterhält |
| `auras auto` | Die Paladine jeder Untergruppe verteilen die Auren untereinander |
| `auras off` | Zurück zur Standard-Aura der Skillung |
| `nc +aura devotion` | Paladin hält fest eine Aura (`devotion`, `retribution`, `concentration`, `sanctity`, `shadow`, `fire`, `frost`, `crusader`) |
| `roll feedback` | Bot begründet seine Würfe im Chat (inkl. „upgrade too small“ / „X needs it more“) |
| `tactic` | Ein Bot nennt die Taktik des Bosses in der Nähe: Kill-Reihenfolge mit Markern, Tank-Zuweisung, Unterbrecher, Kampfrausch, Aura (und ob die Strategie `tbc raid` aktiv ist) |
| `tactic ?` | Jeder Bot nennt seine eigene Aufgabe (Rolle, gehaltener Gegner, Unterbrecher, aktuelles Ziel) |

## Boss-Taktiken: Zuweisungen, Positionen, Marker

Beim Pull eines Bosses aus der Raid-Tabelle schreibt ein Bot (niedrigste GUID) automatisch in den Schlachtzugchat:
`[Taktik] <Boss> geladen. Reihenfolge: ... Tanks: ... Unterbrecher: ...` – fehlt diese Zeile, ist keine Taktik aktiv.

Bausteine in `TbcRaidEncounters.cpp`:

| Baustein | Wirkung |
|----------|---------|
| `.TankAt(entry, slot, vor, seite)` | Fester Tank für einen Gegner (Slot 0 = Haupttank, 1 = erster Offtank …; mit weniger Tanks übernimmt der letzte Offtank den Rest). Der Tank spottet ihn, wenn er loskommt, und zieht ihn an seinen Platz (relativ zum Spawnpunkt und der Blickrichtung, an Wänden automatisch verkürzt) |
| `.ClassTankAt(entry, Klasse, vor, seite, Aura)` | Gegner wird von einer Klasse gehalten (Magier auf Krosh), optional Zauberraub der Aura |
| `.Interrupt(entry, {Zauber}, Anzahl)` | Feste Unterbrecher (Tritt, Zuschlagen, Schildhieb, Erdschock, Gegenzauber, Unterdrückungsschuss, Stille) |
| `.Mark(entry, Symbol)` | Ein Bot setzt die Schlachtzugsymbole, damit Spieler der Reihenfolge folgen können |

### High King Maulgar

| Ogre | Marker | Gehalten von | Platz |
|------|--------|--------------|-------|
| Blindeye | Totenkopf (1.) | Offtank 1, 2 feste Unterbrecher auf Heilung / Gebet der Heilung | neben Maulgar, Richtung Rückwand |
| Olm | Kreuz (2.) | Offtank 1 | wie Blindeye |
| Kiggler | Viereck (3.) | Offtank 2 (mit 2 Tanks: Offtank 1) | Westende |
| Krosh | Mond (4.) | Magier (Zauberraub auf Zauberschild), ohne Magier der letzte Offtank | Ostende |
| Maulgar | Dreieck (5.) | Haupttank | zur Rückwand gedreht |

### Gruul

Haupttank hält Gruul an seinem Spawnpunkt (kein Wandern in den Fernkampf), der Offtank steht auf dem Haupttank
und bleibt Zweiter auf der Bedrohungsliste (fängt Hurtful Strike), Totenkopf auf Gruul. Fernkampf 10 m verteilt,
nach Ground Slam alle 15 m auseinander (Shatter), raus aus Cave In.

### Magtheridon

| Phase | Ablauf |
|-------|--------|
| 1 | Die 5 Kanalisierer werden reihum auf die Tanks verteilt und dort gehalten, wo sie stehen. Marker Totenkopf, Kreuz, Viereck, Mond, Dreieck – der Raid tötet sie in dieser Reihenfolge, der Totenkopf wandert immer auf den nächsten. 3 feste Unterbrecher auf Dark Mending / Shadow Bolt Volley |
| 2+ | Haupttank hält Magtheridon in der Mitte (Totenkopf), 5 Bots klicken die Manticron-Würfel bei Blast Nova (Fernkampf zuerst, dann Heiler, dann Nahkampf; Bots mit Mind Exhaustion werden ersetzt) |

Einstellung über Regeln mit `SLOT_ROUND_ROBIN` (Tanks reihum) und mehreren `.Mark(...)` für denselben Gegnertyp.

### Auren-Verteilung (`auras auto`)

Pro Untergruppe, die Paladine in fester Reihenfolge (GUID), jeder bekommt die erste freie Aura, die er kennt:

- Gruppe mit Tank: Devotion, Retribution, Concentration, Sanctity
- Nahkampf-Gruppe: Sanctity, Retribution, Devotion, Concentration
- Caster-/Heiler-Gruppe: Concentration, Devotion, Retribution, Sanctity

Bei Bossen mit Widerstandsaura (Tabelle `TbcRaidEncounters.cpp`, `.ResistanceAura(...)`) übernimmt der erste
Paladin der Untergruppe automatisch diese Aura, auch ohne `auras auto`. Ein Paladin mit fester Aura (`nc +aura ...`)
wird nie umgestellt.

### Kampfrausch / Heldentum

- In Schlachtzügen beim Pull, außer der Boss hat in der Raid-Tabelle `.LustBelow(x)`:
  Lady Vashj unter 49 %, Kael'thas unter 50 %, Illidan unter 30 %, Kil'jaeden unter 25 %.
- Wirkt raidweit, wenn `sql/mangospro/world/01_raidwide_bloodlust_heroism.sql` eingespielt ist.

## Rotationen (TBC)

| Klasse / Skillung | Änderung |
|-------------------|----------|
| Wiederherstellungsschamane | Healing Wave statt Riptide (WotLK) – im Bereich „niedrige Gesundheit“ wurde vorher nicht geheilt |
| Jäger | Steady Shot nur, wenn der Cast vor dem nächsten Auto Shot fertig ist (Auto-Shot-Weaving); PvE/Raid: Serpent Sting vor Viper Sting |
| Wildheitsdruide (Katze) | Mangle-Debuff wird aufrechterhalten (egal ob Katze oder Bär), Rip erst ab 5 Combo-Punkten |
| Meucheln-Schurke | Neu: Envenom (nur mit eigenem Deadly Poison am Ziel, sonst Eviscerate) und Shiv (frischt Deadly Poison kurz vor Ablauf auf); Slice and Dice > Rupture > Envenom > Eviscerate |
| Furor-Krieger (Raid) | Rüstung zerreißen nur ohne Tank-Krieger in der Gruppe, niedrigste Priorität |
| Vergeltungspaladin | Crusader Strike vor Exorcism |
| Schutzpaladin | Consecration in der Einzelziel-Rotation (bei genug Mana) |
| Schattenpriester | Vampirberührung höher, Hex of Weakness niedriger |
| Elementarschamane | Chain Lightning auf Cooldown |
| Verstärkungsschamane | Neu: Shamanistic Rage bei mittlerem Mana |
| Feuermagier | Living Bomb (WotLK) entfernt |
| Zerstörungshexenmeister | Shadow Bolt als Standard, Incinerate bei Backlash |

## Segen

Große Segen wirken in TBC auf die ganze Klasse. Der Paladin wählt den großen Segen jetzt nach der Rolle,
die die meisten Spieler dieser Klasse im Schlachtzug haben (statt nach dem einen Ziel).

## Ausrüstung, Edelsteine, Würfeln

Einstellungen in `aiplayerbot.conf`:

| Einstellung | Standard hier | Wirkung |
|-------------|---------------|---------|
| `AiPlayerbot.minEnchantingBotLevel` | 60 | war 81 – damit wurden Bots in TBC nie verzaubert |
| `AiPlayerbot.AutoEnchantUpgradeLoot` | 1 | neue Ausrüstung wird beim Anlegen verzaubert |
| `AiPlayerbot.AutoGemUpgradeLoot` | 1 | neue Ausrüstung wird beim Anlegen gesockelt |
| `AiPlayerbot.NeedRollMinUpgradePct` | 5 | Bedarf erst ab 5 % Verbesserung, sonst Gier |
| `AiPlayerbot.NeedRollBestUpgradeOnly` | 1 | mit echtem Spieler in der Gruppe würfelt nur der Bot mit der größten Verbesserung Bedarf |
| `AiPlayerbot.AutoExchangeTierTokens` | 1 | Tier-Token werden wie das passende Set-Teil bewertet und nach dem Gewinn sofort getauscht |

### Tier-Token (T4–T6)

`AiPlayerbot.AutoExchangeTierTokens = 1`: Bots bewerten ein Token wie das Set-Teil ihrer Klasse und Skillung,
gegen das es beim Händler getauscht wird (z. B. Schutzpaladin: „Helm of the Fallen Champion“ = „Justicar Faceguard“),
und würfeln danach (Mindestverbesserung und „nur der größte Gewinn“ gelten auch hier). Ein gewonnenes Token
tauschen sie sofort gegen dieses Set-Teil, legen es an, sockeln und verzaubern es. Die Zuordnung Token → Set-Teile
kommt aus den Händlerdaten (`npc_vendor` / `npc_vendor_template`, Tauschkosten aus `ItemExtendedCost.dbc`).
Echte Spieler tauschen ihre Token weiter normal beim Händler.

Edelsteine: bester Stein für die Skillung (Stat-Gewichtung), Sockelbonus nur wenn passende Farben zusammen
mehr bringen als die besten Steine beliebiger Farbe, keine einzigartigen Steine, Juwelenschleifer-Steine nur
mit dem Beruf. Meta-Edelstein nach Gewichtung, sonst nach Rolle (Tank: Powerful, Heiler/Caster: Insightful,
Nahkampf/Jäger: Relentless Earthstorm Diamond). Die Aktivierungsbedingungen von Meta-Steinen werden nicht geprüft.

Stat-Gewichtungen: `sql/mangospro/world/02_tbc_bot_stat_weights.sql` ersetzt die Gewichte aller TBC-Skillungen
(vorher teils WotLK-Werte, ohne Waffen-DPS für Jäger/Schurken/Krieger, Tanks mit Waffen-DPS als Hauptwert).
Danach `sql/mangospro/characters/01_reset_bot_item_caches.sql` einspielen, der nächste Start baut die Caches neu.
Trefferwertung hat keine Obergrenze in der Bewertung.

## Weniger grundloses Weglaufen

- Fernkämpfer/Caster halten keinen Abstand mehr zu NPCs, die jemand anderen angreifen
  (vorher liefen sie im Raid ständig rückwärts, sobald der Tank den Boss bewegte).
  Jäger verlassen dann nur noch den Totbereich (unter 8 m). Greift der NPC den Bot an, bleibt das Ausweichen.
- Bots fliehen nicht mehr aus Bodeneffekten freundlicher Zauberer (Blizzard, Weihe, Feuerregen, Höllenfeuer ...),
  nur noch aus feindlichen.

## Bedrohung der Tanks

`AiPlayerbot.TankThreatMultiplier = 3.0` – multipliziert die Bedrohung von Spielern in Verteidigungshaltung,
Bärengestalt, Terrorbärengestalt oder mit Zorn der Gerechtigkeit (Bots und echte Spieler).
`1.0` schaltet es ab.
