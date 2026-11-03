<?php
page_title('');
$realms = Realm::all();
$news = News::latest(4);
$online = array_sum(array_map(fn($r) => $r['players']['total'], $realms));
$bots = array_sum(array_map(fn($r) => $r['players']['bots'], $realms));
$anyOnline = (bool)array_filter($realms, fn($r) => $r['online']);
try { $accounts = Account::totalCount(); } catch (PDOException) { $accounts = 0; }
?>
<section class="hero">
    <div class="hero-bg" aria-hidden="true"></div>
    <div class="wrap hero-inner">
        <p class="eyebrow">The Burning Crusade · Patch 2.4.3</p>
        <h1>Das Portal ist offen.<br><span class="accent">Die Scherbenwelt wartet.</span></h1>
        <p class="lead"><?= e(config('site.tagline')) ?>. Stell dir deine Gruppe zusammen – mit Freunden oder mit
            Playerbots, die Boss-Taktiken wirklich kennen.</p>
        <div class="hero-cta">
            <?php if (Auth::check()): ?>
                <a class="btn btn-primary btn-lg" href="<?= e(url('connect')) ?>">Jetzt verbinden</a>
                <a class="btn btn-ghost btn-lg" href="<?= e(url('account')) ?>">Mein Account</a>
            <?php else: ?>
                <a class="btn btn-primary btn-lg" href="<?= e(url('register')) ?>">Kostenlos spielen</a>
                <a class="btn btn-ghost btn-lg" href="<?= e(url('connect')) ?>">So verbindest du dich</a>
            <?php endif; ?>
        </div>
        <ul class="hero-stats">
            <li><strong class="<?= $anyOnline ? 'ok' : 'off' ?>"><?= $anyOnline ? 'Online' : 'Offline' ?></strong><span>Serverstatus</span></li>
            <li><strong><?= fmt_number($online) ?></strong><span>Spieler online</span></li>
            <?php if (config('bots.hide', true)): ?>
                <li><strong><?= fmt_number($bots) ?></strong><span>Bots unterwegs</span></li>
            <?php endif; ?>
            <li><strong><?= fmt_number($accounts) ?></strong><span>Accounts</span></li>
        </ul>
    </div>
</section>

<section class="wrap section grid-main">
    <div>
        <div class="section-head">
            <h2>Neuigkeiten</h2>
            <a class="more" href="<?= e(url('news')) ?>">Alle News →</a>
        </div>
        <?php if (!$news): ?>
            <p class="muted">Noch keine Neuigkeiten.</p>
        <?php endif; ?>
        <div class="news-list">
            <?php foreach ($news as $i => $n): ?>
                <article class="news-card<?= $i === 0 ? ' featured' : '' ?>">
                    <div class="news-meta">
                        <span class="tag tag-<?= e($n['category']) ?>"><?= e(News::CATEGORIES[$n['category']] ?? 'News') ?></span>
                        <?php if ($n['pinned']): ?><span class="tag tag-pinned">Angeheftet</span><?php endif; ?>
                        <time datetime="<?= e(date('c', strtotime($n['created_at']))) ?>"><?= e(fmt_date($n['created_at'])) ?></time>
                    </div>
                    <h3><a href="<?= e(url('news', ['id' => $n['id']])) ?>"><?= e($n['title']) ?></a></h3>
                    <?php if ($i === 0): ?>
                        <div class="prose"><?= News::render($n['body']) ?></div>
                    <?php else: ?>
                        <p class="muted"><?= e(News::excerpt($n['body'])) ?></p>
                    <?php endif; ?>
                </article>
            <?php endforeach; ?>
        </div>
    </div>

    <aside class="sidebar">
        <div class="panel">
            <h2 class="panel-title">Realms</h2>
            <?php if (!$realms): ?>
                <p class="muted small">Keine Realms gefunden. Prüfe die realmd-Verbindung in der Konfiguration.</p>
            <?php endif; ?>
            <?php foreach ($realms as $realm) render_partial('realm_card', ['realm' => $realm]); ?>
            <a class="btn btn-ghost btn-block btn-sm" href="<?= e(url('status')) ?>">Details zum Status</a>
        </div>
        <div class="panel">
            <h2 class="panel-title">In drei Schritten ins Spiel</h2>
            <ol class="steps">
                <li><strong>Account erstellen</strong><span>Name, Passwort, fertig – dauert 30 Sekunden.</span></li>
                <li><strong>Client 2.4.3 bereithalten</strong><span>Version <?= e(config('site.client')) ?>, englisch oder deutsch.</span></li>
                <li><strong>Realmlist eintragen</strong><span><code>set realmlist <?= e(config('site.realmlist')) ?></code></span></li>
            </ol>
            <a class="btn btn-primary btn-block" href="<?= e(url(Auth::check() ? 'connect' : 'register')) ?>">Los geht's</a>
        </div>
    </aside>
</section>

<section class="features-band">
    <div class="wrap section">
        <div class="section-head center">
            <h2>Warum <?= e(config('site.name')) ?>?</h2>
            <p class="muted">Ein TBC-Server, der auch mit kleiner Spielerzahl lebendig ist.</p>
        </div>
        <div class="feature-grid">
            <div class="feature">
                <div class="feature-icon" aria-hidden="true">⚔</div>
                <h3>Raid-fähige Playerbots</h3>
                <p>Bots kennen die Mechaniken von Karazhan bis Sunwell: Positionierung, Interrupts, Spread, Tank-Wechsel.</p>
            </div>
            <div class="feature">
                <div class="feature-icon" aria-hidden="true">🛡</div>
                <h3>Stabiler CMaNGOS-Core</h3>
                <p>Bewährte Open-Source-Basis mit sauberem Scripting für Dungeons, Raids und Schlachtfelder.</p>
            </div>
            <div class="feature">
                <div class="feature-icon" aria-hidden="true">✦</div>
                <h3>Blizzlike-Gefühl</h3>
                <p>Originale Raten und Progression – die Scherbenwelt so, wie du sie in Erinnerung hast.</p>
            </div>
            <div class="feature">
                <div class="feature-icon" aria-hidden="true">⌂</div>
                <h3>Fair &amp; kostenlos</h3>
                <p>Kein Shop, kein Pay-to-Win. Ein Community-Projekt aus Spaß an der Sache.</p>
            </div>
        </div>
        <p class="center"><a class="btn btn-ghost" href="<?= e(url('features')) ?>">Mehr über den Server</a></p>
    </div>
</section>
