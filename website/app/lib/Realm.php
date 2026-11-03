<?php
declare(strict_types=1);

/** Realm-Liste, Status, Uptime und Online-Zahlen. */
final class Realm
{
    /** @return array<int,array> realmlist-Zeilen angereichert um Status */
    public static function all(): array
    {
        static $cache = null;
        if ($cache !== null) return $cache;

        $cache = [];
        try {
            $rows = Database::auth()->query(
                'SELECT id, name, address, port, icon, realmflags, population FROM realmlist ORDER BY id')->fetchAll();
        } catch (PDOException $ex) {
            error_log('[website] realmlist: ' . $ex->getMessage());
            return $cache;
        }
        foreach ($rows as $r) {
            $id = (int)$r['id'];
            $cfg = (array)config("realms.$id", []);
            $online = !((int)$r['realmflags'] & 0x2) && self::portOpen(
                    (string)($cfg['check_host'] ?? $r['address']), (int)($cfg['check_port'] ?? $r['port']));
            $r['online'] = $online;
            $r['type'] = $cfg['type'] ?? self::typeFromIcon((int)$r['icon']);
            $r['rates'] = $cfg['rates'] ?? '';
            $r['description'] = $cfg['description'] ?? '';
            $r['has_chars'] = isset($cfg['characters']);
            $r['uptime'] = $online ? self::uptime($id) : 0;
            $r['players'] = $online && $r['has_chars'] ? self::onlineCount($id) : ['total' => 0, 'alliance' => 0, 'horde' => 0, 'bots' => 0];
            $cache[$id] = $r;
        }
        return $cache;
    }

    public static function get(int $id): ?array
    {
        return self::all()[$id] ?? null;
    }

    /** Erste Realm-ID mit konfigurierter Characters-DB */
    public static function defaultId(): int
    {
        $ids = array_keys((array)config('realms', []));
        return $ids ? (int)$ids[0] : 1;
    }

    /** Realm aus ?realm=, sonst Standard; nur konfigurierte Realms. */
    public static function selected(): int
    {
        $id = (int)query('realm', '0');
        return config("realms.$id.characters") ? $id : self::defaultId();
    }

    private static function typeFromIcon(int $icon): string
    {
        return match ($icon) { 1 => 'PvP', 4 => 'Normal', 6 => 'RP', 8 => 'RP-PvP', default => 'PvE' };
    }

    private static function portOpen(string $host, int $port): bool
    {
        $key = "$host:$port";
        $file = sys_get_temp_dir() . '/mpweb_status_' . md5($key);
        // 30 s Cache, damit nicht jeder Seitenaufruf einen TCP-Connect auslöst
        if (is_file($file) && filemtime($file) > time() - 30) {
            return file_get_contents($file) === '1';
        }
        $fp = @fsockopen($host, $port, $errno, $errstr, 1.5);
        $ok = (bool)$fp;
        if ($fp) fclose($fp);
        @file_put_contents($file, $ok ? '1' : '0', LOCK_EX);
        return $ok;
    }

    private static function uptime(int $id): int
    {
        try {
            $start = (int)Database::run(Database::auth(),
                'SELECT MAX(starttime) FROM uptime WHERE realmid = ?', [$id])->fetchColumn();
            return $start > 0 ? max(0, time() - $start) : 0;
        } catch (PDOException) {
            return 0;
        }
    }

    public static function maxPlayers(int $id): int
    {
        try {
            return (int)Database::run(Database::auth(),
                'SELECT MAX(maxplayers) FROM uptime WHERE realmid = ?', [$id])->fetchColumn();
        } catch (PDOException) {
            return 0;
        }
    }

    /** @return array{total:int,alliance:int,horde:int,bots:int} */
    public static function onlineCount(int $id): array
    {
        $out = ['total' => 0, 'alliance' => 0, 'horde' => 0, 'bots' => 0];
        try {
            $db = Database::chars($id);
            $rows = $db->query('SELECT account, race FROM characters WHERE online = 1')->fetchAll();
        } catch (Throwable $ex) {
            error_log('[website] online count: ' . $ex->getMessage());
            return $out;
        }
        $bots = array_flip(Account::botAccountIds());
        foreach ($rows as $r) {
            if (isset($bots[(int)$r['account']])) { $out['bots']++; continue; }
            $out['total']++;
            $out[GameData::faction((int)$r['race'])]++;
        }
        return $out;
    }
}
