<?php
declare(strict_types=1);

function config(string $path, $default = null)
{
    $node = $GLOBALS['__config'] ?? [];
    foreach (explode('.', $path) as $key) {
        if (!is_array($node) || !array_key_exists($key, $node)) return $default;
        $node = $node[$key];
    }
    return $node;
}

/** HTML-Escaping */
function e($value): string
{
    return htmlspecialchars((string)$value, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8');
}

function base_url(): string
{
    $cfg = rtrim((string)config('site.base_url', ''), '/');
    if ($cfg !== '') return $cfg;
    $https = (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] !== 'off')
        || (($_SERVER['HTTP_X_FORWARDED_PROTO'] ?? '') === 'https' && is_trusted_proxy());
    $host = $_SERVER['HTTP_HOST'] ?? 'localhost';
    $dir = rtrim(str_replace('\\', '/', dirname($_SERVER['SCRIPT_NAME'] ?? '/')), '/');
    return ($https ? 'https' : 'http') . '://' . $host . $dir;
}

/** URL zu einer Route, z. B. url('account/password', ['x' => 1]) */
function url(string $route = '', array $query = []): string
{
    $route = trim($route, '/');
    if (config('site.pretty_urls', true)) {
        $u = base_url() . '/' . $route;
        return $query ? $u . '?' . http_build_query($query) : $u;
    }
    $q = $route === '' ? $query : ['p' => $route] + $query;
    return base_url() . '/index.php' . ($q ? '?' . http_build_query($q) : '');
}

function asset(string $path): string
{
    $file = PUBLIC_DIR . '/assets/' . $path;
    $v = is_file($file) ? substr((string)filemtime($file), -6) : '1';
    return base_url() . '/assets/' . $path . '?v=' . $v;
}

function redirect(string $route, array $query = []): never
{
    header('Location: ' . url($route, $query), true, 303);
    exit;
}

function is_post(): bool
{
    return ($_SERVER['REQUEST_METHOD'] ?? 'GET') === 'POST';
}

function post(string $key, string $default = ''): string
{
    $v = $_POST[$key] ?? $default;
    return is_string($v) ? trim($v) : $default;
}

function query(string $key, string $default = ''): string
{
    $v = $_GET[$key] ?? $default;
    return is_string($v) ? trim($v) : $default;
}

function is_trusted_proxy(): bool
{
    return in_array($_SERVER['REMOTE_ADDR'] ?? '', (array)config('security.trusted_proxies', []), true);
}

function client_ip(): string
{
    $ip = $_SERVER['REMOTE_ADDR'] ?? '0.0.0.0';
    if (is_trusted_proxy() && !empty($_SERVER['HTTP_X_FORWARDED_FOR'])) {
        $parts = array_map('trim', explode(',', $_SERVER['HTTP_X_FORWARDED_FOR']));
        $candidate = end($parts);
        if (filter_var($candidate, FILTER_VALIDATE_IP)) $ip = $candidate;
    }
    return $ip;
}

// ---------------------------------------------------------------- Flash / CSRF

function flash(string $type, string $message): void
{
    $_SESSION['flash'][] = ['type' => $type, 'msg' => $message];
}

function take_flashes(): array
{
    $f = $_SESSION['flash'] ?? [];
    unset($_SESSION['flash']);
    return $f;
}

function csrf_token(): string
{
    if (empty($_SESSION['csrf'])) $_SESSION['csrf'] = bin2hex(random_bytes(32));
    return $_SESSION['csrf'];
}

function csrf_field(): string
{
    return '<input type="hidden" name="_csrf" value="' . e(csrf_token()) . '">';
}

function csrf_check(): bool
{
    $t = $_POST['_csrf'] ?? '';
    return is_string($t) && $t !== '' && hash_equals(csrf_token(), $t);
}

/** Bricht POST-Anfragen mit ungültigem Token ab. */
function require_csrf(): void
{
    if (is_post() && !csrf_check()) {
        http_response_code(400);
        flash('error', 'Sitzung abgelaufen oder ungültiges Formular. Bitte erneut versuchen.');
        redirect(current_route());
    }
}

function current_route(): string
{
    return $GLOBALS['__route'] ?? '';
}

// ---------------------------------------------------------------- Captcha (lokal, ohne Drittanbieter)

function captcha_question(string $form): string
{
    $a = random_int(2, 9);
    $b = random_int(1, 9);
    $_SESSION['captcha'][$form] = $a + $b;
    return "Wie viel ist $a + $b?";
}

function captcha_valid(string $form, string $answer): bool
{
    $expected = $_SESSION['captcha'][$form] ?? null;
    unset($_SESSION['captcha'][$form]);
    return $expected !== null && ctype_digit($answer) && (int)$answer === $expected;
}

// ---------------------------------------------------------------- Formatierung

function fmt_number($n): string
{
    return number_format((float)$n, 0, ',', '.');
}

function fmt_date($ts, bool $time = false): string
{
    if (!is_numeric($ts)) $ts = strtotime((string)$ts) ?: 0;
    return date($time ? 'd.m.Y H:i' : 'd.m.Y', (int)$ts);
}

function fmt_duration(int $seconds): string
{
    $d = intdiv($seconds, 86400);
    $h = intdiv($seconds % 86400, 3600);
    $m = intdiv($seconds % 3600, 60);
    if ($d > 0) return "{$d} T {$h} Std";
    if ($h > 0) return "{$h} Std {$m} Min";
    return "{$m} Min";
}

/** Kupfer -> "12g 34s 56k" als HTML */
function fmt_money(int $copper): string
{
    $g = intdiv($copper, 10000);
    $s = intdiv($copper % 10000, 100);
    $c = $copper % 100;
    return '<span class="coin gold">' . fmt_number($g) . '</span> <span class="coin silver">' . $s
        . '</span> <span class="coin copper">' . $c . '</span>';
}

function render_partial(string $name, array $vars = []): void
{
    extract($vars, EXTR_SKIP);
    require APP_DIR . '/views/' . $name . '.php';
}

final class NotFoundException extends RuntimeException {}

function not_found(): never
{
    throw new NotFoundException();
}

function page_title(?string $title = null): string
{
    if ($title !== null) $GLOBALS['__page_title'] = $title;
    return $GLOBALS['__page_title'] ?? '';
}

// ---------------------------------------------------------------- Grafik-Helfer

/** Icon aus dem SVG-Sprite (assets/img/icons.svg), z. B. icon('sword') oder icon('c-mage', 'icon-lg') */
function icon(string $name, string $class = ''): string
{
    static $sprite = null;
    $sprite ??= asset('img/icons.svg');
    $id = preg_match('/^[cr]-/', $name) ? $name : 'i-' . $name;
    return '<svg class="icon' . ($class !== '' ? ' ' . e($class) : '') . '" aria-hidden="true" focusable="false"><use href="'
        . e($sprite) . '#' . e($id) . '"></use></svg>';
}

/** Fortschrittsring als SVG; $value/$max, Beschriftung in der Mitte */
function ring(int $value, int $max, string $label = '', string $class = ''): string
{
    $pct = $max > 0 ? max(0, min(100, round($value / $max * 100, 1))) : 0;
    $label = $label !== '' ? $label : $value . '/' . $max;
    return '<span class="ring' . ($class !== '' ? ' ' . e($class) : '') . ($pct >= 100 ? ' ring-full' : '') . '" role="img" aria-label="' . e("$value von $max") . '">'
        . '<svg viewBox="0 0 40 40" aria-hidden="true"><circle class="ring-track" cx="20" cy="20" r="17" pathLength="100"/>'
        . '<circle class="ring-value" cx="20" cy="20" r="17" pathLength="100" stroke-dasharray="' . $pct . ' 100"/></svg>'
        . '<span class="ring-label">' . e($label) . '</span></span>';
}

/** Ornament-Trenner unter Überschriften */
function ornament(string $class = ''): string
{
    return '<span class="ornament' . ($class !== '' ? ' ' . e($class) : '') . '" aria-hidden="true"><svg viewBox="0 0 120 12"><path d="M0 6h48M72 6h48" /><path d="M60 1l5 5-5 5-5-5z" /><circle cx="50" cy="6" r="1.4"/><circle cx="70" cy="6" r="1.4"/></svg></span>';
}
