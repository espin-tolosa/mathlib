#!/bin/bash

# Set file names
SRC_FILE="test/cut_elementaries.c"
OUT_FILE="bin/cut_elementaries"
COVERAGE_INFO="coverage.info"
COVERAGE_DIR="coverage_report"

./$OUT_FILE

gcov $SRC_FILE &> /dev/null

if command -v lcov &> /dev/null; then
    lcov --capture --directory . --output-file $COVERAGE_INFO &> /dev/null
    genhtml $COVERAGE_INFO --output-directory $COVERAGE_DIR &> /dev/null
else
    echo "lcov not installed. Skipping HTML report."
fi
