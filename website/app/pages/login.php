<?php
page_title('Anmelden');
if (Auth::check()) redirect('account');

$error = null;
$username = '';
$next = query('next', post('next'));
// Nur interne Routen als Weiterleitung zulassen
if (!preg_match('#^[a-z0-9/_-]*$#', $next) || in_array($next, ['login', 'logout', 'register'], true)) $next = 'account';

if (is_post()) {
    require_csrf();
    $ip = client_ip();
    $username = post('username');
    $password = (string)($_POST['password'] ?? '');
    $max = (int)config('security.login_max_attempts', 5);
    $window = (int)config('security.login_window', 900);

    if (RateLimiter::tooMany('login', $ip, $max, $window)) {
        $error = 'Zu viele Fehlversuche. Bitte warte ' . (int)ceil($window / 60) . ' Minuten.';
    } elseif ($username === '' || $password === '') {
        $error = 'Bitte Accountname und Passwort eingeben.';
    } else {
        $acc = Account::verify($username, $password);
        if (!$acc) {
            RateLimiter::hit('login', $ip);
            $error = 'Accountname oder Passwort ist falsch.';
        } else {
            RateLimiter::clear('login', $ip);
            Auth::login($acc);
            $ban = Account::activeBan((int)$acc['id']);
            if ($ban) flash('error', 'Hinweis: Dieser Account ist gesperrt. Grund: ' . $ban['reason']);
            else flash('success', 'Willkommen zurück, ' . ucfirst(strtolower($acc['username'])) . '!');
            redirect($next);
        }
    }
}
?>
<section class="wrap section auth-page narrow">
    <div class="card auth-card frame" data-reveal>
        <h1 class="card-title"><?= icon('key') ?> Anmelden</h1>
        <p class="muted">Mit deinem Spiel-Account (gleiche Daten wie im WoW-Client).</p>
        <?php if ($error): ?><div class="alert alert-error"><?= e($error) ?></div><?php endif; ?>
        <form method="post" action="<?= e(url('login')) ?>">
            <?= csrf_field() ?>
            <input type="hidden" name="next" value="<?= e($next) ?>">
            <div class="field">
                <label for="username">Accountname</label>
                <input id="username" name="username" type="text" required maxlength="16" autocomplete="username"
                       autocapitalize="characters" spellcheck="false" value="<?= e($username) ?>" autofocus>
            </div>
            <div class="field">
                <label for="password">Passwort <a class="label-link" href="<?= e(url('forgot')) ?>">Vergessen?</a></label>
                <input id="password" name="password" type="password" required maxlength="16" autocomplete="current-password">
            </div>
            <button class="btn btn-primary btn-block btn-lg" type="submit">Anmelden</button>
        </form>
        <p class="center muted small">Noch keinen Account? <a href="<?= e(url('register')) ?>">Jetzt kostenlos erstellen</a></p>
    </div>
</section>
