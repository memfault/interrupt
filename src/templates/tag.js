import React from 'react';
import { graphql, Link } from 'gatsby';
import Layout from '../components/Layout';

const TagTemplate = ({ data, pageContext }) => {
  const { tag } = pageContext;
  const posts = data.allMarkdownRemark.nodes;

  const authorsData = data.allAuthorData.nodes.reduce((acc, node) => {
    acc[node.authorKey] = node;
    return acc;
  }, {});

  return (
    <Layout>
      <ul id="posts">
        <h1>Tag: {tag}</h1>
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
                    <a href={author.web || '#'}>{author.name}</a>
                  </span>
                )}
              </div>
              <p>{post.rangeExcerpt || post.excerpt}</p>
            </li>
          );
        })}
      </ul>
    </Layout>
  );
};

export const Head = ({ pageContext }) => {
  const { tag } = pageContext;
  return (
    <>
      <title>Tag: {tag} | Interrupt</title>
      <meta
        name="description"
        content={`Posts tagged with ${tag} on Interrupt`}
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
    </>
  );
};

export const pageQuery = graphql`
  query TagPosts($tag: String!) {
    allMarkdownRemark(
      filter: {
        fields: { type: { eq: "post" } }
        frontmatter: { tags: { in: [$tag] } }
      }
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
        }
      }
    }
    allAuthorData {
      nodes {
        authorKey
        name
        web
      }
    }
  }
`;

export default TagTemplate;
