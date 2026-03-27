import React from 'react';
import { graphql, Link } from 'gatsby';
import Layout from '../components/Layout';
import AuthorCard from '../components/AuthorCard';

const AuthorTemplate = ({ data, pageContext }) => {
  const { authorKey, author } = pageContext;
  const posts = data.allMarkdownRemark.nodes;

  return (
    <Layout>
      <article id="author-page">
        <h2>
          {author.web ? (
            <a href={author.web}>{author.name}</a>
          ) : (
            author.name
          )}
        </h2>
        <AuthorCard authorKey={authorKey} author={author} />
        <br />
        <h3>Posts</h3>
        <ul>
          {posts.map(post => {
            const formattedDate = new Date(
              post.frontmatter.date
            ).toLocaleDateString('en-GB', {
              day: '2-digit',
              month: 'short',
              year: 'numeric',
            });
            return (
              <li key={post.fields.slug}>
                {formattedDate} -{' '}
                <Link to={post.fields.slug}>{post.frontmatter.title}</Link>
              </li>
            );
          })}
        </ul>
        <br />
      </article>
    </Layout>
  );
};

export const Head = ({ pageContext }) => {
  const { author } = pageContext;
  return (
    <>
      <title>{author.name} | Interrupt</title>
      <meta
        name="description"
        content={`Posts by ${author.name} on Interrupt`}
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
  query AuthorPosts($authorKey: String!) {
    allMarkdownRemark(
      filter: {
        fields: { type: { eq: "post" } }
        frontmatter: { author: { eq: $authorKey } }
      }
      sort: { frontmatter: { date: DESC } }
    ) {
      nodes {
        fields {
          slug
        }
        frontmatter {
          title
          date
        }
      }
    }
  }
`;

export default AuthorTemplate;
