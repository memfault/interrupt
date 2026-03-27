import React from 'react';
import { graphql, Link } from 'gatsby';
import Layout from '../components/Layout';
import ShareButtons from '../components/ShareButtons';

const SITE_URL = 'https://interrupt.memfault.com';

const IndexPage = ({ data }) => {
  const latestPosts = data.latestPosts.nodes;
  const topPostSlugs = data.topPostsData.posts || [];
  const allPosts = data.allMarkdownRemark.nodes;
  const authorsData = data.allAuthorData.nodes.reduce((acc, n) => {
    acc[n.authorKey] = n;
    return acc;
  }, {});

  // Build top posts in order
  const topPosts = topPostSlugs
    .map(filename => {
      // filename is like "2019-07-09-get-the-most-out-of-the-linker-map-file"
      // we need to match against the slug which is /blog/get-the-most-out-of-the-linker-map-file
      const slugPart = filename.replace(/^\d{4}-\d{2}-\d{2}-/, '');
      return allPosts.find(p => p.fields.slug === `/blog/${slugPart}`);
    })
    .filter(Boolean)
    .slice(0, 6);

  return (
    <Layout sidebar homepageLayout pageUrl={`${SITE_URL}/`}>
      <div className="home-container">
        <p>
          The Interrupt community comprises engineers, hobbyists, and
          enthusiasts with a shared passion for hardware and firmware
          development. We come together to share best practices, problem-solve,
          collaborate on projects, advance the embedded community, and elevate
          device reliability engineering (DRE).
        </p>

        <p>
          The Interrupt Community was created and is moderated today by the
          founders of <a href="https://memfault.com">Memfault</a>.
        </p>

        <ul className="list-of-content">
          <li>
            <Link to="/blog">
              <svg className="svg-icon slack">
                <use xlinkHref="/img/main-icons.svg#blog"></use>
              </svg>
              Blog
            </Link>
          </li>
          <li>
            <Link to="/contributing">
              <svg className="svg-icon slack">
                <use xlinkHref="/img/main-icons.svg#contribute"></use>
              </svg>
              Contribute
            </Link>
          </li>
          <li>
            <a href="https://community.memfault.com/">
              <svg className="svg-icon slack">
                <use xlinkHref="/img/main-icons.svg#forum"></use>
              </svg>
              Forums
            </a>
          </li>
          <li>
            <a href="https://github.com/memfault/interrupt">
              <svg className="svg-icon slack">
                <use xlinkHref="/img/social-icons.svg#github"></use>
              </svg>
              Github
            </a>
          </li>
          <li>
            <Link to="/jobs">
              <svg className="svg-icon slack" viewBox="0 0 9 9">
                <use xlinkHref="/img/main-icons.svg#briefcase"></use>
              </svg>
              Jobs
            </Link>
          </li>
          <li>
            <Link to="/events">
              <svg className="svg-icon slack" viewBox="0 0 9 9">
                <use xlinkHref="/img/main-icons.svg#people"></use>
              </svg>
              Events
            </Link>
          </li>
          <li>
            <a href="https://www.meetup.com/interrupt-memfault-meetup/">
              <svg className="svg-icon slack">
                <use xlinkHref="/img/main-icons.svg#meetup"></use>
              </svg>
              Meetup
            </a>
          </li>
          <li>
            <a href="https://interrupt-slack.herokuapp.com/">
              <svg className="svg-icon slack">
                <use xlinkHref="/img/social-icons.svg#slack"></use>
              </svg>
              Slack
            </a>
          </li>
          <li>
            <a href="https://go.memfault.com/interrupt-subscribe">
              <svg className="svg-icon slack">
                <use xlinkHref="/img/main-icons.svg#subscribe"></use>
              </svg>
              Subscribe
            </a>
          </li>
          <li>
            <a href="/feed.xml">
              <svg className="svg-icon slack">
                <use xlinkHref="/img/social-icons.svg#rss"></use>
              </svg>
              RSS
            </a>
          </li>
          <li>
            <Link to="/tags">
              <svg className="svg-icon slack" viewBox="0 0 448 512">
                <use xlinkHref="/img/main-icons.svg#tag"></use>
              </svg>
              Tags
            </Link>
          </li>
          <li>
            <a href="https://memfault.com/webinars">
              <svg className="svg-icon slack" viewBox="0 0 448 512">
                <use xlinkHref="/img/main-icons.svg#webinar"></use>
              </svg>
              Webinars
            </a>
          </li>
        </ul>

        <h2 className="home-header-2" id="latest-blog-posts">
          <Link to="/#latest-blog-posts">Latest Blog Posts</Link>
        </h2>
        <PostList posts={latestPosts} authorsData={authorsData} />
        <Link to="/blog" className="home-show-more">
          View all posts{' '}
          <img className="chevron-icon" src="/img/chevron-right-white.svg" alt="" />
        </Link>

        <h2 className="home-header-2" id="top-blog-posts">
          <Link to="/#top-blog-posts">Top Blog Posts</Link>
        </h2>
        <PostList posts={topPosts} authorsData={authorsData} />
        <Link to="/blog" className="home-show-more">
          View all posts{' '}
          <img className="chevron-icon" src="/img/chevron-right-white.svg" alt="" />
        </Link>

        <h2 className="home-header-2" id="about-memfault">
          About Memfault
        </h2>
        <p>
          Memfault is the first cloud-based observability platform for
          connected device debugging, monitoring, and updating, which brings the
          efficiencies and innovation of software development to hardware
          processes.{' '}
          <a href="https://demo.memfault.com/demo/start?utm_campaign=Sandbox&utm_source=Interrupt&utm_medium=side%20bar">
            Explore Memfault{' '}
            <svg className="svg-icon slack">
              <use xlinkHref="/img/main-icons.svg#arrow-right"></use>
            </svg>
          </a>
        </p>

        <SocialButtons />
      </div>
    </Layout>
  );
};

const PostList = ({ posts, authorsData }) => (
  <ul id="posts">
    {posts.map(post => {
      if (!post) return null;
      const authorKey = post.frontmatter.author;
      const author = authorsData[authorKey];
      const formattedDate = new Date(post.frontmatter.date).toLocaleDateString(
        'en-GB',
        { day: '2-digit', month: 'short', year: 'numeric' }
      );
      const defaultImage = '/img/interrupt-cover-1200px.png';
      const showImage =
        post.frontmatter.image && post.frontmatter.image !== defaultImage;

      return (
        <li key={post.fields.slug} className="post">
          {showImage && (
            <div className="post-image">
              <Link to={post.fields.slug}>
                <img
                  src={post.frontmatter.image}
                  alt={post.frontmatter.title}
                />
              </Link>
            </div>
          )}
          <h2>
            <Link to={post.fields.slug}>{post.frontmatter.title}</Link>
          </h2>
          <div className="by-line">
            <time dateTime={post.frontmatter.date}>{formattedDate}</time>
            {author && (
              <span>
                {' '}
                by <Link to={`/authors/${authorKey}`}>{author.name}</Link>
              </span>
            )}
          </div>
          <p>{post.rangeExcerpt || post.excerpt}</p>
        </li>
      );
    })}
  </ul>
);

const SocialButtons = () => (
  <div className="share-buttons-wrapper">
    <div id="share-buttons">
      <a
        href="https://twitter.com/memfault"
        className="twitter"
        title="Follow Memfault on Twitter"
      >
        <svg
          version="1.1"
          xmlns="http://www.w3.org/2000/svg"
          viewBox="0 0 512 512"
        >
          <path d="M512,97.248c-19.04,8.352-39.328,13.888-60.48,16.576c21.76-12.992,38.368-33.408,46.176-58.016c-20.288,12.096-42.688,20.64-66.56,25.408C411.872,60.704,384.416,48,354.464,48c-58.112,0-104.896,47.168-104.896,104.992c0,8.32,0.704,16.32,2.432,23.936c-87.264-4.256-164.48-46.08-216.352-109.792c-9.056,15.712-14.368,33.696-14.368,53.056c0,36.352,18.72,68.576,46.624,87.232c-16.864-0.32-33.408-5.216-47.424-12.928c0,0.32,0,0.736,0,1.152c0,51.008,36.384,93.376,84.096,103.136c-8.544,2.336-17.856,3.456-27.52,3.456c-6.72,0-13.504-0.384-19.872-1.792c13.6,41.568,52.192,72.128,98.08,73.12c-35.712,27.936-81.056,44.768-130.144,44.768c-8.608,0-16.864-0.384-25.12-1.44C46.496,446.88,101.6,464,161.024,464c193.152,0,298.752-160,298.752-298.688c0-4.64-0.16-9.12-0.384-13.568C480.224,136.96,497.728,118.496,512,97.248z" />
        </svg>
      </a>
      <a
        href="https://www.linkedin.com/company/memfault/"
        className="linkedin"
        title="Follow Memfault on LinkedIn"
      >
        <svg viewBox="0 0 1792 1792" xmlns="http://www.w3.org/2000/svg">
          <path d="M477 625v991h-330v-991h330zm21-306q1 73-50.5 122t-135.5 49h-2q-82 0-132-49t-50-122q0-74 51.5-122.5t134.5-48.5 133 48.5 51 122.5zm1166 729v568h-329v-530q0-105-40.5-164.5t-126.5-59.5q-63 0-105.5 34.5t-63.5 85.5q-11 30-11 81v553h-329q2-399 2-647t-1-296l-1-48h329v144h-2q20-32 41-56t56.5-52 87-43.5 114.5-15.5q171 0 275 113.5t104 332.5z" />
        </svg>
      </a>
      <a
        href="https://www.youtube.com/channel/UCGHAOw3JpB6zOnwA27dlYcQ"
        className="youtube"
        title="Follow Memfault on YouTube"
      >
        <svg
          height="682pt"
          viewBox="-21 -117 682.667 682"
          width="682pt"
          xmlns="http://www.w3.org/2000/svg"
        >
          <path
            fill="inherit"
            d="M626.813 64.035c-7.375-27.418-28.993-49.031-56.407-56.414C520.324-6.082 319.992-6.082 319.992-6.082s-200.324 0-250.406 13.184c-26.887 7.375-49.031 29.52-56.406 56.933C0 114.113 0 217.97 0 217.97s0 104.379 13.18 153.933c7.382 27.414 28.992 49.028 56.41 56.41C120.195 442.02 320 442.02 320 442.02s200.324 0 250.406-13.184c27.418-7.379 49.032-28.992 56.414-56.406 13.176-50.082 13.176-153.934 13.176-153.934s.527-104.383-13.183-154.46zM256.21 313.915V122.022l166.586 95.946zm0 0"
          />
        </svg>
      </a>
      <a
        href="https://github.com/memfault"
        className="github"
        title="Follow Memfault on GitHub"
      >
        <svg className="svg-icon">
          <use xlinkHref="/img/social-icons.svg#github"></use>
        </svg>
      </a>
    </div>
  </div>
);

export const Head = () => (
  <>
    <title>Interrupt by Memfault</title>
    <meta
      name="description"
      content="A community and blog for embedded software makers"
    />
    <link
      href="https://fonts.googleapis.com/css?family=Source+Code+Pro:400%7CSource+Sans+Pro:300,400,700,900,400italic%7CSignika:700,300,400,600"
      rel="stylesheet"
      type="text/css"
    />
    <link
      href="https://maxcdn.bootstrapcdn.com/font-awesome/4.6.3/css/font-awesome.min.css"
      rel="stylesheet"
    />
    <link
      href="https://interrupt.memfault.com/feed.xml"
      type="application/rss+xml"
      rel="alternate"
      title="RSS 2.0 Feed"
    />
    <script
      dangerouslySetInnerHTML={{
        __html: `(function(w,d,s,l,i){w[l]=w[l]||[];w[l].push({'gtm.start':new Date().getTime(),event:'gtm.js'});var f=d.getElementsByTagName(s)[0],j=d.createElement(s),dl=l!='dataLayer'?'&l='+l:'';j.async=true;j.src='https://www.googletagmanager.com/gtm.js?id='+i+dl;f.parentNode.insertBefore(j,f);})(window,document,'script','dataLayer','GTM-PDMNKRX');`,
      }}
    />
  </>
);

export const pageQuery = graphql`
  query IndexPageQuery {
    latestPosts: allMarkdownRemark(
      filter: { fields: { type: { eq: "post" } } }
      sort: { frontmatter: { date: DESC } }
      limit: 5
    ) {
      nodes {
        excerpt(pruneLength: 250)
        rangeExcerpt
        fields {
          slug
        }
        frontmatter {
          title
          date
          author
          image
        }
      }
    }
    allMarkdownRemark(
      filter: { fields: { type: { eq: "post" } } }
      sort: { frontmatter: { date: DESC } }
    ) {
      nodes {
        excerpt(pruneLength: 250)
        rangeExcerpt
        fields {
          slug
        }
        frontmatter {
          title
          date
          author
          image
        }
      }
    }
    allAuthorData {
      nodes {
        authorKey
        name
      }
    }
    topPostsData {
      posts
    }
  }
`;

export default IndexPage;
