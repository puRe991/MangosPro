<?php
page_title('Progress Race');
$realmId = Realm::selected();
Progress::maybeSync($realmId);

$kills = Progress::kills($realmId);
$ranking = Progress::ranking($kills);
$firsts = Progress::firsts($kills);
$counts = Progress::killCounts($kills);
$totalBosses = array_sum(array_map(fn($r) => count($r['bosses']), Progress::RAIDS));
$firstCount = array_sum(array_map('count', $firsts));

// Realm-Firsts chronologisch (neueste zuerst)
$timeline = [];
foreach ($firsts as $map => $bosses) foreach ($bosses as $b => $k) $timeline[] = $k;
usort($timeline, fn($a, $b) => (int)$b['killed_at'] <=> (int)$a['killed_at']);
$latest = $timeline[0] ?? null;
$season = (string)config('progress.season_start', '');
?>
<section class="page-head progress-head">
    <div class="wrap">
        <p class="eyebrow">Realm-Firsts · Gildenwertung</p>
        <h1>Progress Race</h1>
        <p class="lead">Welche Gilde legt die Bosse der Scherbenwelt zuerst? Kills werden automatisch aus den Raid-IDs erkannt.<?= $season !== '' ? ' Wertung seit ' . e(fmt_date($season)) . '.' : '' ?></p>
        <ul class="hero-stats progress-stats" data-stagger>
            <li><span class="stat-ico"><?= icon('users') ?></span><span class="stat-txt"><strong data-count="<?= count($ranking) ?>"><?= count($ranking) ?></strong><span>Gilden im Rennen</span></span></li>
            <li><span class="stat-ico ok"><?= icon('skull') ?></span><span class="stat-txt"><strong><?= $firstCount ?> / <?= $totalBosses ?></strong><span>Bosse besiegt</span></span></li>
            <li><span class="stat-ico"><?= icon('crown') ?></span><span class="stat-txt"><strong><?= $latest ? e(Progress::bossName((int)$latest['map_id'], (int)$latest['boss_index'])) : '–' ?></strong>
                <span><?= $latest ? 'Letzter Realm-First · ' . e($latest['guild_name']) : 'Noch kein Realm-First' ?></span></span></li>
        </ul>
    </div>
</section>

<section class="wrap section">
    <?php render_partial('realm_tabs', ['route' => 'progress', 'current' => $realmId, 'extra' => []]); ?>

    <div class="section-head" data-reveal>
        <div>
            <p class="eyebrow">Rangliste</p>
            <h2>Gildenwertung</h2>
        </div>
        <span class="muted small">Sortiert nach höchster Raidstufe, bei Gleichstand zählt, wer zuerst dort war.</span>
    </div>
    <?php if ($ranking):
        $bestRaid = function (array $g): ?array {
            foreach (array_reverse(Progress::RAIDS, true) as $map => $raid) if (!empty($g['raids'][$map])) return [$map, $raid, $g['raids'][$map]];
            return null;
        }; ?>
        <div class="podium">
            <?php foreach ([1, 0, 2] as $pos): if (!isset($ranking[$pos])) continue; $g = $ranking[$pos]; $b = $bestRaid($g); ?>
                <div class="podium-slot p<?= $pos + 1 ?>" data-reveal>
                    <span class="place"><?= $pos + 1 ?></span>
                    <?php if ($pos === 0): ?><span class="crown"><?= icon('crown') ?></span><?php endif; ?>
                    <h3><a href="<?= e(url('guild', ['realm' => $realmId, 'id' => $g['guild_id']])) ?>">&lt;<?= e($g['name']) ?>&gt;</a></h3>
                    <p class="best"><?= $b ? e($b[1]['name']) . ' ' . $b[2] . '/' . count($b[1]['bosses']) : '' ?></p>
                    <div class="score"><?= (int)$g['total'] ?><small>Bosse besiegt</small></div>
                </div>
            <?php endforeach; ?>
        </div>
    <?php endif; ?>
    <div class="card table-wrap" data-reveal>
        <?php if (!$ranking): ?>
            <div class="empty">
                <?= icon('trophy', 'icon-xl') ?>
                <p><strong>Das Rennen ist eröffnet – noch hat keine Gilde einen Raidboss gelegt.</strong></p>
                <p class="small">Sobald eine Gilde mit mindestens <?= Progress::minMembers(532) ?> eigenen Mitgliedern in Karazhan/Zul'Aman
                    bzw. <?= Progress::minMembers(548) ?> in den übrigen Raids einen Boss besiegt, erscheint sie hier.</p>
            </div>
        <?php else: ?>
            <table class="table progress-table">
                <thead>
                <tr>
                    <th>#</th><th>Gilde</th>
                    <?php foreach (Progress::RAIDS as $map => $raid): ?>
                        <th class="center" title="<?= e($raid['name']) ?>"><?= e($raid['short']) ?></th>
                    <?php endforeach; ?>
                    <th>Letzter Kill</th>
                </tr>
                </thead>
                <tbody>
                <?php foreach ($ranking as $i => $g): ?>
                    <tr>
                        <td class="rank rank-<?= $i + 1 ?>"><?= $i + 1 ?></td>
                        <td class="nowrap"><a href="<?= e(url('guild', ['realm' => $realmId, 'id' => $g['guild_id']])) ?>"><strong>&lt;<?= e($g['name']) ?>&gt;</strong></a></td>
                        <?php foreach (Progress::RAIDS as $map => $raid):
                            $n = $g['raids'][$map] ?? 0; $max = count($raid['bosses']); ?>
                            <td class="center"><span class="prog <?= $n === 0 ? 'prog-none' : ($n >= $max ? 'prog-full' : 'prog-part') ?>"><?= $n ?>/<?= $max ?></span></td>
                        <?php endforeach; ?>
                        <td class="nowrap small">
                            <?php if ($g['last_boss']): ?>
                                <?= e(Progress::bossName($g['last_boss'][0], $g['last_boss'][1])) ?><br>
                                <span class="muted"><?= e(fmt_date($g['last'], true)) ?></span>
                            <?php endif; ?>
                        </td>
                    </tr>
                <?php endforeach; ?>
                </tbody>
            </table>
        <?php endif; ?>
    </div>

    <div class="section-head center" data-reveal>
        <p class="eyebrow">Neun Raids</p>
        <h2>Raids &amp; Realm-Firsts</h2>
        <?= ornament() ?>
    </div>
    <?php foreach (Progress::TIERS as $tier => $tierLabel): ?>
        <h3 class="tier-title"><span class="tier-badge"><?= e($tier) ?></span> <?= e($tierLabel) ?></h3>
        <div class="raid-grid" data-stagger>
            <?php foreach (Progress::RAIDS as $map => $raid): if ($raid['tier'] !== $tier) continue;
                $done = count($firsts[$map] ?? []); $max = count($raid['bosses']); ?>
                <article class="raid-card<?= $done >= $max ? ' cleared' : '' ?>" data-raid="<?= (int)$map ?>" id="raid-<?= (int)$map ?>">
                    <header class="raid-banner">
                        <span class="raid-emblem"><?= icon('r-' . $map) ?></span>
                        <div class="raid-info">
                            <h4><?= e($raid['name']) ?></h4>
                            <p><?= (int)$raid['size'] ?> Spieler · <?= $max ?> Bosse · Wertung ab <?= Progress::minMembers($map) ?> aus der Gilde</p>
                        </div>
                        <?= ring($done, $max, '', 'ring-lg') ?>
                    </header>
                    <ol class="boss-list">
                        <?php foreach ($raid['bosses'] as $b => $boss): $f = $firsts[$map][$b] ?? null; ?>
                            <li class="<?= $f ? 'down' : 'alive' ?>">
                                <span class="boss-name"><span class="boss-mark"><?= icon($f ? 'check' : 'skull') ?></span><?= e($boss) ?></span>
                                <?php if ($f): ?>
                                    <span class="boss-first">
                                        <a href="<?= e(url('guild', ['realm' => $realmId, 'id' => $f['guild_id']])) ?>">&lt;<?= e($f['guild_name']) ?>&gt;</a>
                                        <span class="muted"><?= e(fmt_date($f['killed_at'])) ?><?= ($counts[$map][$b] ?? 0) > 1 ? ' · ' . (int)$counts[$map][$b] . ' Gilden' : '' ?></span>
                                    </span>
                                <?php else: ?>
                                    <span class="boss-first muted">unbesiegt</span>
                                <?php endif; ?>
                            </li>
                        <?php endforeach; ?>
                    </ol>
                </article>
            <?php endforeach; ?>
        </div>
    <?php endforeach; ?>

    <div class="grid-2" data-stagger>
        <div class="card">
            <h2 class="card-title"><?= icon('scroll') ?> Chronik der Realm-Firsts</h2>
            <?php if (!$timeline): ?>
                <p class="muted">Noch keine Einträge – wer schreibt Geschichte?</p>
            <?php else: ?>
                <ol class="timeline">
                    <?php foreach (array_slice($timeline, 0, 20) as $k): ?>
                        <li>
                            <time><?= e(fmt_date($k['killed_at'], true)) ?></time>
                            <div><strong><?= e(Progress::bossName((int)$k['map_id'], (int)$k['boss_index'])) ?></strong>
                                <span class="muted">(<?= e(Progress::RAIDS[(int)$k['map_id']]['name'] ?? '') ?>)</span><br>
                                <a href="<?= e(url('guild', ['realm' => $realmId, 'id' => $k['guild_id']])) ?>">&lt;<?= e($k['guild_name']) ?>&gt;</a></div>
                        </li>
                    <?php endforeach; ?>
                </ol>
            <?php endif; ?>
        </div>
        <div class="card">
            <h2 class="card-title"><?= icon('shield') ?> So wird gewertet</h2>
            <ul class="checklist">
                <li>Ein Kill zählt nur als <strong>Gilden-Kill</strong>, wenn genug Gildenmitglieder in der Raid-ID sind:
                    mindestens <strong><?= Progress::minMembers(532) ?></strong> in Karazhan und Zul'Aman,
                    mindestens <strong><?= Progress::minMembers(548) ?></strong> in allen anderen Raids.</li>
                <li>Mitglieder können Spieler oder Playerbots sein – Hauptsache, sie sind in der Gilde.</li>
                <li>Kills mit weniger Gildenmitgliedern (z. B. gemischte Gruppen) erscheinen weder in der Wertung noch bei den Realm-Firsts.</li>
                <li>Gilden, die von Random-Bots geführt werden, laufen außer Konkurrenz.</li>
                <li>Gewertet wird der erste Kill je Boss; der Zeitpunkt ist die Erkennung durch die Website (wenige Minuten Verzögerung möglich).</li>
                <li>Nur normale Raid-Schwierigkeit (TBC hat keine heroischen Raids).</li>
            </ul>
            <p class="muted small">Fehler entdeckt? Das Team kann Kills im Admin-Bereich korrigieren.</p>
        </div>
    </div>
</section>
