#!/bin/sh
set -e

# Install Node dependencies
npm install

# Preprocess Jekyll posts (converts Liquid tags to plain HTML/URLs)
node scripts/preprocess-posts.js

# Build Gatsby site (outputs to public/)
gatsby build

# Copy static assets that are not in the static/ directory
cp -r img public/img
cp -r assets public/assets

# The _redirects file is already in static/ and was copied to public/ by Gatsby.
# Rename public/ to _site/ to match Cloudflare Pages output directory setting.
rm -rf _site
mv public _site
