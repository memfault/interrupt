#!/bin/sh

RC=${RENODE_CHECKOUT:-~/code/renode}

python -u $RC/tests/run_tests.py tests/test-button.robot -r test_results --variable PATH:$PWD
