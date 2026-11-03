<?php /** @var string|null $dbError */ ?>
<section class="wrap section center error-page">
    <p class="error-code">503</p>
    <h1>Die Datenbank antwortet gerade nicht.</h1>
    <p class="muted">Vermutlich läuft eine Wartung. Bitte versuche es in ein paar Minuten erneut.</p>
    <?php if (!empty($dbError)): ?><pre class="codebox small"><?= e($dbError) ?></pre><?php endif; ?>
    <p><a class="btn btn-ghost" href="<?= e(url()) ?>">Erneut versuchen</a></p>
</section>
