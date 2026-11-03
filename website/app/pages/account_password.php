<?php
Auth::requireLogin();
page_title('Passwort ändern');
$errors = [];

if (is_post()) {
    require_csrf();
    $current = (string)($_POST['current'] ?? '');
    $new = (string)($_POST['password'] ?? '');
    $new2 = (string)($_POST['password2'] ?? '');
    $ip = client_ip();

    if (RateLimiter::tooMany('pwchange', $ip, 5, 900)) {
        $errors['form'] = 'Zu viele Versuche. Bitte später erneut versuchen.';
    } elseif (!Account::verify(Auth::name(), $current)) {
        RateLimiter::hit('pwchange', $ip);
        $errors['current'] = 'Das aktuelle Passwort ist falsch.';
    }
    if (!$errors) {
        if ($e = Account::validatePassword($new, Auth::name())) $errors['password'] = $e;
        elseif ($new !== $new2) $errors['password2'] = 'Die Passwörter stimmen nicht überein.';
        elseif (strcasecmp($new, $current) === 0) $errors['password'] = 'Das neue Passwort muss sich vom alten unterscheiden.';
    }
    if (!$errors) {
        Account::setPassword(Auth::id(), Auth::name(), $new);
        session_regenerate_id(true);
        flash('success', 'Passwort geändert. Nutze ab sofort das neue Passwort im Spiel.');
        redirect('account');
    }
}
$err = fn(string $k) => isset($errors[$k]) ? '<p class="field-error">' . e($errors[$k]) . '</p>' : '';
?>
<section class="wrap section auth-page narrow">
    <div class="card auth-card frame" data-reveal>
        <p><a class="muted small" href="<?= e(url('account')) ?>">← Zurück zum Account</a></p>
        <h1 class="card-title"><?= icon('lock') ?> Passwort ändern</h1>
        <?php if (isset($errors['form'])): ?><div class="alert alert-error"><?= e($errors['form']) ?></div><?php endif; ?>
        <form method="post" action="<?= e(url('account/password')) ?>">
            <?= csrf_field() ?>
            <input type="text" name="username" value="<?= e(Auth::name()) ?>" autocomplete="username" hidden>
            <div class="field">
                <label for="current">Aktuelles Passwort</label>
                <input id="current" name="current" type="password" required maxlength="16" autocomplete="current-password">
                <?= $err('current') ?>
            </div>
            <div class="field">
                <label for="password">Neues Passwort</label>
                <input id="password" name="password" type="password" required minlength="6" maxlength="16" autocomplete="new-password" data-strength>
                <div class="strength" aria-hidden="true"><span></span></div>
                <?= $err('password') ?>
            </div>
            <div class="field">
                <label for="password2">Neues Passwort wiederholen</label>
                <input id="password2" name="password2" type="password" required minlength="6" maxlength="16" autocomplete="new-password">
                <?= $err('password2') ?>
            </div>
            <p class="hint">Nach der Änderung wirst du im Spiel abgemeldet, falls du gerade eingeloggt bist.</p>
            <button class="btn btn-primary btn-block" type="submit">Passwort speichern</button>
        </form>
    </div>
</section>
