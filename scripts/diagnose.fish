#!/usr/bin/env fish
set -l out env/env.json
mkdir -p env
echo '{' > $out
echo '  "session_type": "'(echo $XDG_SESSION_TYPE)'",' >> $out
echo '  "kernel": "'(uname -r)'"' >> $out
echo '}' >> $out
echo "Wrote $out"
