#!/bin/sh

RC=${RENODE_CHECKOUT:-~/code/renode}

python -u $RC/tests/run_tests.py tests/STM32F103.robot -r test_results
