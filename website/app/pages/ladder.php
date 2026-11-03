<?php
page_title('Ranglisten');
$realmId = Realm::selected();
$tab = query('tab', 'level');
$tabs = ['level' => 'Stufe', 'honor' => 'Ehrenhafte Siege', 'arena2' => 'Arena 2v2', 'arena3' => 'Arena 3v3', 'arena5' => 'Arena 5v5', 'played' => 'Spielzeit'];
if (!isset($tabs[$tab])) $tab = 'level';
$db = Database::chars($realmId);
$filter = Account::botFilterSql('c.account');
$base = "SELECT c.guid, c.name, c.race, c.class, c.gender, c.level, c.totalKills, c.totalHonorPoints, c.totaltime, g.name AS guild
         FROM characters c LEFT JOIN guild_member gm ON gm.guid = c.guid LEFT JOIN guild g ON g.guildid = gm.guildid
         WHERE c.deleteInfos_Account IS NULL $filter";

$rows = [];
$teams = [];
switch ($tab) {
    case 'level':  $rows = $db->query("$base ORDER BY c.level DESC, c.xp DESC, c.totaltime ASC LIMIT 100")->fetchAll(); break;
    case 'honor':  $rows = $db->query("$base AND c.totalKills > 0 ORDER BY c.totalKills DESC LIMIT 100")->fetchAll(); break;
    case 'played': $rows = $db->query("$base ORDER BY c.totaltime DESC LIMIT 100")->fetchAll(); break;
    default:
        $type = (int)substr($tab, -1);
        $capFilter = Account::botFilterSql('cap.account');
        $teams = Database::run($db,
            "SELECT t.arenateamid, t.name, s.rating, s.games_season, s.wins_season, cap.name AS captain, cap.race, cap.class
             FROM arena_team t JOIN arena_team_stats s ON s.arenateamid = t.arenateamid
             LEFT JOIN characters cap ON cap.guid = t.captainguid
             WHERE t.type = ? $capFilter ORDER BY s.rating DESC, s.wins_season DESC LIMIT 100", [$type])->fetchAll();
}
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow">Hall of Fame</p>
        <h1>Ranglisten</h1>
    </div>
</section>
<section class="wrap section">
    <?php render_partial('realm_tabs', ['route' => 'ladder', 'current' => $realmId, 'extra' => ['tab' => $tab]]); ?>
    <nav class="segmented" aria-label="Rangliste">
        <?php foreach ($tabs as $k => $label): ?>
            <a href="<?= e(url('ladder', ['realm' => $realmId, 'tab' => $k])) ?>"<?= $k === $tab ? ' class="active"' : '' ?>><?= e($label) ?></a>
        <?php endforeach; ?>
    </nav>

    <div class="card table-wrap">
        <?php if (str_starts_with($tab, 'arena')): ?>
            <?php if (!$teams): ?><p class="empty">Noch keine Arena-Teams in dieser Kategorie.</p><?php else: ?>
            <table class="table">
                <thead><tr><th>#</th><th>Team</th><th>Wertung</th><th>Siege / Spiele (Saison)</th><th>Captain</th></tr></thead>
                <tbody>
                <?php foreach ($teams as $i => $t): ?>
                    <tr>
                        <td class="rank rank-<?= $i + 1 ?>"><?= $i + 1 ?></td>
                        <td><strong><?= e($t['name']) ?></strong></td>
                        <td><?= fmt_number($t['rating']) ?></td>
                        <td><?= fmt_number($t['wins_season']) ?> / <?= fmt_number($t['games_season']) ?></td>
                        <td><?= $t['captain'] ? '<a href="' . e(url('character', ['realm' => $realmId, 'name' => $t['captain']])) . '">'
                                . GameData::charBadge(['name' => $t['captain'], 'race' => $t['race'], 'class' => $t['class']]) . '</a>' : '–' ?></td>
                    </tr>
                <?php endforeach; ?>
                </tbody>
            </table>
            <?php endif; ?>
        <?php else: ?>
            <?php if (!$rows): ?><p class="empty">Noch keine Einträge.</p><?php else: ?>
            <table class="table">
                <thead><tr><th>#</th><th>Name</th><th>Stufe</th><th>Volk / Klasse</th><th>Gilde</th>
                    <th><?= $tab === 'played' ? 'Spielzeit' : 'Ehrenhafte Siege' ?></th></tr></thead>
                <tbody>
                <?php foreach ($rows as $i => $c): ?>
                    <tr>
                        <td class="rank rank-<?= $i + 1 ?>"><?= $i + 1 ?></td>
                        <td><a href="<?= e(url('character', ['realm' => $realmId, 'name' => $c['name']])) ?>"><?= GameData::charBadge($c) ?></a></td>
                        <td><?= (int)$c['level'] ?></td>
                        <td><?= e(GameData::race((int)$c['race'])) ?> · <?= e(GameData::className((int)$c['class'])) ?></td>
                        <td><?= $c['guild'] ? e($c['guild']) : '<span class="muted">–</span>' ?></td>
                        <td><?= $tab === 'played' ? e(fmt_duration((int)$c['totaltime'])) : fmt_number($c['totalKills']) ?></td>
                    </tr>
                <?php endforeach; ?>
                </tbody>
            </table>
            <?php endif; ?>
        <?php endif; ?>
    </div>
    <?php if (config('bots.hide', true)): ?><p class="muted small">Random-Bots sind in den Ranglisten ausgeblendet.</p><?php endif; ?>
</section>
