<?php
page_title('Arsenal');
$realmId = Realm::selected();
$q = query('q');
$chars = [];
$guilds = [];
if (mb_strlen($q) >= 2) {
    $db = Database::chars($realmId);
    $like = addcslashes($q, '%_\\') . '%';
    $filter = Account::botFilterSql('c.account');
    $chars = Database::run($db,
        "SELECT c.name, c.race, c.class, c.gender, c.level, g.name AS guild FROM characters c
         LEFT JOIN guild_member gm ON gm.guid = c.guid LEFT JOIN guild g ON g.guildid = gm.guildid
         WHERE c.name LIKE ? AND c.deleteInfos_Account IS NULL $filter ORDER BY c.level DESC, c.name LIMIT 50", [$like])->fetchAll();
    $guilds = Database::run($db,
        'SELECT g.guildid, g.name, COUNT(gm.guid) AS members FROM guild g LEFT JOIN guild_member gm ON gm.guildid = g.guildid
         WHERE g.name LIKE ? GROUP BY g.guildid, g.name ORDER BY members DESC LIMIT 20', ['%' . $like])->fetchAll();
}
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow">Charaktere &amp; Gilden</p>
        <h1>Arsenal</h1>
        <form class="search" method="get" action="<?= e(url('armory')) ?>" role="search">
            <?php if (!config('site.pretty_urls', true)): ?><input type="hidden" name="p" value="armory"><?php endif; ?>
            <input type="hidden" name="realm" value="<?= (int)$realmId ?>">
            <label class="sr" for="q">Name suchen</label>
            <input id="q" name="q" type="search" minlength="2" maxlength="24" placeholder="Charakter- oder Gildenname …" value="<?= e($q) ?>" autofocus>
            <button class="btn btn-primary" type="submit">Suchen</button>
        </form>
    </div>
</section>
<section class="wrap section">
    <?php render_partial('realm_tabs', ['route' => 'armory', 'current' => $realmId, 'extra' => $q !== '' ? ['q' => $q] : []]); ?>
    <?php if ($q === ''): ?>
        <p class="muted center">Gib mindestens zwei Buchstaben ein, um nach Charakteren oder Gilden zu suchen.</p>
    <?php elseif (mb_strlen($q) < 2): ?>
        <p class="muted center">Bitte mindestens zwei Zeichen eingeben.</p>
    <?php else: ?>
        <div class="grid-2">
            <div class="card">
                <h2 class="card-title">Charaktere <span class="muted">(<?= count($chars) ?>)</span></h2>
                <?php if (!$chars): ?><p class="muted">Keine Treffer.</p><?php else: ?>
                    <ul class="result-list">
                        <?php foreach ($chars as $c): ?>
                            <li>
                                <a href="<?= e(url('character', ['realm' => $realmId, 'name' => $c['name']])) ?>"><?= GameData::charBadge($c) ?></a>
                                <span class="muted small">Stufe <?= (int)$c['level'] ?> <?= e(GameData::race((int)$c['race'])) ?> <?= e(GameData::className((int)$c['class'])) ?><?= $c['guild'] ? ' · &lt;' . e($c['guild']) . '&gt;' : '' ?></span>
                            </li>
                        <?php endforeach; ?>
                    </ul>
                <?php endif; ?>
            </div>
            <div class="card">
                <h2 class="card-title">Gilden <span class="muted">(<?= count($guilds) ?>)</span></h2>
                <?php if (!$guilds): ?><p class="muted">Keine Treffer.</p><?php else: ?>
                    <ul class="result-list">
                        <?php foreach ($guilds as $g): ?>
                            <li><a href="<?= e(url('guild', ['realm' => $realmId, 'id' => $g['guildid']])) ?>">&lt;<?= e($g['name']) ?>&gt;</a>
                                <span class="muted small"><?= (int)$g['members'] ?> Mitglieder</span></li>
                        <?php endforeach; ?>
                    </ul>
                <?php endif; ?>
            </div>
        </div>
    <?php endif; ?>
</section>
