# Prüfung der Bot-Kampfrotationen – 20.10.2026

## Ergebnis und Umfang

**Nein: Der aktuelle Quellstand gewährleistet nicht für alle Bots maximale DPS, wirksame Heilung und Tank-Bedrohung.** Mehrere konkrete Prioritäts- und Auslöserprobleme sind bereits im Code erkennbar.

Geprüft wurden die TBC-Klassenstrategien der neun Klassen unter `src/modules/PlayerBots/playerbot/strategy`, einschließlich zentraler Auslöser, relevanter Zauberaktionen, Ausweichaktionen und der Strategieauswahl in `AiFactory.cpp`. Die Build-Konfiguration aktiviert `BUILD_PLAYERBOTS` und deaktiviert `BUILD_DEPRECATED_PLAYERBOT`; die alte KI unter `src/game/PlayerBot` ist deshalb nicht Gegenstand des Berichts.

Dies ist eine statische Prüfung des aktuellen Arbeitsverzeichnisses, einschließlich bereits vorhandener lokaler Änderungen. Kein Ingame-Benchmark, keine vollständige Simulation aller Talente/Ausrüstungen und kein Nachweis, dass der laufende Server exakt diesen Quellstand verwendet. Es wurden keine Rotationen geändert und keine neuen Laufzeittests durchgeführt. Prozentuale DPS-/HPS-/TPS-Verluste lassen sich daraus nicht seriös bestimmen.

## Bestätigte Befunde

### 1. Waffenkrieger: fehlende TBC-Slam-/Wirbelwind-Einbindung

`warrior/ArmsWarriorStrategy.cpp:401`: Die TBC-Kernrotation verwendet Execute, Overpower, Heroic Strike, Mortal Strike und Rend. Slam fehlt; auch Wirbelwind wird dort nicht eingeplant. Die TBC-AoE-Erweiterung ruft nur die gemeinsame Krieger-AoE-Strategie auf, die Cleave und Thunder Clap hinzufügt. Eine registrierte Wirbelwind-Ausweichaktion allein erzeugt keinen Auslöser für die Rotation.

Damit fehlt insbesondere eine auf verbesserte Slam-Talente und den Waffen-Schwung abgestimmte Rotation. Eine Optimierung muss Talentwahl, Waffenart, Wut und Schwungzeit berücksichtigen.

### 2. Bär: Zerfleischen hinter einer pauschalen 40-Wut-Schwelle

`druid/TankFeralDruidStrategy.cpp:524` verknüpft Mangle (Bear) mit `medium rage available`. Dieser Auslöser verlangt 40 Wut (`triggers/GenericTriggers.h:35`).

Unterhalb dieser Schwelle wird die Aktion nicht durch diesen Rotationszweig eingeplant, auch wenn der konkrete Zauber bereits bezahlbar und bereit wäre. Lacerate besitzt zusätzlich eine höhere Priorität. Maul existiert als Ausweichaktion von Mangle, aber nicht als unabhängig gesteuerter Wutüberschuss-Verbrauch in der TBC-Kernrotation.

Ziel einer Korrektur: tatsächliche Zauberkosten und Bereitschaft prüfen, Mangle gezielt priorisieren und Maul mit einer Ressourcenreserve für die Kernfähigkeiten steuern.

### 3. Bär: Aufschlitzen wird bei vollen Stapeln nicht rechtzeitig erneuert

`druid/DruidTriggers.h:247`: `LacerateTrigger` prüft `!ai->HasAura("lacerate", target, true)`. Der dritte Parameter bedeutet **maximale Stapel**, nicht lediglich „Aura vorhanden“; das wurde gegen `PlayerbotAI::HasAura` geprüft.

Der Stapelaufbau funktioniert damit grundsätzlich. Bei maximalen Stapeln fehlt jedoch eine Prüfung der Restdauer. Der Auslöser wird erst nach Verlust der vollen Stapel wieder aktiv, statt die Aura kurz vor Ablauf zu erneuern. Das gefährdet die durchgehende Aufrechterhaltung des Stapels.

### 4. Kampfschurke: Ausweiden verdrängt Blutung

`rogue/CombatRogueStrategy.cpp:429`: Eviscerate hat `ACTION_HIGH + 1`, Rupture lediglich `ACTION_HIGH`. Beide Auslöser verwenden standardmäßig vier Combopunkte (`rogue/RogueTriggers.h:26`).

Wenn Slice and Dice läuft, Rupture fehlt und beide Finisher ausführbar sind, wird Eviscerate höher bewertet und kann die Combopunkte verbrauchen. Eine Auswahl nach verbleibender Lebensdauer des Ziels oder nach geeignetem Blutungsfenster fehlt in diesen Auslösern. Das ist keine gezielte Rupture-Aufrechterhaltung.

Korrektur nicht durch pauschales Umdrehen für alle Gegner: Rupture nur bei geeigneten, ausreichend langlebigen Zielen bevorzugen; ansonsten direkter Finisher.

### 5. Wiederherstellungsdruide: Nachwachsen vor verfügbarer Rascher Heilung

`druid/RestorationDruidStrategy.cpp:492`: Bei kritischer Gesundheit wird Regrowth mit `ACTION_CRITICAL_HEAL + 1`, Swiftmend mit `ACTION_CRITICAL_HEAL` eingeplant. Dasselbe gilt für Gruppenmitglieder.

Konkreter Problemfall: Auf dem kritischen Ziel liegt Rejuvenation, aber kein eigener Regrowth-HoT; beide Zauber sind ausführbar. Die Rotation bevorzugt den Zauber mit Wirkzeit vor der verfügbaren Sofortheilung. `CastSwiftmendAction` und die Gruppenvariante prüfen bereits, ob ein passender HoT vorhanden ist.

Die Aura-Sperre der Regrowth-Aktion bedeutet, dass dies nicht bei jedem kritischen Ziel passiert. Die Prioritätslücke besteht aber im genannten Fall. Lifebloom auf Tanks ist dagegen vorhanden und berücksichtigt Stapel sowie Restdauer; es fehlt nicht grundsätzlich.

### 6. Elementar-Schamane: Totems berücksichtigen die Gruppenbesetzung nicht

`shaman/ElementalShamanStrategy.cpp:752`: Windfury Totem steht vor Wrath of Air Totem.

`shaman/ShamanActions.h:286`: Windfury wird bereits durch die bloße Mitgliedschaft in einer Gruppe als sinnvoll betrachtet. Wrath of Air wird bei Gruppenmitgliedschaft ohne explizite Strategie `totem air wrath` ausgeschlossen (`ShamanActions.h:233`).

Damit erhält auch eine reine Zauberergruppe standardmäßig diese Nahkampf-orientierte Lufttotemwahl. Eine rollenabhängige Gruppenprüfung fehlt hier. Explizite Totemstrategien können das Verhalten ändern.

### 7. Katze: Rip schon ab drei Combopunkten

`druid/DruidTriggers.h`, `RipTrigger`: Der Standardwert beträgt drei Combopunkte. `DpsFeralDruidStrategy` priorisiert Rip vor Ferocious Bite und den Combopunkt-Erzeugern.

Bei fehlendem Rip wird somit bereits ab drei Punkten ein Finisher angeboten, statt für dieses Ziel bewusst zwischen einem frühen Finisher und einem stärkeren Fünf-Punkte-Finisher zu entscheiden. Ferocious Bite verlangt dagegen fünf Punkte. Die bestehende Logik ist eine einfache Schwellenrotation, keine Optimierung nach Energie, Restlaufzeiten und verbleibender Kampfdauer.

### 8. Zerstörungshexer: ein fester Füllzauber für unterschiedliche Builds

Im TBC-Zweig von `warlock/DestructionWarlockStrategy.cpp` ist Shadow Bolt die Standardaktion. Incinerate wird im Kernzweig über Backlash angeboten; Immolate und Conflagrate sind eingebunden. Eine buildabhängige Auswahl des normalen Füllzaubers ist dort nicht enthalten.

Das ist kein Beleg, dass Shadow Bolt allgemein falsch wäre. Es bedeutet aber, dass eine Rotation für unterschiedliche Schatten-/Feuer-Builds und Ausrüstung nicht automatisch jeweils maximalen Schaden liefern kann. Conflagrate darf zudem bereits bei bis zu sieben Sekunden verbleibendem Immolate auslösen (`WarlockTriggers.cpp`); der Verlust verbleibender DoT-Ticks wird dort nicht abgewogen.

## Einordnung aller Klassen

| Klasse / Rollen | Ergebnis der Sichtung |
|---|---|
| Krieger – Waffen, Furor, Schutz | Waffen-Lücke siehe oben. Furor besitzt Bloodthirst, Whirlwind, Execute und Heroic Strike. Schutz besitzt Shield Slam, Revenge, Devastate, Shield Block und Taunt; Wutreserven und Bedrohungsaufbau sind damit noch nicht als optimal nachgewiesen. |
| Druide – Katze, Bär, Gleichgewicht, Wiederherstellung | Konkrete Probleme bei Bär, Katze und Notfallheilung. Gleichgewicht besitzt Starfire, Moonfire, Insect Swarm und Faerie Fire; keine Bestätigung eines Optimums für jede Kampfdauer. |
| Schurke – Kampf, Meucheln, Täuschung | Kernfähigkeiten und Finisher vorhanden; bestätigter Prioritätskonflikt beim Kampfschurken. Gemeinsame Combopunkt-/Buff-Schwellen sind keine situationsabhängige Finisherplanung. |
| Schamane – Elementar, Verstärkung, Wiederherstellung | Lightning Bolt/Chain Lightning, Stormstrike/Shocks sowie direkte Heilung/Chain Heal sind vorhanden. Totemwahl ist ein bestätigtes Problem; Heilprioritäten allein belegen keine optimale Gruppenheilung. |
| Paladin – Vergeltung, Schutz, Heilig | Crusader Strike/Judgement/Seal, Holy Shield/Consecration/Righteous Defense sowie Holy Light/Flash of Light/Holy Shock vorhanden. Bestehende lokale Schutz-Anpassungen halten Righteous Fury auch im Kampf aufrecht. Seal of Blood ist über eine Ausweichkette berücksichtigt. |
| Priester – Schatten, Heilig, Disziplin | Schatten besitzt Vampiric Touch, Shadow Word: Pain, Mind Blast und Mind Flay. Heilzweige und AoE-Heilstrategien vorhanden; die tatsächliche Leistung hängt von Zielwahl und aktivierten Strategien ab. |
| Magier – Arkan, Feuer, Frost | Kernzauber vorhanden. Arkan berücksichtigt Stapel und einen Mana-Schwellwert; das ist noch keine vorausschauende Mana-/Burstplanung. Feuer und Frost besitzen ihre jeweiligen Füllzauber und Zusatzaktionen. |
| Hexenmeister – Gebrechen, Dämonologie, Zerstörung | DoTs, Shadow Bolt und Pet-/Curse-Erweiterungen vorhanden. Zerstörungs-Füllzauber ist nicht buildabhängig. |
| Jäger – Tierherrschaft, Treffsicherheit, Überleben | Gemeinsame Rotation besitzt Auto Shot, Steady Shot, Kill Command und Arcane Shot. Eine vorhandene TBC-Zeitprüfung lässt Steady Shot nur in ein passendes Auto-Shot-Fenster starten. Spezialfähigkeiten hängen zusätzlich an den jeweiligen Erweiterungen; maximale Leistung ist nicht gemessen. |

`AiFactory.cpp` aktiviert für die Klassen üblicherweise auch `aoe`, `buff`, `boost` und weitere rollenspezifische Erweiterungen. Nur die Kernrotation zu lesen und daraus fehlende Cooldowns/Pets/Totems abzuleiten wäre deshalb unzureichend. Individuelle Strategieänderungen können die tatsächlich aktive Rotation verändern.

## Sinnvolle Reihenfolge

1. Bärenrotation: Mangle-Auslöser, Lacerate-Erneuerung, unabhängige Maul-Steuerung.
2. Druiden-Notfallheilung: verfügbare Sofortheilung vor langsamerem Aufbau.
3. Waffenkrieger und Kampfschurke: talentgerechte Rotation und eindeutige Finisherentscheidung.
4. Gruppenabhängige Totems.
5. Build- und kampfdauerabhängige Feinabstimmung für übrige DPS-/Heilrotationen.

Für einen belastbaren Vorher-/Nachher-Nachweis dieselben Bots mit gleicher Ausrüstung, Talenten, Buffs und Ressourcenbedingungen vergleichen: längere Einzelzielkämpfe, mehrere Gegner und definierter Gruppenschaden. Dabei neben Schaden und Bedrohung auch Buff-/DoT-Lücken, ungenutzte Cooldowns, Wut-/Energie-/Mana-Leerlauf, effektive Heilung, Überheilung und Todesfälle erfassen. Höhere rohe HPS allein bedeutet keine bessere Heilung; höhere DPS unter Missachtung der Bossmechanik bedeutet keine bessere Raidleistung.
