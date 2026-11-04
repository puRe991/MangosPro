<?php
declare(strict_types=1);

/** Zugriff auf realmd.account – Registrierung, Login, Passwort/E-Mail ändern, Banns. */
final class Account
{
    public const MAX_NAME = 16;     // MAX_ACCOUNT_STR im Core
    public const MAX_PASS = 16;     // Client-Limit 2.4.3
    public const MIN_PASS = 6;

    /** @return string|null Fehlermeldung oder null */
    public static function validateUsername(string $u): ?string
    {
        if ($u === '') return 'Bitte einen Accountnamen angeben.';
        if (strlen($u) < 3 || strlen($u) > self::MAX_NAME) return 'Der Accountname muss 3–16 Zeichen lang sein.';
        if (!preg_match('/^[A-Za-z0-9]+$/', $u)) return 'Der Accountname darf nur Buchstaben (A–Z) und Ziffern enthalten.';
        $prefix = strtoupper((string)config('bots.account_prefix', ''));
        if ($prefix !== '' && str_starts_with(strtoupper($u), $prefix)) return 'Dieser Accountname ist reserviert.';
        return null;
    }

    public static function validatePassword(string $p, string $username = ''): ?string
    {
        if (strlen($p) < self::MIN_PASS || strlen($p) > self::MAX_PASS) {
            return 'Das Passwort muss 6–16 Zeichen lang sein (Limit des WoW-Clients).';
        }
        if (!preg_match('/^[\x21-\x7E]+$/', $p)) return 'Das Passwort darf nur sichtbare ASCII-Zeichen ohne Leerzeichen enthalten.';
        if ($username !== '' && strcasecmp($p, $username) === 0) return 'Passwort und Accountname dürfen nicht gleich sein.';
        return null;
    }

    public static function findByName(string $username): ?array
    {
        $row = Database::run(Database::auth(),
            'SELECT id, username, gmlevel, email, joindate FROM account WHERE username = ?',
            [strtoupper($username)])->fetch();
        return $row ?: null;
    }

    public static function find(int $id): ?array
    {
        $row = Database::run(Database::auth(),
            'SELECT id, username, gmlevel, email, joindate, locked, expansion, active_realm_id, flags FROM account WHERE id = ?',
            [$id])->fetch();
        return $row ?: null;
    }

    public static function emailInUse(string $email, int $exceptId = 0): bool
    {
        return (bool)Database::run(Database::auth(),
            'SELECT 1 FROM account WHERE email = ? AND id <> ? LIMIT 1', [$email, $exceptId])->fetchColumn();
    }

    public static function create(string $username, string $password, string $email): int
    {
        $db = Database::auth();
        $username = strtoupper($username);
        [$s, $v] = Srp6::makeRegistration($username, $password);
        Database::run($db,
            'INSERT INTO account (username, v, s, email, joindate, expansion) VALUES (?, ?, ?, ?, NOW(), ?)',
            [$username, $v, $s, $email, (int)config('site.expansion', 1)]);
        $id = (int)$db->lastInsertId();
        // Wie AccountMgr::CreateAccount: Zeichenzähler je Realm anlegen
        Database::run($db,
            'INSERT IGNORE INTO realmcharacters (realmid, acctid, numchars) SELECT id, ?, 0 FROM realmlist', [$id]);
        return $id;
    }

    /** Prüft Name+Passwort. @return array|null Account-Zeile */
    public static function verify(string $username, string $password): ?array
    {
        $row = Database::run(Database::auth(),
            'SELECT id, username, gmlevel, v, s, locked FROM account WHERE username = ?',
            [strtoupper($username)])->fetch();
        if (!$row) {
            // gleiche Rechenzeit wie bei existierendem Account (kein Account-Enumeration-Timing)
            Srp6::verifier($username, $password, Srp6::newSalt());
            return null;
        }
        return Srp6::check($row['username'], $password, (string)$row['s'], (string)$row['v']) ? $row : null;
    }

    public static function setPassword(int $id, string $username, string $password): void
    {
        [$s, $v] = Srp6::makeRegistration($username, $password);
        // sessionkey leeren -> bestehende Spielsitzungen müssen sich neu anmelden
        Database::run(Database::auth(),
            "UPDATE account SET v = ?, s = ?, sessionkey = '' WHERE id = ?", [$v, $s, $id]);
    }

    public static function setEmail(int $id, string $email): void
    {
        Database::run(Database::auth(), 'UPDATE account SET email = ? WHERE id = ?', [$email, $id]);
    }

    /** Aktiver Bann oder null */
    public static function activeBan(int $id): ?array
    {
        $row = Database::run(Database::auth(),
            'SELECT banned_at, expires_at, banned_by, reason FROM account_banned
             WHERE account_id = ? AND active = 1 AND (expires_at = banned_at OR expires_at > UNIX_TIMESTAMP())
             ORDER BY banned_at DESC LIMIT 1', [$id])->fetch();
        return $row ?: null;
    }

    public static function ipBanned(string $ip): bool
    {
        return (bool)Database::run(Database::auth(),
            'SELECT 1 FROM ip_banned WHERE ip = ? AND (expires_at = banned_at OR expires_at > UNIX_TIMESTAMP()) LIMIT 1',
            [$ip])->fetchColumn();
    }

    public static function lastLoginIp(int $id): ?array
    {
        try {
            $row = Database::run(Database::auth(),
                'SELECT ip, loginTime FROM account_logons WHERE accountId = ? ORDER BY id DESC LIMIT 1', [$id])->fetch();
            return $row ?: null;
        } catch (PDOException) {
            return null; // Tabelle existiert in älteren Ständen nicht
        }
    }

    /** IDs der Random-Bot-Accounts (für Filter), 5 Minuten im Session-Cache. */
    public static function botAccountIds(): array
    {
        if (!config('bots.hide', true)) return [];
        $prefix = strtoupper((string)config('bots.account_prefix', ''));
        if ($prefix === '') return [];
        $cache = $_SESSION['__botids'] ?? null;
        if ($cache && $cache['t'] > time() - 300) return $cache['ids'];
        $ids = Database::run(Database::auth(), 'SELECT id FROM account WHERE username LIKE ?',
            [addcslashes($prefix, '%_') . '%'])->fetchAll(PDO::FETCH_COLUMN);
        $ids = array_map('intval', $ids);
        $_SESSION['__botids'] = ['t' => time(), 'ids' => $ids];
        return $ids;
    }

    /** SQL-Fragment "AND account NOT IN (...)" für Characters-Abfragen. */
    public static function botFilterSql(string $column = 'account'): string
    {
        $ids = self::botAccountIds();
        return $ids ? " AND $column NOT IN (" . implode(',', $ids) . ')' : '';
    }

    public static function totalCount(): int
    {
        $sql = 'SELECT COUNT(*) FROM account';
        $prefix = strtoupper((string)config('bots.account_prefix', ''));
        if (config('bots.hide', true) && $prefix !== '') {
            return (int)Database::run(Database::auth(), $sql . ' WHERE username NOT LIKE ?',
                [addcslashes($prefix, '%_') . '%'])->fetchColumn();
        }
        return (int)Database::auth()->query($sql)->fetchColumn();
    }
}
