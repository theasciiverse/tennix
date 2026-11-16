//! Tennix! in the terminal: Thomas Perl's tennis game (2003 on), ported
//! from his C to Rust and Ratatui for Asciiverse.
//!
//! Copyright (C) 2003, 2007-2011, 2013, 2017, 2019-2020, 2023 Thomas Perl;
//! the terminal port Copyright (C) 2026 Asciiverse. GPL-2.0-or-later.

mod app;
mod audio;
mod game;
mod input;
mod locations;
mod pads;
mod render;
mod store;
mod ui;

use app::{App, Screen};
use ratatui::crossterm::event::{
    self, DisableFocusChange, EnableFocusChange, Event, KeyboardEnhancementFlags, PopKeyboardEnhancementFlags, PushKeyboardEnhancementFlags,
};
use ratatui::crossterm::execute;
use ratatui::crossterm::terminal::supports_keyboard_enhancement;
use std::io::{self, Write};
use std::time::{Duration, Instant};

/// The Tennix version this port follows.
pub const UPSTREAM_VERSION: &str = "1.3.5";

const SNAPSHOT_EVERY: Duration = Duration::from_millis(500);

const HELP: &str = "tennix [--demo] [--venue N] [--sets N] [--night | --day] [--mute]

Tennix! Classic Championship Tour, Thomas Perl's tennis game, in the
terminal. Hold the hit key to fill your power bar and let go to swing:
the more power, the harder and higher the ball flies, and too much sends
it out. Meet the ball off the middle of your racket and it goes wide.

Player 1 (left): Up/Down move, Space hit, Left topspin, Right smash
             or W/S move, D hit, E topspin, F smash
Player 2 (right): O/L move, K hit, I topspin, J smash
P pauses, M turns the sound off or on, Esc leaves the match (it waits in
the menu to be resumed), Q quits from the menu.

  --demo        the computer plays itself, venue after venue
  --venue N     start the demo at venue N (1 to 7)
  --sets N      sets to win: 3 (best of five, his), 2 or 1
  --night       play under the floodlights; --day by daylight
                (otherwise night falls as it does at the venue now)
  --mute        start with the sound off
  --fps N       frames a second (default 30)
  --no-gamepad  leave gamepads alone

Settings and a match left unfinished are kept in $XDG_STATE_HOME/tennix.
Logging: RUST_LOG=tennix=debug tennix 2>tennix.log
";

fn main() -> io::Result<()> {
    env_logger::init();
    let args: Vec<String> = std::env::args().skip(1).collect();
    let has = |f: &str| args.iter().any(|a| a == f);
    let after = |f: &str| args.iter().position(|a| a == f).and_then(|i| args.get(i + 1)).cloned();
    if has("--help") || has("-h") {
        print!("{HELP}");
        return Ok(());
    }
    if has("--version") || has("-V") {
        println!("tennix {}", env!("CARGO_PKG_VERSION"));
        return Ok(());
    }
    let mut settings = store::Settings::load();
    if has("--mute") {
        settings.disk_sound = Some(settings.sound);
        settings.sound = false;
    }
    if let Some(s) = after("--sets").and_then(|v| v.parse::<usize>().ok()) {
        settings.disk_sets = Some(settings.sets);
        settings.sets = s.clamp(1, 3);
    }
    let venue = after("--venue").and_then(|v| v.parse::<usize>().ok()).map_or(0, |v| v.clamp(1, locations::LOCATIONS.len()) - 1);
    let seed = after("--seed")
        .and_then(|v| v.parse().ok())
        .unwrap_or_else(|| std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).map_or(1, |d| d.as_nanos() as u64));
    app::set_seed(seed);
    let night = if has("--night") {
        Some(true)
    } else if has("--day") {
        Some(false)
    } else {
        None
    };

    if let Some(n) = after("--trace").and_then(|v| v.parse::<u64>().ok()) {
        // the computer against itself for n ticks with his exact AI, printed
        // in the format of the C harness the port was checked against
        let every = after("--every").and_then(|v| v.parse::<u64>().ok()).unwrap_or(0);
        trace(n, every, has("--human"));
        return Ok(());
    }
    if let Some(secs) = after("--demo-rallies").and_then(|v| v.parse::<f32>().ok()) {
        // the demo headless: when each point falls and how long its rally was
        let mut app = App::new(settings, audio::Audio::none(), pads::Pads::new(false), true, 1);
        app.start_demo(venue);
        let mut t = 0.0;
        let mut last = (0, 0);
        while t < secs * 1000.0 {
            let rally = app.play.as_ref().map_or(0, |p| p.m.rally);
            app.step(15.0);
            t += 15.0;
            if let Some(p) = &app.play {
                let pts = (p.m.players[0].game, p.m.players[1].game);
                if pts != last {
                    println!("{:6.1}s point after a rally of {rally}: {} {}", t / 1000.0, p.m.format_game(), p.m.format_sets());
                    last = pts;
                }
            }
        }
        return Ok(());
    }
    if let Some(path) = after("--shot") {
        return shot(&path, after("--size"), after("--at").and_then(|v| v.parse().ok()).unwrap_or(5.0), after("--screen"), venue, night, settings);
    }

    // open audio before the alternate screen, so any ALSA chatter lands above it
    let audio = audio::Audio::new(!settings.sound);
    let pads = pads::Pads::new(!has("--no-gamepad"));
    let mut terminal = ratatui::init();
    let kitty = supports_keyboard_enhancement().unwrap_or(false);
    if kitty {
        let flags = KeyboardEnhancementFlags::DISAMBIGUATE_ESCAPE_CODES | KeyboardEnhancementFlags::REPORT_EVENT_TYPES;
        execute!(io::stdout(), PushKeyboardEnhancementFlags(flags))?;
    }
    let _ = execute!(io::stdout(), EnableFocusChange);
    // Windows consoles report releases without asking
    let releases = kitty || cfg!(windows);
    let mut app = App::new(settings, audio, pads, releases, seed);
    app.night_override = night;
    app.venue_sel = venue;
    if has("--demo") {
        app.start_demo(venue);
    }
    let fps: f32 = after("--fps").and_then(|v| v.parse().ok()).unwrap_or(30.0f32).clamp(5.0, 60.0);
    let size = terminal.size()?;
    log::info!(
        "start tennix {} (Tennix {UPSTREAM_VERSION}) terminal {}x{} kitty_keyboard {kitty} releases {releases} audio {} sound {} pads {:?} demo {}",
        env!("CARGO_PKG_VERSION"),
        size.width,
        size.height,
        app.audio.available(),
        app.settings.sound,
        app.pad_names,
        app.demo
    );
    let result = run(&mut terminal, &mut app, Duration::from_secs_f32(1.0 / fps));
    app.save_on_exit();
    app.audio.stop_match();
    let _ = execute!(io::stdout(), DisableFocusChange);
    if kitty {
        let _ = execute!(io::stdout(), PopKeyboardEnhancementFlags);
    }
    ratatui::restore();
    result
}

fn trace(n: u64, every: u64, human: bool) {
    let mut m = game::Match::new(3);
    m.players[0].ai = !human;
    let mut ev = Vec::new();
    let (mut hits, mut last) = (0, -1i8);
    for t in 0..n {
        // the C harness's scripted player 1: follows the ball with an
        // offset, winds up while the ball is far, swings when it is near
        let script = |m: &game::Match, _p: u8| {
            let (b, pl) = (&m.ball, &m.players[0]);
            let target = b.y - b.z + ((t / 300) % 7) as f32 * 7.0 - 21.0;
            let d = target - pl.y;
            let axis = if d > 3.0 { 1.0 } else if d < -3.0 { -1.0 } else { 0.0 };
            let which = (t / 2000) % 3;
            let held = if b.inhibit_gravity { t % 60 < 40 } else { (b.x - pl.x).abs() > 40.0 + ((t / 500) % 4) as f32 * 10.0 };
            game::Controls { axis, hit: held && which == 0, topspin: held && which == 1, smash: held && which == 2 }
        };
        m.tick_by(script, &mut ev);
        if m.ball.last_hit_by != last && m.ball.last_hit_by != -1 {
            hits += 1;
        }
        last = m.ball.last_hit_by;
        if every > 0 && t % every == 0 {
            let (b, p) = (&m.ball, &m.players);
            let cs = m.current_set.min(4);
            println!(
                "{t} ball {:.3} {:.3} {:.3} p1 {:.3} {:.1} p2 {:.3} {:.1} games {} {} sets {}:{} set {}",
                b.x, b.y, b.z, p[0].y, p[0].power, p[1].y, p[1].power, p[0].game, p[1].game, p[0].sets[cs], p[1].sets[cs], m.current_set
            );
        }
    }
    println!("hits {hits} winner {} set {} games {} {} sets {}:{}", m.winner, m.current_set, m.players[0].game, m.players[1].game, m.players[0].sets[0], m.players[1].sets[0]);
}

fn run(terminal: &mut ratatui::DefaultTerminal, app: &mut App, frame: Duration) -> io::Result<()> {
    let mut last = Instant::now();
    let mut snapshot = Instant::now();
    let mut next = Instant::now();
    loop {
        terminal.draw(|f| ui::draw(app, f.area(), f.buffer_mut()))?;
        next += frame;
        let now = Instant::now();
        if next < now {
            next = now;
        }
        // read every key until the next frame is due
        while event::poll(next.saturating_duration_since(Instant::now()))? {
            match event::read()? {
                Event::Key(k) => {
                    log::debug!("key {:?} {:?} {:?}", k.code, k.kind, k.modifiers);
                    app.key(k);
                }
                Event::FocusLost => {
                    log::info!("focus lost");
                    app.keys.clear();
                }
                Event::FocusGained => log::info!("focus gained"),
                Event::Resize(w, h) => log::info!("resize {w}x{h}"),
                _ => {}
            }
            if app.quit {
                break;
            }
        }
        let now = Instant::now();
        app.step(now.duration_since(last).as_secs_f32() * 1000.0);
        last = now;
        if app.quit {
            log::info!("quit");
            return Ok(());
        }
        if snapshot.elapsed() >= SNAPSHOT_EVERY {
            snapshot = Instant::now();
            log::debug!("snapshot {}", app.snapshot());
        }
    }
}

/// A frame drawn without a terminal, as a PPM picture (text cells as
/// blocks of their colour): a screen, `at` seconds in. For checking the
/// drawing by eye.
fn shot(path: &str, size: Option<String>, at: f32, screen: Option<String>, venue: usize, night: Option<bool>, settings: store::Settings) -> io::Result<()> {
    let (w, h) = size.and_then(|s| s.split_once('x').and_then(|(a, b)| Some((a.parse::<u16>().ok()?, b.parse::<u16>().ok()?)))).unwrap_or((100, 30));
    let mut app = App::new(settings, audio::Audio::none(), pads::Pads::new(false), true, 1);
    app.night_override = night;
    app.venue_sel = venue;
    match screen.as_deref() {
        Some("menu") => {}
        Some("map") => app.screen = Screen::Map,
        Some("setup") => app.screen = Screen::Setup,
        _ => app.start_demo(venue),
    }
    let mut t = 0.0;
    while t < at * 1000.0 {
        app.step(33.0);
        t += 33.0;
    }
    if screen.as_deref() == Some("paused")
        && let Some(p) = &mut app.play
    {
        p.paused = true;
    }
    let area = ratatui::layout::Rect::new(0, 0, w, h);
    let mut buf = ratatui::buffer::Buffer::empty(area);
    ui::draw(&app, area, &mut buf);
    let mut out = Vec::new();
    write!(out, "P6\n{} {}\n255\n", w, h as usize * 2)?;
    for y in 0..h as usize * 2 {
        for x in 0..w {
            let cell = &buf[(x, (y / 2) as u16)];
            let c = if cell.symbol() == "▀" {
                if y % 2 == 0 { cell.fg } else { cell.bg }
            } else if cell.symbol().trim().is_empty() {
                cell.bg
            } else {
                cell.fg
            };
            let (r, g, b) = match c {
                ratatui::style::Color::Rgb(r, g, b) => (r, g, b),
                ratatui::style::Color::Black | ratatui::style::Color::Reset => (0, 0, 0),
                _ => (200, 200, 200),
            };
            out.extend([r, g, b]);
        }
    }
    std::fs::write(path, out)
}
