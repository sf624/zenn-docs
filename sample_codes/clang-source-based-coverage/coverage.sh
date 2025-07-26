#!/bin/bash

# Set path to coverate html result
html_output_dir=coverage_html

# Clean up previous results
rm -f *.profdata *.profraw main
rm -rf $html_output_dir

# Compile the code with coverage flags
LLVM_VERSION=20
clang++-$LLVM_VERSION foo.cpp bar.cpp main.cpp -o main \
    -fprofile-instr-generate \
    -fcoverage-mapping \
    -fcoverage-mcdc

# Run the program to generate coverage data
LLVM_PROFILE_FILE="main.profraw" ./main

# Merge the raw coverage data into a single profile data file
llvm-profdata-$LLVM_VERSION merge -sparse main.profraw -o main.profdata

# Generate the HTML coverage report
llvm-cov-$LLVM_VERSION show ./main -instr-profile=main.profdata \
    -Xdemangler=c++filt \
    -show-mcdc \
    -show-mcdc-summary \
    -show-line-counts-or-regions \
    -show-branches=count \
    -format=html \
    -output-dir=$html_output_dir
