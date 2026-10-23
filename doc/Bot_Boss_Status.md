# Bot-Raids: Status pro Boss

Stand: 21.10.2026. Einstieg in die Einzelbefunde; Details stehen in den verlinkten Dateien.
"Fix im Code" heißt gebaut bzw. committet, nicht durch einen Kill bestätigt. Nach jedem
Raid-Test die Spalte "Nachweis" aktualisieren.

Statuswerte: **offen** (Befund ohne Korrektur), **Fix im Code** (ungetestet im Kampf),
**getestet** (Mechanik im Log sichtbar korrekt), **Kill** (Boss mit Bots gelegt).

## Karazhan

| Boss | Status | Nachweis / Notiz | Doku |
|---|---|---|---|
| Attumen | Fix im Code | Tank-Aggro (Zorn der Gerechtigkeit im Kampf) | [Bot_Attumen_Fix.md](Bot_Attumen_Fix.md) |
| Moroes | Fix im Code | Heilzielwahl, Jägerdistanz | [Bot_Moroes_Maiden_Fix.md](Bot_Moroes_Maiden_Fix.md) |
| Maiden | Fix im Code | wie Moroes | [Bot_Moroes_Maiden_Fix.md](Bot_Moroes_Maiden_Fix.md) |
| Terestian | Fix im Code | Ketten/Kil'rek-Fokus im Folgepaket | [Bot_Terestian_Findings.md](Bot_Terestian_Findings.md), [Bot_Raid_Followup_Fixes.md](Bot_Raid_Followup_Fixes.md) |
| Aran | Fix im Code | Flammenkranz, 26 yd bei Arkaner Explosion | [Bot_Aran_Findings.md](Bot_Aran_Findings.md) |
| Netherspite | offen | kein Kill, Boss heilt nach Verbannung | [Bot_Netherspite_Findings.md](Bot_Netherspite_Findings.md) |
| Malchezaar | Fix im Code | Infernal-Pfade, Enfeeble/Nova-Abstand | [Bot_Malchezaar_Findings.md](Bot_Malchezaar_Findings.md) |
| Bosssuche allgemein | Fix im Code | Suchradius `DIST_CALC_NONE` | [Bot_Alle_Kaempfe_2026-10-19.md](Bot_Alle_Kaempfe_2026-10-19.md) |

## Gruul's Lair / Magtheridon

| Boss | Status | Nachweis / Notiz | Doku |
|---|---|---|---|
| Maulgar | Fix im Code | Trash-Aktivierung, Krosh/Wirbelwind; neu: Ersatz-Tank für Olm/Blindeye bei nur 2 Tanks (`standin_tanks_test`) | [Bot_Maulgar_Findings.md](Bot_Maulgar_Findings.md) |
| Gruul | Fix im Code | Shatter-Richtung, Fernkampf-Verteilung; Tank-Drag per `raid_positioning_test` | [Bot_Gruul_Findings.md](Bot_Gruul_Findings.md) |
| Magtheridon | Fix im Code | Würfel nur bei Bedarf, Knockback-Decke; max. 3 gleichzeitige Klicker im letzten Log | [Bot_Magtheridon_Findings.md](Bot_Magtheridon_Findings.md) |

## SSC / Tempest Keep / Hyjal / BT / ZA

| Boss | Status | Nachweis / Notiz | Doku |
|---|---|---|---|
| Hydross | Fix im Code | Tankwechsel/DPS-Pause per `ssc_hyjal_regressions_test` | – |
| Leotheras | offen | Boss nur bis ca. 92 %, viele Tode | [Bot_Leotheras_Findings.md](Bot_Leotheras_Findings.md) |
| Karathress | Fix im Code | Main-Tank hält überzählige Adds bei < 4 Tanks | – |
| Vashj | Fix im Code | Kampfrausch-Schwelle per Test | – |
| Al'ar | Fix im Code | Core: Flug-Watchdog, sichere Wiedergeburt | – |
| Akil'zon | Fix im Code | Core: Sturmschutz für Sturmziel | [Bot_Akilzon_Findings.md](Bot_Akilzon_Findings.md) |
| Hyjal, BT | Fix im Code | Encounter-Korrekturen ohne eigenen Befundbericht | – |

## Bekannte Grenzen

- Die Raidgruppe hat meist nur 2 Tanks. Ersatz-Tanks gibt es bisher nur bei Maulgar
  (`raidboss::GetGroupAssistTankOrStandIn`); BT-Council, Lurker-Wächter und Karathress
  nutzen ihn noch nicht.
- Offline-Tests: `perl contrib/tests/run_tests.pl` (MSVC nötig, kein Python).
