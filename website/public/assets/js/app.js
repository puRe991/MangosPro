/* MangosPro Website – Progressive Enhancement (ohne Abhängigkeiten) */
(function () {
    'use strict';
    var reduce = window.matchMedia && window.matchMedia('(prefers-reduced-motion: reduce)').matches;

    // Kopfleiste: kompakter beim Scrollen
    var topbar = document.querySelector('.topbar');
    if (topbar) {
        var onScroll = function () { topbar.classList.toggle('scrolled', window.scrollY > 24); };
        onScroll();
        window.addEventListener('scroll', onScroll, { passive: true });
    }

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

    // Einblenden beim Scrollen, Ringe zeichnen, Zahlen hochzählen
    var countUp = function (el) {
        var target = parseInt(el.getAttribute('data-count'), 10);
        if (isNaN(target) || reduce || target === 0) return;
        var start = null, dur = Math.min(1600, 600 + target * 4);
        var fmt = new Intl.NumberFormat('de-DE');
        var step = function (t) {
            if (!start) start = t;
            var p = Math.min(1, (t - start) / dur);
            el.textContent = fmt.format(Math.round(target * (1 - Math.pow(1 - p, 3))));
            if (p < 1) requestAnimationFrame(step);
        };
        el.textContent = '0';
        requestAnimationFrame(step);
    };
    var reveal = function (el) {
        el.classList.add('is-visible');
        el.querySelectorAll('.ring').forEach(function (r) { r.classList.add('is-drawn'); });
        if (el.classList.contains('ring')) el.classList.add('is-drawn');
        el.querySelectorAll('[data-count]').forEach(countUp);
        if (el.hasAttribute('data-count')) countUp(el);
    };
    var targets = document.querySelectorAll('[data-reveal], .ring, [data-count]');
    if ('IntersectionObserver' in window && !reduce) {
        var io = new IntersectionObserver(function (entries) {
            entries.forEach(function (en) {
                if (en.isIntersecting) { reveal(en.target); io.unobserve(en.target); }
            });
        }, { rootMargin: '0px 0px -8% 0px', threshold: 0.08 });
        targets.forEach(function (el) { io.observe(el); });
    } else {
        targets.forEach(reveal);
    }
    // Gestaffelte Verzögerung in Rastern
    document.querySelectorAll('[data-stagger]').forEach(function (grid) {
        Array.prototype.forEach.call(grid.children, function (c, i) {
            c.setAttribute('data-reveal', '');
            c.style.setProperty('--d', (i * 0.08) + 's');
            if (io) io.observe(c); else reveal(c);
        });
    });

    // Glut-Partikel im Hero
    var canvas = document.querySelector('canvas.embers');
    if (canvas && canvas.getContext && !reduce) {
        var ctx = canvas.getContext('2d'), parts = [], W = 0, H = 0, dpr = Math.min(window.devicePixelRatio || 1, 2), running = true;
        var resize = function () {
            W = canvas.clientWidth; H = canvas.clientHeight;
            canvas.width = W * dpr; canvas.height = H * dpr;
            ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
        };
        var spawn = function (initial) {
            var fel = Math.random() < 0.55;
            return {
                x: W * (0.45 + Math.random() * 0.55), y: initial ? Math.random() * H : H + 10,
                r: 0.6 + Math.random() * 2.1, vy: 0.25 + Math.random() * 0.75, vx: -0.25 + Math.random() * 0.5,
                life: 0, max: 260 + Math.random() * 380, sway: Math.random() * Math.PI * 2,
                c: fel ? '143,232,90' : '243,190,110'
            };
        };
        resize();
        var count = Math.round(Math.min(90, W / 16));
        for (var i = 0; i < count; i++) parts.push(spawn(true));
        window.addEventListener('resize', resize);
        document.addEventListener('visibilitychange', function () { running = !document.hidden; if (running) requestAnimationFrame(tick); });
        var tick = function () {
            if (!running) return;
            ctx.clearRect(0, 0, W, H);
            ctx.globalCompositeOperation = 'lighter';
            for (var j = 0; j < parts.length; j++) {
                var p = parts[j];
                p.life++; p.sway += 0.02;
                p.y -= p.vy; p.x += p.vx + Math.sin(p.sway) * 0.35;
                var a = Math.sin(Math.min(1, p.life / p.max) * Math.PI);
                if (p.life > p.max || p.y < -20) { parts[j] = spawn(false); continue; }
                var g = ctx.createRadialGradient(p.x, p.y, 0, p.x, p.y, p.r * 5);
                g.addColorStop(0, 'rgba(' + p.c + ',' + (0.9 * a) + ')');
                g.addColorStop(1, 'rgba(' + p.c + ',0)');
                ctx.fillStyle = g;
                ctx.beginPath(); ctx.arc(p.x, p.y, p.r * 5, 0, Math.PI * 2); ctx.fill();
            }
            requestAnimationFrame(tick);
        };
        requestAnimationFrame(tick);
    }

    // In die Zwischenablage kopieren
    document.querySelectorAll('[data-copy]').forEach(function (btn) {
        btn.addEventListener('click', function () {
            var src = document.querySelector(btn.getAttribute('data-copy'));
            if (!src || !navigator.clipboard) return;
            navigator.clipboard.writeText(src.textContent.trim()).then(function () {
                var label = btn.querySelector('span') || btn;
                var old = label.textContent;
                label.textContent = 'Kopiert ✓';
                setTimeout(function () { label.textContent = old; }, 1800);
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
