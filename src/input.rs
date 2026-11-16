//! Keys held and let go. Tennix needs both: you hold the hit key to fill
//! the power bar and let go to swing. Terminals that speak the kitty
//! keyboard protocol (and Windows consoles) report releases; for the rest,
//! releases are inferred from the stream of key repeats, and the hit key
//! works as tap to wind up, tap again to swing.

use ratatui::crossterm::event::{KeyCode, KeyEvent, KeyEventKind};
use std::collections::HashMap;
use std::time::{Duration, Instant};

/// Without release events: how long a fresh press of a movement key
/// counts as held, and how long once the terminal's repeats are flowing.
const MOVE_FRESH: Duration = Duration::from_millis(160);
const MOVE_REPEATING: Duration = Duration::from_millis(110);
/// Two events of one key closer than this are the terminal's repeat.
const REPEAT_GAP: Duration = Duration::from_millis(150);

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Action {
    Up,
    Down,
    Hit,
    Topspin,
    Smash,
}

/// His three keyboard layouts (input.cc), with keys a terminal can report.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Layout {
    /// up and down arrows; Space hits, Left arrow topspin, Right arrow smash
    Arrows,
    /// W and S; D hits, E topspin, F smash
    Ws,
    /// O and L; K hits, I topspin, J smash
    Ol,
}

impl Layout {
    pub fn name(self) -> &'static str {
        match self {
            Layout::Arrows => "Keyboard (arrows)",
            Layout::Ws => "Keyboard (WS/DEF)",
            Layout::Ol => "Keyboard (OL/KIJ)",
        }
    }

    pub fn keys_help(self) -> &'static str {
        match self {
            Layout::Arrows => "Up/Down move, Space hit, Left topspin, Right smash",
            Layout::Ws => "W/S move, D hit, E topspin, F smash",
            Layout::Ol => "O/L move, K hit, I topspin, J smash",
        }
    }

    pub fn action(self, code: KeyCode) -> Option<Action> {
        let c = match code {
            KeyCode::Char(c) => Some(c.to_ascii_lowercase()),
            _ => None,
        };
        match self {
            Layout::Arrows => match code {
                KeyCode::Up => Some(Action::Up),
                KeyCode::Down => Some(Action::Down),
                KeyCode::Char(' ') => Some(Action::Hit),
                KeyCode::Left => Some(Action::Topspin),
                KeyCode::Right => Some(Action::Smash),
                _ => None,
            },
            Layout::Ws => match c? {
                'w' => Some(Action::Up),
                's' => Some(Action::Down),
                'd' => Some(Action::Hit),
                'e' => Some(Action::Topspin),
                'f' => Some(Action::Smash),
                _ => None,
            },
            Layout::Ol => match c? {
                'o' => Some(Action::Up),
                'l' => Some(Action::Down),
                'k' => Some(Action::Hit),
                'i' => Some(Action::Topspin),
                'j' => Some(Action::Smash),
                _ => None,
            },
        }
    }
}

#[derive(Clone, Copy, Debug)]
struct Held {
    /// held now (with releases reported) or until this moment (inferred)
    until: Option<Instant>,
    /// the last event of this key, press or repeat
    last: Instant,
    /// inferred mode, swing keys: wound up and waiting for the second tap
    wound: bool,
}

/// What is held, for every key that matters.
pub struct Keys {
    /// the terminal reports releases
    pub releases: bool,
    held: HashMap<KeyCode, Held>,
}

fn is_swing(a: Action) -> bool {
    matches!(a, Action::Hit | Action::Topspin | Action::Smash)
}

impl Keys {
    pub fn new(releases: bool) -> Keys {
        Keys { releases, held: HashMap::new() }
    }

    /// Feed one key event; `action` is what it means in the match, if anything.
    pub fn event(&mut self, k: KeyEvent, action: Option<Action>, now: Instant) {
        let code = normalise(k.code);
        if !self.releases && k.kind == KeyEventKind::Release {
            // the terminal does report releases after all (a Windows
            // console, or a kitty terminal that missed the query)
            log::info!("key releases seen: switching to held keys");
            self.releases = true;
            self.held.clear();
        }
        if self.releases {
            match k.kind {
                KeyEventKind::Press => {
                    self.held.insert(code, Held { until: None, last: now, wound: false });
                }
                KeyEventKind::Repeat => {}
                KeyEventKind::Release => {
                    self.held.remove(&code);
                }
            }
            return;
        }
        let prev = self.held.get(&code).copied();
        let fresh = prev.is_none_or(|h| now.duration_since(h.last) >= REPEAT_GAP);
        match action {
            Some(a) if is_swing(a) => {
                // tap to wind up, tap again to swing; the terminal's repeats
                // of a held key change nothing
                let mut h = prev.unwrap_or(Held { until: None, last: now, wound: false });
                if fresh {
                    h.wound = !h.wound;
                }
                h.last = now;
                h.until = None;
                self.held.insert(code, h);
            }
            _ => {
                let hold = if fresh { MOVE_FRESH } else { MOVE_REPEATING };
                self.held.insert(code, Held { until: Some(now + hold), last: now, wound: false });
            }
        }
    }

    pub fn down(&self, code: KeyCode, now: Instant) -> bool {
        let Some(h) = self.held.get(&normalise(code)) else { return false };
        if self.releases {
            return true;
        }
        match h.until {
            Some(t) => now < t,
            None => h.wound,
        }
    }

    /// Everything let go (focus lost, a menu opened, a point scored in
    /// inferred mode so a wound-up swing does not linger).
    pub fn clear(&mut self) {
        self.held.clear();
    }
}

fn normalise(code: KeyCode) -> KeyCode {
    match code {
        KeyCode::Char(c) => KeyCode::Char(c.to_ascii_lowercase()),
        c => c,
    }
}

/// All keys of a layout for an action, for asking `Keys::down`.
pub fn codes(layout: Layout, a: Action) -> &'static [KeyCode] {
    use KeyCode::*;
    match (layout, a) {
        (Layout::Arrows, Action::Up) => &[Up],
        (Layout::Arrows, Action::Down) => &[Down],
        (Layout::Arrows, Action::Hit) => &[Char(' ')],
        (Layout::Arrows, Action::Topspin) => &[Left],
        (Layout::Arrows, Action::Smash) => &[Right],
        (Layout::Ws, Action::Up) => &[Char('w')],
        (Layout::Ws, Action::Down) => &[Char('s')],
        (Layout::Ws, Action::Hit) => &[Char('d')],
        (Layout::Ws, Action::Topspin) => &[Char('e')],
        (Layout::Ws, Action::Smash) => &[Char('f')],
        (Layout::Ol, Action::Up) => &[Char('o')],
        (Layout::Ol, Action::Down) => &[Char('l')],
        (Layout::Ol, Action::Hit) => &[Char('k')],
        (Layout::Ol, Action::Topspin) => &[Char('i')],
        (Layout::Ol, Action::Smash) => &[Char('j')],
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use ratatui::crossterm::event::{KeyEventState, KeyModifiers};

    fn ev(code: KeyCode, kind: KeyEventKind) -> KeyEvent {
        KeyEvent { code, modifiers: KeyModifiers::NONE, kind, state: KeyEventState::NONE }
    }

    #[test]
    fn with_releases_a_key_is_held_until_let_go() {
        let t = Instant::now();
        let mut k = Keys::new(true);
        k.event(ev(KeyCode::Char(' '), KeyEventKind::Press), Some(Action::Hit), t);
        assert!(k.down(KeyCode::Char(' '), t + Duration::from_secs(5)));
        k.event(ev(KeyCode::Char(' '), KeyEventKind::Release), Some(Action::Hit), t);
        assert!(!k.down(KeyCode::Char(' '), t));
    }

    #[test]
    fn without_releases_tap_winds_and_tap_again_swings() {
        let t = Instant::now();
        let ms = Duration::from_millis;
        let mut k = Keys::new(false);
        let sp = KeyCode::Char(' ');
        k.event(ev(sp, KeyEventKind::Press), Some(Action::Hit), t);
        assert!(k.down(sp, t + ms(400)));
        // the terminal's repeats of a held key change nothing
        for i in 0..10 {
            k.event(ev(sp, KeyEventKind::Press), Some(Action::Hit), t + ms(500 + i * 30));
        }
        // the first repeat came after a long gap: it counts as the second tap
        assert!(!k.down(sp, t + ms(900)));
        k.event(ev(sp, KeyEventKind::Press), Some(Action::Hit), t + ms(1500));
        assert!(k.down(sp, t + ms(1600)));
        k.event(ev(sp, KeyEventKind::Press), Some(Action::Hit), t + ms(1800));
        assert!(!k.down(sp, t + ms(1810)));
    }

    #[test]
    fn without_releases_a_movement_tap_is_a_short_step() {
        let t = Instant::now();
        let ms = Duration::from_millis;
        let mut k = Keys::new(false);
        k.event(ev(KeyCode::Up, KeyEventKind::Press), Some(Action::Up), t);
        assert!(k.down(KeyCode::Up, t + ms(100)));
        assert!(!k.down(KeyCode::Up, t + ms(200)));
    }

    #[test]
    fn layouts() {
        assert_eq!(Layout::Ws.action(KeyCode::Char('D')), Some(Action::Hit));
        assert_eq!(Layout::Ol.action(KeyCode::Char('i')), Some(Action::Topspin));
        assert_eq!(Layout::Arrows.action(KeyCode::Right), Some(Action::Smash));
        assert_eq!(Layout::Arrows.action(KeyCode::Char('d')), None);
    }
}
