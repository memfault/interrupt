import React from 'react';
import { Link } from 'gatsby';
import ShareButtons from './ShareButtons';

const Sidebar = ({ pageUrl, pageTitle }) => (
  <aside>
    {/* Search */}
    <form id="search-form">
      <label htmlFor="search-input">
        <h3>Search</h3>
      </label>
      <script
        async
        src="https://cse.google.com/cse.js?cx=8478db8b25997435c"
      />
      <div className="gcse-search"></div>
    </form>

    {/* Subscribe (HubSpot form) */}
    <div id="subscribe">
      <script
        charSet="utf-8"
        type="text/javascript"
        src="//js.hsforms.net/forms/v2.js"
      />
      <script
        dangerouslySetInnerHTML={{
          __html: `
            if (typeof hbspt !== 'undefined') {
              hbspt.forms.create({ portalId: "8551838", formId: "a61bb5a0-4339-4398-b639-c0894f961532" });
            }
          `,
        }}
      />
    </div>

    {/* Posts navigation */}
    <div className="memfault-info-wrapper">
      <h3>Latest Blog Posts</h3>
      <p>See the latest content from the Interrupt community.</p>
      <p>
        <Link to="/#latest-blog-posts">
          View latest blog posts{' '}
          <img className="chevron-icon" src="/img/chevron-right.svg" alt="" />
        </Link>
      </p>
    </div>

    <div className="memfault-info-wrapper">
      <h3>Top Blog Posts</h3>
      <p>Read the posts that readers love the most.</p>
      <p>
        <Link to="/#top-blog-posts">
          View top blog posts{' '}
          <img className="chevron-icon" src="/img/chevron-right.svg" alt="" />
        </Link>
      </p>
    </div>

    {/* By Memfault */}
    <div className="memfault-info-wrapper">
      <p>
        <img className="mf-icon" src="/img/memfault-logo.png" alt="Memfault" />
        Brought to you with <span style={{ fontSize: '10px' }}>❤️</span> by
        Memfault.
        <br />
        <a href="https://memfault.com/">
          Learn more{' '}
          <img className="chevron-icon" src="/img/chevron-right.svg" alt="" />
        </a>
      </p>
    </div>

    {/* Share buttons */}
    <ShareButtons url={pageUrl} title={pageTitle} />
  </aside>
);

export default Sidebar;
