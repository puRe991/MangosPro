<?php
Auth::requireLogin();
Auth::refresh();
page_title('Mein Account');

$acc = Account::find(Auth::id());
if (!$acc) { Auth::logout(); redirect('login'); }
$ban = Account::activeBan((int)$acc['id']);
$lastLogin = Account::lastLoginIp((int)$acc['id']);

// Charaktere aller konfigurierten Realms
$realms = Realm::all();
$chars = [];
foreach ((array)config('realms', []) as $rid => $rcfg) {
    if (empty($rcfg['characters'])) continue;
    try {
        $rows = Database::run(Database::chars((int)$rid),
            'SELECT c.guid, c.name, c.race, c.class, c.gender, c.level, c.money, c.online, c.totaltime, c.zone,
                    c.totalKills, g.name AS guild
             FROM characters c
             LEFT JOIN guild_member gm ON gm.guid = c.guid
             LEFT JOIN guild g ON g.guildid = gm.guildid
             WHERE c.account = ? AND c.deleteInfos_Account IS NULL
             ORDER BY c.level DESC, c.name', [(int)$acc['id']])->fetchAll();
    } catch (Throwable $ex) {
        error_log('[website] account chars: ' . $ex->getMessage());
        $rows = [];
    }
    foreach ($rows as $r) {
        $r['realm_id'] = (int)$rid;
        $r['realm_name'] = $realms[$rid]['name'] ?? "Realm $rid";
        $chars[] = $r;
    }
}
$totalGold = array_sum(array_column($chars, 'money'));
$totalPlayed = array_sum(array_column($chars, 'totaltime'));
$gmNames = [0 => 'Spieler', 1 => 'Moderator', 2 => 'Gamemaster', 3 => 'Administrator'];
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow">Account-Verwaltung</p>
        <h1><?= e(ucfirst(strtolower($acc['username']))) ?></h1>
    </div>
</section>

<section class="wrap section account-grid">
    <aside class="account-nav">
        <nav class="panel" aria-label="Account">
            <a class="active" href="<?= e(url('account')) ?>"><?= icon('user') ?> Übersicht</a>
            <a href="<?= e(url('account/password')) ?>"><?= icon('lock') ?> Passwort ändern</a>
            <a href="<?= e(url('account/email')) ?>"><?= icon('mail') ?> E-Mail ändern</a>
            <a href="<?= e(url('connect')) ?>"><?= icon('book') ?> Verbindungsanleitung</a>
        </nav>
    </aside>

    <div>
        <?php if ($ban): ?>
            <div class="alert alert-error">
                <strong>Account gesperrt.</strong> Grund: <?= e($ban['reason']) ?> ·
                <?= (int)$ban['expires_at'] === (int)$ban['banned_at'] ? 'dauerhaft' : 'bis ' . e(fmt_date($ban['expires_at'], true)) ?>
            </div>
        <?php endif; ?>

        <div class="stat-grid" data-stagger>
            <div class="stat"><span>Status</span><strong><?= e($gmNames[(int)$acc['gmlevel']] ?? 'GM ' . (int)$acc['gmlevel']) ?></strong></div>
            <div class="stat"><span>Mitglied seit</span><strong><?= e(fmt_date($acc['joindate'])) ?></strong></div>
            <div class="stat"><span>Charaktere</span><strong><?= count($chars) ?></strong></div>
            <div class="stat"><span>Gold gesamt</span><strong><?= fmt_money((int)$totalGold) ?></strong></div>
            <div class="stat"><span>Spielzeit</span><strong><?= e(fmt_duration((int)$totalPlayed)) ?></strong></div>
            <div class="stat"><span>Erweiterung</span><strong><?= (int)$acc['expansion'] >= 1 ? 'The Burning Crusade' : 'Classic' ?></strong></div>
        </div>

        <div class="card">
            <h2 class="card-title"><?= icon('users') ?> Deine Charaktere</h2>
            <?php if (!$chars): ?>
                <div class="empty">
                    <p>Du hast noch keine Charaktere. Logge dich im Spiel ein und erstelle deinen ersten Helden!</p>
                    <a class="btn btn-primary" href="<?= e(url('connect')) ?>">Zur Anleitung</a>
                </div>
            <?php else: ?>
                <div class="table-wrap">
                    <table class="table">
                        <thead><tr><th>Name</th><th>Stufe</th><th>Volk / Klasse</th><th>Gilde</th><th>Ort</th><th>Gold</th><?= count($realms) > 1 ? '<th>Realm</th>' : '' ?></tr></thead>
                        <tbody>
                        <?php foreach ($chars as $c): ?>
                            <tr>
                                <td>
                                    <a href="<?= e(url('character', ['realm' => $c['realm_id'], 'name' => $c['name']])) ?>"><?= GameData::charBadge($c) ?></a>
                                    <?php if ($c['online']): ?><span class="badge badge-ok badge-xs">online</span><?php endif; ?>
                                </td>
                                <td><?= (int)$c['level'] ?></td>
                                <td><?= e(GameData::race((int)$c['race'])) ?> · <?= e(GameData::className((int)$c['class'])) ?></td>
                                <td><?= $c['guild'] ? e($c['guild']) : '<span class="muted">–</span>' ?></td>
                                <td><?= e(GameData::zone((int)$c['zone'])) ?></td>
                                <td class="nowrap"><?= fmt_money((int)$c['money']) ?></td>
                                <?= count($realms) > 1 ? '<td>' . e($c['realm_name']) . '</td>' : '' ?>
                            </tr>
                        <?php endforeach; ?>
                        </tbody>
                    </table>
                </div>
            <?php endif; ?>
        </div>

        <div class="card">
            <h2 class="card-title"><?= icon('lock') ?> Sicherheit</h2>
            <dl class="kv">
                <div><dt>E-Mail</dt><dd><?= $acc['email'] ? e($acc['email']) : '<span class="muted">nicht hinterlegt</span>' ?> · <a href="<?= e(url('account/email')) ?>">ändern</a></dd></div>
                <div><dt>Letzter Spiel-Login</dt><dd><?= $lastLogin ? e(fmt_date($lastLogin['loginTime'], true)) . ' von ' . e($lastLogin['ip']) : '<span class="muted">noch nie</span>' ?></dd></div>
                <div><dt>Account-Sperre (IP)</dt><dd><?= (int)$acc['locked'] ? 'aktiv – Login nur von der letzten IP' : 'aus' ?></dd></div>
            </dl>
            <p class="muted small">Tipp: Im Spiel kannst du mit <code>.account lock on</code> deinen Account an deine aktuelle IP binden.</p>
        </div>
    </div>
</section>
