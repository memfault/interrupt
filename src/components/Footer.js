import React from 'react';

const Footer = () => (
  <footer className="footer">
    <div className="footer-links">
      <a href="/feed.xml">
        <svg className="svg-icon orange">
          <use xlinkHref="/img/social-icons.svg#rss"></use>
        </svg>
        <span>RSS</span>
      </a>
      &nbsp;&nbsp;&nbsp;&nbsp;
      <a href="https://interrupt-slack.herokuapp.com/">
        <svg className="svg-icon slack">
          <use xlinkHref="/img/social-icons.svg#slack"></use>
        </svg>
        <span>Slack</span>
      </a>
      &nbsp;&nbsp;&nbsp;&nbsp;
      <a href="https://go.memfault.com/interrupt-subscribe">Subscribe</a>
      &nbsp;&nbsp;&nbsp;&nbsp;
      <a href="/contributing">Contribute</a>
      &nbsp;&nbsp;&nbsp;&nbsp;
      <a href="https://community.memfault.com">Community</a>
      &nbsp;&nbsp;&nbsp;&nbsp;
      <a href="https://memfault.com">Memfault.com</a>
      &nbsp;&nbsp;&nbsp;&nbsp;
    </div>
    <div>© {new Date().getFullYear()} - Memfault, Inc.</div>
  </footer>
);

export default Footer;
