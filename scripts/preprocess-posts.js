#!/usr/bin/env node
/**
 * Preprocesses Jekyll posts by converting Liquid tags to plain HTML/URLs.
 * Output is written to _posts_processed/ which Gatsby reads from.
 */
const fs = require('fs');
const path = require('path');

const POSTS_DIR = path.join(__dirname, '..', '_posts');
const OUTPUT_DIR = path.join(__dirname, '..', '_posts_processed');

const NEWSLETTER_HTML =
  '<div class="newsletter"><p class="newsletter-content">Like Interrupt? ' +
  '<a class="newsletter-link" href="https://go.memfault.com/interrupt-subscribe" target="_blank">' +
  '<b>Subscribe</b></a> to get our latest posts straight to your inbox.</p></div>';

const SUBMIT_PR_HTML =
  '<div class="submit-pr"><p class="submit-pr-content">See anything you\'d like to change? ' +
  'Submit a pull request or open an issue on our ' +
  '<a class="submit-pr-link" href="https://github.com/memfault/interrupt" target="_blank">GitHub</a>' +
  '</p></div>';

/**
 * Replace Liquid template tags in markdown content.
 */
function processLiquid(content) {
  // Remove blockdiag diagram blocks entirely (Python rendering not available in Gatsby)
  content = content.replace(
    /\{%-?\s*blockdiag[\s\S]*?\{%-?\s*endblockdiag\s*-?%\}/g,
    '<!-- blockdiag diagram removed -->'
  );

  // Remove Liquid comment blocks
  content = content.replace(
    /\{%-?\s*comment\s*-?%\}[\s\S]*?\{%-?\s*endcomment\s*-?%\}/g,
    ''
  );

  // Remove raw/endraw markers but keep content between them
  content = content.replace(/\{%-?\s*raw\s*-?%\}/g, '');
  content = content.replace(/\{%-?\s*endraw\s*-?%\}/g, '');

  // Replace known includes with HTML equivalents
  content = content.replace(
    /\{%-?\s*include newsletter\.html\s*-?%\}/g,
    NEWSLETTER_HTML
  );
  content = content.replace(
    /\{%-?\s*include toc\.html\s*-?%\}/g,
    '<div id="toc"></div>'
  );
  content = content.replace(
    /\{%-?\s*include submit-pr\.html\s*-?%\}/g,
    SUBMIT_PR_HTML
  );

  // Replace post_url: strip date prefix from slug
  // Handles both single-line and multi-line tags
  content = content.replace(
    /\{%-?\s*post_url\s+(?:\d{4}-\d{2}-\d{2}-)?([\w-]+)\s*-?%\}/g,
    '/blog/$1'
  );

  // Replace tag_url
  content = content.replace(
    /\{%-?\s*tag_url\s+([\w-]+)\s*-?%\}/g,
    '/tag/$1'
  );

  // Replace img_url
  content = content.replace(
    /\{%-?\s*img_url\s+([^\s%}]+?)\s*-?%\}/g,
    '/img/$1'
  );

  // Replace {% link _posts/YYYY-MM-DD-slug.md %}
  content = content.replace(
    /\{%-?\s*link\s+_posts\/(?:\d{4}-\d{2}-\d{2}-)?([\w-]+)\.md\s*-?%\}/g,
    '/blog/$1'
  );

  // Replace {{ site.baseurl }} with empty string (site is at root)
  content = content.replace(/\{\{\s*site\.baseurl\s*\}\}/g, '');

  // Remove remaining Liquid tags ({% ... %})
  content = content.replace(/\{%-?[\s\S]*?-?%\}/g, '');

  return content;
}

// Ensure output directory exists
if (!fs.existsSync(OUTPUT_DIR)) {
  fs.mkdirSync(OUTPUT_DIR, { recursive: true });
}

/**
 * Inject the date (from filename) into the frontmatter if not already present.
 * Jekyll derives date from the filename; Gatsby needs it explicitly in frontmatter.
 */
function injectDate(content, filename) {
  const dateMatch = filename.match(/^(\d{4}-\d{2}-\d{2})/);
  if (!dateMatch) return content;

  const date = dateMatch[1];

  // Check if date already exists in frontmatter
  const fmEnd = content.indexOf('\n---', 3); // find closing ---
  if (fmEnd === -1) return content;

  const frontmatter = content.substring(0, fmEnd);
  if (/^date:/m.test(frontmatter)) return content; // already has date

  // Inject date after opening ---
  return content.replace(/^---\n/, `---\ndate: "${date}"\n`);
}

// Process each post
const files = fs.readdirSync(POSTS_DIR).filter(f => f.endsWith('.md'));
let count = 0;
files.forEach(filename => {
  const inputPath = path.join(POSTS_DIR, filename);
  const outputPath = path.join(OUTPUT_DIR, filename);
  let content = fs.readFileSync(inputPath, 'utf8');
  content = injectDate(content, filename);
  content = processLiquid(content);
  fs.writeFileSync(outputPath, content);
  count++;
});

console.log(`Preprocessed ${count} posts → ${OUTPUT_DIR}`);
