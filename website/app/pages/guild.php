<?php
$realmId = Realm::selected();
$id = (int)query('id', '0');
$db = Database::chars($realmId);
$g = Database::run($db, 'SELECT g.guildid, g.name, g.info, g.motd, g.createdate, l.name AS leader, l.race AS lrace, l.class AS lclass
    FROM guild g LEFT JOIN characters l ON l.guid = g.leaderguid WHERE g.guildid = ?', [$id])->fetch();
if (!$g) not_found();
$members = Database::run($db,
    'SELECT c.name, c.race, c.class, c.gender, c.level, c.online, gm.rank, r.rname
     FROM guild_member gm JOIN characters c ON c.guid = gm.guid
     LEFT JOIN guild_rank r ON r.guildid = gm.guildid AND r.rid = gm.rank
     WHERE gm.guildid = ? ORDER BY gm.rank, c.level DESC, c.name', [$id])->fetchAll();
$online = count(array_filter($members, fn($m) => $m['online']));
Progress::maybeSync($realmId);
$gk = Progress::guildKills($realmId, $id);
page_title('<' . $g['name'] . '>');
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow">Gilde</p>
        <h1>&lt;<?= e($g['name']) ?>&gt;</h1>
        <p class="lead"><?= count($members) ?> Mitglieder · <?= $online ?> online<?= $g['leader'] ? ' · Gildenmeister ' . GameData::charBadge(['name' => $g['leader'], 'race' => $g['lrace'], 'class' => $g['lclass']]) : '' ?></p>
    </div>
</section>
<section class="wrap section">
    <?php if (trim((string)$g['info']) !== ''): ?>
        <div class="card"><h2 class="card-title"><?= icon('scroll') ?> Gildeninfo</h2><p><?= nl2br(e($g['info'])) ?></p></div>
    <?php endif; ?>
    <?php if ($gk): ?>
        <div class="card">
            <h2 class="card-title"><?= icon('g-trophy') ?> Raid-Fortschritt <a class="more small" href="<?= e(url('progress', ['realm' => $realmId])) ?>">Progress Race <?= icon('arrow-right') ?></a></h2>
            <div class="guild-progress">
                <?php foreach (Progress::RAIDS as $map => $raid): $n = count($gk[$map] ?? []); $max = count($raid['bosses']); if (!$n) continue; ?>
                    <div class="skillbar">
                        <div class="skillbar-label"><span><?= e($raid['name']) ?></span><span><?= $n ?>/<?= $max ?></span></div>
                        <progress max="<?= $max ?>" value="<?= $n ?>"></progress>
                    </div>
                <?php endforeach; ?>
            </div>
        </div>
    <?php endif; ?>
    <div class="card table-wrap">
        <table class="table" data-sortable>
            <thead><tr><th>Name</th><th data-sort="num">Stufe</th><th>Volk</th><th>Klasse</th><th>Rang</th></tr></thead>
            <tbody>
            <?php foreach ($members as $m): ?>
                <tr>
                    <td><a href="<?= e(url('character', ['realm' => $realmId, 'name' => $m['name']])) ?>"><?= GameData::charBadge($m) ?></a>
                        <?php if ($m['online']): ?><span class="badge badge-ok badge-xs">online</span><?php endif; ?></td>
                    <td><?= (int)$m['level'] ?></td>
                    <td><?= e(GameData::race((int)$m['race'])) ?></td>
                    <td class="cls-<?= GameData::classSlug((int)$m['class']) ?>"><?= e(GameData::className((int)$m['class'])) ?></td>
                    <td><?= e($m['rname'] ?? ('Rang ' . (int)$m['rank'])) ?></td>
                </tr>
            <?php endforeach; ?>
            </tbody>
        </table>
    </div>
</section>
