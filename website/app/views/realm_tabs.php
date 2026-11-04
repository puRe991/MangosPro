<?php /** @var string $route @var int $current @var array $extra */
$configured = array_keys(array_filter((array)config('realms', []), fn($r) => !empty($r['characters'])));
if (count($configured) <= 1) return;
$names = array_map(fn($r) => $r['name'], Realm::all());
?>
<nav class="tabs" aria-label="Realm">
    <?php foreach ($configured as $rid): ?>
        <a href="<?= e(url($route, ['realm' => $rid] + ($extra ?? []))) ?>"<?= (int)$rid === $current ? ' class="active"' : '' ?>><?= e($names[$rid] ?? "Realm $rid") ?></a>
    <?php endforeach; ?>
</nav>
