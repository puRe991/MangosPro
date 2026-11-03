<?php /** @var int $page @var int $pages @var string $route @var array $params */
if ($pages <= 1) return; ?>
<nav class="pager" aria-label="Seiten">
    <?php if ($page > 1): ?><a href="<?= e(url($route, $params + ['page' => $page - 1])) ?>">← Zurück</a><?php endif; ?>
    <?php for ($i = max(1, $page - 3); $i <= min($pages, $page + 3); $i++): ?>
        <?php if ($i === $page): ?><span class="current" aria-current="page"><?= $i ?></span>
        <?php else: ?><a href="<?= e(url($route, $params + ['page' => $i])) ?>"><?= $i ?></a><?php endif; ?>
    <?php endfor; ?>
    <?php if ($page < $pages): ?><a href="<?= e(url($route, $params + ['page' => $page + 1])) ?>">Weiter →</a><?php endif; ?>
</nav>
