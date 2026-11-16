#!/bin/sh
# Records asciiverse/tennix.cast, the catalog preview (100x30), with hub's
# castrec (../../hub). Run from the repo after cargo build --release. The
# computer plays itself (--demo) on the grass of Melbourne; seed 2 gives a
# five-shot point at 10 s, a twelve-shot rally won at 31 s and two more
# points before 44 s. 15 frames a second keeps the file small.
set -eu
hub=${HUB:-../../hub}
seed=${SEED:-2}
home=$(mktemp -d)
CAST_COLS=100 CAST_ROWS=30 HOME="$home" XDG_STATE_HOME="$home/state" \
  python3 "$hub/tools/castrec.py" asciiverse/tennix.cast . "w44;kq" -- target/release/tennix --demo --venue 1 --seed "$seed" --mute --no-gamepad --fps 15
rm -rf "$home"
