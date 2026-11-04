<?php
declare(strict_types=1);

/**
 * Progress Race: Boss-Kills pro Gilde.
 *
 * Quelle: characters.instance.encountersMask. Der Core setzt dort Bit `EncounterIndex`
 * (siehe sql/base/dbc/original_data/DungeonEncounter.sql), sobald ein Boss in dieser Raid-ID fällt.
 * sync() ordnet jede Raid-ID der Gilde zu, die die meisten gebundenen Mitglieder darin hat,
 * und speichert jeden neuen Boss einmalig mit Erkennungszeitpunkt in web_progress_kills.
 */
final class Progress
{
    /** Map-ID => Raid (Boss-Reihenfolge = EncounterIndex) */
    public const RAIDS = [
        532 => ['name' => 'Karazhan', 'short' => 'Kara', 'tier' => 'T4', 'size' => 10, 'bosses' => [
            'Attumen the Huntsman', 'Moroes', 'Maiden of Virtue', 'Opera Event', 'The Curator', 'Terestian Illhoof',
            'Shade of Aran', 'Netherspite', 'Chess Event', 'Prince Malchezaar', 'Nightbane']],
        565 => ['name' => "Gruul's Lair", 'short' => 'Gruul', 'tier' => 'T4', 'size' => 25, 'bosses' => [
            'High King Maulgar', 'Gruul the Dragonkiller']],
        544 => ['name' => "Magtheridon's Lair", 'short' => 'Mag', 'tier' => 'T4', 'size' => 25, 'bosses' => [
            'Magtheridon']],
        548 => ['name' => 'Serpentshrine Cavern', 'short' => 'SSC', 'tier' => 'T5', 'size' => 25, 'bosses' => [
            'Hydross the Unstable', 'The Lurker Below', 'Leotheras the Blind', 'Fathom-Lord Karathress',
            'Morogrim Tidewalker', 'Lady Vashj']],
        550 => ['name' => 'Tempest Keep', 'short' => 'TK', 'tier' => 'T5', 'size' => 25, 'bosses' => [
            "Al'ar", 'Void Reaver', 'High Astromancer Solarian', "Kael'thas Sunstrider"]],
        568 => ['name' => "Zul'Aman", 'short' => 'ZA', 'tier' => 'ZA', 'size' => 10, 'bosses' => [
            "Akil'zon", 'Nalorakk', "Jan'alai", 'Halazzi', 'Hex Lord Malacrass', "Zul'jin"]],
        534 => ['name' => 'Hyjal Summit', 'short' => 'Hyjal', 'tier' => 'T6', 'size' => 25, 'bosses' => [
            'Rage Winterchill', 'Anetheron', "Kaz'rogal", 'Azgalor', 'Archimonde']],
        564 => ['name' => 'Black Temple', 'short' => 'BT', 'tier' => 'T6', 'size' => 25, 'bosses' => [
            "High Warlord Naj'entus", 'Supremus', 'Shade of Akama', 'Teron Gorefiend', 'Gurtogg Bloodboil',
            'Reliquary of Souls', 'Mother Shahraz', 'The Illidari Council', 'Illidan Stormrage']],
        580 => ['name' => 'Sunwell Plateau', 'short' => 'SWP', 'tier' => 'T6.5', 'size' => 25, 'bosses' => [
            'Kalecgos', 'Brutallus', 'Felmyst', 'Eredar Twins', "M'uru", "Kil'jaeden"]],
    ];

    /** Reihenfolge = Gewichtung im Ranking (höchste Stufe zuerst) */
    public const TIERS = [
        'T6.5' => 'Sunwell',
        'T6'   => 'Tier 6',
        'T5'   => 'Tier 5',
        'ZA'   => "Zul'Aman",
        'T4'   => 'Tier 4',
    ];

    /** Mindestzahl gebundener Gildenmitglieder (Spieler oder Bots) für einen Gilden-Kill */
    public static function minMembers(int $map): int
    {
        return (self::RAIDS[$map]['size'] ?? 25) >= 25
            ? (int)config('progress.min_members_25', 20)
            : (int)config('progress.min_members_10', 10);
    }

    public static function lastBoss(int $map): int
    {
        return count(self::RAIDS[$map]['bosses']) - 1;
    }

    // ------------------------------------------------------------------ Synchronisation

    /** Seitenaufrufe stoßen höchstens alle `sync_interval` Sekunden einen Abgleich an (ohne Cron). */
    public static function maybeSync(int $realmId): void
    {
        if (!config('progress.auto_sync', true)) return;
        $interval = max(30, (int)config('progress.sync_interval', 120));
        $file = sys_get_temp_dir() . '/mpweb_progress_' . md5(__DIR__ . $realmId);
        if (is_file($file) && filemtime($file) > time() - $interval) return;
        $fp = @fopen($file, 'c');
        if (!$fp || !flock($fp, LOCK_EX | LOCK_NB)) return;
        touch($file);
        try {
            self::sync($realmId);
        } catch (Throwable $ex) {
            error_log('[website] progress sync: ' . $ex->getMessage());
        } finally {
            flock($fp, LOCK_UN);
            fclose($fp);
        }
    }

    /** @return int Anzahl neu erkannter Kills */
    public static function sync(int $realmId): int
    {
        $db = Database::chars($realmId);
        $maps = implode(',', array_keys(self::RAIDS));
        $rows = $db->query(
            "SELECT i.id, i.map, i.encountersMask, gm.guildid, g.name AS guild, g.leaderguid, COUNT(*) AS members
             FROM instance i
             JOIN character_instance ci ON ci.instance = i.id
             JOIN guild_member gm ON gm.guid = ci.guid
             JOIN guild g ON g.guildid = gm.guildid
             WHERE i.map IN ($maps) AND i.difficulty = 0 AND i.encountersMask <> 0
             GROUP BY i.id, i.map, i.encountersMask, gm.guildid, g.name, g.leaderguid")->fetchAll();

        // Pro Raid-ID die Gilde mit den meisten gebundenen Mitgliedern
        $best = [];
        foreach ($rows as $r) {
            $id = (int)$r['id'];
            if (!isset($best[$id]) || (int)$r['members'] > (int)$best[$id]['members']) $best[$id] = $r;
        }
        if (!$best) return 0;

        $botGuildLeaders = [];
        if (config('progress.hide_bot_guilds', true) && ($botIds = Account::botAccountIds())) {
            $leaders = array_unique(array_map(fn($r) => (int)$r['leaderguid'], $best));
            $st = $db->query('SELECT guid FROM characters WHERE guid IN (' . implode(',', $leaders ?: [0]) . ')
                              AND account IN (' . implode(',', $botIds) . ')');
            $botGuildLeaders = array_flip(array_map('intval', $st->fetchAll(PDO::FETCH_COLUMN)));
        }

        $insert = Database::web()->prepare(
            "INSERT IGNORE INTO web_progress_kills (realm_id, guild_id, guild_name, map_id, boss_index, killed_at, source)
             VALUES (?, ?, ?, ?, ?, ?, 'auto')");
        $new = 0;
        $now = time();
        foreach ($best as $r) {
            $map = (int)$r['map'];
            $raid = self::RAIDS[$map];
            $min = self::minMembers($map);
            if ((int)$r['members'] < $min || isset($botGuildLeaders[(int)$r['leaderguid']])) continue;
            $mask = (int)$r['encountersMask'];
            foreach ($raid['bosses'] as $idx => $_) {
                if (!($mask & (1 << $idx))) continue;
                $insert->execute([$realmId, (int)$r['guildid'], $r['guild'], $map, $idx, $now]);
                $new += $insert->rowCount();
            }
        }
        return $new;
    }

    // ------------------------------------------------------------------ Abfragen

    /** @return array<int,array> alle Kills eines Realms */
    public static function kills(int $realmId): array
    {
        try {
            return Database::run(Database::web(),
                'SELECT guild_id, guild_name, map_id, boss_index, killed_at, source FROM web_progress_kills
                 WHERE realm_id = ? ORDER BY killed_at, id', [$realmId])->fetchAll();
        } catch (PDOException $ex) {
            error_log('[website] progress: ' . $ex->getMessage());
            return [];
        }
    }

    /**
     * Gilden-Ranking.
     * @return array<int,array{guild_id:int,name:string,raids:array<int,int>,tiers:array<string,int>,total:int,last:int}>
     */
    public static function ranking(array $kills): array
    {
        $g = [];
        foreach ($kills as $k) {
            $id = (int)$k['guild_id'];
            $map = (int)$k['map_id'];
            if (!isset(self::RAIDS[$map])) continue;
            $g[$id] ??= ['guild_id' => $id, 'name' => $k['guild_name'], 'raids' => [], 'tiers' => array_fill_keys(array_keys(self::TIERS), 0),
                         'total' => 0, 'last' => 0, 'last_boss' => null];
            $g[$id]['raids'][$map] = ($g[$id]['raids'][$map] ?? 0) + 1;
            $g[$id]['tiers'][self::RAIDS[$map]['tier']]++;
            $g[$id]['total']++;
            if ((int)$k['killed_at'] >= $g[$id]['last']) {
                $g[$id]['last'] = (int)$k['killed_at'];
                $g[$id]['last_boss'] = [$map, (int)$k['boss_index']];
            }
            $g[$id]['name'] = $k['guild_name'];
        }
        // Höchste Stufe zuerst, bei Gleichstand gewinnt, wer den Stand früher erreicht hat
        usort($g, function ($a, $b) {
            foreach (array_keys(self::TIERS) as $t) {
                if ($a['tiers'][$t] !== $b['tiers'][$t]) return $b['tiers'][$t] <=> $a['tiers'][$t];
            }
            return $a['last'] <=> $b['last'];
        });
        return $g;
    }

    /** Realm-First je Boss: [map][boss] => kill */
    public static function firsts(array $kills): array
    {
        $f = [];
        foreach ($kills as $k) {
            $m = (int)$k['map_id'];
            $b = (int)$k['boss_index'];
            if (!isset($f[$m][$b]) || (int)$k['killed_at'] < (int)$f[$m][$b]['killed_at']) $f[$m][$b] = $k;
        }
        return $f;
    }

    /** Anzahl Gilden pro Boss */
    public static function killCounts(array $kills): array
    {
        $c = [];
        foreach ($kills as $k) $c[(int)$k['map_id']][(int)$k['boss_index']] = ($c[(int)$k['map_id']][(int)$k['boss_index']] ?? 0) + 1;
        return $c;
    }

    /** Kills einer Gilde: [map][boss] => killed_at */
    public static function guildKills(int $realmId, int $guildId): array
    {
        $out = [];
        try {
            $rows = Database::run(Database::web(),
                'SELECT map_id, boss_index, killed_at FROM web_progress_kills WHERE realm_id = ? AND guild_id = ?',
                [$realmId, $guildId])->fetchAll();
        } catch (PDOException) {
            return [];
        }
        foreach ($rows as $r) $out[(int)$r['map_id']][(int)$r['boss_index']] = (int)$r['killed_at'];
        return $out;
    }

    public static function bossName(int $map, int $boss): string
    {
        return self::RAIDS[$map]['bosses'][$boss] ?? '?';
    }

    public static function addManual(int $realmId, int $guildId, string $guildName, int $map, int $boss, int $ts): bool
    {
        $st = Database::run(Database::web(),
            "INSERT INTO web_progress_kills (realm_id, guild_id, guild_name, map_id, boss_index, killed_at, source)
             VALUES (?, ?, ?, ?, ?, ?, 'manual')
             ON DUPLICATE KEY UPDATE killed_at = VALUES(killed_at), guild_name = VALUES(guild_name), source = 'manual'",
            [$realmId, $guildId, $guildName, $map, $boss, $ts]);
        return $st->rowCount() > 0;
    }

    public static function delete(int $id): void
    {
        Database::run(Database::web(), 'DELETE FROM web_progress_kills WHERE id = ?', [$id]);
    }
}
