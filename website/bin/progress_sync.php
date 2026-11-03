<?php
// Progress Race abgleichen (für Cron), z. B. alle 2 Minuten in /etc/cron.d/mangospro:
//   */2 * * * *  www-data  php /var/www/mangospro/bin/progress_sync.php
// Dann in config.php 'progress' => ['auto_sync' => false] setzen.
declare(strict_types=1);
if (PHP_SAPI !== 'cli') { http_response_code(403); exit; }
require dirname(__DIR__) . '/app/bootstrap.php';

$total = 0;
foreach (array_keys((array)config('realms', [])) as $realmId) {
    if (!config("realms.$realmId.characters")) continue;
    try {
        $n = Progress::sync((int)$realmId);
        $total += $n;
        echo "Realm $realmId: $n neue Kills\n";
    } catch (Throwable $ex) {
        fwrite(STDERR, "Realm $realmId: " . $ex->getMessage() . "\n");
    }
}
exit(0);
