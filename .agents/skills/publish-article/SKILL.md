---
name: publish-article
description:
  Publish an Interrupt draft. Moves a user-named article from _drafts/ to
  _posts/ after auditing it against the publishing checklist in
  .gitmessage-publish-article and proposing fixes. Use when the user asks to
  publish, ship, or promote a draft article.
---

# Publish an article

Takes one draft from `_drafts/`, audits it against the publishing checklist,
proposes fixes, and moves it to `_posts/`. The user is the gatekeeper: every
file change (edits, rename, move) is shown as a proposal first and applied only
after the user approves it.

The checklist lives in `.gitmessage-publish-article` at the repo root. Read it
at the start of every run; it is the source of truth for which items to check.
The per-item guidance below explains how to check each one. If the file holds an
item this skill does not describe, check it with your best judgement and say so
in the report.

## Steps

1. **Resolve the draft.** Use the draft the user named. If they named none, or
   the name matches more than one file, list the files in `_drafts/` (skip the
   `_`-prefixed templates) and ask which one. Done when you have one existing
   file path.

2. **Settle the publish date and slug.** Ask the user for the publish date,
   offering today's date as the default. Propose a slug (see "URL" below). Done
   when the user has confirmed both, which fixes the target path
   `_posts/YYYY-MM-DD-<slug>.md`.

3. **Audit.** Read `.gitmessage-publish-article`, the whole draft, and
   `_data/authors.yml`. Check the draft against every checklist item using the
   guidance below. Done when every item has a status: pass, fail (with a
   concrete proposed change), or needs-human (something you cannot verify, such
   as how an image looks, with the reason).

4. **Propose.** Show the user a report: one row per checklist item with its
   status, then each proposed change as a before/after snippet, numbered. Ask
   the user to approve, reject, or amend each change (they may approve all at
   once). Done when every proposed change has an explicit answer.

5. **Apply.** Move the file with
   `git mv _drafts/<draft>.md _posts/YYYY-MM-DD-<slug>.md`, then apply the
   approved changes and only those. If the user rejected the move itself, stop
   here and report.

6. **Verify.** Run `yarn prettier --write --prose-wrap=always <post>` and
   `yarn typos --config _typos.toml <post>` (the same checks the pre-commit hook
   runs) and propose fixes for any typos it reports, back through step 4. Offer
   to build the site locally (`bundle exec jekyll serve --livereload --future`)
   so the user can eyeball images and code blocks.

7. **Hand off.** Fill in `.gitmessage-publish-article` as a ready-to-use commit
   / PR message: article title, author's GitHub handle from `authors.yml`, and
   each checklist box ticked (`[x]`) only for items that passed or that the user
   confirmed. Leave unverified boxes unticked and list them. Show the message;
   commit, push, or open a PR only if the user asks.

## Checklist guidance

Repo facts the checks rely on:

- Posts are served at `/blog/:title` (`permalink` in `_config.yml`), where
  `:title` is the filename after the date. The filename is the URL.
- Front matter keys used by recent posts: `title`, `description`, `author`,
  `tags`. The publish date comes from the filename; a `date:` key in front
  matter overrides it.
- Internal links use `{% post_url YYYY-MM-DD-slug %}`. Images use
  `{% img_url path/under/img %}`, resolving to `img/<path>`.
- The excerpt sits between `<!-- excerpt start -->` and `<!-- excerpt end -->`.

### Date

The filename date equals the confirmed publish date. Any `date:` front matter
key agrees with it or is proposed for removal.

### URL

The slug is lowercase, hyphen-separated, short (2 to 6 words), and carries the
article's main keyword. Draft markers (`draft_`, `wip`, underscores) are gone.
No existing file in `_posts/` uses the same slug, and the slug is not a source
path in `_redirects`.

### Markdown file name

`_posts/YYYY-MM-DD-<slug>.md`, date and slug as settled in step 2.

### Title optimized for SEO

Roughly 50 to 65 characters, main keyword near the front, specific about what
the reader gets. If it falls short, propose one or two alternatives and keep the
author's voice.

### Tags

A `tags:` list exists with 3 to 6 lowercase tags. Prefer tags already used in
`_posts/` (grep the `tags:` lines) over new spellings of the same idea; flag any
new tag so the user can confirm it.

### Meta description

A `description:` exists, is a quoted one-to-two sentence summary, contains the
main keyword, and is similar in length to the descriptions of the last few
posts. No placeholder text from `_drafts/_template.md`.

### Full author info

The `author:` key exists in `_data/authors.yml`, and that entry has `name`,
`image` (the file exists under `img/`), `blurb`, and at least one of `web`,
`github`, `linkedin`, or `twitter`. If the entry is missing or incomplete,
propose the addition to `authors.yml` and ask the user for any details you do
not have; never invent them.

### Excerpt

Both excerpt markers are present, the excerpt reads on its own as a teaser for
the article, and it matches what the article delivers.

### Links work

Collect every link. For external URLs, request each with
`curl -sSL -o /dev/null -w '%{http_code}' --max-time 15 <url>` and report
anything that is not 2xx (some sites reject bots; mark those needs-human). For
`post_url` and relative `/blog/...` links, confirm the target exists in
`_posts/`. Flag links to localhost, staging hosts, or `_drafts`.

### Images rendered

Every `img_url` / image path resolves to an existing file, each image has alt
text, and image files live under a folder named after the article. Mark how they
look as needs-human and point the user at the local build.

### Code snippets rendered

Code fences are balanced and carry a language tag. Code containing `{{` or `{%`
is wrapped in `{% raw %}` / `{% endraw %}`, otherwise Liquid eats it.

### Cross-linked to relevant articles

Search `_posts/` by the article's tags and key terms for related articles and
series parts. Propose specific `{% post_url %}` links at the sentences where
they fit. Separately, list older posts that would benefit from linking to this
one, as an optional follow-up the user can accept or skip.
