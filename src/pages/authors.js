import React from 'react';
import { graphql, Link } from 'gatsby';
import Layout from '../components/Layout';

const AuthorsPage = ({ data }) => {
  const authorsData = data.allAuthorData.nodes;
  const allPosts = data.allMarkdownRemark.nodes;

  // Group posts by author
  const postsByAuthor = allPosts.reduce((acc, post) => {
    const key = post.frontmatter.author;
    if (!acc[key]) acc[key] = [];
    acc[key].push(post);
    return acc;
  }, {});

  return (
    <Layout>
      <ul>
        {authorsData.map(author => {
          const authorPosts = postsByAuthor[author.authorKey] || [];
          return (
            <li key={author.authorKey}>
              <h2>
                <Link to={`/authors/${author.authorKey}`}>{author.name}</Link>
              </h2>
              <ul>
                {authorPosts.map(post => (
                  <li key={post.fields.slug}>
                    <Link to={post.fields.slug}>{post.frontmatter.title}</Link>
                  </li>
                ))}
              </ul>
            </li>
          );
        })}
      </ul>
    </Layout>
  );
};

export const Head = () => (
  <>
    <title>Authors | Interrupt</title>
    <meta name="description" content="Authors of the Interrupt blog" />
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

export const pageQuery = graphql`
  query AuthorsPageQuery {
    allAuthorData {
      nodes {
        authorKey
        name
      }
    }
    allMarkdownRemark(
      filter: { fields: { type: { eq: "post" } } }
      sort: { frontmatter: { date: DESC } }
    ) {
      nodes {
        fields {
          slug
        }
        frontmatter {
          title
          author
        }
      }
    }
  }
`;

export default AuthorsPage;
