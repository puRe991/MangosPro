<?php
$id = (int)query('id', '0');
if ($id > 0) {
    $n = News::find($id);
    if (!$n) not_found();
    page_title($n['title']);
    ?>
    <section class="wrap section narrow-content">
        <p><a class="muted small" href="<?= e(url('news')) ?>">← Alle News</a></p>
        <article class="card article">
            <div class="news-meta">
                <span class="tag tag-<?= e($n['category']) ?>"><?= e(News::CATEGORIES[$n['category']] ?? 'News') ?></span>
                <time datetime="<?= e(date('c', strtotime($n['created_at']))) ?>"><?= e(fmt_date($n['created_at'], true)) ?></time>
                <span class="muted">von <?= e(ucfirst(strtolower($n['author']))) ?></span>
            </div>
            <h1><?= e($n['title']) ?></h1>
            <div class="prose"><?= News::render($n['body']) ?></div>
            <?php if ($n['updated_at']): ?><p class="muted small">Zuletzt bearbeitet: <?= e(fmt_date($n['updated_at'], true)) ?></p><?php endif; ?>
        </article>
    </section>
    <?php
    return;
}

page_title('News & Patch-Notes');
$perPage = 10;
$total = News::count();
$pages = max(1, (int)ceil($total / $perPage));
$page = min($pages, max(1, (int)query('page', '1')));
$items = News::latest($perPage, ($page - 1) * $perPage);
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow">Was gibt's Neues?</p>
        <h1>News &amp; Patch-Notes</h1>
    </div>
</section>
<section class="wrap section narrow-content">
    <?php if (!$items): ?><p class="muted">Noch keine Neuigkeiten.</p><?php endif; ?>
    <div class="news-list">
        <?php foreach ($items as $n): ?>
            <article class="news-card">
                <div class="news-meta">
                    <span class="tag tag-<?= e($n['category']) ?>"><?= e(News::CATEGORIES[$n['category']] ?? 'News') ?></span>
                    <?php if ($n['pinned']): ?><span class="tag tag-pinned">Angeheftet</span><?php endif; ?>
                    <time><?= e(fmt_date($n['created_at'])) ?></time>
                </div>
                <h2><a href="<?= e(url('news', ['id' => $n['id']])) ?>"><?= e($n['title']) ?></a></h2>
                <p class="muted"><?= e(News::excerpt($n['body'], 400)) ?></p>
                <a class="more" href="<?= e(url('news', ['id' => $n['id']])) ?>">Weiterlesen →</a>
            </article>
        <?php endforeach; ?>
    </div>
    <?php render_partial('pager', ['page' => $page, 'pages' => $pages, 'route' => 'news', 'params' => []]); ?>
</section>
