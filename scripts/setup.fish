#!/usr/bin/env fish
set -l required base-devel cmake ninja git gcc shaderc vulkan-tools vulkan-headers sdl3
set -l missing
for p in $required
  pacman -Qi $p >/dev/null 2>&1; or set missing $missing $p
end
if test (count $missing) -gt 0
  echo "Missing packages:"; echo "sudo pacman -S --needed $missing"
else
  echo "All required packages present"
end
