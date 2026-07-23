/* ============================================================
   matt.bonis — portfolio interactions
   no frameworks, vanilla JS
   ============================================================ */
(() => {
  const $  = (s, r = document) => r.querySelector(s);
  const $$ = (s, r = document) => Array.from(r.querySelectorAll(s));

  // ───── footer year
  const yearEl = $('#year');
  if (yearEl) yearEl.textContent = new Date().getFullYear();

  // ───── mobile nav toggle
  const nav = $('#nav');
  const navToggle = $('#navToggle');
  if (nav && navToggle) {
    navToggle.addEventListener('click', () => {
      const open = nav.classList.toggle('is-open');
      navToggle.setAttribute('aria-expanded', String(open));
    });
    $$('.nav__links a').forEach(a =>
      a.addEventListener('click', () => nav.classList.remove('is-open'))
    );
  }

  // ───── smooth scroll for in-page anchors (with header offset)
  const HEADER_H = 64;
  $$('a[href^="#"]').forEach(a => {
    a.addEventListener('click', e => {
      const id = a.getAttribute('href');
      if (!id || id === '#') return;
      const target = document.querySelector(id);
      if (!target) return;
      e.preventDefault();
      const y = target.getBoundingClientRect().top + window.scrollY - HEADER_H + 1;
      window.scrollTo({ top: y, behavior: 'smooth' });
    });
  });

  // ───── active section highlight in nav
  const sections = $$('section[id]');
  const navLinks = $$('.nav__links a');
  const setActive = () => {
    const y = window.scrollY + HEADER_H + 10;
    let current = null;
    for (const s of sections) {
      if (s.offsetTop <= y) current = s.id;
    }
    navLinks.forEach(a => {
      a.classList.toggle('is-active', a.getAttribute('href') === '#' + current);
    });
  };
  window.addEventListener('scroll', setActive, { passive: true });
  setActive();

  // ───── reveal on scroll
  const targets = [
    '.hero__text > *',
    '.hero__panel',
    '.about__bio > *',
    '.stats li',
    '.card',
    '.contact__list li',
    '.section-head'
  ];
  const revealEls = $$(targets.join(','));
  revealEls.forEach(el => el.classList.add('reveal'));

  if ('IntersectionObserver' in window) {
    const io = new IntersectionObserver(entries => {
      entries.forEach((e, i) => {
        if (e.isIntersecting) {
          // small stagger for groups
          setTimeout(() => e.target.classList.add('is-in'), Math.min(i * 60, 240));
          io.unobserve(e.target);
        }
      });
    }, { threshold: 0.12, rootMargin: '0px 0px -40px 0px' });
    revealEls.forEach(el => io.observe(el));
  } else {
    revealEls.forEach(el => el.classList.add('is-in'));
  }
})();
