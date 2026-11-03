<?php
page_title('Spieler online');
$realmId = Realm::selected();
$realm = Realm::get($realmId);
$filter = Account::botFilterSql('c.account');
$showBots = query('bots') === '1' && config('bots.hide', true);
if ($showBots) {
    $ids = Account::botAccountIds();
    $filter = $ids ? ' AND c.account IN (' . implode(',', $ids) . ')' : ' AND 0';
}
$rows = Database::chars($realmId)->query(
    "SELECT c.name, c.race, c.class, c.gender, c.level, c.zone, g.name AS guild
     FROM characters c
     LEFT JOIN guild_member gm ON gm.guid = c.guid
     LEFT JOIN guild g ON g.guildid = gm.guildid
     WHERE c.online = 1 $filter
     ORDER BY c.level DESC, c.name LIMIT 500")->fetchAll();
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow"><?= e($realm['name'] ?? 'Realm') ?></p>
        <h1><?= $showBots ? 'Aktive Bots' : 'Spieler online' ?> <span class="muted">(<?= count($rows) ?>)</span></h1>
    </div>
</section>
<section class="wrap section">
    <?php render_partial('realm_tabs', ['route' => 'online', 'current' => $realmId, 'extra' => $showBots ? ['bots' => 1] : []]); ?>
    <?php if (config('bots.hide', true)): ?>
        <div class="segmented">
            <a href="<?= e(url('online', ['realm' => $realmId])) ?>"<?= $showBots ? '' : ' class="active"' ?>>Spieler</a>
            <a href="<?= e(url('online', ['realm' => $realmId, 'bots' => 1])) ?>"<?= $showBots ? ' class="active"' : '' ?>>Bots</a>
        </div>
    <?php endif; ?>
    <?php if (!$rows): ?>
        <div class="empty card"><p>Gerade ist niemand online. Sei der Erste!</p></div>
    <?php else: ?>
        <div class="table-wrap card">
            <table class="table" data-sortable>
                <thead><tr><th>Name</th><th data-sort="num">Stufe</th><th>Volk</th><th>Klasse</th><th>Gilde</th><th>Zone</th></tr></thead>
                <tbody>
                <?php foreach ($rows as $c): ?>
                    <tr>
                        <td><a href="<?= e(url('character', ['realm' => $realmId, 'name' => $c['name']])) ?>"><?= GameData::charBadge($c) ?></a></td>
                        <td><?= (int)$c['level'] ?></td>
                        <td><?= e(GameData::race((int)$c['race'])) ?></td>
                        <td class="cls-<?= GameData::classSlug((int)$c['class']) ?>"><?= e(GameData::className((int)$c['class'])) ?></td>
                        <td><?= $c['guild'] ? e($c['guild']) : '<span class="muted">–</span>' ?></td>
                        <td><?= e(GameData::zone((int)$c['zone'])) ?></td>
                    </tr>
                <?php endforeach; ?>
                </tbody>
            </table>
        </div>
    <?php endif; ?>
</section>
