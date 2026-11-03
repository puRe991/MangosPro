<?php
Auth::requireLogin();
page_title('E-Mail ändern');
$acc = Account::find(Auth::id());
$errors = [];
$email = (string)($acc['email'] ?? '');

if (is_post()) {
    require_csrf();
    $email = strtolower(post('email'));
    $ip = client_ip();
    if (RateLimiter::tooMany('pwchange', $ip, 5, 900)) {
        $errors['form'] = 'Zu viele Versuche. Bitte später erneut versuchen.';
    } elseif (!Account::verify(Auth::name(), (string)($_POST['current'] ?? ''))) {
        RateLimiter::hit('pwchange', $ip);
        $errors['current'] = 'Das Passwort ist falsch.';
    }
    if (!filter_var($email, FILTER_VALIDATE_EMAIL) || strlen($email) > 120) $errors['email'] = 'Bitte eine gültige E-Mail-Adresse angeben.';
    elseif (Account::emailInUse($email, Auth::id())) $errors['email'] = 'Diese E-Mail-Adresse wird bereits verwendet.';

    if (!$errors) {
        Account::setEmail(Auth::id(), $email);
        flash('success', 'E-Mail-Adresse gespeichert.');
        redirect('account');
    }
}
$err = fn(string $k) => isset($errors[$k]) ? '<p class="field-error">' . e($errors[$k]) . '</p>' : '';
?>
<section class="wrap section auth-page narrow">
    <div class="card auth-card">
        <p><a class="muted small" href="<?= e(url('account')) ?>">← Zurück zum Account</a></p>
        <h1 class="card-title">E-Mail ändern</h1>
        <?php if (isset($errors['form'])): ?><div class="alert alert-error"><?= e($errors['form']) ?></div><?php endif; ?>
        <form method="post" action="<?= e(url('account/email')) ?>">
            <?= csrf_field() ?>
            <div class="field">
                <label for="email">Neue E-Mail-Adresse</label>
                <input id="email" name="email" type="email" required maxlength="120" autocomplete="email" value="<?= e($email) ?>">
                <?= $err('email') ?>
            </div>
            <div class="field">
                <label for="current">Passwort zur Bestätigung</label>
                <input id="current" name="current" type="password" required maxlength="16" autocomplete="current-password">
                <?= $err('current') ?>
            </div>
            <button class="btn btn-primary btn-block" type="submit">Speichern</button>
        </form>
    </div>
</section>
