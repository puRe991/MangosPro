<?php
declare(strict_types=1);

/** Kleiner Mailer: PHP mail() oder SMTP (STARTTLS/SSL, AUTH LOGIN). Nur Text-Mails. */
final class Mailer
{
    public static function enabled(): bool
    {
        return (bool)config('mail.enabled', false);
    }

    public static function send(string $to, string $subject, string $body): bool
    {
        if (!self::enabled() || !filter_var($to, FILTER_VALIDATE_EMAIL)) return false;
        $from = (string)config('mail.from');
        $fromName = (string)config('mail.from_name', config('site.name'));
        $encSubject = '=?UTF-8?B?' . base64_encode($subject) . '?=';
        $headers = [
            'From: =?UTF-8?B?' . base64_encode($fromName) . "?= <$from>",
            'MIME-Version: 1.0',
            'Content-Type: text/plain; charset=UTF-8',
            'Content-Transfer-Encoding: 8bit',
            'Date: ' . date('r'),
            'Message-ID: <' . bin2hex(random_bytes(12)) . '@' . (explode('@', $from)[1] ?? 'localhost') . '>',
        ];
        try {
            if (config('mail.transport') === 'mail') {
                return mail($to, $encSubject, $body, implode("\r\n", $headers), '-f' . $from);
            }
            return self::smtp($from, $to, "To: <$to>\r\nSubject: $encSubject\r\n" . implode("\r\n", $headers), $body);
        } catch (Throwable $ex) {
            error_log('[website] Mailversand fehlgeschlagen: ' . $ex->getMessage());
            return false;
        }
    }

    private static function smtp(string $from, string $to, string $headers, string $body): bool
    {
        $c = (array)config('mail.smtp');
        $host = ($c['secure'] ?? '') === 'ssl' ? 'ssl://' . $c['host'] : $c['host'];
        $fp = @stream_socket_client($host . ':' . (int)$c['port'], $errno, $errstr, 10);
        if (!$fp) throw new RuntimeException("SMTP-Verbindung fehlgeschlagen: $errstr");
        stream_set_timeout($fp, 10);
        $ehloHost = parse_url(base_url(), PHP_URL_HOST) ?: 'localhost';

        self::expect($fp, 220);
        self::cmd($fp, "EHLO $ehloHost", 250);
        if (($c['secure'] ?? '') === 'tls') {
            self::cmd($fp, 'STARTTLS', 220);
            if (!stream_socket_enable_crypto($fp, true, STREAM_CRYPTO_METHOD_TLS_CLIENT)) {
                throw new RuntimeException('STARTTLS fehlgeschlagen');
            }
            self::cmd($fp, "EHLO $ehloHost", 250);
        }
        if (!empty($c['user'])) {
            self::cmd($fp, 'AUTH LOGIN', 334);
            self::cmd($fp, base64_encode($c['user']), 334);
            self::cmd($fp, base64_encode((string)$c['pass']), 235);
        }
        self::cmd($fp, "MAIL FROM:<$from>", 250);
        self::cmd($fp, "RCPT TO:<$to>", 250);
        self::cmd($fp, 'DATA', 354);
        $body = str_replace(["\r\n", "\r"], "\n", $body);
        $body = preg_replace('/^\./m', '..', $body);
        self::cmd($fp, $headers . "\r\n\r\n" . str_replace("\n", "\r\n", $body) . "\r\n.", 250);
        self::cmd($fp, 'QUIT', 221);
        fclose($fp);
        return true;
    }

    private static function cmd($fp, string $line, int $code): void
    {
        fwrite($fp, $line . "\r\n");
        self::expect($fp, $code);
    }

    private static function expect($fp, int $code): void
    {
        $resp = '';
        while (($l = fgets($fp, 515)) !== false) {
            $resp .= $l;
            if (strlen($l) < 4 || $l[3] === ' ') break;
        }
        if ((int)substr($resp, 0, 3) !== $code) {
            throw new RuntimeException("SMTP: erwartet $code, erhalten: " . trim($resp));
        }
    }
}
