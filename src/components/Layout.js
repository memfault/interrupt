import React from 'react';
import Menu from './Menu';
import Header from './Header';
import Footer from './Footer';
import NotificationBanner from './NotificationBanner';
import Sidebar from './Sidebar';

/**
 * Base layout component.
 *
 * Props:
 *   sidebar     - boolean: show the split-view sidebar
 *   homepageLayout - boolean: use homepage header style (logo-centered)
 *   pageUrl     - string: current page URL (passed to ShareButtons)
 *   pageTitle   - string: current page title (passed to ShareButtons)
 *   children    - main content
 */
const Layout = ({
  children,
  sidebar = false,
  homepageLayout = false,
  pageUrl,
  pageTitle,
}) => {
  return (
    <div id="wrap">
      {/* Hamburger nav */}
      <Menu />

      {/* Top notification banner */}
      <NotificationBanner />

      {/* Menu icon (hamburger button) */}
      <a id="nav-menu">
        <div id="menu"></div>
      </a>

      {homepageLayout ? (
        // Homepage uses a centred logo header
        <div id="header">
          <img id="logo" src="/img/interrupt-logo.svg" alt="Interrupt Logo" />
          <h1 className="header">Interrupt</h1>
          <h3 className="header">
            by <a href="https://memfault.com">Memfault</a>
          </h3>
        </div>
      ) : (
        <Header />
      )}

      {sidebar ? (
        // Split view: main content + sidebar
        <div id="split-container-wrapper">
          <div id="split-container">
            <main>{children}</main>
            <Sidebar pageUrl={pageUrl} pageTitle={pageTitle} />
          </div>
        </div>
      ) : (
        // Default: full-width main content
        <div id="container">
          <main>{children}</main>
        </div>
      )}

      <Footer />

      {/* HubSpot analytics */}
      <script
        type="text/javascript"
        id="hs-script-loader"
        async
        defer
        src="//js.hs-scripts.com/8551838.js"
      />
    </div>
  );
};

export default Layout;
