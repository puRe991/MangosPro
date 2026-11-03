<?php
declare(strict_types=1);

/** Website-Sitzung (PHP-Session) für eingeloggte Accounts. */
final class Auth
{
    public static function login(array $account): void
    {
        session_regenerate_id(true);
        $_SESSION['uid'] = (int)$account['id'];
        $_SESSION['uname'] = $account['username'];
        $_SESSION['gm'] = (int)$account['gmlevel'];
        $_SESSION['login_at'] = time();
        unset($_SESSION['csrf']);
    }

    public static function logout(): void
    {
        $_SESSION = [];
        if (ini_get('session.use_cookies')) {
            $p = session_get_cookie_params();
            setcookie(session_name(), '', time() - 42000, $p['path'], $p['domain'], $p['secure'], $p['httponly']);
        }
        session_destroy();
    }

    public static function check(): bool
    {
        return !empty($_SESSION['uid']);
    }

    public static function id(): int
    {
        return (int)($_SESSION['uid'] ?? 0);
    }

    public static function name(): string
    {
        return (string)($_SESSION['uname'] ?? '');
    }

    public static function isAdmin(): bool
    {
        return self::check() && (int)($_SESSION['gm'] ?? 0) >= (int)config('security.admin_gmlevel', 3);
    }

    /** gmlevel aus der DB nachladen (z. B. falls im Spiel geändert). */
    public static function refresh(): void
    {
        if (!self::check()) return;
        $acc = Account::find(self::id());
        if (!$acc) { self::logout(); return; }
        $_SESSION['gm'] = (int)$acc['gmlevel'];
    }

    public static function requireLogin(): void
    {
        if (!self::check()) {
            flash('info', 'Bitte melde dich zuerst an.');
            redirect('login', ['next' => current_route()]);
        }
    }

    public static function requireAdmin(): void
    {
        self::requireLogin();
        self::refresh();
        if (!self::isAdmin()) not_found();
    }
}
