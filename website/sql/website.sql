-- MangosPro Website – zusätzliche Tabellen.
-- In die realmd-Datenbank importieren (oder in eine eigene DB, dann config 'db.web' setzen):
--   mysql -u mangos -p tbcrealmd < website/sql/website.sql
-- Die Core-Tabellen (account, realmlist, ...) werden NICHT verändert.

CREATE TABLE IF NOT EXISTS `web_news` (
  `id`         int(10) unsigned NOT NULL AUTO_INCREMENT,
  `title`      varchar(150) NOT NULL,
  `category`   varchar(20)  NOT NULL DEFAULT 'news',
  `body`       mediumtext   NOT NULL,
  `author`     varchar(32)  NOT NULL DEFAULT '',
  `published`  tinyint(1)   NOT NULL DEFAULT '1',
  `pinned`     tinyint(1)   NOT NULL DEFAULT '0',
  `created_at` datetime     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime     NULL DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `idx_published` (`published`, `pinned`, `created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Website News';

CREATE TABLE IF NOT EXISTS `web_attempts` (
  `id`         int(10) unsigned NOT NULL AUTO_INCREMENT,
  `action`     varchar(20) NOT NULL,
  `ip`         varchar(45) NOT NULL,
  `created_at` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `idx_lookup` (`action`, `ip`, `created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Website Rate-Limiting';

CREATE TABLE IF NOT EXISTS `web_password_resets` (
  `account_id` int(10) unsigned NOT NULL,
  `token_hash` char(64)    NOT NULL,
  `expires_at` int(10) unsigned NOT NULL,
  `created_ip` varchar(45) NOT NULL DEFAULT '',
  PRIMARY KEY (`account_id`),
  UNIQUE KEY `idx_token` (`token_hash`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Website Passwort-Reset';

CREATE TABLE IF NOT EXISTS `web_registrations` (
  `account_id` int(10) unsigned NOT NULL,
  `ip`         varchar(45) NOT NULL,
  `created_at` int(10) unsigned NOT NULL,
  PRIMARY KEY (`account_id`),
  KEY `idx_ip` (`ip`, `created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Website Registrierungen (IP-Limit)';

CREATE TABLE IF NOT EXISTS `web_progress_kills` (
  `id`         int(10) unsigned NOT NULL AUTO_INCREMENT,
  `realm_id`   int(10) unsigned NOT NULL,
  `guild_id`   int(10) unsigned NOT NULL,
  `guild_name` varchar(64) NOT NULL,
  `map_id`     smallint(5) unsigned NOT NULL,
  `boss_index` tinyint(3) unsigned NOT NULL,
  `killed_at`  int(10) unsigned NOT NULL,
  `source`     enum('auto','manual') NOT NULL DEFAULT 'auto',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_kill` (`realm_id`, `guild_id`, `map_id`, `boss_index`),
  KEY `idx_first` (`realm_id`, `map_id`, `boss_index`, `killed_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Progress Race: erster Kill je Gilde und Boss';

INSERT INTO `web_news` (`title`, `category`, `body`, `author`, `pinned`)
SELECT 'Willkommen auf MangosPro!', 'news',
'Der Server ist online. Erstelle dir oben über **Registrieren** einen Account, stelle deine `realmlist.wtf` um und leg los.

## Was dich erwartet
- The Burning Crusade 2.4.3, blizzlike Raten
- Playerbots, die Raid-Bosse von Karazhan bis Sunwell mit echten Taktiken spielen
- Alle Raids und Dungeons gescriptet

Viel Spaß in der Scherbenwelt!', 'ADMINISTRATOR', 1
FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM `web_news`);
