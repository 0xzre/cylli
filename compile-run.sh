#!/bin/bash

# Compile main with exceptions enabled
clang++ -o cylli-llvm $(llvm-config --cxxflags --ldflags --system-libs --libs core) -fexceptions cylli-llvm.cpp

# Run bin main
./cylli-llvm

# Run via ll interpreter
lli ./output.ll

# Print exit status of the last command
echo $?

printf "\n"
