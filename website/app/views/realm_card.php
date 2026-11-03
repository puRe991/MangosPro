<?php /** @var array $realm */
$p = $realm['players'];
$total = max(1, $p['alliance'] + $p['horde']);
$allyPct = (int)round($p['alliance'] / $total * 100);
?>
<article class="realm-card <?= $realm['online'] ? 'is-online' : 'is-offline' ?>">
    <header>
        <span class="status-dot" aria-hidden="true"></span>
        <div>
            <h3><?= e($realm['name']) ?></h3>
            <p class="muted small"><?= e($realm['type']) ?><?= $realm['rates'] ? ' · ' . e($realm['rates']) : '' ?></p>
        </div>
        <span class="badge <?= $realm['online'] ? 'badge-ok' : 'badge-off' ?>"><?= $realm['online'] ? 'Online' : 'Offline' ?></span>
    </header>
    <?php if ($realm['online']): ?>
        <dl class="realm-stats">
            <div><dt>Spieler online</dt><dd><?= fmt_number($p['total']) ?></dd></div>
            <?php if (config('bots.hide', true)): ?>
                <div><dt>Bots aktiv</dt><dd><?= fmt_number($p['bots']) ?></dd></div>
            <?php endif; ?>
            <div><dt>Uptime</dt><dd><?= $realm['uptime'] ? e(fmt_duration($realm['uptime'])) : '–' ?></dd></div>
        </dl>
        <?php if ($p['total'] > 0): ?>
            <div class="faction-bar" role="img" aria-label="Allianz <?= $allyPct ?> %, Horde <?= 100 - $allyPct ?> %">
                <span class="ally" data-w="<?= $allyPct ?>"><?= $allyPct ?>%</span>
                <span class="horde" data-w="<?= 100 - $allyPct ?>"><?= 100 - $allyPct ?>%</span>
            </div>
        <?php endif; ?>
    <?php else: ?>
        <p class="muted small">Der Realm ist gerade nicht erreichbar – vermutlich Wartung oder Neustart.</p>
    <?php endif; ?>
</article>
