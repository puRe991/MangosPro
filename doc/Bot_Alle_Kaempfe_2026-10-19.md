# Prüfung aller Kämpfe der letzten Raid-Aufzeichnung

Quelle: sieben Dateien `build/bin/x64_Release/logs/raid-ai-1792392027-397650-{0..6}.log`, 19.10.2026, 08:40:27–10:56:53. Dies ist die neueste vorhandene Raid-Aufzeichnung. Vollständig eingelesen; 26 Kampfsegmente, darunter 13 Boss-Kampfsegmente mit zwölf unterschiedlichen Begegnungen (Magtheridon zweimal). Segmente werden nach Map/Instanz und mehr als 30 Sekunden ohne protokolliertes `combat=1` getrennt. Sie sind keine exakte Pull-/Kill-Erkennung.

Todeszahlen zählen beobachtete Übergänge `alive=1` zu `alive=0` bei Bots, auch wiederholte Tode nach Wiederbelebung. Sie zählen weder unterschiedliche Spieler noch sämtliche menschlichen Teilnehmer. Entscheidungen sind gedrosselte Diagnosen; EXECUTED bestätigt keinen erreichten sicheren Ort. Das Log enthält keine vollständigen Schadensursachen. Boss-HP 0 alleine beweist keinen regulären Kill.

## Zentraler bestätigter Fehler: Karazhan-Bosssuche

`KarazhanValues.cpp`, `KarazhanNearbyCreaturesValue::Calculate`, übergab `120` zusammen mit `DIST_CALC_NONE` an `AnyUnitFulfillingConditionInRangeCheck`. In diesem Core liefert `WorldObject::GetDistance(..., DIST_CALC_NONE)` die **quadrierte** räumliche Entfernung. Der Checker vergleicht sie direkt mit seinem Radiusparameter. Damit war die tatsächliche Reichweite sqrt(120), also ungefähr **10,95 Yards**, statt 120 Yards.

`GetKaraUnit` hängt für Bosse ausschließlich an dieser Liste. Der direkte Such-Rückfall gilt bislang nur für Portale, Leerezonen und Infernals. Verlässt der Boss die kleine Suchzone, verlieren Fluchttrigger, Gefahrenprüfung und die daran gebundene Cast-Sperre ihren Boss. Der Ein-Sekunden-Cache erklärt, warum Abbrüche leicht oberhalb der Grenze auftreten. Fernkämpfer außerhalb der Zone erkennen die Mechanik zum Teil von Anfang an nicht.

Die Wirkung reicht über Malchezaar hinaus: Aran-Flucht und Flammenkranz, Wolf-Kiten, Maiden-Fernkampfpositionen, Netherspite-Strahlen sowie weitere Karazhan-Aktionen verwenden diese Suche. Der Codefehler ist bestätigt; welcher Anteil jedes einzelnen Todes auf ihn entfällt, ist ohne Schadenslog nicht beweisbar.

Korrektur: Suchradius auf die in der gemeinsamen Raidsuche üblichen **150 Yards** angeglichen; der Distanz-Checker erhält `range * range`, die räumliche Grid-Suche weiterhin `range`. Keine Erhöhung von Schadensradien, Zauberreichweiten oder Aggroreichweiten.

Übergreifende Prüfung: `raidboss::FindUnit` verwendet Angreiferliste plus `FindUnits`, das über `TbcRaidHelper::FindCreatures` und `AllCreaturesOfEntryInRangeCheck` korrekt in Yards sucht. Standardradius 150 Yards; einzelne Begegnungen haben größere explizite Radien. Gruul, Maulgar, Magtheridon und die darauf aufbauenden Raid-Helfer haben diesen Karazhan-Einheitenfehler nicht. Ein pauschales Vergrößern ihrer Gefahren- und Add-Radien ist daraus nicht begründet.

## Bosskämpfe

| Begegnung | Kampfsegment | Bot-Tode | Befund |
|---|---|---:|---|
| Maulgar/Rat | 08:44:31–08:50:27 | 120 | Sehr viele wiederholte Tode; unnötige Ausweichreaktionen und Cast-Unterbrechungen. Bekannte Ursache und Korrekturen in `Bot_Maulgar_Findings.md`; kein Karazhan-Suchfehler. |
| Gruul | 09:02:04–09:07:01 | 61 | Shatter-Todesgruppen und zusätzliche einzelne Tode. Bekannte Verteil-/Phasenprobleme; Offtank/Hurtful-Strike-Verdacht bleibt bestehen. Details in `Bot_Gruul_Findings.md`. |
| Magtheridon, Versuch 1 | 09:18:01–09:19:56 | 54 | Viele Tode schon während der Kanalisierer. In den Sekundenbildern maximal vier gleichzeitig beobachtete Bot-Würfelkanäle; keine Shadow-Cage-Aura am beobachteten Ziel. |
| Magtheridon, Versuch 2 | 09:27:53–09:29:19 | 19 | Maximal drei gleichzeitig beobachtete Bot-Würfelkanäle; wieder zahlreiche Tode. Zwei Tode bis 09:29:30 werden dem auslaufenden Segment zugeordnet. |
| Attumen/Midnight | 09:42:57–09:47:14 | 0 | Beide Phasen bzw. berittener Attumen als Ziel erfasst, keine beobachteten Bot-Tode. Kein Beweis, dass jede Einzelmechanik korrekt läuft. |
| Moroes | 09:55:53–09:59:02 | 0 | Boss und vier Gäste als Ziele erfasst; Zielmarkierung aktiv. Keine beobachteten Bot-Tode. |
| Maid der Tugend | 10:02:51–10:05:42 | 3 | Sailan 10:04:23, Darlina 10:05:05, Derick 10:05:34; jeweils schon geringe HP. Nur sieben ausgeführte Fernkampfpositionierungen. Suchfehler betrifft Fernkampftrigger; konkrete Todesursachen offen. |
| Großer böser Wolf | 10:11:30–10:14:25 | 4 | Drei Tode mit Rotkäppchen-Auren: Domenelin, Darlina, Sailan. Letzte Zustandsbilder stehen mit `moving=0`. Dericks früher Tod hat noch ein Trashziel und wird nicht sicher dem Wolf zugeordnet. |
| Kurator | 10:20:43–10:23:14 | 1 | Sangon stirbt 10:22:36 bei Astral-Flare-Ziel, vorher 10,24% HP. Flare-Zielwechsel und acht erfolgreiche Markierungen vorhanden. Todesursache offen. |
| Terestian | 10:27:51–10:31:13 | 0 | Dämonenketten, Kil'rek und Boss werden angegriffen; Ketten bis 0 HP erfasst. Gegenüber dem früheren fehlenden Kettenfokus ein positives Signal. |
| Aran | 10:32:31–10:36:04 | 3 | Galexandro, Derick und Sosho sterben gleichzeitig 10:33:52 bei aktiver Arkaner Explosion, letzter Abstand 10,51–11,16 Yards. Flucht bricht an der Suchgrenze ab. |
| Netherspite | 10:45:04–10:46:32 | 3 | Rot auf Lengocow und Grün auf Sangon nachgewiesen; kein blauer Bot-Strahlbuff und keine ausgeführte blaue Strahlaktion. Lengocow stirbt mit 28 roten Stapeln. Fernkampf-Bosserkennung ist durch den Suchfehler betroffen. |
| Malchezaar | 10:51:22–10:53:34 | 15 | Zwölf Tode in drei Nova-Wellen mit aktivem Enfeeble. Zu kurze/fehlende Flucht und erneutes Zaubern belegt; drei weitere Tode nicht sicher Nova zuzuordnen. |

## Präzise Belege für Fluchtabbrüche

Alle folgenden Zeilen stammen aus `raid-ai-1792392027-397650-6.log`:

- Malchezaar, Lengocow: 10:51:54 Flucht bei 9,81 Yards EXECUTED (Zeile 98194), anschließend bei 11,62 Yards FAILED und `moving=0` (98219). Tod 10:51:59 mit Enfeeble und Nova als Zielzauber (98584).
- Galexandro bleibt während derselben Enfeeble-Welle bei 27,74 Yards stehen und zaubert weiter; STATE mit Enfeeble, laufendem Frostbolt und Nova 10:51:59 (98548). Sosho steht bei 24,39, Domenelin bei 21,19 Yards. Alle drei sterben in derselben Sekunde.
- Nova-Todeswellen: 10:51:59 fünf, 10:52:29 drei, 10:52:59 vier Bot-Tode mit Enfeeble. Die protokollierten letzten Abstände liegen zwischen 11,12 und 27,74 Yards. Der Flucht-Zielabstand im Code ist 36 Yards.
- Aran, Domenelin: 10:33:49 Flucht bei 10,99 Yards FAILED (9117), danach Arkanschlag EXECUTED an unveränderter Position (9158), obwohl die Explosion noch läuft. Die drei tatsächlichen Todesfälle haben letzte Zustandsbilder um 10:33:51 bei rund elf Yards und Zielzauber 29973 (9428–9470). Der Flucht-Zielabstand im Code ist 26 Yards.
- Wolf, Domenelin: Rotkäppchen ist ab 10:12:04 erfasst, aber zunächst keine Bewegung bei 25,95 Yards. Erste erfolgreiche Flucht erst 10:12:08, als der Wolf bereits bis auf 2,50 Yards herangekommen ist (`...-5.log:51547`). Letztes Zustandsbild vor dem Tod: 10:12:11, stehend, 11,02 Yards, Rotkäppchen noch aktiv. Der Trigger benötigt ausdrücklich `GetKaraUnit(wolf)` und ist somit ebenfalls von der Suchgrenze abhängig.

Die angezeigte `distance` ist der Diagnosewert; die Suche prüft dreidimensional und wird gecacht. Eine exakte Gleichheit aller Abbruchwerte mit sqrt(120) ist daher nicht zu erwarten.

## Netherspite und Magtheridon: Grenzen der Beobachtung

Netherspite: 51 Zustandsbilder mit rotem Buff auf Lengocow, 63 mit grünem Buff auf Sangon, keine mit blauem Buff auf einem Bot. Dies erfasst keine menschlichen Strahlspieler. Neben dem Suchfehler bleiben Rollenwechsel und tatsächliche Strahlpositionen nach dessen Korrektur zu prüfen. Letzte lebende Zustandsbilder bei Galexandro/Darlina liegen neun bzw. acht Sekunden vor dem gemeldeten Tod; eine genaue Schadenszuordnung wäre spekulativ.

Magtheridon: Bot-Kanäle 30410 werden im ersten Versuch 09:19:25–30 bei Darlina, Onise, Saewash und Shohkak erfasst; im zweiten 09:29:06–12 bei Domenelin, Rohander und Shohkak. Das sind gleichzeitige Sekundenbilder, keine vollständige Messung aller menschlichen und Bot-Klicker. Keine beobachtete Aura 30168 bestätigt einen erfolgreichen Fünfer-Käfig. In beiden Versuchen wird überwiegend Kanalisierer Entry 17257 angegriffen; Boss Entry 17256 muss davon getrennt betrachtet werden. Bereits vorhandene Magtheridon-Korrekturen sind separat im aktuellen Build enthalten; die Morgenlogs validieren diese späteren Änderungen nicht.

## Alle weiteren Kampfsegmente

| Zeit | Einordnung / Ziel-Entries | Bot-Tode |
|---|---|---:|
| 08:40:49–08:43:21 | Gruul-Trash 21350; fälschliche Maulgar-Reaktionen, siehe Maulgar-Befund | 1 |
| 09:40:23–09:40:36 | Karazhan 15548 | 0 |
| 09:42:17 | 14881 | 0 |
| 09:48:09–09:48:11 | 16407, totes Attumen-Ziel noch erfasst | 0 |
| 09:50:05–09:52:07 | 16415, 16407, 16409 | 0 |
| 09:59:55–10:01:13 | 16407, 16409, 16424 | 0 |
| 10:01:47–10:02:11 | 16407 | 0 |
| 10:07:35–10:08:16 | 16460/16459; Domenelin hat selbst Aggro auf 16459 beim letzten Bild | 1 |
| 10:09:26–10:09:28 | 16471; Domenelin erneut selbst Aggro, nur noch 4,25% HP | 1 |
| 10:10:10 | Einzelnes Kampfereignis mit Entry 4075; keine belastbare Bossaussage | 0 |
| 10:17:08–10:17:11 | 16482; sehr kurzer Ausschnitt | 0 |
| 10:25:14–10:26:54 | 16525, 16530, 16504; Galexandro stirbt ohne aussagekräftiges Ziel im letzten Bild | 1 |
| 10:43:35 | Einzelnes Ereignis 16525 | 0 |

Keine aktuellen Kämpfe gegen Nightbane, andere Opernvarianten, SSC/TK oder Zul'Aman in dieser Sitzung. Frühere Tageslogs sind nicht Bestandteil dieser erneuten Prüfung der letzten Kämpfe.

## Prüfung und Status

- `build/test_karazhan_search.py`: kompiliert extrahierte Produktionsfunktionen für Core-Distanz, Grid-Prädikat und vollständige Karazhan-Suchmethode. Tests bei 12/26/36/120/150 Yards, außerhalb des Radius, mit Höhenunterschied, toten/unpassenden Einheiten und falscher Map bestanden. Der Grid-Test prüft zusätzlich, dass der Traversierungsradius nicht quadriert wird.
- Auswertung reproduzierbar mit `build/analyze_all_latest.py`, `build/summarize_all_latest.py` und `build/detail_latest.py`. JSON-Ergebnisse und genaue Quellzeilen liegen in `build/all-latest-analysis.json` und `build/latest-detail.json`.
- Priorität: korrigierte Bosssuche im nächsten Karazhan-Test gegen Malchezaar, Aran, Wolf und Netherspite überprüfen; danach verbleibende Pfad-/Tempo-, Strahlwechsel- und Heilprobleme isolieren. Arans protokollierter Fluchtfortschritt ist teilweise langsam; das wird durch die Suchkorrektur allein nicht abschließend erklärt.
- Keine Serverinstallation oder Neustart im Rahmen dieser Prüfung. Build-Status wird nach Abschluss ergänzt.
