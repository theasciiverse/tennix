#!/bin/sh
# Checks the port's match against Thomas Perl's C, tick for tick: builds
# harness.cc from his own game.cc functions, runs his computer against
# itself and a scripted player against his computer, and diffs the traces
# with `tennix --trace`. Run from the repo after cargo build --release.
set -eu
t=$(mktemp -d)
python3 tools/c-check/extract.py c/src "$t"
cp tools/c-check/harness.cc "$t/"
g++ -O1 -o "$t/harness" "$t/harness.cc"
"$t/harness" 200000 7 > "$t/c-ai.txt"
target/release/tennix --trace 200000 --every 7 > "$t/rust-ai.txt"
"$t/harness" 400000 7 human > "$t/c-human.txt"
target/release/tennix --trace 400000 --every 7 --human > "$t/rust-human.txt"
diff "$t/c-ai.txt" "$t/rust-ai.txt" && echo "computer against computer: same for 200000 ticks ($(tail -1 "$t/c-ai.txt"))"
diff "$t/c-human.txt" "$t/rust-human.txt" && echo "scripted player against computer: same for 400000 ticks ($(tail -1 "$t/c-human.txt"))"
rm -rf "$t"
