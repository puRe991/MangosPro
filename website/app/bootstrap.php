<?php
declare(strict_types=1);

define('APP_DIR', __DIR__);
define('PUBLIC_DIR', defined('WEB_PUBLIC_DIR') ? WEB_PUBLIC_DIR : dirname(__DIR__) . '/public');

if (PHP_VERSION_ID < 80100) {
    http_response_code(500);
    exit('MangosPro Website benötigt PHP 8.1 oder neuer.');
}

$configFile = APP_DIR . '/config.php';
if (!is_file($configFile)) {
    http_response_code(500);
    exit('Konfiguration fehlt: app/config.dist.php nach app/config.php kopieren und anpassen.');
}
$GLOBALS['__config'] = require $configFile;

require APP_DIR . '/lib/helpers.php';
require APP_DIR . '/lib/Database.php';
require APP_DIR . '/lib/Srp6.php';
require APP_DIR . '/lib/Account.php';
require APP_DIR . '/lib/Auth.php';
require APP_DIR . '/lib/RateLimiter.php';
require APP_DIR . '/lib/Mailer.php';
require APP_DIR . '/lib/GameData.php';
require APP_DIR . '/lib/Realm.php';
require APP_DIR . '/lib/News.php';

date_default_timezone_set((string)config('site.timezone', 'Europe/Berlin'));

if (config('debug')) {
    ini_set('display_errors', '1');
    error_reporting(E_ALL);
} else {
    ini_set('display_errors', '0');
}

if (PHP_SAPI !== 'cli') {
    $secure = str_starts_with(base_url(), 'https://');
    session_name('mpweb');
    session_set_cookie_params([
        'lifetime' => 0,
        'path'     => (parse_url(base_url(), PHP_URL_PATH) ?: '') . '/',
        'secure'   => $secure,
        'httponly' => true,
        'samesite' => 'Lax',
    ]);
    ini_set('session.use_strict_mode', '1');
    ini_set('session.use_only_cookies', '1');
    session_start();

    // Sitzungen nach 12 h Inaktivität verwerfen
    if (isset($_SESSION['last_seen']) && $_SESSION['last_seen'] < time() - 43200) {
        $_SESSION = [];
        session_regenerate_id(true);
    }
    $_SESSION['last_seen'] = time();

    header('X-Content-Type-Options: nosniff');
    header('X-Frame-Options: DENY');
    header('Referrer-Policy: strict-origin-when-cross-origin');
    header("Content-Security-Policy: default-src 'self'; img-src 'self' data:; style-src 'self'; script-src 'self'; frame-ancestors 'none'; form-action 'self'; base-uri 'self'");
    if ($secure) header('Strict-Transport-Security: max-age=31536000');
}
