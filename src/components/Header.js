import React from 'react';
import { Link } from 'gatsby';

const Header = () => (
  <header id="header">
    <Link to="/">
      <img id="logo" src="/img/interrupt-logo.svg" alt="Interrupt Logo" />
      <h1>Interrupt</h1>
    </Link>
    <h3 className="no-anchor">
      by <a href="https://memfault.com">Memfault</a>
    </h3>
    <br />
    <a href="/feed.xml">
      <svg className="svg-icon rss">
        <use xlinkHref="/img/social-icons.svg#rss"></use>
      </svg>
      <span>RSS</span>
    </a>
    &nbsp;&nbsp;&nbsp;&nbsp;
    <a href="https://go.memfault.com/interrupt-subscribe">Subscribe</a>
    &nbsp;&nbsp;&nbsp;&nbsp;
    <Link to="/contributing">Contribute</Link>
    &nbsp;&nbsp;&nbsp;&nbsp;
    <Link to="/tags">Tags</Link>
    &nbsp;&nbsp;
    <a href="https://interrupt-slack.herokuapp.com/">
      <svg className="svg-icon slack">
        <use xlinkHref="/img/social-icons.svg#slack"></use>
      </svg>
      <span>Slack</span>
    </a>
    &nbsp;&nbsp;&nbsp;&nbsp;
    <Link to="/jobs">Jobs</Link>
    &nbsp;&nbsp;&nbsp;&nbsp;
    <Link to="/events">Events</Link>
    &nbsp;&nbsp;&nbsp;&nbsp;
  </header>
);

export default Header;
