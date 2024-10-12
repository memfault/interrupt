#!/usr/bin/env bash

set -euo pipefail

. /venv/bin/activate
bundle exec jekyll serve --host 0.0.0.0 -D "$@" --trace --future --incremental --livereload
