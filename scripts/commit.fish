#!/usr/bin/env fish
set allow_broken 0
for a in $argv
  if test $a = '--allow-broken'; set allow_broken 1; end
end
if test $allow_broken -eq 0
  ./scripts/build.fish; or exit 1
  ./scripts/test.fish; or exit 1
end
echo "# Progress" > PROGRESS.md
echo "Scaffold updates ("(date)")" >> PROGRESS.md

git add .
if test (count $argv) -gt 0
  set msg $argv[1]
else
  set msg "chore: update"
end
git commit -m "$msg"
