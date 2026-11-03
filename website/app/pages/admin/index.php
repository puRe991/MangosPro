<?php
Auth::requireAdmin();
page_title('Admin');
$db = Database::auth();
$stats = [
    'Accounts gesamt'        => Account::totalCount(),
    'Neu heute'              => (int)$db->query('SELECT COUNT(*) FROM account WHERE joindate >= CURDATE()')->fetchColumn(),
    'Neu (7 Tage)'           => (int)$db->query('SELECT COUNT(*) FROM account WHERE joindate >= NOW() - INTERVAL 7 DAY')->fetchColumn(),
    'Aktive Account-Sperren' => (int)$db->query('SELECT COUNT(*) FROM account_banned WHERE active = 1 AND (expires_at = banned_at OR expires_at > UNIX_TIMESTAMP())')->fetchColumn(),
    'News-Beiträge'          => News::count(false),
];
$recent = $db->query("SELECT id, username, email, joindate, gmlevel FROM account ORDER BY id DESC LIMIT 15")->fetchAll();
$prefix = strtoupper((string)config('bots.account_prefix', ''));
if (config('bots.hide', true) && $prefix !== '') {
    $recent = array_values(array_filter($recent, fn($a) => !str_starts_with($a['username'], $prefix)));
}
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow">Verwaltung</p>
        <h1>Admin-Bereich</h1>
    </div>
</section>
<section class="wrap section">
    <nav class="segmented">
        <a class="active" href="<?= e(url('admin')) ?>">Übersicht</a>
        <a href="<?= e(url('admin/news')) ?>">News</a>
        <a href="<?= e(url('admin/progress')) ?>">Progress</a>
    </nav>
    <div class="stat-grid">
        <?php foreach ($stats as $label => $v): ?>
            <div class="stat"><span><?= e($label) ?></span><strong><?= fmt_number($v) ?></strong></div>
        <?php endforeach; ?>
        <div class="stat"><span>SRP6-Backend</span><strong><?= e(Srp6::backend()) ?></strong></div>
    </div>
    <div class="card table-wrap">
        <h2 class="card-title">Neueste Accounts</h2>
        <table class="table">
            <thead><tr><th>ID</th><th>Name</th><th>E-Mail</th><th>Registriert</th><th>GM</th></tr></thead>
            <tbody>
            <?php foreach ($recent as $a): ?>
                <tr><td><?= (int)$a['id'] ?></td><td><?= e($a['username']) ?></td><td><?= e($a['email'] ?? '') ?></td>
                    <td><?= e(fmt_date($a['joindate'], true)) ?></td><td><?= (int)$a['gmlevel'] ?></td></tr>
            <?php endforeach; ?>
            </tbody>
        </table>
        <p class="muted small">Sperren, GM-Rechte usw. bitte weiterhin über die Server-Konsole bzw. Ingame-Befehle (<code>.ban account</code>, <code>.account set gmlevel</code>) verwalten.</p>
    </div>
</section>
