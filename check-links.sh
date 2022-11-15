#!/usr/bin/env bash

bundle exec htmlproofer \
    --assume-extension \
    --empty-alt-ignore \
    --http-status-ignore 429,999 \
    --typhoeus '{"followlocation":true,"connecttimeout":600,"timeout":600}' \
    --url-ignore '\
/fromplantoprototype.com/r/noisefloor,\
/rideskip.com,\
/pankajraghav.com/2021/04/03/PINETIME-STOPCLOCK.html,\
' \
    --file-ignore '\
/_site/misc/llvm_scan_build_report/report-f11c81.html,\
' \
    ./_site
