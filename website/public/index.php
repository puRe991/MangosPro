<?php
/**
 * MangosPro Website – Front-Controller. DocumentRoot von Apache zeigt auf dieses Verzeichnis (public/).
 */
declare(strict_types=1);

define('WEB_PUBLIC_DIR', __DIR__);
// Liegt app/ woanders (z. B. /var/www/mangospro-app), hier anpassen:
require dirname(__DIR__) . '/app/bootstrap.php';

$routes = [
    ''                 => 'home',
    'news'             => 'news',
    'register'         => 'register',
    'login'            => 'login',
    'logout'           => 'logout',
    'forgot'           => 'forgot',
    'reset'            => 'reset',
    'account'          => 'account',
    'account/password' => 'account_password',
    'account/email'    => 'account_email',
    'connect'          => 'connect',
    'status'           => 'status',
    'online'           => 'online',
    'ladder'           => 'ladder',
    'armory'           => 'armory',
    'character'        => 'character',
    'guild'            => 'guild',
    'features'         => 'features',
    'rules'            => 'rules',
    'impressum'        => 'impressum',
    'privacy'          => 'privacy',
    'admin'            => 'admin/index',
    'admin/news'       => 'admin/news',
    'admin/news/edit'  => 'admin/news_edit',
    'api/status'       => 'api_status',
];

if (config('site.pretty_urls', true) && !isset($_GET['p'])) {
    $path = parse_url($_SERVER['REQUEST_URI'] ?? '/', PHP_URL_PATH) ?: '/';
    $base = parse_url(base_url(), PHP_URL_PATH) ?: '';
    if ($base !== '' && str_starts_with($path, $base)) $path = substr($path, strlen($base));
    $route = trim(rawurldecode($path), '/');
    if ($route === 'index.php') $route = '';
} else {
    $route = trim(query('p'), '/');
}

$GLOBALS['__route'] = $route;
$page = $routes[$route] ?? null;

$render = function (string $content): void {
    require APP_DIR . '/views/layout.php';
};

ob_start();
try {
    if ($page === null) not_found();
    require APP_DIR . '/pages/' . $page . '.php';
} catch (NotFoundException) {
    ob_clean();
    http_response_code(404);
    page_title('Seite nicht gefunden');
    require APP_DIR . '/pages/404.php';
} catch (PDOException $ex) {
    ob_clean();
    error_log('[website] DB-Fehler: ' . $ex->getMessage());
    http_response_code(503);
    page_title('Datenbank nicht erreichbar');
    $dbError = config('debug') ? $ex->getMessage() : null;
    require APP_DIR . '/pages/error.php';
}
$render((string)ob_get_clean());
