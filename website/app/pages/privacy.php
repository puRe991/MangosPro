<?php page_title('Datenschutz'); ?>
<section class="wrap section narrow-content">
    <div class="card prose">
        <h1>Datenschutzerklärung</h1>
        <p class="muted small">Vorlage – bitte vom Betreiber prüfen und an die eigene Situation anpassen.</p>

        <h3>1. Verantwortlicher</h3>
        <p>Verantwortlich im Sinne der DSGVO ist der im <a href="<?= e(url('impressum')) ?>">Impressum</a> genannte Betreiber.
            Kontakt: <a href="mailto:<?= e(config('site.contact')) ?>"><?= e(config('site.contact')) ?></a></p>

        <h3>2. Welche Daten wir verarbeiten</h3>
        <ul>
            <li><strong>Account-Daten:</strong> Accountname, E-Mail-Adresse (falls angegeben), Registrierungsdatum. Das Passwort wird nie im Klartext
                gespeichert, sondern nur als SRP6-Verifier mit zufälligem Salz.</li>
            <li><strong>Spieldaten:</strong> Charaktere, Spielfortschritt und Spiel-Logins inkl. IP-Adresse (zur Missbrauchsabwehr und für die IP-Sperre).</li>
            <li><strong>Website:</strong> IP-Adresse bei Registrierung und fehlgeschlagenen Logins (Schutz vor Missbrauch und Brute-Force),
                Server-Logfiles des Webservers.</li>
        </ul>

        <h3>3. Zweck und Rechtsgrundlage</h3>
        <p>Die Verarbeitung erfolgt zur Bereitstellung des Spiel-Accounts (Art. 6 Abs. 1 lit. b DSGVO) und zum Schutz vor Missbrauch
            (berechtigtes Interesse, Art. 6 Abs. 1 lit. f DSGVO).</p>

        <h3>4. Cookies</h3>
        <p>Wir verwenden ausschließlich ein technisch notwendiges Sitzungs-Cookie (<code>mpweb</code>), das beim Schließen des Browsers gelöscht wird.
            Es gibt kein Tracking, keine Analyse-Tools, keine externen Schriftarten und keine Einbindung von Drittanbietern.</p>

        <h3>5. Speicherdauer</h3>
        <p>Rate-Limit-Einträge werden nach spätestens 48 Stunden gelöscht, Passwort-Reset-Links verfallen nach 60 Minuten.
            Account-Daten bleiben bis zur Löschung des Accounts gespeichert. Die Löschung kannst du jederzeit per E-Mail beantragen.</p>

        <h3>6. Deine Rechte</h3>
        <p>Du hast das Recht auf Auskunft, Berichtigung, Löschung, Einschränkung der Verarbeitung, Datenübertragbarkeit und Widerspruch
            sowie das Recht auf Beschwerde bei einer Datenschutz-Aufsichtsbehörde.</p>
    </div>
</section>
