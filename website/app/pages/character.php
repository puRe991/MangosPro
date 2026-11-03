<?php
$realmId = Realm::selected();
$name = query('name');
if ($name === '' || mb_strlen($name) > 12) not_found();
$db = Database::chars($realmId);
$c = Database::run($db,
    'SELECT c.guid, c.account, c.name, c.race, c.class, c.gender, c.level, c.zone, c.online, c.totaltime,
            c.totalKills, c.totalHonorPoints, c.arenaPoints, g.guildid, g.name AS guild
     FROM characters c LEFT JOIN guild_member gm ON gm.guid = c.guid LEFT JOIN guild g ON g.guildid = gm.guildid
     WHERE c.name = ? AND c.deleteInfos_Account IS NULL', [$name])->fetch();
if (!$c) not_found();
$isBot = in_array((int)$c['account'], Account::botAccountIds(), true);

$teams = Database::run($db,
    'SELECT t.name, t.type, s.rating FROM arena_team_member m JOIN arena_team t ON t.arenateamid = m.arenateamid
     JOIN arena_team_stats s ON s.arenateamid = t.arenateamid WHERE m.guid = ? ORDER BY t.type', [(int)$c['guid']])->fetchAll();

$professions = [];
try {
    // Hauptberufe (SkillLine-IDs)
    $profNames = [164 => 'Schmiedekunst', 165 => 'Lederverarbeitung', 171 => 'Alchemie', 182 => 'Kräuterkunde',
        186 => 'Bergbau', 197 => 'Schneiderei', 202 => 'Ingenieurskunst', 333 => 'Verzauberkunst', 393 => 'Kürschnerei',
        755 => 'Juwelierskunst', 129 => 'Erste Hilfe', 185 => 'Kochkunst', 356 => 'Angeln'];
    $rows = Database::run($db, 'SELECT skill, value, max FROM character_skills WHERE guid = ? AND skill IN (' . implode(',', array_keys($profNames)) . ')',
        [(int)$c['guid']])->fetchAll();
    foreach ($rows as $r) $professions[] = ['name' => $profNames[(int)$r['skill']], 'value' => (int)$r['value'], 'max' => (int)$r['max']];
    usort($professions, fn($a, $b) => $b['value'] <=> $a['value']);
} catch (PDOException) {}

page_title($c['name']);
$faction = GameData::faction((int)$c['race']);
?>
<section class="page-head char-head <?= e($faction) ?>">
    <div class="wrap char-head-inner">
        <div class="char-crest cls-bg-<?= GameData::classSlug((int)$c['class']) ?>" aria-hidden="true"><?= icon('c-' . GameData::classSlug((int)$c['class'])) ?></div>
        <div>
            <p class="eyebrow"><?= e(GameData::factionName((int)$c['race'])) ?><?= $isBot ? ' · Playerbot' : '' ?></p>
            <h1 class="cls-<?= GameData::classSlug((int)$c['class']) ?>"><?= e($c['name']) ?></h1>
            <p class="lead">Stufe <?= (int)$c['level'] ?> <?= e(GameData::race((int)$c['race'])) ?> <?= e(GameData::className((int)$c['class'])) ?>
                <?php if ($c['guild']): ?> · <a href="<?= e(url('guild', ['realm' => $realmId, 'id' => $c['guildid']])) ?>">&lt;<?= e($c['guild']) ?>&gt;</a><?php endif; ?>
            </p>
            <p><span class="badge <?= $c['online'] ? 'badge-ok' : 'badge-off' ?>"><?= $c['online'] ? 'Online · ' . e(GameData::zone((int)$c['zone'])) : 'Offline' ?></span></p>
        </div>
    </div>
</section>
<section class="wrap section">
    <div class="stat-grid" data-stagger>
        <div class="stat"><span>Ehrenhafte Siege</span><strong><?= fmt_number($c['totalKills']) ?></strong></div>
        <div class="stat"><span>Ehrenpunkte</span><strong><?= fmt_number($c['totalHonorPoints']) ?></strong></div>
        <div class="stat"><span>Arenapunkte</span><strong><?= fmt_number($c['arenaPoints']) ?></strong></div>
        <div class="stat"><span>Spielzeit</span><strong><?= e(fmt_duration((int)$c['totaltime'])) ?></strong></div>
    </div>
    <div class="grid-2">
        <div class="card">
            <h2 class="card-title"><?= icon('gem') ?> Berufe</h2>
            <?php if (!$professions): ?><p class="muted">Keine Berufe erlernt.</p><?php endif; ?>
            <?php foreach ($professions as $p): ?>
                <div class="skillbar">
                    <div class="skillbar-label"><span><?= e($p['name']) ?></span><span><?= $p['value'] ?> / <?= $p['max'] ?></span></div>
                    <progress max="<?= max(1, $p['max']) ?>" value="<?= $p['value'] ?>"></progress>
                </div>
            <?php endforeach; ?>
        </div>
        <div class="card">
            <h2 class="card-title"><?= icon('g-swords') ?> Arena-Teams</h2>
            <?php if (!$teams): ?><p class="muted">Kein Arena-Team.</p><?php else: ?>
                <ul class="result-list">
                    <?php foreach ($teams as $t): ?>
                        <li><strong><?= (int)$t['type'] ?>v<?= (int)$t['type'] ?></strong> <?= e($t['name']) ?> <span class="muted small">Wertung <?= fmt_number($t['rating']) ?></span></li>
                    <?php endforeach; ?>
                </ul>
            <?php endif; ?>
        </div>
    </div>
</section>
