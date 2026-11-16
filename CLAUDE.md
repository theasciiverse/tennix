# Tennix

Thomas Perl's Tennix! (2003 on, GPL-2.0-or-later, https://repo.or.cz/tennix.git)
ported from C/SDL2 to Rust + Ratatui for the Asciiverse catalog. David asked
for it 2026-11-16 (sports is one of the catalog's thinnest genres). Version
1.3.5-av.1 is the port of his 1.3.5 (2026-08-28).

## Branches

- `master`: his history, tracking the `upstream` remote (repo.or.cz)
- `asciiverse` (default): his 1.3.5, his C moved unchanged into `c/` (the
  reference), the three non-free location photos left out of `c/data`, then
  the port at the top

## Files

| File | What |
|---|---|
| `src/game.rs` | his game.cc/game.h line by line: ball, bounce, net, in/out, scoring (no tiebreak), racket hit, power bar, his AI (Carl Van Court); `variation` (ours) makes computer-vs-computer miss now and then |
| `src/locations.rs` | his locations.h; six venues renamed (real stadiums), training camp kept; `day_night` (his sum, without the 64-bit unsigned wrap) |
| `src/render.rs` | half-block canvas: court, lines, net, umpire and chair, player figures with rackets, ball and shadow, rain, fog, floodlit night |
| `src/ui.rs` | screens: match (score row, court, power bars and status row), menu over a computer match, world map (`assets/worldmap.bits`, 250x110 bits from his public-domain worldmap.png), controller setup |
| `src/app.rs` | screens and flow, controllers per side, demo, pause, resume (`$XDG_STATE_HOME/tennix/match`), sound events, logging |
| `src/input.rs` | held keys: kitty releases or Windows; else inferred (movement held 160 ms fresh / 110 ms repeating; hit keys tap to wind, tap to swing) |
| `src/audio.rs` | his ogg sounds (assets/sounds, Debian ships them in main) via rodio |
| `src/pads.rs` | gilrs gamepads |

## Faithfulness

`tennix --trace N --every K [--human]` prints the match tick by tick in the
format of a C harness built from his own `c/src/game.cc` functions (step,
get_move_y, input_ai, input_human, game_setup_serve, score_game,
game_get_winner) with stub headers. On 2026-11-16 both matched exactly:
AI vs AI for 200,000 ticks (one endless rally, 1,770 hits, as his) and a
scripted human against his AI through a full best-of-five (400,000 ticks).
`tools/c-check/check.sh` rebuilds the harness from `c/src` and runs both
diffs; run it after any change to game.rs.

## Commands

```bash
cargo run --release -- --demo --venue 1
cargo run --release -- --shot out.ppm --screen menu|map|setup|paused --at 5 --size 100x30
cargo run --release -- --demo-rallies 45 --seed 2 --venue 1   # when demo points fall
cargo test --release      # 22 tests
RUST_LOG=tennix=debug cargo run --release 2>tennix.log
```

## Release

- Tag `v1.3.5-av.1`; `.github/workflows/release.yml` (Rust 1.93.1, glibc
  Linux on 22.04 needing libasound2 and libudev1, macos-15 and
  macos-15-intel, Windows x86-64 and ARM64)
- Preview `asciiverse/tennix.cast` by `asciiverse/record-preview.sh`
  (demo in Melbourne, seed 2, 44 s, 15 fps, 1.2 MB); thumbnail at 25 s
- Catalog drafts `asciiverse/tennix.toml` and `.md`, copied to `hub/catalog`

## Not done yet

- Not played by a person yet (tests, tmux, a kitty-protocol pty and frames)
- Gamepads untested on hardware
