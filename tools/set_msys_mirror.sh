#!/usr/bin/env bash
set -euo pipefail
for f in /etc/pacman.d/mirrorlist.*; do
  cp "$f" "$f.bak"
  sed -i 's@^Server = .*@Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/$repo/$arch@' "$f"
done
