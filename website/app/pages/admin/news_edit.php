<?php
Auth::requireAdmin();
$id = (int)query('id', post('id', '0'));
$item = $id ? News::find($id, false) : null;
if ($id && !$item) not_found();
page_title($item ? 'Beitrag bearbeiten' : 'Neuer Beitrag');

$data = $item ?: ['title' => '', 'category' => 'news', 'body' => '', 'published' => 1, 'pinned' => 0];
$errors = [];
if (is_post()) {
    require_csrf();
    $data = [
        'title'     => post('title'),
        'category'  => array_key_exists(post('category'), News::CATEGORIES) ? post('category') : 'news',
        'body'      => (string)($_POST['body'] ?? ''),
        'published' => isset($_POST['published']) ? 1 : 0,
        'pinned'    => isset($_POST['pinned']) ? 1 : 0,
        'author'    => Auth::name(),
    ];
    if ($data['title'] === '' || mb_strlen($data['title']) > 150) $errors['title'] = 'Titel fehlt oder ist länger als 150 Zeichen.';
    if (trim($data['body']) === '') $errors['body'] = 'Der Text darf nicht leer sein.';
    if (!$errors) {
        $newId = News::save($item ? $id : null, $data);
        flash('success', 'Beitrag gespeichert.');
        redirect('admin/news/edit', ['id' => $newId]);
    }
}
$err = fn(string $k) => isset($errors[$k]) ? '<p class="field-error">' . e($errors[$k]) . '</p>' : '';
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow"><a href="<?= e(url('admin/news')) ?>">← News verwalten</a></p>
        <h1><?= $item ? 'Beitrag bearbeiten' : 'Neuer Beitrag' ?></h1>
    </div>
</section>
<section class="wrap section grid-main">
    <form class="card" method="post" action="<?= e(url('admin/news/edit', $item ? ['id' => $id] : [])) ?>">
        <?= csrf_field() ?>
        <div class="field">
            <label for="title">Titel</label>
            <input id="title" name="title" type="text" required maxlength="150" value="<?= e($data['title']) ?>">
            <?= $err('title') ?>
        </div>
        <div class="field">
            <label for="category">Kategorie</label>
            <select id="category" name="category">
                <?php foreach (News::CATEGORIES as $k => $label): ?>
                    <option value="<?= e($k) ?>"<?= $data['category'] === $k ? ' selected' : '' ?>><?= e($label) ?></option>
                <?php endforeach; ?>
            </select>
        </div>
        <div class="field">
            <label for="body">Text</label>
            <textarea id="body" name="body" rows="16" required><?= e($data['body']) ?></textarea>
            <?= $err('body') ?>
        </div>
        <label class="check"><input type="checkbox" name="published" value="1"<?= $data['published'] ? ' checked' : '' ?>><span>Veröffentlicht</span></label>
        <label class="check"><input type="checkbox" name="pinned" value="1"<?= $data['pinned'] ? ' checked' : '' ?>><span>Oben anheften</span></label>
        <button class="btn btn-primary" type="submit">Speichern</button>
        <?php if ($item && $item['published']): ?>
            <a class="btn btn-ghost" href="<?= e(url('news', ['id' => $id])) ?>" target="_blank">Ansehen</a>
        <?php endif; ?>
    </form>
    <aside class="sidebar">
        <div class="panel">
            <h2 class="panel-title">Formatierung</h2>
            <ul class="small format-help">
                <li><code>**fett**</code> → <strong>fett</strong></li>
                <li><code>*kursiv*</code> → <em>kursiv</em></li>
                <li><code>`code`</code> → <code>code</code></li>
                <li><code>[Text](https://…)</code> → Link</li>
                <li><code>## Überschrift</code></li>
                <li><code>- Punkt</code> → Liste</li>
                <li>Leerzeile → neuer Absatz</li>
            </ul>
            <p class="muted small">HTML wird aus Sicherheitsgründen nicht interpretiert.</p>
        </div>
    </aside>
</section>
