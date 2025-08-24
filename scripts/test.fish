#!/usr/bin/env fish
if test ! -d build; ./scripts/build.fish; end
ctest --test-dir build --output-on-failure
