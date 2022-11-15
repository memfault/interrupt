#!/usr/bin/env bash

# Run the htmlproofer utility with some pre-set options, including some example
# ignores.

# Ignoring 429's is important because github throttles very aggressively, so
# pretty much all github links get a 429 😞.
#
# Ignoring 999's is important because some sites (like linkedin) return 999's
# for curl. Can probably tweak user agent to work around but 🤷

set -ex

bundle exec htmlproofer \
    --assume-extension \
    --empty-alt-ignore \
    --http-status-ignore 429,999 \
    --typhoeus '{"followlocation":true,"connecttimeout":600,"timeout":600}' \
    --url-ignore "\
https://fromplantoprototype.com/r/noisefloor,\
https://rideskip.com/,\
https://pankajraghav.com/2021/04/03/PINETIME-STOPCLOCK.html,\
" \
    --file-ignore "\
/_site/misc/llvm_scan_build_report/report-f11c81.html,\
" \
    ./_site
