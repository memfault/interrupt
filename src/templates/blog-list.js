import React from 'react';
import { graphql, Link } from 'gatsby';
import Layout from '../components/Layout';
import Pagination from '../components/Pagination';

const BlogListTemplate = ({ data, pageContext }) => {
  const posts = data.allMarkdownRemark.nodes;
  const { currentPage, numPages } = pageContext;

  // Load authors from static query is complex here; we'll use the embedded data
  const authorsData = data.allAuthorData.nodes.reduce((acc, node) => {
    acc[node.authorKey] = node;
    return acc;
  }, {});

  return (
    <Layout sidebar pageUrl="https://interrupt.memfault.com/blog">
      <ul id="posts">
        {posts.map(post => {
          const authorKey = post.frontmatter.author;
          const author = authorsData[authorKey];
          const formattedDate = new Date(
            post.frontmatter.date
          ).toLocaleDateString('en-GB', {
            day: '2-digit',
            month: 'short',
            year: 'numeric',
          });

          return (
            <li key={post.fields.slug} className="post">
              <h2>
                <Link to={post.fields.slug}>{post.frontmatter.title}</Link>
              </h2>
              <div className="by-line">
                <time dateTime={post.frontmatter.date}>{formattedDate}</time>
                {author && (
                  <span>
                    {' '}
                    by{' '}
                    <Link to={`/authors/${authorKey}`}>{author.name}</Link>
                  </span>
                )}
              </div>
              <p>{post.rangeExcerpt || post.excerpt}</p>
            </li>
          );
        })}
      </ul>

      <Pagination currentPage={currentPage} numPages={numPages} />
    </Layout>
  );
};

export const Head = ({ pageContext }) => {
  const siteUrl = 'https://interrupt.memfault.com';
  const title =
    pageContext.currentPage === 1
      ? 'Blog | Interrupt'
      : `Blog - Page ${pageContext.currentPage} | Interrupt`;

  return (
    <>
      <title>{title}</title>
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
        href={`${siteUrl}/feed.xml`}
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
};

export const pageQuery = graphql`
  query BlogListQuery($limit: Int!, $skip: Int!) {
    allMarkdownRemark(
      filter: { fields: { type: { eq: "post" } } }
      sort: { frontmatter: { date: DESC } }
      limit: $limit
      skip: $skip
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
        }
      }
    }
    allAuthorData {
      nodes {
        authorKey
        name
      }
    }
  }
`;

export default BlogListTemplate;
