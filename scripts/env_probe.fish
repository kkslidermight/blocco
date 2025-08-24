#!/usr/bin/env fish
# env_probe.fish
#  - Without flags: check required packages, print install command if missing, write env/env.json
#  - With --json-only: only (re)generate env/env.json (silent about missing packages)

set json_only 0
for a in $argv
  if test $a = '--json-only'; set json_only 1; end
end

set -l required base-devel cmake ninja git gcc shaderc vulkan-tools vulkan-headers sdl3
set -l missing
if test $json_only -eq 0
  for p in $required
    pacman -Qi $p >/dev/null 2>&1; or set missing $missing $p
  end
  if test (count $missing) -gt 0
    echo "Missing packages:" >&2
    echo "sudo pacman -S --needed $missing" >&2
  else
    echo "All required packages present" >&2
  end
end

mkdir -p env
set -l out env/env.json
set -l ts (date --iso-8601=seconds)
set -l session (echo $XDG_SESSION_TYPE)
set -l kernel (uname -r)
set -l host (hostname)
set -l user (whoami)

echo '{' > $out
echo '  "timestamp": "'$ts'",' >> $out
echo '  "user": "'$user'",' >> $out
echo '  "host": "'$host'",' >> $out
echo '  "kernel": "'$kernel'",' >> $out
echo '  "session_type": "'$session'",' >> $out
echo '  "missing_packages": [' >> $out
set -l first 1
if test $json_only -eq 0
  for m in $missing
    if test $first -eq 0
      echo ',' >> $out
    end
    echo -n '    "'$m'"' >> $out
    set first 0
  end
end
echo '' >> $out
echo '  ]' >> $out
echo '}' >> $out
echo "Wrote $out" >&2
