#!/usr/bin/env fish
set -l dir build
cmake -S . -B $dir -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build $dir
