<?php
declare(strict_types=1);

/** News-Beiträge (Tabelle web_news). */
final class News
{
    public const CATEGORIES = [
        'news' => 'Neuigkeiten', 'update' => 'Patch-Notes', 'event' => 'Event', 'maintenance' => 'Wartung',
    ];

    public static function latest(int $limit = 5, int $offset = 0): array
    {
        try {
            $st = Database::web()->prepare(
                'SELECT id, title, category, body, author, created_at, pinned FROM web_news
                 WHERE published = 1 ORDER BY pinned DESC, created_at DESC LIMIT ? OFFSET ?');
            $st->bindValue(1, $limit, PDO::PARAM_INT);
            $st->bindValue(2, $offset, PDO::PARAM_INT);
            $st->execute();
            return $st->fetchAll();
        } catch (PDOException $ex) {
            error_log('[website] news: ' . $ex->getMessage());
            return [];
        }
    }

    public static function count(bool $onlyPublished = true): int
    {
        try {
            return (int)Database::web()->query('SELECT COUNT(*) FROM web_news' . ($onlyPublished ? ' WHERE published = 1' : ''))->fetchColumn();
        } catch (PDOException) {
            return 0;
        }
    }

    public static function find(int $id, bool $onlyPublished = true): ?array
    {
        $row = Database::run(Database::web(),
            'SELECT * FROM web_news WHERE id = ?' . ($onlyPublished ? ' AND published = 1' : ''), [$id])->fetch();
        return $row ?: null;
    }

    public static function all(): array
    {
        return Database::web()->query('SELECT id, title, category, author, created_at, published, pinned FROM web_news ORDER BY created_at DESC')->fetchAll();
    }

    public static function save(?int $id, array $d): int
    {
        $params = [$d['title'], $d['category'], $d['body'], (int)$d['published'], (int)$d['pinned']];
        if ($id) {
            Database::run(Database::web(),
                'UPDATE web_news SET title = ?, category = ?, body = ?, published = ?, pinned = ?, updated_at = NOW() WHERE id = ?',
                [...$params, $id]);
            return $id;
        }
        Database::run(Database::web(),
            'INSERT INTO web_news (title, category, body, published, pinned, author, created_at) VALUES (?, ?, ?, ?, ?, ?, NOW())',
            [...$params, $d['author']]);
        return (int)Database::web()->lastInsertId();
    }

    public static function delete(int $id): void
    {
        Database::run(Database::web(), 'DELETE FROM web_news WHERE id = ?', [$id]);
    }

    /**
     * Sehr kleines, sicheres Markup: Absätze, **fett**, *kursiv*, `code`, [Text](https://link), Listen mit "- ".
     * Alles wird zuerst escaped – kein rohes HTML möglich.
     */
    public static function render(string $text): string
    {
        $lines = explode("\n", str_replace(["\r\n", "\r"], "\n", trim($text)));
        $html = '';
        $para = [];
        $list = [];
        $flush = function () use (&$html, &$para, &$list) {
            if ($para) { $html .= '<p>' . implode('<br>', array_map([self::class, 'inline'], $para)) . '</p>'; $para = []; }
            if ($list) { $html .= '<ul>' . implode('', array_map(fn($l) => '<li>' . self::inline($l) . '</li>', $list)) . '</ul>'; $list = []; }
        };
        foreach ($lines as $line) {
            if (trim($line) === '') { $flush(); continue; }
            if (preg_match('/^(#{2,3}) (.+)$/', $line, $m)) {
                $flush();
                $tag = strlen($m[1]) === 2 ? 'h3' : 'h4';
                $html .= "<$tag>" . self::inline($m[2]) . "</$tag>";
            } elseif (preg_match('/^\s*[-*] (.+)$/', $line, $m)) {
                if ($para) { $l = $list; $list = []; $flush(); $list = $l; }
                $list[] = $m[1];
            } else {
                if ($list) { $p = $para; $para = []; $flush(); $para = $p; }
                $para[] = $line;
            }
        }
        $flush();
        return $html;
    }

    private static function inline(string $s): string
    {
        $s = e($s);
        $s = preg_replace('/`([^`]+)`/', '<code>$1</code>', $s);
        $s = preg_replace('/\*\*(.+?)\*\*/', '<strong>$1</strong>', $s);
        $s = preg_replace('/(?<![*\w])\*(?!\s)(.+?)(?<!\s)\*(?![*\w])/', '<em>$1</em>', $s);
        $s = preg_replace_callback('/\[([^\]]+)\]\((https?:\/\/[^\s)]+)\)/', function ($m) {
            return '<a href="' . $m[2] . '" rel="noopener nofollow" target="_blank">' . $m[1] . '</a>';
        }, $s);
        return $s;
    }

    public static function excerpt(string $text, int $len = 260): string
    {
        $plain = preg_replace('/[*`#\[\]]|\(https?:[^)]+\)/', '', $text);
        $plain = trim(preg_replace('/\s+/', ' ', (string)$plain));
        return mb_strlen($plain) > $len ? mb_substr($plain, 0, $len) . '…' : $plain;
    }
}
