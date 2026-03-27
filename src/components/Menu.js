import React, { useEffect } from 'react';
import { Link } from 'gatsby';

const Menu = () => {
  useEffect(() => {
    // Initialize dark mode toggle
    const btn = document.getElementById('theme-toggle');
    if (!btn) return;

    updateThemeToggle(btn);

    btn.addEventListener('click', function () {
      const current = document.documentElement.getAttribute('data-theme');
      let next;
      if (!current) {
        next = 'dark';
      } else if (current === 'dark') {
        next = 'light';
      } else {
        next = null;
      }

      if (next) {
        document.documentElement.setAttribute('data-theme', next);
        localStorage.setItem('theme', next);
      } else {
        document.documentElement.removeAttribute('data-theme');
        localStorage.removeItem('theme');
      }

      updateThemeToggle(btn);
    });
  }, []);

  useEffect(() => {
    // Initialize hamburger menu toggle
    const icon = document.getElementById('nav-menu');
    if (!icon) return;

    const handleClick = () => {
      const nav = document.getElementById('nav');
      const button = document.getElementById('menu');
      const site = document.getElementById('wrap');

      if (nav.className === 'menu-open') {
        nav.className = '';
        button.className = '';
        site.className = '';
      } else {
        nav.className += 'menu-open';
        button.className += 'btn-close';
        site.className += 'fixed';
      }
    };

    icon.addEventListener('click', handleClick);
    return () => icon.removeEventListener('click', handleClick);
  }, []);

  return (
    <nav id="nav">
      <div id="nav-list">
        <Link to="/">Home</Link>
        <Link to="/blog" title="Blog">Blog</Link>
        <Link to="/jobs" title="Jobs">Jobs</Link>
        <Link to="/events" title="Events">Events</Link>
        <a href="https://github.com/memfault/interrupt">Github</a>
        <a href="https://memfault.com">About Memfault</a>
      </div>
      <footer>
        <button
          id="theme-toggle"
          title="System theme (click for dark)"
          aria-label="Toggle dark mode"
        >
          <i className="fa fa-adjust" aria-hidden="true"></i>
          <span id="theme-toggle-label">System theme</span>
        </button>
      </footer>
    </nav>
  );
};

function updateThemeToggle(btn) {
  if (!btn) return;
  const theme = document.documentElement.getAttribute('data-theme');
  const icon = btn.querySelector('i');
  const label = btn.querySelector('#theme-toggle-label');

  if (theme === 'dark') {
    icon.className = 'fa fa-moon-o';
    btn.title = 'Dark mode (click for light)';
    if (label) label.textContent = 'Dark mode';
  } else if (theme === 'light') {
    icon.className = 'fa fa-sun-o';
    btn.title = 'Light mode (click for system)';
    if (label) label.textContent = 'Light mode';
  } else {
    icon.className = 'fa fa-adjust';
    btn.title = 'System theme (click for dark)';
    if (label) label.textContent = 'System theme';
  }
}

export default Menu;
