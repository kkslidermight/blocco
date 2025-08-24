#!/usr/bin/env fish
# Conventional commit helper with build+test gate and author enforcement.
set allow_broken 0
set expected_author "kkslidermight"
for a in $argv
  if test $a = '--allow-broken'; set allow_broken 1; end
end

set current_author (git config user.name 2>/dev/null)
if test -z "$current_author"
  echo "git user.name is not set"; exit 2
end
if test "$current_author" != "$expected_author"
  echo "Refusing commit: git user.name '$current_author' != expected '$expected_author'"; exit 3
end

if test $allow_broken -eq 0
  ./scripts/build.fish; or begin; echo "Build failed"; exit 1; end
  ./scripts/test.fish; or begin; echo "Tests failed"; exit 1; end
end

if test -x scripts/git_hook.fish
  # Run hook before committing so its changes are included.
  scripts/git_hook.fish
end

git add .
if test (count $argv) -gt 0
  set msg $argv[1]
else
  set msg "chore: update"
end

git commit -m "$msg"
