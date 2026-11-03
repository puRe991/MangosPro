<?php
Auth::requireAdmin();
page_title('Progress verwalten');
$realmId = Realm::selected();
$db = Database::chars($realmId);

if (is_post()) {
    require_csrf();
    $action = post('action');
    if ($action === 'sync') {
        $n = Progress::sync($realmId);
        flash('success', "Abgleich fertig: $n neue Kills erkannt.");
    } elseif ($action === 'delete' && ($id = (int)post('id'))) {
        Progress::delete($id);
        flash('success', 'Eintrag gelöscht.');
    } elseif ($action === 'add') {
        $guildId = (int)post('guild_id');
        [$map, $boss] = array_map('intval', explode(':', post('boss') . ':'));
        $ts = strtotime(post('killed_at')) ?: 0;
        $guild = Database::run($db, 'SELECT guildid, name FROM guild WHERE guildid = ?', [$guildId])->fetch();
        if (!$guild || !isset(Progress::RAIDS[$map]['bosses'][$boss]) || $ts <= 0) {
            flash('error', 'Bitte Gilde, Boss und Datum angeben.');
        } else {
            Progress::addManual($realmId, (int)$guild['guildid'], $guild['name'], $map, $boss, $ts);
            flash('success', 'Kill gespeichert: ' . Progress::bossName($map, $boss) . ' – <' . $guild['name'] . '>');
        }
    }
    redirect('admin/progress', ['realm' => $realmId]);
}

$guilds = $db->query('SELECT guildid, name FROM guild ORDER BY name')->fetchAll();
$rows = Database::run(Database::web(),
    'SELECT id, guild_name, map_id, boss_index, killed_at, source FROM web_progress_kills WHERE realm_id = ? ORDER BY killed_at DESC LIMIT 200',
    [$realmId])->fetchAll();
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow">Verwaltung</p>
        <h1>Progress Race verwalten</h1>
    </div>
</section>
<section class="wrap section">
    <nav class="segmented">
        <a href="<?= e(url('admin')) ?>">Übersicht</a>
        <a href="<?= e(url('admin/news')) ?>">News</a>
        <a class="active" href="<?= e(url('admin/progress')) ?>">Progress</a>
    </nav>
    <?php render_partial('realm_tabs', ['route' => 'admin/progress', 'current' => $realmId, 'extra' => []]); ?>

    <div class="grid-main">
        <div class="card table-wrap">
            <h2 class="card-title">Erfasste Kills <span class="muted">(<?= count($rows) ?>)</span></h2>
            <?php if (!$rows): ?><p class="empty">Noch keine Kills erfasst.</p><?php else: ?>
                <table class="table">
                    <thead><tr><th>Datum</th><th>Gilde</th><th>Boss</th><th>Quelle</th><th></th></tr></thead>
                    <tbody>
                    <?php foreach ($rows as $r): ?>
                        <tr>
                            <td class="nowrap"><?= e(fmt_date($r['killed_at'], true)) ?></td>
                            <td>&lt;<?= e($r['guild_name']) ?>&gt;</td>
                            <td><?= e(Progress::bossName((int)$r['map_id'], (int)$r['boss_index'])) ?>
                                <span class="muted small">· <?= e(Progress::RAIDS[(int)$r['map_id']]['short'] ?? '') ?></span></td>
                            <td><span class="badge <?= $r['source'] === 'auto' ? 'badge-ok' : 'badge-off' ?> badge-xs"><?= $r['source'] === 'auto' ? 'automatisch' : 'manuell' ?></span></td>
                            <td class="actions">
                                <form method="post" action="<?= e(url('admin/progress', ['realm' => $realmId])) ?>" class="inline" data-confirm="Eintrag löschen?">
                                    <?= csrf_field() ?><input type="hidden" name="action" value="delete"><input type="hidden" name="id" value="<?= (int)$r['id'] ?>">
                                    <button class="btn btn-danger btn-sm" type="submit">Löschen</button>
                                </form>
                            </td>
                        </tr>
                    <?php endforeach; ?>
                    </tbody>
                </table>
            <?php endif; ?>
        </div>

        <aside class="sidebar">
            <div class="panel">
                <h2 class="panel-title">Automatischer Abgleich</h2>
                <p class="small muted">Liest <code>instance.encountersMask</code> aus der Characters-DB und erkennt neue Boss-Kills.
                    Läuft <?= config('progress.auto_sync', true) ? 'automatisch alle ' . (int)config('progress.sync_interval', 120) . ' s beim Seitenaufruf' : 'per Cron (<code>bin/progress_sync.php</code>)' ?>.</p>
                <form method="post" action="<?= e(url('admin/progress', ['realm' => $realmId])) ?>">
                    <?= csrf_field() ?><input type="hidden" name="action" value="sync">
                    <button class="btn btn-ghost btn-block" type="submit">Jetzt abgleichen</button>
                </form>
            </div>
            <form class="panel" method="post" action="<?= e(url('admin/progress', ['realm' => $realmId])) ?>">
                <h2 class="panel-title">Kill manuell eintragen</h2>
                <?= csrf_field() ?><input type="hidden" name="action" value="add">
                <div class="field">
                    <label for="guild_id">Gilde</label>
                    <select id="guild_id" name="guild_id" required>
                        <option value="">– wählen –</option>
                        <?php foreach ($guilds as $g): ?><option value="<?= (int)$g['guildid'] ?>"><?= e($g['name']) ?></option><?php endforeach; ?>
                    </select>
                </div>
                <div class="field">
                    <label for="boss">Boss</label>
                    <select id="boss" name="boss" required>
                        <?php foreach (Progress::RAIDS as $map => $raid): ?>
                            <optgroup label="<?= e($raid['name']) ?>">
                                <?php foreach ($raid['bosses'] as $b => $name): ?><option value="<?= $map ?>:<?= $b ?>"><?= e($name) ?></option><?php endforeach; ?>
                            </optgroup>
                        <?php endforeach; ?>
                    </select>
                </div>
                <div class="field">
                    <label for="killed_at">Zeitpunkt</label>
                    <input id="killed_at" name="killed_at" type="datetime-local" required value="<?= e(date('Y-m-d\TH:i')) ?>">
                </div>
                <p class="hint">Ein bestehender Eintrag derselben Gilde für diesen Boss wird überschrieben.</p>
                <button class="btn btn-primary btn-block" type="submit">Speichern</button>
            </form>
        </aside>
    </div>
</section>
