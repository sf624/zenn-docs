#!/bin/bash
HTML_OUTPUT_DIR=gcc_coverage_html

# Clean up previous results
rm -f *.gcda *.gcno *.gcov main
rm -rf $HTML_OUTPUT_DIR

# Compile the code with coverage flags
GCC_VERSION=14
SRCS=$(ls *.cpp)
g++-$GCC_VERSION $SRCS -o main \
    -O0 \
    -coverage \
    -dumpbase ''

# Run the program to generate coverage data
./main

# Generate the coverage report
gcov-$GCC_VERSION -b $SRCS

# Generate the HTML coverage report
lcov --capture --directory . --output-file coverage.info \
    --gcov-tool gcov-$GCC_VERSION \
    --branch-coverage

genhtml coverage.info \
    --branch-coverage \
    --output-directory $HTML_OUTPUT_DIR
