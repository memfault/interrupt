import React from 'react';
import { graphql, Link } from 'gatsby';
import Layout from '../components/Layout';

const TagsPage = ({ data }) => {
  const posts = data.allMarkdownRemark.nodes;

  // Build tag map: tag -> posts[]
  const tagMap = {};
  posts.forEach(post => {
    (post.frontmatter.tags || []).forEach(tag => {
      if (!tagMap[tag]) tagMap[tag] = [];
      tagMap[tag].push(post);
    });
  });

  const sortedTags = Object.keys(tagMap).sort();

  return (
    <Layout>
      <div className="tags-expo">
        <div className="tags-expo-list">
          {sortedTags.map(tag => (
            <Link
              key={tag}
              to={`/tag/${tag}`}
              className="post-tag"
            >
              {tag}
            </Link>
          ))}
        </div>
        <hr />
        <div className="tags-expo-section">
          {sortedTags.map(tag => (
            <div key={tag}>
              <h2 id={tag.toLowerCase().replace(/\s+/g, '-')}>{tag}</h2>
              <ul className="tags-expo-posts">
                {tagMap[tag].map(post => (
                  <Link
                    key={post.fields.slug}
                    className="post-title"
                    to={post.fields.slug}
                  >
                    <li>
                      {post.frontmatter.title}
                      <small className="post-date">
                        {new Date(post.frontmatter.date).toLocaleDateString(
                          'en-GB',
                          { day: '2-digit', month: 'short', year: 'numeric' }
                        )}
                      </small>
                    </li>
                  </Link>
                ))}
              </ul>
            </div>
          ))}
        </div>
      </div>
    </Layout>
  );
};

export const Head = () => (
  <>
    <title>Tags | Interrupt</title>
    <meta name="description" content="All tags on the Interrupt blog" />
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
  query TagsPageQuery {
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
          date
          tags
        }
      }
    }
  }
`;

export default TagsPage;
