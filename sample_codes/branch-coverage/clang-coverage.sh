#!/bin/bash
HTML_OUTPUT_DIR=clang_coverage_html

# Clean up previous results
rm -f *.profdata *.profraw main
rm -rf $HTML_OUTPUT_DIR

# Compile the code with coverage flags
LLVM_VERSION=20
SRCS=$(ls *.cpp)
clang++-$LLVM_VERSION $SRCS -o main \
    -fprofile-instr-generate \
    -fcoverage-mapping \
    -fcoverage-mcdc

# Run the program to generate coverage data
LLVM_PROFILE_FILE="main.profraw" ./main

# Merge the raw coverage data into a single profile data file
llvm-profdata-$LLVM_VERSION merge -sparse main.profraw -o main.profdata

# Generate the coverage report
llvm-cov-$LLVM_VERSION report ./main -instr-profile=main.profdata -show-mcdc-summary

# Generate the HTML coverage report
llvm-cov-$LLVM_VERSION show ./main -instr-profile=main.profdata \
    -Xdemangler=c++filt \
    -show-line-counts-or-regions \
    -show-branches=count \
    -show-mcdc \
    -show-mcdc-summary \
    -format=html \
    -output-dir=$HTML_OUTPUT_DIR
