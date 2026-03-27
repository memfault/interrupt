import React, { useEffect } from 'react';
import { graphql, Link } from 'gatsby';
import Layout from '../components/Layout';
import AuthorCard from '../components/AuthorCard';
import Sidebar from '../components/Sidebar';

const PostTemplate = ({ data, pageContext, location }) => {
  const { markdownRemark } = data;
  const { frontmatter, html, fields } = markdownRemark;
  const { authorKey, author } = pageContext;

  const siteUrl = 'https://interrupt.memfault.com';
  const postUrl = `${siteUrl}${fields.slug}`;

  const formattedDate = new Date(frontmatter.date).toLocaleDateString('en-GB', {
    day: '2-digit',
    month: 'short',
    year: 'numeric',
  });

  useEffect(() => {
    // Anchor links (equivalent of anchor.min.js)
    if (typeof window !== 'undefined' && window.anchors) {
      window.anchors.options.visible = 'hover';
      window.anchors.add('.post-content > h1, h2, h3, h4, h5, h6');
      window.anchors.remove('.no-anchor');
    }

    // Discourse comments
    if (typeof window !== 'undefined') {
      window.DiscourseEmbed = {
        discourseUrl: 'https://community.memfault.com/',
        discourseEmbedUrl: postUrl,
      };
      const d = document.createElement('script');
      d.type = 'text/javascript';
      d.async = true;
      d.src = window.DiscourseEmbed.discourseUrl + 'javascripts/embed.js';
      (
        document.getElementsByTagName('head')[0] ||
        document.getElementsByTagName('body')[0]
      ).appendChild(d);
    }
  }, [postUrl]);

  return (
    <>
      <Layout sidebar pageUrl={postUrl} pageTitle={frontmatter.title}>
        <article id="post-page">
          <h2>{frontmatter.title}</h2>
          <div className="by-line">
            <time dateTime={frontmatter.date}>{formattedDate}</time>
            {author && (
              <span>
                {' '}
                by{' '}
                <Link to={`/authors/${authorKey}`}>{author.name}</Link>
              </span>
            )}
          </div>
          <div
            className="content post-content"
            dangerouslySetInnerHTML={{ __html: html }}
          />

          {author && (
            <AuthorCard authorKey={authorKey} author={author} />
          )}

          <div id="discourse-comments"></div>
        </article>
      </Layout>

      {/* Mermaid support for posts that opt in */}
      {frontmatter.mermaid && (
        <>
          <script src="https://unpkg.com/mermaid/dist/mermaid.min.js" />
          <script
            dangerouslySetInnerHTML={{
              __html: `
                document.addEventListener('DOMContentLoaded', function() {
                  mermaid.initialize({ startOnLoad: true, theme: 'default' });
                  if (window.mermaid) {
                    window.mermaid.init(undefined, document.querySelectorAll('.language-mermaid'));
                  }
                });
              `,
            }}
          />
        </>
      )}
    </>
  );
};

export const Head = ({ data, pageContext }) => {
  const { frontmatter, excerpt, fields } = data.markdownRemark;
  const siteUrl = 'https://interrupt.memfault.com';
  const postUrl = `${siteUrl}${fields.slug}`;
  const description = frontmatter.description || excerpt;
  const image = frontmatter.image || '/img/interrupt-cover-1200px.png';

  return (
    <>
      <title>{frontmatter.title} | Interrupt</title>
      <meta name="description" content={description} />
      <meta property="og:title" content={frontmatter.title} />
      <meta property="og:description" content={description} />
      <meta property="og:url" content={postUrl} />
      <meta property="og:image" content={`${siteUrl}${image}`} />
      <meta property="og:type" content="article" />
      <link rel="canonical" href={postUrl} />
      {/* Google Tag Manager */}
      <script
        dangerouslySetInnerHTML={{
          __html: `(function(w,d,s,l,i){w[l]=w[l]||[];w[l].push({'gtm.start':new Date().getTime(),event:'gtm.js'});var f=d.getElementsByTagName(s)[0],j=d.createElement(s),dl=l!='dataLayer'?'&l='+l:'';j.async=true;j.src='https://www.googletagmanager.com/gtm.js?id='+i+dl;f.parentNode.insertBefore(j,f);})(window,document,'script','dataLayer','GTM-PDMNKRX');`,
        }}
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
        href={`${siteUrl}/feed.xml`}
        type="application/rss+xml"
        rel="alternate"
        title="RSS 2.0 Feed"
      />
    </>
  );
};

export const pageQuery = graphql`
  query PostBySlug($id: String!) {
    markdownRemark(id: { eq: $id }) {
      html
      excerpt(pruneLength: 250)
      rawMarkdownBody
      fields {
        slug
      }
      frontmatter {
        title
        description
        date
        author
        tags
        image
        mermaid
      }
    }
  }
`;

export default PostTemplate;
