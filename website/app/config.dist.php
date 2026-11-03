<?php
/**
 * MangosPro Website – Konfiguration.
 * Kopieren nach app/config.php und anpassen. config.php wird nicht ins Git eingecheckt.
 */
return [
    'site' => [
        'name'        => 'MangosPro',
        'tagline'     => 'The Burning Crusade 2.4.3 – mit intelligenten Raid-Playerbots',
        // Absolute Basis-URL ohne abschließenden Slash, z. B. https://wow.example.de
        // Leer lassen = automatisch ermitteln.
        'base_url'    => '',
        // true = schöne URLs (/register) über mod_rewrite, false = index.php?p=register
        'pretty_urls' => true,
        'timezone'    => 'Europe/Berlin',
        'realmlist'   => 'logon.example.de',   // wird in der Anleitung als "set realmlist ..." angezeigt
        'client'      => '2.4.3 (Build 8606)',
        'discord'     => '',                   // Einladungslink, leer = ausblenden
        'contact'     => 'admin@example.de',
        // Expansion für neue Accounts: 0 = Classic, 1 = TBC
        'expansion'   => 1,
    ],

    'registration' => [
        'open'                => true,
        'require_email'       => true,
        'max_per_ip_per_day'  => 3,
        'captcha'             => true,        // eingebaute Rechen-Captcha, keine Drittanbieter
    ],

    // Playerbots: Random-Bot-Accounts (AiPlayerbot.RandomBotAccountPrefix) aus Listen/Ranglisten herausfiltern
    'bots' => [
        'hide'           => true,
        'account_prefix' => 'RNDBOT',
    ],

    // Progress Race (Seite /progress): Kills werden aus characters.instance.encountersMask erkannt
    'progress' => [
        'auto_sync'       => true,   // beim Seitenaufruf abgleichen (alternativ Cron: php bin/progress_sync.php)
        'sync_interval'   => 120,    // Sekunden zwischen zwei Abgleichen
        // Gildenmitglieder (Spieler oder Bots), die in der Raid-ID gebunden sein müssen, damit ein Kill als Gilden-Kill zählt
        'min_members_10'  => 10,     // Karazhan, Zul'Aman (10er)
        'min_members_25'  => 20,     // Gruul, Magtheridon, SSC, TK, Hyjal, BT, Sunwell (25er)
        'hide_bot_guilds' => true,   // Gilden mit Random-Bot als Gildenmeister nicht werten
        'season_start'    => '',     // optional, z. B. '2026-11-01' – nur als Text auf der Seite
    ],

    'db' => [
        // realmd / Login-Datenbank
        'auth' => [
            'host' => '127.0.0.1', 'port' => 3306,
            'name' => 'tbcrealmd', 'user' => 'mangos', 'pass' => 'mangos',
        ],
        // Website-Tabellen (sql/website.sql). null = gleiche DB wie 'auth'.
        'web' => null,
    ],

    // Ein Eintrag pro Realm, Schlüssel = realmlist.id
    'realms' => [
        1 => [
            'characters' => [
                'host' => '127.0.0.1', 'port' => 3306,
                'name' => 'tbccharacters', 'user' => 'mangos', 'pass' => 'mangos',
            ],
            // Optional: anderer Host/Port für den Online-Check (Standard: Werte aus realmlist)
            'check_host' => null,
            'check_port' => null,
            'type'        => 'PvE',
            'rates'       => 'XP x1 · Drop x1 · Gold x1',
            'description' => 'Blizzlike-Progress mit Playerbots, die Raid-Taktiken beherrschen.',
        ],
    ],

    'mail' => [
        'enabled'   => false,             // für "Passwort vergessen" nötig
        'transport' => 'smtp',            // 'smtp' oder 'mail' (PHP mail())
        'from'      => 'noreply@example.de',
        'from_name' => 'MangosPro',
        'smtp' => [
            'host' => 'smtp.example.de', 'port' => 587,
            'secure' => 'tls',            // 'tls' (STARTTLS), 'ssl' oder ''
            'user' => '', 'pass' => '',
        ],
    ],

    'security' => [
        'login_max_attempts' => 5,        // pro IP im Zeitfenster
        'login_window'       => 900,      // Sekunden
        // IPs von Reverse-Proxies, denen X-Forwarded-For geglaubt wird (z. B. ['127.0.0.1'])
        'trusted_proxies'    => [],
        'admin_gmlevel'      => 3,        // ab diesem account.gmlevel: Admin-Bereich
    ],

    // Rechtliches (Pflicht in Deutschland). HTML erlaubt.
    'legal' => [
        'impressum' => '<p>Max Mustermann<br>Musterstraße 1<br>12345 Musterstadt</p><p>E-Mail: admin@example.de</p>',
    ],

    'debug' => false,
];
