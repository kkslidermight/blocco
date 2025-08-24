#!/usr/bin/env fish
if test ! -d build; ./scripts/build.fish; end
./build/blocco $argv
