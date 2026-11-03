<?php
page_title('Über den Server');
$raids = [
    ['Karazhan', '10', 'Moroes, Maiden, Oper, Kurator, Illhoof, Aran, Netherspite, Prince'],
    ["Gruul's Lair", '25', 'High King Maulgar (Rat), Gruul'],
    ["Magtheridon's Lair", '25', 'Würfel-Koordination, Kanalisierer'],
    ['Serpentshrine Cavern', '25', 'Hydross, Lurker, Leotheras, Karathress, Morogrim, Vashj'],
    ['Festung der Stürme', '25', "Al'ar, Leerhäscher, Solarian, Kael'thas"],
    ['Hyjal', '25', 'Wellen, Winterchill bis Archimonde'],
    ['Black Temple', '25', "Naj'entus, Supremus, Akama, Teron, Reliquiar, Illidari-Rat, Illidan"],
    ["Zul'Aman", '10', 'Tierbosse, Hex Lord, Zul\'jin'],
    ['Sunwell Plateau', '25', "Kalecgos, Brutallus, Felmyst, Zwillinge, M'uru, Kil'jaeden"],
];
?>
<section class="page-head">
    <div class="wrap">
        <p class="eyebrow">Über den Server</p>
        <h1><?= e(config('site.name')) ?></h1>
        <p class="lead"><?= e(config('site.tagline')) ?></p>
    </div>
</section>

<section class="wrap section">
    <div class="feature-grid" data-stagger>
        <div class="feature"><div class="feature-icon"><?= icon('server') ?></div><h3>Core</h3>
            <p>CMaNGOS mangos-tbc, Client <?= e(config('site.client')) ?>. Open Source, GPL-2.0.</p></div>
        <div class="feature"><div class="feature-icon"><?= icon('bot') ?></div><h3>Playerbots</h3>
            <p>Fest integriertes Playerbots-Modul mit eigenen Raid-Strategien (Strategie <code>tbc raid</code>).</p></div>
        <div class="feature"><div class="feature-icon"><?= icon('chart') ?></div><h3>Raten</h3>
            <p><?php foreach ((array)config('realms', []) as $r): ?><?= e($r['rates'] ?? '') ?><br><?php endforeach; ?></p></div>
        <div class="feature"><div class="feature-icon"><?= icon('map') ?></div><h3>Inhalte</h3>
            <p>Alle Zonen der Scherbenwelt, Dungeons inkl. heroischer Modi, Schlachtfelder und Arena.</p></div>
    </div>
</section>

<section class="features-band">
    <div class="wrap section">
        <div class="section-head center">
            <h2>Raids mit Bots, die mitdenken</h2>
            <p class="muted">Jeder TBC-Raid hat eigene Boss-Taktiken für die Playerbots.</p>
        </div>
        <div class="card table-wrap">
            <table class="table">
                <thead><tr><th>Raid</th><th>Größe</th><th>Was die Bots können</th></tr></thead>
                <tbody>
                <?php foreach ($raids as [$n, $size, $what]): ?>
                    <tr><td><strong><?= e($n) ?></strong></td><td><?= e($size) ?></td><td class="muted"><?= e($what) ?></td></tr>
                <?php endforeach; ?>
                </tbody>
            </table>
        </div>
    </div>
</section>

<section class="wrap section">
    <div class="grid-2" data-stagger>
        <div class="card">
            <h2 class="card-title"><?= icon('bot') ?> Bots im Raid</h2>
            <ul class="checklist">
                <li>Tanks halten Aggro und spotten bei Tank-Wechsel-Mechaniken</li>
                <li>Heiler priorisieren Tank und Ziele mit Debuffs</li>
                <li>Fernkämpfer verteilen sich bei Spread-Mechaniken</li>
                <li>Unterbrechungen und Dispels werden koordiniert</li>
                <li>Heldentum/Kampfrausch auf Befehl (<code>lust</code>)</li>
            </ul>
        </div>
        <div class="card">
            <h2 class="card-title"><?= icon('shield') ?> Fair Play</h2>
            <ul class="checklist">
                <li>Kein Item-Shop, keine Spenden-Vorteile</li>
                <li>Aktive Moderation und klare <a href="<?= e(url('rules')) ?>">Regeln</a></li>
                <li>Regelmäßige Backups</li>
                <li>Transparente Entwicklung – der Code ist öffentlich</li>
            </ul>
        </div>
    </div>
    <p class="center"><a class="btn btn-primary btn-lg" href="<?= e(url(Auth::check() ? 'connect' : 'register')) ?>">Jetzt mitspielen</a></p>
</section>
