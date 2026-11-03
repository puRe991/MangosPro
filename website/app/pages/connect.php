<?php
page_title('So verbindest du dich');
$realmlist = (string)config('site.realmlist');
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow">Anleitung</p>
        <h1>In 5 Minuten im Spiel</h1>
        <p class="lead">Du brauchst einen WoW-Client in Version <strong><?= e(config('site.client')) ?></strong>.</p>
    </div>
</section>

<section class="wrap section narrow-content">
    <ol class="guide">
        <li>
            <h2>Account erstellen</h2>
            <?php if (Auth::check()): ?>
                <p>Erledigt – du bist als <strong><?= e(Auth::name()) ?></strong> angemeldet. ✓</p>
            <?php else: ?>
                <p>Registriere dich kostenlos. Accountname und Passwort sind später deine Zugangsdaten im Spiel.</p>
                <p><a class="btn btn-primary" href="<?= e(url('register')) ?>">Account erstellen</a></p>
            <?php endif; ?>
        </li>
        <li>
            <h2>Client bereitstellen</h2>
            <p>Du benötigst World of Warcraft: The Burning Crusade in Version <strong>2.4.3</strong>. Prüfe die Version unten links im Login-Bildschirm.
                Wir stellen aus rechtlichen Gründen keinen Client zum Download bereit – nutze deine eigene Installation.</p>
        </li>
        <li>
            <h2>Realmlist anpassen</h2>
            <p>Öffne im WoW-Ordner die Datei <code>realmlist.wtf</code> (bei deutschen Clients unter <code>Data/deDE/</code>, bei englischen unter <code>Data/enUS/</code> bzw. <code>enGB</code>) mit einem Texteditor und ersetze den Inhalt durch:</p>
            <div class="codebox">
                <code id="realmlist-line">set realmlist <?= e($realmlist) ?></code>
                <button class="btn btn-ghost btn-sm" type="button" data-copy="#realmlist-line"><?= icon('copy') ?> <span>Kopieren</span></button>
            </div>
            <p class="muted small">Tipp: Lösche zusätzlich den Ordner <code>Cache</code>, wenn du vorher auf einem anderen Server gespielt hast.</p>
        </li>
        <li>
            <h2>Spiel starten</h2>
            <p>Starte WoW über <code>Wow.exe</code> (nicht über den Launcher, der würde die Realmlist überschreiben) und logge dich mit deinem Account ein.</p>
        </li>
        <li>
            <h2>Playerbots nutzen</h2>
            <p>Auf dem Server laufen Playerbots. Ein paar nützliche Chatbefehle für den Start:</p>
            <div class="table-wrap">
                <table class="table table-compact">
                    <thead><tr><th>Befehl</th><th>Wirkung</th></tr></thead>
                    <tbody>
                    <tr><td><code>.bot add &lt;Name&gt;</code></td><td>Einen eigenen Charakter (gleicher Account) als Bot einloggen</td></tr>
                    <tr><td><code>/invite &lt;Bot&gt;</code></td><td>Einen Random-Bot in die Gruppe einladen</td></tr>
                    <tr><td><code>/p follow</code> · <code>/p stay</code></td><td>Gruppe folgt dir bzw. bleibt stehen</td></tr>
                    <tr><td><code>/p attack</code></td><td>Bots greifen dein Ziel an</td></tr>
                    <tr><td><code>/p lust</code> · <code>/p auras</code></td><td>Heldentum/Kampfrausch zünden · Raid-Auren verteilen</td></tr>
                    <tr><td><code>/p summon</code></td><td>Bots zu dir holen</td></tr>
                    </tbody>
                </table>
            </div>
        </li>
    </ol>

    <div class="card">
        <h2 class="card-title"><?= icon('chat') ?> Probleme?</h2>
        <dl class="faq">
            <dt>"Verbindung getrennt" direkt nach dem Login</dt>
            <dd>Falsche Client-Version oder Realmlist. Prüfe, dass du 2.4.3 (8606) nutzt und die <code>realmlist.wtf</code> korrekt ist.</dd>
            <dt>"Benutzername oder Passwort falsch"</dt>
            <dd>Achte darauf, den Account<strong>namen</strong> zu verwenden, nicht die E-Mail-Adresse. Passwort vergessen? <a href="<?= e(url('forgot')) ?>">Hier zurücksetzen</a>.</dd>
            <dt>Realm wird als offline angezeigt</dt>
            <dd>Schau auf die <a href="<?= e(url('status')) ?>">Statusseite</a> – vermutlich Wartung oder Neustart.</dd>
        </dl>
    </div>
</section>
