//! The game around the match: his main menu, the world map to pick a
//! location, the controller setup, the match itself, pausing and resuming,
//! and the sounds and log lines the match gives off.

use crate::audio::{Audio, Sound};
use crate::game::{self, Controls, Event, Match};
use crate::input::{self, Action, Keys, Layout};
use crate::locations::{self, LOCATIONS};
use crate::pads::{MenuPress, Pads};
use crate::store::{self, Settings};
use ratatui::crossterm::event::{KeyCode, KeyEvent, KeyEventKind, KeyModifiers};
use std::time::Instant;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Screen {
    Menu,
    Map,
    Setup,
    Play,
}

/// Who plays a side.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Ctl {
    Computer,
    Keys(Layout),
    Pad(usize),
}

impl Ctl {
    pub fn id(self) -> String {
        match self {
            Ctl::Computer => "computer".into(),
            Ctl::Keys(Layout::Arrows) => "arrows".into(),
            Ctl::Keys(Layout::Ws) => "ws".into(),
            Ctl::Keys(Layout::Ol) => "ol".into(),
            Ctl::Pad(n) => format!("pad{n}"),
        }
    }

    pub fn parse(s: &str) -> Option<Ctl> {
        Some(match s {
            "computer" => Ctl::Computer,
            "arrows" => Ctl::Keys(Layout::Arrows),
            "ws" => Ctl::Keys(Layout::Ws),
            "ol" => Ctl::Keys(Layout::Ol),
            p => Ctl::Pad(p.strip_prefix("pad")?.parse().ok()?),
        })
    }

    pub fn name(self, pads: &[String]) -> String {
        match self {
            Ctl::Computer => format!("{} (computer)", game::AI_NAME),
            Ctl::Keys(l) => l.name().into(),
            Ctl::Pad(n) => pads.get(n).cloned().unwrap_or_else(|| format!("Gamepad {}", n + 1)),
        }
    }
}

/// The choices for a side, in his cycling order: player 1 starts on the
/// arrows, player 2 on the computer; WS is player 1's own, OL player 2's.
pub fn choices(side: u8, pads: usize) -> Vec<Ctl> {
    let mut v = if side == 1 {
        vec![Ctl::Keys(Layout::Arrows), Ctl::Keys(Layout::Ws)]
    } else {
        vec![Ctl::Computer, Ctl::Keys(Layout::Ol), Ctl::Keys(Layout::Arrows)]
    };
    v.extend((0..pads).map(Ctl::Pad));
    if side == 1 {
        v.push(Ctl::Computer);
    }
    v
}

pub struct Play {
    pub m: Match,
    pub venue: usize,
    pub night: bool,
    pub ctl: [Ctl; 2],
    /// match time in ms (the umpire's animation and the fog)
    pub time_ms: u64,
    pub frame: u64,
    acc: f32,
    pub paused: bool,
    /// ms since the match was won
    pub over_ms: u64,
}

impl Play {
    pub fn new(sets: usize, venue: usize, night: bool, ctl: [Ctl; 2]) -> Play {
        let mut m = Match::new(sets);
        for i in 0..2 {
            m.players[i].ai = ctl[i] == Ctl::Computer;
        }
        if ctl == [Ctl::Computer; 2] {
            m.variation = Some(seed_now() ^ (venue as u64 + 1).wrapping_mul(0x9e37_79b9_7f4a_7c15));
        }
        Play { m, venue, night, ctl, time_ms: 0, frame: 0, acc: 0.0, paused: false, over_ms: 0 }
    }

    pub fn loc(&self) -> &'static locations::Location {
        &LOCATIONS[self.venue]
    }

    fn save_text(&self) -> String {
        format!("venue {}\nnight {}\nctl {} {}\n{}", self.venue, self.night as u8, self.ctl[0].id(), self.ctl[1].id(), self.m.save())
    }

    fn load_text(text: &str) -> Option<Play> {
        let mut venue = None;
        let mut night = false;
        let mut ctl = None;
        for line in text.lines() {
            let f: Vec<&str> = line.split_whitespace().collect();
            match f.as_slice() {
                ["venue", v] => venue = v.parse::<usize>().ok().filter(|v| *v < LOCATIONS.len()),
                ["night", n] => night = *n == "1",
                ["ctl", a, b] => ctl = Some([Ctl::parse(a)?, Ctl::parse(b)?]),
                _ => {}
            }
        }
        let m = Match::load(text)?;
        let mut p = Play::new(m.sets_to_win, venue?, night, ctl?);
        p.m = m;
        Some(p)
    }

    /// Runs the match on by `dt` ms in his 15 ms ticks.
    fn advance(&mut self, dt: f32, controls: impl Fn(&Play, u8) -> Controls, events: &mut Vec<Event>) -> u32 {
        if self.paused {
            return 0;
        }
        self.frame += 1;
        if self.m.winner != 0 {
            self.over_ms += dt as u64;
            return 0;
        }
        self.acc += dt;
        let mut ticks = 0;
        while self.acc >= game::GAME_TICKS as f32 {
            let c = [controls(self, 1), controls(self, 2)];
            self.m.tick(c, events);
            self.acc -= game::GAME_TICKS as f32;
            self.time_ms += game::GAME_TICKS as u64;
            ticks += 1;
        }
        ticks
    }
}

static SEED: std::sync::atomic::AtomicU64 = std::sync::atomic::AtomicU64::new(0x2003_0510);

/// Seeds the computer-against-itself variation (--seed, or the clock).
pub fn set_seed(s: u64) {
    SEED.store(s | 1, std::sync::atomic::Ordering::Relaxed);
}

fn seed_now() -> u64 {
    // a new seed for every match, from the one set at the start
    let s = SEED.load(std::sync::atomic::Ordering::Relaxed);
    let n = s.wrapping_mul(6_364_136_223_846_793_005).wrapping_add(1_442_695_040_888_963_407);
    SEED.store(n, std::sync::atomic::Ordering::Relaxed);
    n | 1
}

pub const MENU_PLAY: &str = "Play";
pub const MENU_RESUME: &str = "Resume match";
pub const MENU_QUIT: &str = "Quit";

pub struct App {
    pub screen: Screen,
    pub sel: usize,
    pub settings: Settings,
    pub play: Option<Play>,
    /// the computer playing itself behind the menus
    pub attract: Play,
    pub venue_sel: usize,
    pub setup_row: usize,
    pub setup: [Ctl; 2],
    pub keys: Keys,
    pub pads: Pads,
    pub pad_names: Vec<String>,
    pub audio: Audio,
    pub demo: bool,
    pub quit: bool,
    /// the sound effect variations (his rand())
    rng: u64,
    events: Vec<Event>,
    /// seconds since 1970, for night and day on the map
    pub clock: u64,
    /// night forced on (true) or off (false) by an option
    pub night_override: Option<bool>,
    pub menu_time_ms: u64,
}

impl App {
    pub fn new(settings: Settings, audio: Audio, pads: Pads, releases: bool, seed: u64) -> App {
        let pad_names = pads.names();
        let setup = [
            Ctl::parse(&settings.p1).filter(|c| choices(1, pad_names.len()).contains(c)).unwrap_or(Ctl::Keys(Layout::Arrows)),
            Ctl::parse(&settings.p2).filter(|c| choices(2, pad_names.len()).contains(c)).unwrap_or(Ctl::Computer),
        ];
        let play = store::read("match").and_then(|t| Play::load_text(&t)).filter(|p| p.m.winner == 0);
        if play.is_some() {
            log::info!("a paused match was found");
        }
        App {
            screen: Screen::Menu,
            sel: 0,
            settings,
            play,
            attract: Play::new(3, 1, false, [Ctl::Computer; 2]),
            venue_sel: 0,
            setup_row: 2,
            setup,
            keys: Keys::new(releases),
            pads,
            pad_names,
            audio,
            demo: false,
            quit: false,
            rng: seed | 1,
            events: Vec::new(),
            clock: std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).map_or(0, |d| d.as_secs()),
            night_override: None,
            menu_time_ms: 0,
        }
    }

    fn rand(&mut self) -> u64 {
        self.rng ^= self.rng << 13;
        self.rng ^= self.rng >> 7;
        self.rng ^= self.rng << 17;
        self.rng
    }

    pub fn menu_items(&self) -> Vec<String> {
        let mut v = vec![MENU_PLAY.to_string()];
        if self.play.is_some() {
            v.push(MENU_RESUME.into());
        }
        v.push(format!("Match: {}", match self.settings.sets {
            1 => "one set",
            2 => "best of three",
            _ => "best of five",
        }));
        v.push(format!("Sound: {}", if self.settings.sound { "on" } else { "off" }));
        v.push(MENU_QUIT.into());
        v
    }

    fn set_screen(&mut self, s: Screen) {
        if self.screen != s {
            log::info!("screen {:?} -> {s:?}", self.screen);
            self.screen = s;
            self.keys.clear();
        }
    }

    /// The computer against itself, straight into a match (--demo).
    pub fn start_demo(&mut self, venue: usize) {
        self.demo = true;
        let night = self.night_override.unwrap_or(false);
        self.play = Some(Play::new(self.settings.sets, venue, night, [Ctl::Computer; 2]));
        self.begin_play();
    }

    fn night_at(&self, venue: usize) -> bool {
        self.night_override.unwrap_or_else(|| locations::is_night(LOCATIONS[venue].worldmap_x, locations::day_night(self.clock, 500)))
    }

    fn start_match(&mut self) {
        let venue = self.venue_sel;
        let p = Play::new(self.settings.sets, venue, self.night_at(venue), self.setup);
        self.settings.p1 = self.setup[0].id();
        self.settings.p2 = self.setup[1].id();
        self.settings.save();
        log::info!(
            "new match at {} ({}), {} against {}, {} sets to win, night {}",
            p.loc().name,
            p.loc().city,
            self.setup[0].id(),
            self.setup[1].id(),
            self.settings.sets,
            p.night
        );
        self.play = Some(p);
        self.begin_play();
    }

    fn begin_play(&mut self) {
        if let Some(p) = &self.play {
            let loc = p.loc();
            self.audio.start_match(loc.max_visitors > 100, loc.rain > 0);
        }
        self.set_screen(Screen::Play);
    }

    /// Back to the menu with the match kept to resume (his Escape).
    fn leave_match(&mut self) {
        self.audio.stop_match();
        if let Some(p) = &mut self.play {
            p.paused = false;
            if p.m.winner != 0 {
                self.play = None;
                store::remove("match");
            } else {
                store::write("match", &p.save_text());
                log::info!("match left, kept to resume: {} {}", p.m.format_game(), p.m.format_sets());
            }
        }
        self.sel = 0;
        self.set_screen(Screen::Menu);
    }

    pub fn save_on_exit(&self) {
        if let Some(p) = &self.play
            && !self.demo
        {
            if p.m.winner == 0 {
                store::write("match", &p.save_text());
            } else {
                store::remove("match");
            }
        }
    }

    pub fn toggle_sound(&mut self) {
        self.settings.sound = !self.settings.sound;
        self.settings.disk_sound = None;
        self.audio.muted = !self.settings.sound;
        if self.settings.sound {
            if let (Screen::Play, Some(p)) = (self.screen, &self.play) {
                let loc = p.loc();
                self.audio.start_match(loc.max_visitors > 100, loc.rain > 0);
            }
        } else {
            self.audio.stop_match();
        }
        self.settings.save();
        log::info!("sound {}", self.settings.sound);
    }

    fn click(&self, s: Sound) {
        self.audio.play(s, 0.6, 0.5);
    }

    /// Controls of a side right now, from the keys or a pad.
    fn controls(keys: &Keys, pads: &Pads, ctl: Ctl, now: Instant) -> Controls {
        match ctl {
            Ctl::Computer => Controls::default(),
            Ctl::Pad(n) => pads.controls(n),
            Ctl::Keys(l) => {
                let d = |a: Action| input::codes(l, a).iter().any(|c| keys.down(*c, now));
                let axis = d(Action::Down) as i32 as f32 - d(Action::Up) as i32 as f32;
                Controls { axis, hit: d(Action::Hit), topspin: d(Action::Topspin), smash: d(Action::Smash) }
            }
        }
    }

    /// Time passes: the match (or the one behind the menu) runs on.
    pub fn step(&mut self, dt_ms: f32) {
        let dt = dt_ms.min(100.0);
        for press in self.pads.poll() {
            self.pad_press(press);
        }
        let now = Instant::now();
        let mut events = std::mem::take(&mut self.events);
        events.clear();
        if self.screen == Screen::Play {
            let (keys, pads) = (&self.keys, &self.pads);
            if let Some(p) = &mut self.play {
                p.advance(dt, |p, side| Self::controls(keys, pads, p.ctl[side as usize - 1], now), &mut events);
            }
            for e in &events {
                self.match_event(*e);
            }
            if let Some(p) = &self.play
                && p.m.winner != 0
                && p.over_ms > 0
                && p.over_ms <= dt as u64
            {
                log::info!("match over: player {} wins {}", p.m.winner, p.m.format_sets());
                self.audio.stop_match();
                if !self.demo {
                    store::remove("match");
                }
            }
            // the demo goes on to the next venue after a match
            if self.demo
                && let Some(p) = &self.play
                && p.over_ms > 6000
            {
                let next = (p.venue + 1) % LOCATIONS.len();
                self.start_demo(next);
            }
        } else {
            self.menu_time_ms += dt as u64;
            self.attract.advance(dt, |_, _| Controls::default(), &mut events);
            if self.attract.over_ms > 3000 {
                let next = (self.attract.venue + 1) % LOCATIONS.len();
                self.attract = Play::new(1, next, false, [Ctl::Computer; 2]);
            }
        }
        self.events = events;
    }

    fn match_event(&mut self, e: Event) {
        let Some(p) = &self.play else { return };
        let loc = p.loc();
        match e {
            Event::Ground { speed, x } => {
                let s = if self.rand() % 2 == 0 { Sound::Ground1 } else { Sound::Ground2 };
                self.audio.play(s, (speed / 2.0).clamp(0.0, 1.0), (x / game::WIDTH).clamp(0.0, 1.0));
            }
            Event::Out => {
                log::debug!("out: {}", p.m.format_status());
                self.audio.play(Sound::Out, 1.0, 0.5);
            }
            Event::Applause => {
                if loc.max_visitors > 100 {
                    self.audio.applause();
                }
            }
            Event::Racket { player, x } => {
                let pl = p.m.p(player);
                log::debug!("hit by player {player}, rally {}, {:?}, ball ({:.0},{:.0}) going ({:.2},{:.2},{:.2})", p.m.rally, pl.desire, p.m.ball.x, p.m.ball.y, p.m.ball.move_x, p.m.ball.move_y, p.m.ball.move_z);
                self.audio.stop_applause();
                let s = if self.rand() % 2 == 0 { Sound::Racket1 } else { Sound::Racket2 };
                self.audio.play(s, 1.0, if x / game::WIDTH < 0.5 { 0.3 } else { 0.7 });
            }
            Event::Point(w) => {
                log::info!("point to player {w}: game {} sets {} serve {}", p.m.format_game(), p.m.format_sets(), p.m.serving_player);
            }
        }
    }

    fn pad_press(&mut self, press: MenuPress) {
        let code = match press {
            MenuPress::Up => KeyCode::Up,
            MenuPress::Down => KeyCode::Down,
            MenuPress::Left => KeyCode::Left,
            MenuPress::Right => KeyCode::Right,
            MenuPress::Confirm => KeyCode::Enter,
            MenuPress::Back => KeyCode::Esc,
            MenuPress::Pause => KeyCode::Char('p'),
        };
        if self.screen == Screen::Play {
            // in a match the pad's buttons play; Start pauses
            if press == MenuPress::Pause {
                self.toggle_pause();
            }
            return;
        }
        self.menu_key(code);
    }

    fn toggle_pause(&mut self) {
        if let Some(p) = &mut self.play {
            p.paused = !p.paused;
            log::info!("paused {}", p.paused);
            self.keys.clear();
        }
    }

    /// The meaning of a key in the match for the sides played from keys.
    fn action(&self, code: KeyCode) -> Option<Action> {
        let ctl = self.play.as_ref().map_or(self.setup, |p| p.ctl);
        ctl.iter().find_map(|c| match c {
            Ctl::Keys(l) => l.action(code),
            _ => None,
        })
    }

    pub fn key(&mut self, k: KeyEvent) {
        let now = Instant::now();
        let action = if self.screen == Screen::Play { self.action(k.code) } else { None };
        self.keys.event(k, action, now);
        if k.kind == KeyEventKind::Release {
            return;
        }
        if k.modifiers.contains(KeyModifiers::CONTROL) && k.code == KeyCode::Char('c') {
            self.quit = true;
            return;
        }
        if self.screen == Screen::Play {
            if k.kind == KeyEventKind::Repeat || action.is_some() {
                return;
            }
            let over = self.play.as_ref().is_some_and(|p| p.m.winner != 0);
            match k.code {
                KeyCode::Esc | KeyCode::Char('q') | KeyCode::Char('Q') => {
                    if self.demo {
                        self.quit = true;
                    } else {
                        self.leave_match();
                    }
                }
                KeyCode::Enter if over && !self.demo => self.leave_match(),
                KeyCode::Char('p') | KeyCode::Char('P') if !over => self.toggle_pause(),
                KeyCode::Char('m') | KeyCode::Char('M') => self.toggle_sound(),
                _ => {}
            }
            return;
        }
        if k.kind == KeyEventKind::Repeat && matches!(k.code, KeyCode::Enter | KeyCode::Esc) {
            return;
        }
        self.menu_key(k.code);
    }

    fn menu_key(&mut self, code: KeyCode) {
        match self.screen {
            Screen::Menu => {
                let items = self.menu_items();
                match code {
                    KeyCode::Up | KeyCode::Char('w') | KeyCode::Char('k') => {
                        self.sel = (self.sel + items.len() - 1) % items.len();
                        self.click(Sound::MouseOver);
                    }
                    KeyCode::Down | KeyCode::Char('s') | KeyCode::Char('j') => {
                        self.sel = (self.sel + 1) % items.len();
                        self.click(Sound::MouseOver);
                    }
                    KeyCode::Esc | KeyCode::Char('q') => self.quit = true,
                    KeyCode::Char('m') => self.toggle_sound(),
                    KeyCode::Enter | KeyCode::Char(' ') | KeyCode::Left | KeyCode::Right => {
                        let item = items[self.sel.min(items.len() - 1)].clone();
                        let enter = matches!(code, KeyCode::Enter | KeyCode::Char(' '));
                        self.click(Sound::MouseClick);
                        if item == MENU_PLAY && enter {
                            self.set_screen(Screen::Map);
                        } else if item == MENU_RESUME && enter {
                            log::info!("resume match");
                            self.begin_play();
                        } else if item == MENU_QUIT && enter {
                            self.quit = true;
                        } else if item.starts_with("Match") {
                            // best of five, best of three, one set
                            let order = [3, 2, 1];
                            let i = order.iter().position(|s| *s == self.settings.sets).unwrap_or(0);
                            let i = if code == KeyCode::Left { (i + 2) % 3 } else { (i + 1) % 3 };
                            self.settings.sets = order[i];
                            self.settings.disk_sets = None;
                            self.settings.save();
                            log::info!("match length: {} sets to win", self.settings.sets);
                        } else if item.starts_with("Sound") {
                            self.toggle_sound();
                        }
                    }
                    _ => {}
                }
            }
            Screen::Map => match code {
                KeyCode::Left | KeyCode::Up | KeyCode::Char('a') | KeyCode::Char('w') => {
                    self.venue_sel = (self.venue_sel + LOCATIONS.len() - 1) % LOCATIONS.len();
                    self.click(Sound::MouseOver);
                }
                KeyCode::Right | KeyCode::Down | KeyCode::Char('d') | KeyCode::Char('s') | KeyCode::Tab => {
                    self.venue_sel = (self.venue_sel + 1) % LOCATIONS.len();
                    self.click(Sound::MouseOver);
                }
                KeyCode::Enter | KeyCode::Char(' ') => {
                    self.click(Sound::MouseClick);
                    self.setup_row = 2;
                    self.set_screen(Screen::Setup);
                }
                KeyCode::Esc | KeyCode::Backspace | KeyCode::Char('q') => self.set_screen(Screen::Menu),
                _ => {}
            },
            Screen::Setup => match code {
                KeyCode::Up | KeyCode::Char('w') => {
                    self.setup_row = (self.setup_row + 2) % 3;
                    self.click(Sound::MouseOver);
                }
                KeyCode::Down | KeyCode::Char('s') | KeyCode::Tab => {
                    self.setup_row = (self.setup_row + 1) % 3;
                    self.click(Sound::MouseOver);
                }
                KeyCode::Left | KeyCode::Right if self.setup_row < 2 => {
                    self.cycle(self.setup_row as u8 + 1, code == KeyCode::Right);
                    self.click(Sound::MouseOver);
                }
                KeyCode::Char('1') => self.cycle(1, true),
                KeyCode::Char('2') => self.cycle(2, true),
                KeyCode::Enter | KeyCode::Char(' ') => {
                    self.click(Sound::MouseClick);
                    if self.setup_row < 2 {
                        self.cycle(self.setup_row as u8 + 1, true);
                    } else {
                        self.start_match();
                    }
                }
                KeyCode::Esc | KeyCode::Backspace | KeyCode::Char('q') => self.set_screen(Screen::Map),
                _ => {}
            },
            Screen::Play => {}
        }
    }

    /// The next controller for a side, skipping one the other side uses.
    fn cycle(&mut self, side: u8, forward: bool) {
        let opts = choices(side, self.pad_names.len());
        let i = side as usize - 1;
        let other = self.setup[1 - i];
        let mut k = opts.iter().position(|c| *c == self.setup[i]).unwrap_or(0);
        for _ in 0..opts.len() {
            k = if forward { (k + 1) % opts.len() } else { (k + opts.len() - 1) % opts.len() };
            if opts[k] == Ctl::Computer || opts[k] != other {
                break;
            }
        }
        self.setup[i] = opts[k];
        log::info!("player {side} plays with {}", self.setup[i].id());
    }

    pub fn snapshot(&self) -> String {
        match (&self.play, self.screen) {
            (Some(p), Screen::Play) => format!(
                "play at {} paused {} night {} keys_released {} | {}",
                p.loc().name,
                p.paused,
                p.night,
                self.keys.releases,
                p.m.snapshot()
            ),
            _ => format!("screen {:?} sel {} venue {} setup {} {}", self.screen, self.sel, self.venue_sel, self.setup[0].id(), self.setup[1].id()),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn ctl_ids_round_trip() {
        for c in [Ctl::Computer, Ctl::Keys(Layout::Arrows), Ctl::Keys(Layout::Ws), Ctl::Keys(Layout::Ol), Ctl::Pad(2)] {
            assert_eq!(Ctl::parse(&c.id()), Some(c));
        }
    }

    #[test]
    fn a_kept_match_comes_back() {
        let mut p = Play::new(2, 3, true, [Ctl::Keys(Layout::Ws), Ctl::Computer]);
        let mut ev = Vec::new();
        p.advance(3000.0, |_, _| Controls::default(), &mut ev);
        let q = Play::load_text(&p.save_text()).unwrap();
        assert_eq!(q.venue, 3);
        assert!(q.night);
        assert_eq!(q.ctl, p.ctl);
        assert_eq!(q.m.format_sets(), p.m.format_sets());
    }
}
