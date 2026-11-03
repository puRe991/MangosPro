<?php
Auth::requireAdmin();
page_title('News verwalten');

if (is_post()) {
    require_csrf();
    if (post('action') === 'delete' && ($id = (int)post('id'))) {
        News::delete($id);
        flash('success', 'Beitrag gelöscht.');
    }
    redirect('admin/news');
}
$items = News::all();
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow">Verwaltung</p>
        <h1>News verwalten</h1>
    </div>
</section>
<section class="wrap section">
    <nav class="segmented">
        <a href="<?= e(url('admin')) ?>">Übersicht</a>
        <a class="active" href="<?= e(url('admin/news')) ?>">News</a>
        <a href="<?= e(url('admin/progress')) ?>">Progress</a>
    </nav>
    <p><a class="btn btn-primary" href="<?= e(url('admin/news/edit')) ?>">+ Neuer Beitrag</a></p>
    <div class="card table-wrap">
        <?php if (!$items): ?><p class="empty">Noch keine Beiträge.</p><?php else: ?>
        <table class="table">
            <thead><tr><th>Titel</th><th>Kategorie</th><th>Autor</th><th>Datum</th><th>Status</th><th></th></tr></thead>
            <tbody>
            <?php foreach ($items as $n): ?>
                <tr>
                    <td><a href="<?= e(url('admin/news/edit', ['id' => $n['id']])) ?>"><?= e($n['title']) ?></a><?= $n['pinned'] ? ' <span class="tag tag-pinned">Angeheftet</span>' : '' ?></td>
                    <td><?= e(News::CATEGORIES[$n['category']] ?? $n['category']) ?></td>
                    <td><?= e($n['author']) ?></td>
                    <td class="nowrap"><?= e(fmt_date($n['created_at'], true)) ?></td>
                    <td><?= $n['published'] ? '<span class="badge badge-ok badge-xs">veröffentlicht</span>' : '<span class="badge badge-off badge-xs">Entwurf</span>' ?></td>
                    <td class="nowrap actions">
                        <a class="btn btn-ghost btn-sm" href="<?= e(url('admin/news/edit', ['id' => $n['id']])) ?>">Bearbeiten</a>
                        <form method="post" action="<?= e(url('admin/news')) ?>" class="inline" data-confirm="Beitrag wirklich löschen?">
                            <?= csrf_field() ?>
                            <input type="hidden" name="action" value="delete">
                            <input type="hidden" name="id" value="<?= (int)$n['id'] ?>">
                            <button class="btn btn-danger btn-sm" type="submit">Löschen</button>
                        </form>
                    </td>
                </tr>
            <?php endforeach; ?>
            </tbody>
        </table>
        <?php endif; ?>
    </div>
</section>
