<?php
page_title('Updates & Patch-Notes');
try {
    $items = Database::web()->query(
        "SELECT id, title, body, author, created_at FROM web_news WHERE published = 1 AND category = 'update'
         ORDER BY created_at DESC")->fetchAll();
} catch (PDOException $ex) {
    error_log('[website] updates: ' . $ex->getMessage());
    $items = [];
}
// "Update 1.3 – Titel" -> Version + Titel
$split = function (string $title): array {
    if (preg_match('/^(?:Update|Patch|Version)\s+v?([\d.]+)\s*[–—:-]\s*(.+)$/u', $title, $m)) return [$m[1], $m[2]];
    return [null, $title];
};
$tagHeadings = function (string $html): string {
    $map = ['neu' => 'new', 'verbessert' => 'improved', 'geändert' => 'improved', 'behoben' => 'fixed',
            'bekannte probleme' => 'known', 'website' => 'web', 'core' => 'core', 'playerbots' => 'bots', 'hinweis' => 'note'];
    return preg_replace_callback('#<h3>([^<]+)</h3>#u', function ($m) use ($map) {
        $key = mb_strtolower(trim(preg_replace('/[^\p{L} ]/u', '', $m[1])));
        foreach ($map as $word => $cls) if (str_starts_with($key, $word)) return '<h3 class="cl-' . $cls . '">' . $m[1] . '</h3>';
        return $m[0];
    }, $html);
};
$current = $items ? $split($items[0]['title'])[0] : null;
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow">Changelog</p>
        <h1>Updates &amp; Patch-Notes</h1>
        <p class="lead">Alle Änderungen am Server – Core, Playerbots, Raids und Website.
            <?php if ($current): ?>Aktuelle Version: <strong class="accent">v<?= e($current) ?></strong><?php endif; ?></p>
    </div>
</section>

<section class="wrap section updates-layout">
    <aside class="updates-nav">
        <div class="panel">
            <h2 class="panel-title">Versionen</h2>
            <?php if (!$items): ?><p class="muted small">Noch keine Updates.</p><?php endif; ?>
            <ul>
                <?php foreach ($items as $n): [$ver, $t] = $split($n['title']); ?>
                    <li><a href="#u<?= (int)$n['id'] ?>"><?= $ver ? '<strong>v' . e($ver) . '</strong> ' : '' ?><span class="muted small"><?= e(fmt_date($n['created_at'])) ?></span></a></li>
                <?php endforeach; ?>
            </ul>
        </div>
    </aside>

    <div class="changelog">
        <?php if (!$items): ?>
            <div class="card empty"><p>Noch keine Patch-Notes veröffentlicht.</p></div>
        <?php endif; ?>
        <?php foreach ($items as $i => $n): [$ver, $t] = $split($n['title']); ?>
            <article class="card release" id="u<?= (int)$n['id'] ?>">
                <header class="release-head">
                    <?php if ($ver): ?><span class="version<?= $i === 0 ? ' latest' : '' ?>">v<?= e($ver) ?></span><?php endif; ?>
                    <div>
                        <h2><a href="<?= e(url('news', ['id' => $n['id']])) ?>"><?= e($t) ?></a></h2>
                        <p class="muted small"><time datetime="<?= e(date('c', strtotime($n['created_at']))) ?>"><?= e(fmt_date($n['created_at'])) ?></time>
                            <?= $i === 0 ? ' · <span class="tag tag-pinned">Aktuell</span>' : '' ?></p>
                    </div>
                </header>
                <div class="prose changelog-body"><?= $tagHeadings(News::render($n['body'])) ?></div>
            </article>
        <?php endforeach; ?>
    </div>
</section>
