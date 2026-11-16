# Tennix!

**Hold it... hold it... now let go.**

The crowd goes quiet. The umpire leans forward in the high chair. The
ball hangs in the air beside you, waiting to be served, and your power bar
is empty. Press the hit key and hold it: the bar fills, your racket tilts
back, and the whole stadium can see what is coming. Let go, and the ball
cracks across the net. Too timid and it floats over for your opponent to
punish. Too greedy and it sails long, the umpire's arms go up, and the
line reads **out!**

This is Tennix, Thomas Perl's tennis game, served on a court of coloured
half blocks right in your terminal: the clay of Paris, the grass of
London, the hard courts of New York, with the ball and its shadow racing
across them, the crowd murmuring, and Carl Van Court, the computer, waiting
at the other baseline. He never gets tired. He rarely misses. Beat him.

## How a point plays

- **The power bar is everything.** Hold the hit key and it climbs; let go
  and you swing. More power means a faster, higher, longer ball. The bar
  drains the moment you let go, so let go just as the ball arrives: too
  early and your swing has nothing left in it
- **Where the ball meets the racket decides where it goes.** Catch it in
  the middle and it flies straight down the court. Catch it on the top or
  bottom of the frame and it angles off towards the side lines, and maybe
  past them
- **Three strokes**: the plain hit, the **topspin** that loops high and
  drops in, and the **smash** that goes like a bullet
- **Everything is called**: *net!*, *out!*, *fault!*, *volley!*, and the
  bitter *did not catch the ball.* The umpire waves the point to whoever
  won it, and the crowd breaks into applause

Matches are played the way Thomas Perl wrote them: best of five sets, six
games to a set with a lead of two, or seven, deuce and advantage in every
game. Short of time? Best of three or a single set are in the menu.

## The Classic Championship Tour

Pick your venue on a map of the world where night is falling right now:
play a city after dark and the match is lit by two floodlights.

- **Eucalyptus Arena**, Melbourne: Plexicushion, 6,000 seats
- **Stade Terre-Rouge**, Paris: red terre battue, and a little rain
- **The Lawn Court**, London: ryegrass, rain and mist, 14,000 seats
- **Hudson Stadium**, New York City: acrylic hard court in thick fog, 22,547 seats
- **Tennis Club Bunabhainneadar**, Husinish, North Harris: artificial
  grass, thirty seats, no umpire, the Atlantic for company
- **Alpine Center Court**, Kitzbühel: clay in the mountains
- **Jade Court**, Beijing: deco turf, 10,000 seats

## Controls

| | Player 1 (left) | or | Player 2 (right) |
|---|---|---|---|
| move | Up / Down | W / S | O / L |
| hit | Space | D | K |
| topspin | Left | E | I |
| smash | Right | F | J |

Two people share one keyboard; either side can be Carl Van Court, and
gamepads play too (stick or cross to move, A to hit, B or Y for topspin,
X to smash, Start to pause). Choose who plays what on the controller
setup screen.

**P** pauses, **M** turns the sound off and on, **Esc** leaves the match
and keeps it waiting in the menu under *Resume match*, even after you quit.

Terminals that report key releases (kitty, foot, WezTerm, Ghostty,
Alacritty, Windows consoles) let you hold and let go, as Tennix
was made to be played. Where the terminal hides releases, tap the hit key
to wind up and tap it again to swing.

## Options

```
tennix                 the menu
tennix --demo          the computer plays itself, venue after venue
tennix --venue 2       start the demo in Paris (1 to 7)
tennix --sets 1        a one-set match (2: best of three; 3: best of five)
tennix --night         under the floodlights (--day: by daylight)
tennix --mute          start with the sound off
```

Settings and the match you left are kept in `$XDG_STATE_HOME/tennix`
(`~/.local/state/tennix`, or `%LOCALAPPDATA%\tennix` on Windows).
`RUST_LOG=tennix=debug tennix 2>tennix.log` writes every key, every hit,
every point and a snapshot of the match twice a second.

## History

Thomas Perl wrote Tennix! in 2003, a little tennis game at 320x200. On the
10th of May 2007 he ported it to SDL and doubled it to 640x480, and then
he kept going: an umpire in the chair, rain, fog and floodlit night
matches in 2008, and in February 2009 the power bar, the world map and
the tour of the four great tournaments, with night falling on the map as
it falls on the real cities. Tennix ran on Linux, Windows, Mac OS X and
on Nokia's Maemo internet tablets, grew Python robots that could play it,
and in 2011 became the *Classic Championship Tour 2011*. In 2023 it
moved to SDL2, and in August 2026 Tennix 1.3.5 came out, still being
served twenty-three years after the first ball.

This edition is a port of his 1.3.5 from C to Rust for the terminal. The
match is his code, line by line: checked against his C, tick for tick,
through a whole best-of-five match, the ball lands where his lands. The
drawing is new, in half-block pixels after his pictures. What changed is
listed in [PATCHES.md](PATCHES.md); his C is kept unchanged in [c/](c/),
and his history in this repository's `master` branch.

## Building

```bash
cargo build --release
./target/release/tennix
cargo test --release
```

Rust 1.93 or newer. On Linux the sound needs ALSA (`libasound2-dev` to
build) and gamepads need udev (`libudev-dev`); `cargo build
--no-default-features` leaves gamepads out.

## Credits

- **Tennix!**: Thomas Perl ([thp.io](https://thp.io)), code, graphics,
  sounds and idea, 2003 to 2026
- Some of the sound effects are based on data from the Freesound Project,
  as Tennix's README says
- The world map outline: from the public domain BlankMap-World by Vardion
  on Wikimedia Commons, as in Tennix
- Terminal edition: Asciiverse, 2026

Copyright (C) 2003, 2007-2011, 2013, 2017, 2019-2020, 2023 Thomas Perl;
terminal edition Copyright (C) 2026 Asciiverse. Free software under the
GNU General Public License, version 2 or (at your option) any later
version: see [LICENSE](LICENSE).
