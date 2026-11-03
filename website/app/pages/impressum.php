<?php page_title('Impressum'); ?>
<section class="wrap section narrow-content">
    <div class="card prose">
        <h1>Impressum</h1>
        <p class="muted small">Angaben gemäß § 5 DDG</p>
        <?= config('legal.impressum') /* vom Betreiber in config.php gepflegtes HTML */ ?>
        <h3>Haftungsausschluss</h3>
        <p><?= e(config('site.name')) ?> ist ein privates, nicht-kommerzielles Hobbyprojekt. World of Warcraft und The Burning Crusade
            sind eingetragene Marken von Blizzard Entertainment, Inc. Dieses Projekt steht in keiner Verbindung zu Blizzard Entertainment
            und wird von Blizzard weder unterstützt noch gesponsert.</p>
        <p>Für Inhalte externer Links übernehmen wir keine Haftung; für deren Inhalt sind ausschließlich deren Betreiber verantwortlich.</p>
    </div>
</section>
