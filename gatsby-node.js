const path = require('path');
const fs = require('fs');
const yaml = require('js-yaml');

// Load static data files
const authorsData = yaml.load(
  fs.readFileSync(path.join(__dirname, '_data/authors.yml'), 'utf8')
);
const topPostsData = yaml.load(
  fs.readFileSync(path.join(__dirname, '_data/top_posts.yml'), 'utf8')
);
let notificationsData = {};
try {
  notificationsData = yaml.load(
    fs.readFileSync(path.join(__dirname, '_data/notifications.yml'), 'utf8')
  );
} catch (e) {
  // notifications.yml is optional
}

// Create GraphQL nodes for authors and notifications
exports.sourceNodes = ({ actions, createNodeId, createContentDigest }) => {
  const { createNode } = actions;

  // Create author nodes
  Object.entries(authorsData).forEach(([key, data]) => {
    createNode({
      ...data,
      authorKey: key,
      id: createNodeId(`author-${key}`),
      parent: null,
      children: [],
      internal: {
        type: 'AuthorData',
        contentDigest: createContentDigest({ key, ...data }),
      },
    });
  });

  // Create notification nodes
  Object.entries(notificationsData).forEach(([key, data]) => {
    createNode({
      ...data,
      notificationKey: key,
      id: createNodeId(`notification-${key}`),
      parent: null,
      children: [],
      internal: {
        type: 'NotificationData',
        contentDigest: createContentDigest({ key, ...data }),
      },
    });
  });

  // Create top posts node
  createNode({
    posts: topPostsData.posts || [],
    id: createNodeId('top-posts'),
    parent: null,
    children: [],
    internal: {
      type: 'TopPostsData',
      contentDigest: createContentDigest(topPostsData),
    },
  });
};

// Add slug and type fields to MarkdownRemark nodes
exports.onCreateNode = ({ node, actions, getNode }) => {
  const { createNodeField } = actions;

  if (node.internal.type === 'MarkdownRemark') {
    const fileNode = getNode(node.parent);
    if (!fileNode) return;

    const filename = fileNode.name; // e.g., "2019-04-25-getting-started-with-ibdap"
    // Extract slug: remove YYYY-MM-DD- prefix
    const slugMatch = filename.match(/^\d{4}-\d{2}-\d{2}-(.+)$/);
    const slug = slugMatch ? `/blog/${slugMatch[1]}` : `/blog/${filename}`;

    createNodeField({ name: 'slug', node, value: slug });
    createNodeField({ name: 'type', node, value: 'post' });
  }
};

// Add custom resolvers
exports.createResolvers = ({ createResolvers }) => {
  createResolvers({
    MarkdownRemark: {
      // Extract excerpt from between <!-- excerpt start --> and <!-- excerpt end -->
      rangeExcerpt: {
        type: 'String',
        resolve(source) {
          const rawBody = source.rawMarkdownBody || '';
          const startMarker = '<!-- excerpt start -->';
          const endMarker = '<!-- excerpt end -->';

          const startIdx = rawBody.indexOf(startMarker);
          const endIdx = rawBody.indexOf(endMarker);

          let excerptText;
          if (startIdx !== -1 && endIdx !== -1 && startIdx < endIdx) {
            excerptText = rawBody.substring(
              startIdx + startMarker.length,
              endIdx
            );
          } else if (endIdx !== -1) {
            excerptText = rawBody.substring(0, endIdx);
          } else {
            // Fall back to first 2 paragraphs
            const paragraphs = rawBody.split(/\n\n+/);
            excerptText = paragraphs.slice(0, 2).join('\n\n');
          }

          // Strip markdown formatting for plain text display
          return excerptText
            .replace(/<!--[\s\S]*?-->/g, '') // HTML comments
            .replace(/\[([^\]]+)\]\([^)]+\)/g, '$1') // links
            .replace(/!\[[^\]]*\]\([^)]+\)/g, '') // images
            .replace(/^#+\s+/gm, '') // headings
            .replace(/[*_`]/g, '') // emphasis/code
            .replace(/\n+/g, ' ')
            .trim();
        },
      },
    },
  });
};

// Create all pages
exports.createPages = async ({ graphql, actions }) => {
  const { createPage } = actions;

  const result = await graphql(`
    query {
      allMarkdownRemark(
        filter: { fields: { type: { eq: "post" } } }
        sort: { frontmatter: { date: DESC } }
      ) {
        nodes {
          id
          fields {
            slug
          }
          frontmatter {
            title
            author
            tags
            date
          }
        }
      }
    }
  `);

  if (result.errors) {
    throw result.errors;
  }

  const posts = result.data.allMarkdownRemark.nodes;

  // Create individual post pages
  posts.forEach(post => {
    const authorKey = post.frontmatter.author;
    const author = authorsData[authorKey] || null;

    createPage({
      path: post.fields.slug,
      component: path.resolve('./src/templates/post.js'),
      context: {
        id: post.id,
        authorKey,
        author,
      },
    });
  });

  // Create paginated blog list pages
  const POSTS_PER_PAGE = 8;
  const numPages = Math.ceil(posts.length / POSTS_PER_PAGE);

  Array.from({ length: numPages }).forEach((_, i) => {
    const currentPage = i + 1;
    createPage({
      path: currentPage === 1 ? '/blog' : `/blog/page${currentPage}`,
      component: path.resolve('./src/templates/blog-list.js'),
      context: {
        limit: POSTS_PER_PAGE,
        skip: i * POSTS_PER_PAGE,
        numPages,
        currentPage,
      },
    });
  });

  // Create author pages
  Object.entries(authorsData).forEach(([key, author]) => {
    createPage({
      path: `/authors/${key}`,
      component: path.resolve('./src/templates/author.js'),
      context: {
        authorKey: key,
        author,
      },
    });
  });

  // Collect all tags
  const tagsSet = new Set();
  posts.forEach(post => {
    (post.frontmatter.tags || []).forEach(tag => tagsSet.add(tag));
  });

  // Create tag pages
  tagsSet.forEach(tag => {
    createPage({
      path: `/tag/${tag}`,
      component: path.resolve('./src/templates/tag.js'),
      context: { tag },
    });
  });
};
