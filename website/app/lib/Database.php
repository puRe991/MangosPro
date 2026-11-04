<?php
declare(strict_types=1);

/** Lazy PDO-Verbindungen: 'auth', 'web', 'chars:<realmId>'. */
final class Database
{
    /** @var array<string,PDO> */
    private static array $pool = [];

    public static function auth(): PDO
    {
        return self::get('auth', config('db.auth'));
    }

    public static function web(): PDO
    {
        $cfg = config('db.web');
        return $cfg ? self::get('web', $cfg) : self::auth();
    }

    public static function chars(int $realmId): PDO
    {
        $cfg = config("realms.$realmId.characters");
        if (!$cfg) {
            throw new RuntimeException("Keine Characters-DB für Realm $realmId konfiguriert.");
        }
        return self::get("chars:$realmId", $cfg);
    }

    private static function get(string $key, array $c): PDO
    {
        if (!isset(self::$pool[$key])) {
            $dsn = sprintf('mysql:host=%s;port=%d;dbname=%s;charset=utf8mb4',
                $c['host'], (int)($c['port'] ?? 3306), $c['name']);
            self::$pool[$key] = new PDO($dsn, $c['user'], $c['pass'], [
                PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
                PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
                PDO::ATTR_EMULATE_PREPARES   => false,
                PDO::ATTR_TIMEOUT            => 5,
            ]);
        }
        return self::$pool[$key];
    }

    /** Hilfsfunktion: vorbereitete Abfrage ausführen. */
    public static function run(PDO $db, string $sql, array $params = []): PDOStatement
    {
        $st = $db->prepare($sql);
        $st->execute($params);
        return $st;
    }
}
