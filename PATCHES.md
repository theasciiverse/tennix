# What this edition changes

Tennix by Thomas Perl, version 1.3.5 (2026-08-28, upstream tag
`tennix-1.3.5`, commit e527182 of https://repo.or.cz/tennix.git), ported
from C and SDL2 to Rust and Ratatui for the terminal. Version
`1.3.5-av.1` is this port of his 1.3.5.

## The repository

- `master`: his history, unchanged, tracking the `upstream` remote
- `asciiverse` (the default branch): his 1.3.5, then his C moved into
  `c/` unchanged, then the port at the top
- `c/data` on this branch leaves out the three photos his `data/COPYING`
  marks non-free (`loc_training_camp.png`, all rights reserved by Martin
  Third with permission for Tennix only; `loc_austrian_open.png` and
  `loc_olympic_green_tennis.png`, no permission recorded), the files
  Debian also excludes. They stay in `master`. The port uses none of his
  pictures.

## The match: his code

`src/game.rs` is his `game.cc`/`game.h` line by line: the ball's flight
with gravity, bounce and restitution, the net, in and out on the singles
court, the scoring delay, the racket hit and its three strokes, where the
ball goes off the racket, the power bar, the player's acceleration, his
computer player, the serve and the scoring (no tiebreak: a set is six
games with a lead of two, or seven). His C does its sums with double
constants and stores floats; the port does the same, and the result was
checked against his own functions compiled from `c/src/game.cc`: the
computer against itself for 200,000 ticks, and a scripted player against
his computer through a whole best-of-five match (400,000 ticks), print the
same ball, players, power and score to three decimals at every 7th tick.
`tennix --trace N --every K [--human]` prints that trace.

## Changed or added

- **Drawing**: the court, lines, net, umpire's chair, umpire, players,
  ball and shadow are drawn in half-block pixels after his pictures, at
  his 640x480 coordinates scaled to the terminal. His players are rackets
  only; here a small figure holds each racket. His winner screen (the
  winner's racket) adds the line "player N wins the match" with the sets,
  which his code has commented out.
- **Weather and night** as he has them per venue: his rain streaks, his
  fog drawn as drifting noise instead of his fog.png, his night.png
  floodlights as two lit ovals.
- **Night on the map**: his `day_night()` casts a negative difference to
  an unsigned int before wrapping it, which on 64-bit systems shifts the
  night about 14 hours round the world; this edition does the sum his
  comment intends, so London is dark at midnight and lit at noon.
- **Venue names**: his four tour venues and the two he left switched off
  (no photo permission) are real stadiums; here they carry names of our
  own (Eucalyptus Arena, Stade Terre-Rouge, The Lawn Court, Hudson
  Stadium, Alpine Center Court, Jade Court). Cities, surfaces, crowds,
  umpires and weather are his. The training camp, Tennis Club
  Bunabhainneadar on Harris, keeps his name. All seven are on the tour,
  since none of his photos are used.
- **Keys**: his three keyboard layouts, with the arrows layout's Ctrl and
  Alt (topspin, smash), which terminals cannot report alone, moved to the
  Left and Right arrows. Releases come from the kitty keyboard protocol or
  a Windows console; elsewhere they are inferred (movement keys count as
  held while the terminal's repeats flow; the hit keys wind up on one tap
  and swing on the next).
- **Gamepads** through gilrs instead of SDL joysticks. No mouse player,
  no network play, no Python bots (his `--master`/`--slave` and
  `USE_PYTHON` features).
- **Computer against computer**: his computer never misses, so when it
  plays itself one rally goes on for ever (as in his `--benchmark`).
  When both sides are the computer (the `--demo`, the match behind the
  menu, or a match set up that way), each one aims a little off the ball
  for every return, so balls go wide and are missed and points are won.
  Against a person his computer plays exactly as his.
- **Match length**: best of three and one set added beside his best of
  five.
- **Pause** on P (his README lists it; his 1.3.5 code does not have it).
- **Resume**: his match file (`.tennix-current_match-1.3.5` in the
  current folder) is `$XDG_STATE_HOME/tennix/match`, in a text format,
  with the settings beside it.
- **Sounds**: his sound files, played as his `sound.cc` does (ground
  volume and pan, racket pan, crowd loop over 100 seats, applause that
  stops at the next hit, rain fading in). His menu music
  (`background.ogg`) is not played in 1.3.5 and is not shipped.
- `--demo`, `--venue`, `--sets`, `--night`, `--day`, `--mute`, `--fps`,
  `--no-gamepad`; `--shot` draws a frame to a PPM picture.
- Session logging (`RUST_LOG=tennix=debug`): every key, hits, points,
  screens, a snapshot every 500 ms.

## Not ported

His intro and credits animations, the umpire's voice (`HAVE_VOICE_FILES`;
the voice files are not in his repository), the location photos, the
fullscreen toggle.
