<?php
declare(strict_types=1);

/** Einfaches Rate-Limiting über die Tabelle web_attempts. */
final class RateLimiter
{
    public static function tooMany(string $action, string $ip, int $max, int $window): bool
    {
        $n = (int)Database::run(Database::web(),
            'SELECT COUNT(*) FROM web_attempts WHERE action = ? AND ip = ? AND created_at > ?',
            [$action, $ip, time() - $window])->fetchColumn();
        return $n >= $max;
    }

    public static function hit(string $action, string $ip): void
    {
        Database::run(Database::web(), 'INSERT INTO web_attempts (action, ip, created_at) VALUES (?, ?, ?)',
            [$action, $ip, time()]);
        // gelegentlich aufräumen
        if (random_int(1, 50) === 1) {
            Database::run(Database::web(), 'DELETE FROM web_attempts WHERE created_at < ?', [time() - 86400 * 2]);
        }
    }

    public static function clear(string $action, string $ip): void
    {
        Database::run(Database::web(), 'DELETE FROM web_attempts WHERE action = ? AND ip = ?', [$action, $ip]);
    }
}
