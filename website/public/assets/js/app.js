/* MangosPro Website – kleine Progressive-Enhancement-Helfer (ohne Abhängigkeiten) */
(function () {
    'use strict';

    // Mobiles Menü
    var toggle = document.querySelector('[data-nav-toggle]');
    var nav = document.getElementById('mainnav');
    if (toggle && nav) {
        toggle.addEventListener('click', function () {
            var open = nav.classList.toggle('open');
            toggle.setAttribute('aria-expanded', open ? 'true' : 'false');
        });
    }

    // Fraktionsbalken (Breite per JS, da die CSP keine Inline-Styles erlaubt)
    document.querySelectorAll('.faction-bar [data-w]').forEach(function (el) {
        var w = parseInt(el.getAttribute('data-w'), 10) || 0;
        el.style.flexBasis = w + '%';
        el.style.flexGrow = '0';
        if (w === 0) el.style.display = 'none';
    });

    // In die Zwischenablage kopieren
    document.querySelectorAll('[data-copy]').forEach(function (btn) {
        btn.addEventListener('click', function () {
            var src = document.querySelector(btn.getAttribute('data-copy'));
            if (!src || !navigator.clipboard) return;
            navigator.clipboard.writeText(src.textContent.trim()).then(function () {
                var old = btn.textContent;
                btn.textContent = 'Kopiert ✓';
                setTimeout(function () { btn.textContent = old; }, 1800);
            });
        });
    });

    // Passwortstärke
    document.querySelectorAll('input[data-strength]').forEach(function (input) {
        var bar = input.parentNode.querySelector('.strength');
        if (!bar) return;
        input.addEventListener('input', function () {
            var v = input.value, s = 0;
            if (v.length >= 6) s++;
            if (v.length >= 10) s++;
            if (/[0-9]/.test(v) && /[a-zA-Z]/.test(v)) s++;
            if (/[^a-zA-Z0-9]/.test(v)) s++;
            if (v.length < 6) s = v.length ? 1 : 0;
            bar.setAttribute('data-level', String(s));
            bar.firstElementChild.style.width = (s * 25) + '%';
        });
    });

    // Live-Prüfung im Registrierungsformular
    var reg = document.querySelector('form[data-register]');
    if (reg) {
        var p1 = reg.querySelector('#password'), p2 = reg.querySelector('#password2'), user = reg.querySelector('#username');
        var check = function () {
            if (p2.value) p2.setCustomValidity(p1.value === p2.value ? '' : 'Die Passwörter stimmen nicht überein.');
        };
        p1.addEventListener('input', check);
        p2.addEventListener('input', check);
        user.addEventListener('input', function () {
            user.value = user.value.replace(/[^A-Za-z0-9]/g, '').toUpperCase();
        });
    }

    // Bestätigungsdialoge
    document.querySelectorAll('form[data-confirm]').forEach(function (f) {
        f.addEventListener('submit', function (ev) {
            if (!window.confirm(f.getAttribute('data-confirm'))) ev.preventDefault();
        });
    });

    // Sortierbare Tabellen
    document.querySelectorAll('table[data-sortable]').forEach(function (table) {
        table.querySelectorAll('th').forEach(function (th, idx) {
            if (!th.hasAttribute('data-sort')) th.setAttribute('data-sort', 'text');
            var asc = true;
            th.addEventListener('click', function () {
                var tbody = table.tBodies[0];
                var rows = Array.prototype.slice.call(tbody.rows);
                var num = th.getAttribute('data-sort') === 'num';
                rows.sort(function (a, b) {
                    var x = a.cells[idx].textContent.trim(), y = b.cells[idx].textContent.trim();
                    var r = num ? (parseFloat(x) || 0) - (parseFloat(y) || 0) : x.localeCompare(y, 'de');
                    return asc ? r : -r;
                });
                asc = !asc;
                rows.forEach(function (r) { tbody.appendChild(r); });
            });
        });
    });
})();
