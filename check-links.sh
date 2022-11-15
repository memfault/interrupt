#!/usr/bin/env bash

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
