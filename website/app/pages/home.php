<?php
page_title('');
$realms = Realm::all();
$news = News::latest(4);
$progressTop = [];
$firsts = [];
try {
    Progress::maybeSync(Realm::defaultId());
    $allKills = Progress::kills(Realm::defaultId());
    $progressTop = array_slice(Progress::ranking($allKills), 0, 5);
    $firsts = Progress::firsts($allKills);
} catch (Throwable) {}
$online = array_sum(array_map(fn($r) => $r['players']['total'], $realms));
$bots = array_sum(array_map(fn($r) => $r['players']['bots'], $realms));
$anyOnline = (bool)array_filter($realms, fn($r) => $r['online']);
try { $accounts = Account::totalCount(); } catch (PDOException) { $accounts = 0; }
$catIcon = ['news' => 'news', 'update' => 'scroll', 'event' => 'sparkle', 'maintenance' => 'wrench'];
?>
<section class="hero">
    <div class="hero-bg" aria-hidden="true"></div>
    <div class="hero-glow" aria-hidden="true"></div>
    <canvas class="embers" aria-hidden="true"></canvas>
    <div class="hero-shade" aria-hidden="true"></div>
    <div class="wrap hero-inner">
        <p class="eyebrow" data-reveal>The Burning Crusade · Patch 2.4.3</p>
        <h1 data-reveal>Das Portal ist offen.<br><span class="accent">Die Scherbenwelt wartet.</span></h1>
        <p class="lead" data-reveal><?= e(config('site.tagline')) ?>. Stell dir deine Gruppe zusammen – mit Freunden oder mit
            Playerbots, die Boss-Taktiken wirklich kennen.</p>
        <div class="hero-cta" data-reveal>
            <?php if (Auth::check()): ?>
                <a class="btn btn-primary btn-lg" href="<?= e(url('connect')) ?>"><?= icon('portal') ?> Jetzt verbinden</a>
                <a class="btn btn-ghost btn-lg" href="<?= e(url('account')) ?>"><?= icon('user') ?> Mein Account</a>
            <?php else: ?>
                <a class="btn btn-primary btn-lg" href="<?= e(url('register')) ?>"><?= icon('portal') ?> Kostenlos spielen</a>
                <a class="btn btn-ghost btn-lg" href="<?= e(url('connect')) ?>"><?= icon('book') ?> So verbindest du dich</a>
            <?php endif; ?>
        </div>
        <ul class="hero-stats" data-stagger>
            <li><span class="stat-ico <?= $anyOnline ? 'ok' : 'off' ?>"><?= icon('signal') ?></span>
                <span class="stat-txt"><strong class="<?= $anyOnline ? 'ok' : 'off' ?>"><?= $anyOnline ? 'Online' : 'Offline' ?></strong><span>Serverstatus</span></span></li>
            <li><span class="stat-ico"><?= icon('users') ?></span>
                <span class="stat-txt"><strong data-count="<?= (int)$online ?>"><?= fmt_number($online) ?></strong><span>Spieler online</span></span></li>
            <?php if (config('bots.hide', true)): ?>
                <li><span class="stat-ico"><?= icon('bot') ?></span>
                    <span class="stat-txt"><strong data-count="<?= (int)$bots ?>"><?= fmt_number($bots) ?></strong><span>Bots unterwegs</span></span></li>
            <?php endif; ?>
            <li><span class="stat-ico"><?= icon('shield') ?></span>
                <span class="stat-txt"><strong data-count="<?= (int)$accounts ?>"><?= fmt_number($accounts) ?></strong><span>Accounts</span></span></li>
        </ul>
    </div>
    <a class="scroll-cue" href="#raids" aria-label="Weiter nach unten"><?= icon('chevron-down') ?></a>
</section>

<section class="wrap section raid-showcase" id="raids">
    <div class="section-head center" data-reveal>
        <p class="eyebrow">Neun Raids · <?= array_sum(array_map(fn($r) => count($r['bosses']), Progress::RAIDS)) ?> Bosse</p>
        <h2>Von Karazhan bis zum Sonnenbrunnen</h2>
        <?= ornament() ?>
    </div>
    <div class="showcase-grid" data-stagger>
        <?php foreach (Progress::RAIDS as $map => $raid): $done = count($firsts[$map] ?? []); $max = count($raid['bosses']); ?>
            <a class="showcase-tile raid-card" data-raid="<?= (int)$map ?>" href="<?= e(url('progress')) ?>#raid-<?= (int)$map ?>">
                <span class="raid-emblem wow-icon"><?= icon('r-' . $map) ?></span>
                <span class="showcase-name"><?= e($raid['name']) ?></span>
                <span class="showcase-meta"><?= e(Progress::TIERS[$raid['tier']] ?? $raid['tier']) ?> · <?= (int)$raid['size'] ?>er</span>
                <?= ring($done, $max) ?>
            </a>
        <?php endforeach; ?>
    </div>
</section>

<section class="wrap section grid-main">
    <div>
        <div class="section-head" data-reveal>
            <div>
                <p class="eyebrow">Aktuelles</p>
                <h2>Neuigkeiten</h2>
            </div>
            <a class="more" href="<?= e(url('news')) ?>">Alle News <?= icon('arrow-right') ?></a>
        </div>
        <?php if (!$news): ?>
            <div class="card empty"><?= icon('news', 'icon-xl') ?><p>Noch keine Neuigkeiten.</p></div>
        <?php endif; ?>
        <div class="news-list" data-stagger>
            <?php foreach ($news as $i => $n): ?>
                <article class="news-card cat-<?= e($n['category']) ?><?= $i === 0 ? ' featured frame' : '' ?>">
                    <div class="news-meta">
                        <span class="tag tag-<?= e($n['category']) ?>"><?= icon($catIcon[$n['category']] ?? 'news') ?> <?= e(News::CATEGORIES[$n['category']] ?? 'News') ?></span>
                        <?php if ($n['pinned']): ?><span class="tag tag-pinned"><?= icon('star') ?> Angeheftet</span><?php endif; ?>
                        <time datetime="<?= e(date('c', strtotime($n['created_at']))) ?>"><?= icon('calendar') ?> <?= e(fmt_date($n['created_at'])) ?></time>
                    </div>
                    <h3><a href="<?= e(url('news', ['id' => $n['id']])) ?>"><?= e($n['title']) ?></a></h3>
                    <?php if ($i === 0): ?>
                        <div class="prose"><?= News::render($n['body']) ?></div>
                    <?php else: ?>
                        <p class="muted"><?= e(News::excerpt($n['body'])) ?></p>
                        <a class="more" href="<?= e(url('news', ['id' => $n['id']])) ?>">Weiterlesen <?= icon('arrow-right') ?></a>
                    <?php endif; ?>
                </article>
            <?php endforeach; ?>
        </div>
    </div>

    <aside class="sidebar">
        <div class="panel" data-reveal>
            <h2 class="panel-title"><?= icon('server') ?> Realms</h2>
            <?php if (!$realms): ?>
                <p class="muted small">Keine Realms gefunden. Prüfe die realmd-Verbindung in der Konfiguration.</p>
            <?php endif; ?>
            <?php foreach ($realms as $realm) render_partial('realm_card', ['realm' => $realm]); ?>
            <a class="btn btn-ghost btn-block btn-sm" href="<?= e(url('status')) ?>">Details zum Status</a>
        </div>
        <div class="panel" data-reveal>
            <h2 class="panel-title"><?= icon('g-trophy') ?> Progress Race</h2>
            <?php if (!$progressTop): ?>
                <p class="muted small">Noch hat keine Gilde einen Raidboss gelegt. Holt euch den ersten Realm-First!</p>
            <?php else: ?>
                <ol class="mini-ranking">
                    <?php foreach ($progressTop as $i => $g):
                        $best = null;
                        foreach (array_reverse(Progress::RAIDS, true) as $map => $raid) { if (!empty($g['raids'][$map])) { $best = [$raid, $g['raids'][$map]]; break; } } ?>
                        <li>
                            <span class="rank rank-<?= $i + 1 ?>"><?= $i + 1 ?></span>
                            <a href="<?= e(url('guild', ['id' => $g['guild_id']])) ?>">&lt;<?= e($g['name']) ?>&gt;</a>
                            <?php if ($best): ?><span class="prog <?= $best[1] >= count($best[0]['bosses']) ? 'prog-full' : 'prog-part' ?>"><?= $best[1] ?>/<?= count($best[0]['bosses']) ?> <?= e($best[0]['short']) ?></span><?php endif; ?>
                        </li>
                    <?php endforeach; ?>
                </ol>
            <?php endif; ?>
            <a class="btn btn-ghost btn-block btn-sm" href="<?= e(url('progress')) ?>">Zur Progress-Wertung</a>
        </div>
        <div class="panel frame" data-reveal>
            <h2 class="panel-title"><?= icon('g-portal') ?> In drei Schritten ins Spiel</h2>
            <ol class="steps">
                <li><strong>Account erstellen</strong><span>Name, Passwort, fertig – dauert 30 Sekunden.</span></li>
                <li><strong>Client 2.4.3 bereithalten</strong><span>Version <?= e(config('site.client')) ?>, englisch oder deutsch.</span></li>
                <li><strong>Realmlist eintragen</strong><span><code>set realmlist <?= e(config('site.realmlist')) ?></code></span></li>
            </ol>
            <a class="btn btn-primary btn-block" href="<?= e(url(Auth::check() ? 'connect' : 'register')) ?>">Los geht's <?= icon('arrow-right') ?></a>
        </div>
    </aside>
</section>

<section class="features-band">
    <div class="wrap section">
        <div class="section-head center" data-reveal>
            <p class="eyebrow">Was uns ausmacht</p>
            <h2>Warum <?= e(config('site.name')) ?>?</h2>
            <p class="muted">Ein TBC-Server, der auch mit kleiner Spielerzahl lebendig ist.</p>
            <?= ornament() ?>
        </div>
        <div class="feature-grid" data-stagger>
            <div class="feature">
                <div class="feature-icon wow-icon"><?= icon('g-bot') ?></div>
                <h3>Raid-fähige Playerbots</h3>
                <p>Bots kennen die Mechaniken von Karazhan bis Sunwell: Positionierung, Interrupts, Spread, Tank-Wechsel.</p>
            </div>
            <div class="feature">
                <div class="feature-icon wow-icon"><?= icon('g-castle') ?></div>
                <h3>Stabiler CMaNGOS-Core</h3>
                <p>Bewährte Open-Source-Basis mit sauberem Scripting für Dungeons, Raids und Schlachtfelder.</p>
            </div>
            <div class="feature">
                <div class="feature-icon wow-icon"><?= icon('g-swords') ?></div>
                <h3>Blizzlike-Gefühl</h3>
                <p>Originale Raten und Progression – die Scherbenwelt so, wie du sie in Erinnerung hast.</p>
            </div>
            <div class="feature">
                <div class="feature-icon wow-icon"><?= icon('g-gems') ?></div>
                <h3>Fair &amp; kostenlos</h3>
                <p>Kein Shop, kein Pay-to-Win. Ein Community-Projekt aus Spaß an der Sache.</p>
            </div>
        </div>
    </div>
</section>

<section class="wrap section">
    <div class="cta-banner frame" data-reveal>
        <img class="cta-emblem" src="<?= e(asset('img/emblem.svg')) ?>" alt="" width="120" height="120">
        <div>
            <p class="eyebrow">Bereit für die Scherbenwelt?</p>
            <h2>Dein Abenteuer beginnt in 30 Sekunden.</h2>
            <p class="muted">Account erstellen, Realmlist eintragen, einloggen – und deine eigene Raidgruppe aus Bots zusammenstellen.</p>
        </div>
        <div class="cta-actions">
            <a class="btn btn-primary btn-lg" href="<?= e(url(Auth::check() ? 'connect' : 'register')) ?>"><?= icon('portal') ?> <?= Auth::check() ? 'Jetzt verbinden' : 'Account erstellen' ?></a>
            <a class="btn btn-ghost" href="<?= e(url('features')) ?>">Mehr über den Server</a>
        </div>
    </div>
</section>
