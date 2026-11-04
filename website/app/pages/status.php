<?php
page_title('Serverstatus');
$realms = Realm::all();
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow">Live</p>
        <h1>Serverstatus</h1>
        <p class="muted">Wird alle 30 Sekunden aktualisiert.</p>
    </div>
</section>
<section class="wrap section">
    <?php if (!$realms): ?>
        <div class="alert alert-error">Es konnten keine Realms geladen werden.</div>
    <?php endif; ?>
    <div class="realm-grid" data-stagger>
        <?php foreach ($realms as $realm): ?>
            <div>
                <?php render_partial('realm_card', ['realm' => $realm]); ?>
                <div class="card">
                    <dl class="kv">
                        <div><dt>Realm-ID</dt><dd><?= (int)$realm['id'] ?></dd></div>
                        <div><dt>Typ</dt><dd><?= e($realm['type']) ?></dd></div>
                        <?php if ($realm['rates']): ?><div><dt>Raten</dt><dd><?= e($realm['rates']) ?></dd></div><?php endif; ?>
                        <div><dt>Allianz online</dt><dd><?= fmt_number($realm['players']['alliance']) ?></dd></div>
                        <div><dt>Horde online</dt><dd><?= fmt_number($realm['players']['horde']) ?></dd></div>
                        <div><dt>Rekord (gleichzeitig)</dt><dd><?= fmt_number(Realm::maxPlayers((int)$realm['id'])) ?></dd></div>
                    </dl>
                    <?php if ($realm['description']): ?><p class="muted"><?= e($realm['description']) ?></p><?php endif; ?>
                    <?php if ($realm['has_chars']): ?>
                        <a class="btn btn-ghost btn-sm" href="<?= e(url('online', ['realm' => $realm['id']])) ?>">Wer ist online?</a>
                    <?php endif; ?>
                </div>
            </div>
        <?php endforeach; ?>
    </div>
</section>
