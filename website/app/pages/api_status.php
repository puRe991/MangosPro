<?php
// JSON-Status für Widgets, Discord-Bots o. ä.: GET /api/status
$out = ['generated' => time(), 'realms' => []];
foreach (Realm::all() as $r) {
    $out['realms'][] = [
        'id'       => (int)$r['id'],
        'name'     => $r['name'],
        'online'   => (bool)$r['online'],
        'type'     => $r['type'],
        'uptime'   => (int)$r['uptime'],
        'players'  => $r['players'],
    ];
}
while (ob_get_level()) ob_end_clean();
header('Content-Type: application/json; charset=utf-8');
header('Cache-Control: public, max-age=30');
header('Access-Control-Allow-Origin: *');
echo json_encode($out, JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
exit;
