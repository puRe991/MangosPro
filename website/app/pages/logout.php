<?php
// Abmelden nur per POST mit CSRF-Token (kein Logout über fremde Links)
if (is_post() && csrf_check()) {
    Auth::logout();
    session_start();
    flash('info', 'Du wurdest abgemeldet.');
}
redirect('');
