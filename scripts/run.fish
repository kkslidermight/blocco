#!/usr/bin/env fish
if test ! -d build; ./scripts/build.fish; end
set exe ./build/src/blocco
if test ! -x $exe
	echo "Executable $exe missing; rebuilding"; ./scripts/build.fish; or exit 1
end
$exe $argv
