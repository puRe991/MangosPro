<?php
page_title('Passwort vergessen');
if (Auth::check()) redirect('account/password');
$sent = false;
$error = null;

if (is_post() && Mailer::enabled()) {
    require_csrf();
    $ip = client_ip();
    $username = post('username');
    $email = strtolower(post('email'));
    if (RateLimiter::tooMany('reset', $ip, 3, 3600)) {
        $error = 'Zu viele Anfragen. Bitte versuche es in einer Stunde erneut.';
    } elseif ($username === '' || $email === '') {
        $error = 'Bitte Accountname und E-Mail-Adresse angeben.';
    } else {
        RateLimiter::hit('reset', $ip);
        $acc = Account::findByName($username);
        // Immer dieselbe Antwort – verrät nicht, ob Account/E-Mail existieren
        if ($acc && $acc['email'] !== '' && hash_equals(strtolower((string)$acc['email']), $email)) {
            $token = bin2hex(random_bytes(32));
            Database::run(Database::web(),
                'REPLACE INTO web_password_resets (account_id, token_hash, expires_at, created_ip) VALUES (?, ?, ?, ?)',
                [(int)$acc['id'], hash('sha256', $token), time() + 3600, $ip]);
            Mailer::send($email, 'Passwort zurücksetzen – ' . config('site.name'),
                "Hallo {$acc['username']},\n\nfür deinen Account wurde ein neues Passwort angefordert.\n"
                . "Über diesen Link kannst du es innerhalb der nächsten Stunde festlegen:\n\n"
                . url('reset', ['token' => $token]) . "\n\n"
                . "Wenn du das nicht warst, ignoriere diese E-Mail einfach – dein Passwort bleibt unverändert.\n"
                . "(Anfrage von IP $ip)\n\n" . config('site.name'));
        }
        $sent = true;
    }
}
?>
<section class="wrap section auth-page narrow">
    <div class="card auth-card">
        <h1 class="card-title">Passwort vergessen</h1>
        <?php if (!Mailer::enabled()): ?>
            <div class="alert alert-info">Das automatische Zurücksetzen ist auf diesem Server nicht aktiviert.
                Bitte wende dich an <a href="mailto:<?= e(config('site.contact')) ?>"><?= e(config('site.contact')) ?></a>
                <?= config('site.discord') ? 'oder melde dich im <a href="' . e(config('site.discord')) . '" target="_blank" rel="noopener">Discord</a>' : '' ?>.</div>
        <?php elseif ($sent): ?>
            <div class="alert alert-success">Falls Accountname und E-Mail zusammenpassen, haben wir dir einen Link geschickt.
                Prüfe auch deinen Spam-Ordner. Der Link ist 60 Minuten gültig.</div>
        <?php else: ?>
            <p class="muted">Gib deinen Accountnamen und die hinterlegte E-Mail-Adresse ein. Du bekommst einen Link zum Festlegen eines neuen Passworts.</p>
            <?php if ($error): ?><div class="alert alert-error"><?= e($error) ?></div><?php endif; ?>
            <form method="post" action="<?= e(url('forgot')) ?>">
                <?= csrf_field() ?>
                <div class="field">
                    <label for="username">Accountname</label>
                    <input id="username" name="username" type="text" required maxlength="16" autocomplete="username">
                </div>
                <div class="field">
                    <label for="email">E-Mail-Adresse</label>
                    <input id="email" name="email" type="email" required maxlength="120" autocomplete="email">
                </div>
                <button class="btn btn-primary btn-block" type="submit">Link anfordern</button>
            </form>
        <?php endif; ?>
        <p class="center muted small"><a href="<?= e(url('login')) ?>">Zurück zur Anmeldung</a></p>
    </div>
</section>
