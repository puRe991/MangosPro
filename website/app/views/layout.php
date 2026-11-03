<?php
/** @var string $content */
$siteName = (string)config('site.name');
$title = page_title();
$route = current_route();
$nav = [
    ''         => 'Start',
    'news'     => 'News',
    'updates'  => 'Updates',
    'progress' => 'Progress',
    'ladder'   => 'Rangliste',
    'armory'   => 'Arsenal',
    'online'   => 'Online',
    'connect'  => 'Spielen',
];
$isActive = function (string $r) use ($route): bool {
    if ($r === '') return $route === '';
    return $route === $r || str_starts_with($route, $r . '/')
        || ($r === 'armory' && in_array($route, ['character', 'guild'], true));
};
?><!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title><?= e($title !== '' ? "$title · $siteName" : "$siteName – " . config('site.tagline')) ?></title>
    <meta name="description" content="<?= e(config('site.tagline')) ?>">
    <meta name="theme-color" content="#0b0d12">
    <link rel="icon" href="<?= e(asset('img/emblem.svg')) ?>" type="image/svg+xml">
    <link rel="preload" href="<?= e(base_url()) ?>/assets/fonts/InterDisplay-ExtraBold.woff2" as="font" type="font/woff2" crossorigin>
    <link rel="preload" href="<?= e(base_url()) ?>/assets/fonts/Inter-Regular.woff2" as="font" type="font/woff2" crossorigin>
    <link rel="stylesheet" href="<?= e(asset('css/style.css')) ?>">
    <script src="<?= e(asset('js/boot.js')) ?>"></script>
    <script src="<?= e(asset('js/app.js')) ?>" defer></script>
</head>
<body class="route-<?= e($route === '' ? 'home' : str_replace('/', '-', $route)) ?>">
<a class="skip" href="#main">Zum Inhalt springen</a>

<header class="topbar">
    <div class="wrap topbar-inner">
        <a class="brand" href="<?= e(url()) ?>">
            <img src="<?= e(asset('img/emblem.svg')) ?>" alt="" width="40" height="40">
            <span><?= e($siteName) ?></span>
        </a>

        <button class="nav-toggle" type="button" aria-expanded="false" aria-controls="mainnav" data-nav-toggle>
            <span></span><span></span><span></span><span class="sr">Menü</span>
        </button>

        <nav id="mainnav" class="mainnav" aria-label="Hauptnavigation">
            <ul class="nav-links">
                <?php foreach ($nav as $r => $label): ?>
                    <li><a href="<?= e(url($r)) ?>"<?= $isActive($r) ? ' class="active" aria-current="page"' : '' ?>><?= e($label) ?></a></li>
                <?php endforeach; ?>
            </ul>
            <div class="nav-user">
                <?php if (Auth::check()): ?>
                    <?php if (Auth::isAdmin()): ?>
                        <a class="btn btn-ghost btn-sm" href="<?= e(url('admin')) ?>"><?= icon('wrench') ?> Admin</a>
                    <?php endif; ?>
                    <a class="btn btn-ghost btn-sm" href="<?= e(url('account')) ?>"><?= icon('user') ?> <?= e(ucfirst(strtolower(Auth::name()))) ?></a>
                    <form method="post" action="<?= e(url('logout')) ?>" class="inline">
                        <?= csrf_field() ?>
                        <button class="btn btn-link btn-sm" type="submit" title="Abmelden"><?= icon('logout') ?><span class="sr">Abmelden</span></button>
                    </form>
                <?php else: ?>
                    <a class="btn btn-ghost btn-sm" href="<?= e(url('login')) ?>"><?= icon('key') ?> Anmelden</a>
                    <a class="btn btn-primary btn-sm" href="<?= e(url('register')) ?>">Account erstellen</a>
                <?php endif; ?>
            </div>
        </nav>
    </div>
</header>

<main id="main">
    <?php $flashes = take_flashes(); if ($flashes): ?>
        <div class="wrap flashes">
            <?php foreach ($flashes as $f): ?>
                <div class="alert alert-<?= e($f['type']) ?>" role="status"><?= icon(['success' => 'check', 'error' => 'x', 'info' => 'sparkle'][$f['type']] ?? 'sparkle') ?><span><?= e($f['msg']) ?></span></div>
            <?php endforeach; ?>
        </div>
    <?php endif; ?>
    <?= $content ?>
</main>

<footer class="footer">
    <div class="wrap footer-grid">
        <div>
            <a class="brand brand-sm" href="<?= e(url()) ?>">
                <img src="<?= e(asset('img/emblem.svg')) ?>" alt="" width="34" height="34">
                <span><?= e($siteName) ?></span>
            </a>
            <p class="muted small"><?= e(config('site.tagline')) ?></p>
            <p class="small"><a class="more" href="<?= e(url('register')) ?>">Jetzt kostenlos mitspielen <?= icon('arrow-right') ?></a></p>
        </div>
        <div>
            <h4>Spielen</h4>
            <ul>
                <li><a href="<?= e(url('register')) ?>">Account erstellen</a></li>
                <li><a href="<?= e(url('connect')) ?>">Verbindungsanleitung</a></li>
                <li><a href="<?= e(url('status')) ?>">Serverstatus</a></li>
                <li><a href="<?= e(url('features')) ?>">Über den Server</a></li>
                <li><a href="<?= e(url('rules')) ?>">Regeln</a></li>
            </ul>
        </div>
        <div>
            <h4>Community</h4>
            <ul>
                <li><a href="<?= e(url('news')) ?>">News</a></li>
                <li><a href="<?= e(url('updates')) ?>">Updates &amp; Patch-Notes</a></li>
                <li><a href="<?= e(url('progress')) ?>">Progress Race</a></li>
                <li><a href="<?= e(url('ladder')) ?>">Ranglisten</a></li>
                <li><a href="<?= e(url('armory')) ?>">Arsenal</a></li>
                <?php if (config('site.discord')): ?>
                    <li><a href="<?= e(config('site.discord')) ?>" rel="noopener" target="_blank">Discord</a></li>
                <?php endif; ?>
            </ul>
        </div>
        <div>
            <h4>Rechtliches</h4>
            <ul>
                <li><a href="<?= e(url('impressum')) ?>">Impressum</a></li>
                <li><a href="<?= e(url('privacy')) ?>">Datenschutz</a></li>
            </ul>
        </div>
    </div>
    <div class="wrap footer-legal small muted">
        <p>&copy; <?= date('Y') ?> <?= e($siteName) ?>. Ein privates, nicht-kommerzielles Fanprojekt auf Basis von CMaNGOS (GPL-2.0).
            World of Warcraft und The Burning Crusade sind Marken von Blizzard Entertainment, Inc.; dieses Projekt steht in keiner Verbindung zu Blizzard.</p>
        <p>Icons made by Lorc and Delapouite, available on <a href="https://game-icons.net" rel="noopener" target="_blank">game-icons.net</a>
            (<a href="https://creativecommons.org/licenses/by/3.0/" rel="noopener license" target="_blank">CC BY 3.0</a>, eingefärbt).
            Schriften: Inter, JetBrains Mono (SIL OFL).</p>
    </div>
</footer>
</body>
</html>
