# MangosPro Website

Server-Website für MangosPro (CMaNGOS TBC 2.4.3). Läuft auf Apache mit PHP; Frameworks, Composer und externe CDNs braucht sie nicht.

## Funktionen

| Bereich | Inhalt |
|---|---|
| **Accounts** | Registrierung (SRP6, kompatibel mit `realmd.account.s/v`), Login, Passwort/E-Mail ändern, „Passwort vergessen“ per Mail-Link |
| **Account-Übersicht** | Charaktere aller Realms mit Gilde, Zone, Gold, Spielzeit, Bann-Status, letzter Spiel-Login |
| **Server** | Live-Realmstatus (Port-Check + `realmflags`), Uptime, Allianz/Horde-Verteilung, JSON-API `/api/status` |
| **Community** | Wer ist online, Ranglisten (Stufe, Ehrenhafte Siege, Arena 2v2/3v3/5v5, Spielzeit), Arsenal mit Charakter- und Gildenseiten |
| **Playerbots** | Random-Bot-Accounts (`RNDBOT…`) werden aus Listen und Ranglisten gefiltert und separat gezählt; der Präfix ist für die Registrierung gesperrt |
| **Progress Race** | Boss-Kills der Gilden werden automatisch aus `instance.encountersMask` erkannt: Realm-Firsts je Boss, Gildenwertung, Chronik, Fortschritt auf der Gildenseite, Korrektur im Admin-Bereich |
| **Inhalte** | News mit Kategorien, Changelog-Seite `/updates` für Patch-Notes, Admin-Editor (ab `gmlevel` 3), Verbindungsanleitung, Regeln, Impressum, Datenschutz |
| **Sicherheit** | CSRF-Tokens, Prepared Statements, Rate-Limiting (Login, Passwort, Reset), lokale Captcha + Honeypot, IP-Limit pro Tag, `ip_banned`-Prüfung, strikte CSP, Session-Härtung |
| **DSGVO** | Keine externen Schriften, kein Tracking, nur ein technisch notwendiges Session-Cookie |

## Voraussetzungen

- Apache 2.4 mit `mod_rewrite` (empfohlen auch `mod_headers`, `mod_expires`, `mod_deflate`)
- PHP **8.1+** mit `pdo_mysql`, empfohlen zusätzlich `gmp` oder `bcmath`. Ohne diese Erweiterungen rechnet ein reiner PHP-Fallback (etwa 0,1 s pro Login).
- Zugriff auf die realmd- und characters-Datenbank des Servers (MySQL/MariaDB)

```bash
sudo apt install apache2 libapache2-mod-php php-mysql php-gmp
```

## Installation

```bash
# 1. Dateien kopieren
sudo cp -r website /var/www/mangospro

# 2. Website-Tabellen in die realmd-DB importieren (Core-Tabellen werden nicht verändert)
mysql -u root -p tbcrealmd < /var/www/mangospro/sql/website.sql
# optional: Start-News, Patch-Notes 1.0–1.4 und Event-Beitrag
mysql -u root -p tbcrealmd < /var/www/mangospro/sql/website_content.sql

# 3. Konfiguration
sudo cp /var/www/mangospro/app/config.dist.php /var/www/mangospro/app/config.php
sudo nano /var/www/mangospro/app/config.php      # DB-Zugänge, realmlist, Impressum, Mail ...
sudo chown -R root:www-data /var/www/mangospro && sudo chmod 640 /var/www/mangospro/app/config.php

# 4. Apache
sudo cp /var/www/mangospro/apache/mangospro-website.conf /etc/apache2/sites-available/
sudo nano /etc/apache2/sites-available/mangospro-website.conf   # ServerName anpassen
sudo a2enmod rewrite headers expires deflate
sudo a2ensite mangospro-website && sudo systemctl reload apache2

# 5. HTTPS (dringend empfohlen, sonst gehen Passwörter im Klartext übers Netz)
sudo certbot --apache -d wow.example.de
```

`DocumentRoot` muss auf **`public/`** zeigen. `app/` mit den DB-Passwörtern liegt außerhalb und ist zusätzlich per `.htaccess` gesperrt.

**Shared Hosting ohne eigenen VirtualHost:** Wenn sich nur `public/` hochladen lässt, `app/` eine Ebene darüber ablegen. Liegt `app/` woanders, den `require`-Pfad in `public/index.php` anpassen. Ohne `mod_rewrite` in der Konfiguration `'pretty_urls' => false` setzen, dann laufen die URLs über `index.php?p=register`.

### Datenbank-Benutzer

Die Website braucht auf realmd `SELECT, INSERT, UPDATE` (Accounts) sowie `DELETE` auf die `web_*`-Tabellen und auf characters nur `SELECT`:

```sql
CREATE USER 'mpweb'@'localhost' IDENTIFIED BY 'starkes-passwort';
GRANT SELECT, INSERT, UPDATE ON tbcrealmd.* TO 'mpweb'@'localhost';
GRANT DELETE ON tbcrealmd.web_attempts TO 'mpweb'@'localhost';
GRANT DELETE ON tbcrealmd.web_password_resets TO 'mpweb'@'localhost';
GRANT DELETE ON tbcrealmd.web_news TO 'mpweb'@'localhost';
GRANT SELECT ON tbccharacters.* TO 'mpweb'@'localhost';
```

## Progress Race

Der Core setzt beim Boss-Kill ein Bit in `characters.instance.encountersMask` (Index aus `DungeonEncounter.sql`;
die World-DB braucht dafür die `instance_encounters`-Daten aus tbc-db). Die Website ordnet jede Raid-ID der Gilde
mit den meisten gebundenen Mitgliedern zu (mindestens `min_members_10` bzw. `min_members_25`) und speichert den
ersten Kill je Gilde und Boss in `web_progress_kills`. Gilden mit Random-Bot als Gildenmeister werden nicht gewertet.

- Ohne Cron: Abgleich beim Seitenaufruf, höchstens alle `sync_interval` Sekunden (Standard)
- Mit Cron (genauer, keine Last beim Seitenaufruf): `*/2 * * * * www-data php /var/www/mangospro/bin/progress_sync.php`
  und `'auto_sync' => false`
- Der gespeicherte Zeitpunkt ist der Zeitpunkt der Erkennung. Raid-IDs werden beim wöchentlichen Reset gelöscht,
  deshalb sollte der Abgleich mindestens täglich laufen. Falsche oder fehlende Kills unter **Admin → Progress** korrigieren.

## Admin

Wer sich mit einem Account ab `gmlevel` 3 anmeldet (`security.admin_gmlevel`), sieht oben den Button **Admin**. Dort gibt es Statistiken, die neuesten Registrierungen, die News-Verwaltung und die Progress-Korrektur. Sperren und GM-Rechte bleiben bei den Core-Befehlen (`.ban account`, `.account set gmlevel`).

## Aufbau

```
website/
├── public/            ← DocumentRoot (index.php, .htaccess, assets/)
├── app/
│   ├── config.dist.php
│   ├── bootstrap.php
│   ├── lib/           Srp6, Account, Auth, Realm, News, Mailer, RateLimiter, GameData
│   ├── pages/         eine Datei pro Route (admin/ für den Admin-Bereich)
│   └── views/         Layout und Teilansichten
├── bin/progress_sync.php  Cron-Skript für das Progress Race
├── sql/website.sql    zusätzliche Tabellen (web_*)
├── sql/website_content.sql  Start-Inhalte (News, Patch-Notes)
├── apache/            VirtualHost-Beispiel
└── tests/srp6_test.php
```

Neue Seite anlegen: Datei in `app/pages/` erstellen und die Route in `public/index.php` eintragen.

## Tests

```bash
php website/tests/srp6_test.php
```

Der Test prüft den SRP6-Verifier gegen die Standard-Accounts aus `sql/base/realmd.sql` (ADMINISTRATOR, GAMEMASTER, …) und einen Roundtrip.
