//! Gamepads through gilrs, as his joystick input: the stick or the pad's
//! cross moves, A (south) hits, B or Y topspin, X smash; the bar fills
//! while the button is held, as with his joysticks.

use crate::game::Controls;

/// A button press for the menus.
#[cfg_attr(not(feature = "gamepad"), allow(dead_code))]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum MenuPress {
    Up,
    Down,
    Left,
    Right,
    Confirm,
    Back,
    Pause,
}

pub struct Pads {
    #[cfg(feature = "gamepad")]
    gilrs: Option<gilrs::Gilrs>,
}

impl Pads {
    pub fn new(enabled: bool) -> Pads {
        #[cfg(feature = "gamepad")]
        {
            let gilrs = if enabled {
                match gilrs::Gilrs::new() {
                    Ok(g) => Some(g),
                    Err(e) => {
                        log::warn!("no gamepads: {e}");
                        None
                    }
                }
            } else {
                None
            };
            Pads { gilrs }
        }
        #[cfg(not(feature = "gamepad"))]
        {
            let _ = enabled;
            Pads {}
        }
    }

    /// Reads waiting events; returns presses for the menus.
    pub fn poll(&mut self) -> Vec<MenuPress> {
        #[cfg(feature = "gamepad")]
        {
            use gilrs::{Button, EventType};
            let mut out = Vec::new();
            let Some(g) = &mut self.gilrs else { return out };
            while let Some(ev) = g.next_event() {
                match ev.event {
                    EventType::Connected => log::info!("gamepad connected: {}", g.gamepad(ev.id).name()),
                    EventType::Disconnected => log::info!("gamepad disconnected"),
                    EventType::ButtonPressed(b, _) => {
                        log::debug!("pad {:?} press {b:?}", ev.id);
                        out.extend(match b {
                            Button::DPadUp => Some(MenuPress::Up),
                            Button::DPadDown => Some(MenuPress::Down),
                            Button::DPadLeft => Some(MenuPress::Left),
                            Button::DPadRight => Some(MenuPress::Right),
                            Button::South => Some(MenuPress::Confirm),
                            Button::East | Button::Select => Some(MenuPress::Back),
                            Button::Start => Some(MenuPress::Pause),
                            _ => None,
                        });
                    }
                    EventType::ButtonReleased(b, _) => log::debug!("pad {:?} release {b:?}", ev.id),
                    _ => {}
                }
            }
            out
        }
        #[cfg(not(feature = "gamepad"))]
        Vec::new()
    }

    /// Names of the connected pads, in order.
    pub fn names(&self) -> Vec<String> {
        #[cfg(feature = "gamepad")]
        if let Some(g) = &self.gilrs {
            return g.gamepads().map(|(_, p)| p.name().to_string()).collect();
        }
        Vec::new()
    }

    /// The controls of the n-th connected pad right now.
    pub fn controls(&self, n: usize) -> Controls {
        #[cfg(feature = "gamepad")]
        if let Some(g) = &self.gilrs
            && let Some((_, p)) = g.gamepads().nth(n)
        {
            use gilrs::{Axis, Button};
            let stick = -p.value(Axis::LeftStickY);
            let mut axis = if stick.abs() > 0.25 { stick.clamp(-1.0, 1.0) } else { 0.0 };
            if p.is_pressed(Button::DPadUp) {
                axis = -1.0;
            } else if p.is_pressed(Button::DPadDown) {
                axis = 1.0;
            }
            return Controls {
                axis,
                hit: p.is_pressed(Button::South) || p.is_pressed(Button::RightTrigger),
                topspin: p.is_pressed(Button::East) || p.is_pressed(Button::North),
                smash: p.is_pressed(Button::West) || p.is_pressed(Button::LeftTrigger),
            };
        }
        let _ = n;
        Controls::default()
    }
}
