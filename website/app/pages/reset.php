<?php
page_title('Neues Passwort festlegen');
$token = query('token', post('token'));
$row = null;
if (preg_match('/^[a-f0-9]{64}$/', $token)) {
    $row = Database::run(Database::web(),
        'SELECT r.account_id, r.expires_at FROM web_password_resets r WHERE r.token_hash = ? AND r.expires_at > ?',
        [hash('sha256', $token), time()])->fetch() ?: null;
}
$acc = $row ? Account::find((int)$row['account_id']) : null;
$errors = [];

if ($acc && is_post()) {
    require_csrf();
    $new = (string)($_POST['password'] ?? '');
    if ($e = Account::validatePassword($new, $acc['username'])) $errors['password'] = $e;
    elseif ($new !== (string)($_POST['password2'] ?? '')) $errors['password2'] = 'Die Passwörter stimmen nicht überein.';
    if (!$errors) {
        Account::setPassword((int)$acc['id'], $acc['username'], $new);
        Database::run(Database::web(), 'DELETE FROM web_password_resets WHERE account_id = ?', [(int)$acc['id']]);
        flash('success', 'Dein Passwort wurde geändert. Du kannst dich jetzt anmelden.');
        redirect('login');
    }
}
$err = fn(string $k) => isset($errors[$k]) ? '<p class="field-error">' . e($errors[$k]) . '</p>' : '';
?>
<section class="wrap section auth-page narrow">
    <div class="card auth-card frame" data-reveal>
        <h1 class="card-title"><?= icon('lock') ?> Neues Passwort</h1>
        <?php if (!$acc): ?>
            <div class="alert alert-error">Dieser Link ist ungültig oder abgelaufen.</div>
            <a class="btn btn-ghost btn-block" href="<?= e(url('forgot')) ?>">Neuen Link anfordern</a>
        <?php else: ?>
            <p class="muted">Account: <strong><?= e($acc['username']) ?></strong></p>
            <form method="post" action="<?= e(url('reset')) ?>">
                <?= csrf_field() ?>
                <input type="hidden" name="token" value="<?= e($token) ?>">
                <input type="text" name="username" value="<?= e($acc['username']) ?>" autocomplete="username" hidden>
                <div class="field">
                    <label for="password">Neues Passwort</label>
                    <input id="password" name="password" type="password" required minlength="6" maxlength="16" autocomplete="new-password" data-strength>
                    <div class="strength" aria-hidden="true"><span></span></div>
                    <?= $err('password') ?>
                </div>
                <div class="field">
                    <label for="password2">Wiederholen</label>
                    <input id="password2" name="password2" type="password" required minlength="6" maxlength="16" autocomplete="new-password">
                    <?= $err('password2') ?>
                </div>
                <button class="btn btn-primary btn-block" type="submit">Passwort speichern</button>
            </form>
        <?php endif; ?>
    </div>
</section>
