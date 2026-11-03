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
        <ul class="hero-stats progress-stats">
            <li><strong><?= count($ranking) ?></strong><span>Gilden im Rennen</span></li>
            <li><strong><?= $firstCount ?> / <?= $totalBosses ?></strong><span>Bosse besiegt</span></li>
            <li><strong><?= $latest ? e(Progress::bossName((int)$latest['map_id'], (int)$latest['boss_index'])) : '–' ?></strong>
                <span><?= $latest ? 'Letzter Realm-First · ' . e($latest['guild_name']) : 'Noch kein Realm-First' ?></span></li>
        </ul>
    </div>
</section>

<section class="wrap section">
    <?php render_partial('realm_tabs', ['route' => 'progress', 'current' => $realmId, 'extra' => []]); ?>

    <div class="section-head">
        <h2>Gildenwertung</h2>
        <span class="muted small">Sortiert nach höchster Raidstufe, bei Gleichstand zählt, wer zuerst dort war.</span>
    </div>
    <div class="card table-wrap">
        <?php if (!$ranking): ?>
            <div class="empty">
                <p><strong>Das Rennen ist eröffnet – noch hat keine Gilde einen Raidboss gelegt.</strong></p>
                <p class="small">Sobald eine Gilde mit mindestens <?= (int)config('progress.min_members_10', 5) ?> (10er) bzw.
                    <?= (int)config('progress.min_members_25', 10) ?> (25er) Mitgliedern einen Boss besiegt, erscheint sie hier.</p>
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

    <div class="section-head">
        <h2>Raids &amp; Realm-Firsts</h2>
    </div>
    <?php foreach (Progress::TIERS as $tier => $tierLabel): ?>
        <h3 class="tier-title"><span class="tier-badge"><?= e($tier) ?></span> <?= e($tierLabel) ?></h3>
        <div class="raid-grid">
            <?php foreach (Progress::RAIDS as $map => $raid): if ($raid['tier'] !== $tier) continue;
                $done = count($firsts[$map] ?? []); $max = count($raid['bosses']); ?>
                <article class="raid-card<?= $done >= $max ? ' cleared' : '' ?>">
                    <header>
                        <div>
                            <h4><?= e($raid['name']) ?></h4>
                            <p class="muted small"><?= (int)$raid['size'] ?> Spieler · <?= $max ?> Bosse</p>
                        </div>
                        <span class="prog <?= $done === 0 ? 'prog-none' : ($done >= $max ? 'prog-full' : 'prog-part') ?>"><?= $done ?>/<?= $max ?></span>
                    </header>
                    <progress max="<?= $max ?>" value="<?= $done ?>"></progress>
                    <ol class="boss-list">
                        <?php foreach ($raid['bosses'] as $b => $boss): $f = $firsts[$map][$b] ?? null; ?>
                            <li class="<?= $f ? 'down' : 'alive' ?>">
                                <span class="boss-name"><?= e($boss) ?></span>
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

    <div class="grid-2">
        <div class="card">
            <h2 class="card-title">Chronik der Realm-Firsts</h2>
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
            <h2 class="card-title">So wird gewertet</h2>
            <ul class="checklist">
                <li>Ein Kill zählt für die Gilde, die in der Raid-ID die meisten Mitglieder stellt – mindestens
                    <?= (int)config('progress.min_members_10', 5) ?> im 10er- und <?= (int)config('progress.min_members_25', 10) ?> im 25er-Raid.</li>
                <li>Eigene Playerbots in der Gilde zählen als Mitglieder – genau dafür sind sie da.</li>
                <li>Gilden, die von Random-Bots geführt werden, laufen außer Konkurrenz.</li>
                <li>Gewertet wird der erste Kill je Boss; der Zeitpunkt ist die Erkennung durch die Website (wenige Minuten Verzögerung möglich).</li>
                <li>Nur normale Raid-Schwierigkeit (TBC hat keine heroischen Raids).</li>
            </ul>
            <p class="muted small">Fehler entdeckt? Das Team kann Kills im Admin-Bereich korrigieren.</p>
        </div>
    </div>
</section>
