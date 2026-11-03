<?php
/**
 * SRP6-Verifier exakt wie CMaNGOS (src/shared/Auth/SRP6.cpp, AccountMgr::CalculateShaPassHash).
 *
 *   h    = SHA1(UPPER(user) ":" UPPER(pass))            (rohe 20 Bytes)
 *   x    = SHA1(salt_le || h)  als Little-Endian-Zahl
 *   v    = g^x mod N   (g = 7)
 *   s, v werden als Big-Endian-Hex (OpenSSL BN_bn2hex) in realmd.account gespeichert.
 *
 * Rechen-Backend: GMP > BCMath > reines PHP (langsamer, aber ohne Erweiterung lauffähig).
 */
declare(strict_types=1);

final class Srp6
{
    public const N_HEX = '894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7';
    public const G = 7;

    /** Neues Salz: 32 Byte, erstes Byte != 0, damit Hex-Länge und Byte-Länge eindeutig bleiben. */
    public static function newSalt(): string
    {
        do {
            $bytes = random_bytes(32);
        } while ($bytes[0] === "\0");
        return strtoupper(bin2hex($bytes));
    }

    /** @return array{0:string,1:string} [salt, verifier] */
    public static function makeRegistration(string $username, string $password): array
    {
        $salt = self::newSalt();
        return [$salt, self::verifier($username, $password, $salt)];
    }

    public static function verifier(string $username, string $password, string $saltHex): string
    {
        $h = sha1(strtoupper($username) . ':' . strtoupper($password), true);
        $saltLe = strrev(self::hexToMinimalBytes($saltHex));
        $xBe = strrev(sha1($saltLe . $h, true));
        $vBe = self::powmod(self::G, $xBe, hex2bin(self::N_HEX));
        return strtoupper(bin2hex(self::stripLeadingZeros($vBe)));
    }

    public static function check(string $username, string $password, string $saltHex, string $storedV): bool
    {
        if ($saltHex === '' || $storedV === '') {
            return false;
        }
        $calc = ltrim(self::verifier($username, $password, $saltHex), '0');
        return hash_equals(ltrim(strtoupper($storedV), '0'), $calc);
    }

    public static function backend(): string
    {
        if (function_exists('gmp_powm')) return 'gmp';
        if (function_exists('bcpowmod')) return 'bcmath';
        return 'php';
    }

    // ------------------------------------------------------------------

    private static function hexToMinimalBytes(string $hex): string
    {
        $hex = ltrim(trim($hex), '0');
        if ($hex === '') return "\0";
        if (strlen($hex) % 2) $hex = '0' . $hex;
        return hex2bin($hex);
    }

    private static function stripLeadingZeros(string $bin): string
    {
        $s = ltrim($bin, "\0");
        return $s === '' ? "\0" : $s;
    }

    /** base^exp mod mod, exp und mod als Big-Endian-Bytes; Ergebnis Big-Endian-Bytes. */
    private static function powmod(int $base, string $expBe, string $modBe): string
    {
        switch (self::backend()) {
            case 'gmp':
                $r = gmp_powm($base, gmp_init(bin2hex($expBe), 16), gmp_init(bin2hex($modBe), 16));
                $hex = gmp_strval($r, 16);
                return hex2bin(strlen($hex) % 2 ? '0' . $hex : $hex);
            case 'bcmath':
                $r = bcpowmod((string)$base, self::bytesToDec($expBe), self::bytesToDec($modBe));
                return self::decToBytes($r);
            default:
                return PureBigInt::powmod($base, $expBe, $modBe);
        }
    }

    private static function bytesToDec(string $be): string
    {
        $dec = '0';
        foreach (str_split($be) as $c) {
            $dec = bcadd(bcmul($dec, '256'), (string)ord($c));
        }
        return $dec;
    }

    private static function decToBytes(string $dec): string
    {
        $out = '';
        while (bccomp($dec, '0') > 0) {
            $out = chr((int)bcmod($dec, '256')) . $out;
            $dec = bcdiv($dec, '256', 0);
        }
        return $out === '' ? "\0" : $out;
    }
}

/**
 * Minimaler Fallback ohne GMP/BCMath: modulare Exponentiation über 16-Bit-Limbs
 * (Multiplikation per Double-and-Add, damit nur Addition/Vergleich/Subtraktion nötig sind).
 */
final class PureBigInt
{
    public static function powmod(int $base, string $expBe, string $modBe): string
    {
        $n = self::fromBytes($modBe);
        $len = count($n) + 1;
        $n = array_pad($n, $len, 0);
        $b = array_pad(self::fromBytes(chr($base)), $len, 0);
        $r = array_pad([1], $len, 0);
        $bits = self::bits($expBe);
        foreach ($bits as $bit) {
            $r = self::mulmod($r, $r, $n, $len);
            if ($bit) $r = self::mulmod($r, $b, $n, $len);
        }
        return self::toBytes($r);
    }

    /** @return int[] Little-Endian 16-Bit-Limbs */
    private static function fromBytes(string $be): array
    {
        if (strlen($be) % 2) $be = "\0" . $be;
        $limbs = [];
        for ($i = strlen($be) - 2; $i >= 0; $i -= 2) {
            $limbs[] = (ord($be[$i]) << 8) | ord($be[$i + 1]);
        }
        while (count($limbs) > 1 && end($limbs) === 0) array_pop($limbs);
        return $limbs;
    }

    private static function toBytes(array $l): string
    {
        $s = '';
        for ($i = count($l) - 1; $i >= 0; $i--) $s .= chr($l[$i] >> 8) . chr($l[$i] & 0xFF);
        $s = ltrim($s, "\0");
        return $s === '' ? "\0" : $s;
    }

    /** @return int[] Bits MSB zuerst */
    private static function bits(string $be): array
    {
        $bits = [];
        foreach (str_split($be) as $c) {
            $o = ord($c);
            for ($i = 7; $i >= 0; $i--) $bits[] = ($o >> $i) & 1;
        }
        return $bits;
    }

    private static function mulmod(array $a, array $b, array $n, int $len): array
    {
        $r = array_fill(0, $len, 0);
        for ($i = $len - 1; $i >= 0; $i--) {
            $word = $b[$i];
            for ($bit = 15; $bit >= 0; $bit--) {
                // r = 2r mod n
                $carry = 0;
                for ($k = 0; $k < $len; $k++) {
                    $v = ($r[$k] << 1) | $carry;
                    $r[$k] = $v & 0xFFFF;
                    $carry = $v >> 16;
                }
                if (self::cmp($r, $n, $len) >= 0) $r = self::sub($r, $n, $len);
                if (($word >> $bit) & 1) {
                    $carry = 0;
                    for ($k = 0; $k < $len; $k++) {
                        $v = $r[$k] + $a[$k] + $carry;
                        $r[$k] = $v & 0xFFFF;
                        $carry = $v >> 16;
                    }
                    if (self::cmp($r, $n, $len) >= 0) $r = self::sub($r, $n, $len);
                }
            }
        }
        return $r;
    }

    private static function cmp(array $a, array $b, int $len): int
    {
        for ($i = $len - 1; $i >= 0; $i--) {
            if ($a[$i] !== $b[$i]) return $a[$i] > $b[$i] ? 1 : -1;
        }
        return 0;
    }

    private static function sub(array $a, array $b, int $len): array
    {
        $borrow = 0;
        for ($i = 0; $i < $len; $i++) {
            $v = $a[$i] - $b[$i] - $borrow;
            if ($v < 0) { $v += 0x10000; $borrow = 1; } else { $borrow = 0; }
            $a[$i] = $v;
        }
        return $a;
    }
}
