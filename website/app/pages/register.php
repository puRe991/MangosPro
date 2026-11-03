<?php
page_title('Account erstellen');
if (Auth::check()) redirect('account');

$errors = [];
$old = ['username' => '', 'email' => ''];
$open = (bool)config('registration.open', true);
$needEmail = (bool)config('registration.require_email', true);

if (is_post() && $open) {
    require_csrf();
    $ip = client_ip();
    $old['username'] = $username = post('username');
    $old['email'] = $email = strtolower(post('email'));
    $password = (string)($_POST['password'] ?? '');
    $password2 = (string)($_POST['password2'] ?? '');

    if (post('website') !== '') {
        // Honeypot ausgefüllt -> Bot. Still "erfolgreich" tun.
        flash('success', 'Account erstellt.');
        redirect('');
    }
    if (config('registration.captcha', true) && !captcha_valid('register', post('captcha'))) {
        $errors['captcha'] = 'Die Rechenaufgabe wurde falsch gelöst.';
    }
    if ($e = Account::validateUsername($username)) $errors['username'] = $e;
    if ($e = Account::validatePassword($password, $username)) $errors['password'] = $e;
    if ($password !== $password2) $errors['password2'] = 'Die Passwörter stimmen nicht überein.';
    if ($needEmail || $email !== '') {
        if (!filter_var($email, FILTER_VALIDATE_EMAIL) || strlen($email) > 120) $errors['email'] = 'Bitte eine gültige E-Mail-Adresse angeben.';
        elseif (Account::emailInUse($email)) $errors['email'] = 'Diese E-Mail-Adresse wird bereits verwendet.';
    }
    if (!isset($_POST['accept'])) $errors['accept'] = 'Bitte akzeptiere die Regeln und die Datenschutzerklärung.';

    if (!$errors && Account::ipBanned($ip)) {
        $errors['form'] = 'Von deiner IP-Adresse aus ist keine Registrierung möglich.';
    }
    if (!$errors) {
        $max = (int)config('registration.max_per_ip_per_day', 3);
        $count = (int)Database::run(Database::web(),
            'SELECT COUNT(*) FROM web_registrations WHERE ip = ? AND created_at > ?', [$ip, time() - 86400])->fetchColumn();
        if ($max > 0 && $count >= $max) $errors['form'] = 'Von dieser IP-Adresse wurden heute bereits zu viele Accounts erstellt.';
    }
    if (!$errors && Account::findByName($username)) {
        $errors['username'] = 'Dieser Accountname ist bereits vergeben.';
    }

    if (!$errors) {
        try {
            $id = Account::create($username, $password, $email);
        } catch (PDOException $ex) {
            if ($ex->getCode() === '23000') {
                $errors['username'] = 'Dieser Accountname ist bereits vergeben.';
            } else {
                throw $ex;
            }
        }
        if (!$errors) {
            Database::run(Database::web(), 'INSERT INTO web_registrations (account_id, ip, created_at) VALUES (?, ?, ?)',
                [$id, $ip, time()]);
            Auth::login(['id' => $id, 'username' => strtoupper($username), 'gmlevel' => 0]);
            if ($email !== '' && Mailer::enabled()) {
                Mailer::send($email, 'Willkommen bei ' . config('site.name'),
                    "Hallo " . strtoupper($username) . ",\n\ndein Account wurde erstellt.\n\n"
                    . "Realmlist: set realmlist " . config('site.realmlist') . "\n"
                    . "Anleitung: " . url('connect') . "\n\nViel Spaß!\n" . config('site.name'));
            }
            flash('success', 'Willkommen! Dein Account ' . strtoupper($username) . ' wurde erstellt. Du kannst dich jetzt im Spiel einloggen.');
            redirect('connect');
        }
    }
}
$captcha = config('registration.captcha', true) ? captcha_question('register') : null;
$err = fn(string $k) => isset($errors[$k]) ? '<p class="field-error">' . e($errors[$k]) . '</p>' : '';
?>
<section class="wrap section auth-page">
    <div class="auth-aside">
        <p class="eyebrow">Kostenlos · Keine Downloads von uns nötig</p>
        <h1>Erstelle deinen Account</h1>
        <p class="lead">Mit diesem Account loggst du dich im WoW-Client <strong><?= e(config('site.client')) ?></strong> ein.</p>
        <ul class="checklist">
            <li>Ein Account für alle Realms</li>
            <li>Bis zu 10 Charaktere pro Realm</li>
            <li>Passwort jederzeit hier änderbar</li>
            <li>Playerbots per Chatbefehl in deine Gruppe holen</li>
        </ul>
        <p class="muted small">Schon registriert? <a href="<?= e(url('login')) ?>">Hier anmelden</a>.</p>
    </div>

    <div class="card auth-card">
        <?php if (!$open): ?>
            <div class="alert alert-info">Die Registrierung ist momentan geschlossen.</div>
        <?php else: ?>
        <?php if (isset($errors['form'])): ?><div class="alert alert-error"><?= e($errors['form']) ?></div><?php endif; ?>
        <form method="post" action="<?= e(url('register')) ?>" novalidate data-register>
            <?= csrf_field() ?>
            <div class="field">
                <label for="username">Accountname</label>
                <input id="username" name="username" type="text" required minlength="3" maxlength="16" pattern="[A-Za-z0-9]+"
                       autocomplete="username" autocapitalize="characters" spellcheck="false" value="<?= e($old['username']) ?>">
                <p class="hint">3–16 Zeichen, nur A–Z und 0–9. Damit loggst du dich im Spiel ein.</p>
                <?= $err('username') ?>
            </div>
            <div class="field">
                <label for="email">E-Mail<?= $needEmail ? '' : ' <span class="muted">(optional)</span>' ?></label>
                <input id="email" name="email" type="email" maxlength="120" <?= $needEmail ? 'required' : '' ?>
                       autocomplete="email" value="<?= e($old['email']) ?>">
                <p class="hint">Nur für "Passwort vergessen". Wird nicht weitergegeben.</p>
                <?= $err('email') ?>
            </div>
            <div class="field-row">
                <div class="field">
                    <label for="password">Passwort</label>
                    <input id="password" name="password" type="password" required minlength="6" maxlength="16" autocomplete="new-password" data-strength>
                    <div class="strength" aria-hidden="true"><span></span></div>
                    <?= $err('password') ?>
                </div>
                <div class="field">
                    <label for="password2">Passwort wiederholen</label>
                    <input id="password2" name="password2" type="password" required minlength="6" maxlength="16" autocomplete="new-password">
                    <?= $err('password2') ?>
                </div>
            </div>
            <p class="hint">6–16 Zeichen (Limit des 2.4.3-Clients). Groß-/Kleinschreibung spielt im Spiel keine Rolle.</p>

            <?php if ($captcha): ?>
                <div class="field">
                    <label for="captcha"><?= e($captcha) ?></label>
                    <input id="captcha" name="captcha" type="text" inputmode="numeric" required maxlength="3" autocomplete="off" class="input-short">
                    <?= $err('captcha') ?>
                </div>
            <?php endif; ?>
            <div class="hp" aria-hidden="true">
                <label for="website">Website</label>
                <input id="website" name="website" type="text" tabindex="-1" autocomplete="off">
            </div>

            <label class="check">
                <input type="checkbox" name="accept" value="1" required <?= isset($_POST['accept']) ? 'checked' : '' ?>>
                <span>Ich akzeptiere die <a href="<?= e(url('rules')) ?>" target="_blank">Serverregeln</a> und die
                    <a href="<?= e(url('privacy')) ?>" target="_blank">Datenschutzerklärung</a>.</span>
            </label>
            <?= $err('accept') ?>

            <button class="btn btn-primary btn-block btn-lg" type="submit">Account erstellen</button>
        </form>
        <?php endif; ?>
    </div>
</section>
