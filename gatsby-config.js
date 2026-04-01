const path = require("path");

module.exports = {
  siteMetadata: {
    title: "Interrupt",
    description: "A community and blog for embedded software makers",
    siteUrl: "https://interrupt.memfault.com",
    author: {
      name: "Memfault",
      email: "interrupt@memfault.com",
    },
    googleSiteVerification: "2qyLRd8BiI5o3XnqssLvbzPDJ-2S6ytb9iLoNoz1Z9M",
  },
  plugins: [
    // Source posts from preprocessed directory
    {
      resolve: "gatsby-source-filesystem",
      options: {
        name: "posts",
        path: path.join(__dirname, "_posts"),
      },
    },
    // Source images (for gatsby-remark-images)
    {
      resolve: "gatsby-source-filesystem",
      options: {
        name: "images",
        path: path.join(__dirname, "img"),
      },
    },
    // Markdown transformer
    {
      resolve: "gatsby-transformer-remark",
      options: {
        excerpt_separator: "<!-- excerpt end -->",
        plugins: [
          "gatsby-remark-autolink-headers",
          {
            resolve: "gatsby-remark-prismjs",
            options: {
              classPrefix: "language-",
              inlineCodeMarker: null,
              aliases: {},
              showLineNumbers: false,
              noInlineHighlight: false,
            },
          },
        ],
      },
    },
    // Image handling
    "gatsby-plugin-sharp",
    "gatsby-transformer-sharp",
    "gatsby-plugin-image",
    // SCSS support (includes _sass directory in load path)
    {
      resolve: "gatsby-plugin-sass",
      options: {
        sassOptions: {
          includePaths: [path.join(__dirname, "_sass")],
          quietDeps: true,
        },
      },
    },
    // Sitemap
    {
      resolve: "gatsby-plugin-sitemap",
      options: {
        output: "/",
        query: `
          {
            site {
              siteMetadata {
                siteUrl
              }
            }
            allSitePage {
              nodes {
                path
              }
            }
          }
        `,
        resolveSiteUrl: ({ site }) => site.siteMetadata.siteUrl,
        resolvePages: ({ allSitePage: { nodes: pages } }) => pages,
        serialize: ({ path: pagePath }) => ({
          url: pagePath,
          changefreq: "weekly",
          priority: 0.7,
        }),
      },
    },
    // RSS Feed
    {
      resolve: "gatsby-plugin-feed",
      options: {
        query: `
          {
            site {
              siteMetadata {
                title
                description
                siteUrl
              }
            }
          }
        `,
        feeds: [
          {
            serialize: ({ query: { site, allMarkdownRemark } }) => {
              return allMarkdownRemark.nodes.map((node) => ({
                title: node.frontmatter.title,
                date: node.frontmatter.date,
                url: `${site.siteMetadata.siteUrl}${node.fields.slug}`,
                guid: `${site.siteMetadata.siteUrl}${node.fields.slug}`,
                description: node.excerpt,
                custom_elements: [{ "content:encoded": node.html }],
              }));
            },
            query: `
              {
                allMarkdownRemark(
                  filter: { fields: { type: { eq: "post" } } }
                  sort: { frontmatter: { date: DESC } }
                  limit: 20
                ) {
                  nodes {
                    excerpt
                    html
                    fields { slug }
                    frontmatter { title date author }
                  }
                }
              }
            `,
            output: "/feed.xml",
            title: "Interrupt RSS Feed",
          },
        ],
      },
    },
  ],
};
