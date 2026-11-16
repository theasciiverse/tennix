//! What carries over between runs, in $XDG_STATE_HOME/tennix
//! (~/.local/state/tennix, or %LOCALAPPDATA%\tennix on Windows):
//! `settings` (sound, match length, who plays) and `match`, the match you
//! left, to resume (his .tennix-current_match file).

use std::path::PathBuf;

pub fn dir() -> Option<PathBuf> {
    let env = |k: &str| std::env::var_os(k).filter(|v| !v.is_empty()).map(PathBuf::from);
    let base = env("XDG_STATE_HOME")
        .or_else(|| if cfg!(windows) { env("LOCALAPPDATA") } else { None })
        .or_else(|| env("HOME").map(|h| h.join(".local/state")))?;
    Some(base.join("tennix"))
}

pub fn read(name: &str) -> Option<String> {
    std::fs::read_to_string(dir()?.join(name)).ok()
}

pub fn write(name: &str, text: &str) {
    let Some(d) = dir() else { return };
    let p = d.join(name);
    if let Err(e) = std::fs::create_dir_all(&d).and_then(|_| std::fs::write(&p, text)) {
        log::warn!("can't save {}: {e}", p.display());
    }
}

pub fn remove(name: &str) {
    if let Some(d) = dir() {
        let _ = std::fs::remove_file(d.join(name));
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Settings {
    pub sound: bool,
    /// sets to win: 3 (best of five, his), 2 or 1
    pub sets: usize,
    pub p1: String,
    pub p2: String,
    /// what is on disk while an option (--mute, --sets) overrides it for
    /// this run only; cleared when the menu changes it
    pub disk_sound: Option<bool>,
    pub disk_sets: Option<usize>,
}

impl Default for Settings {
    fn default() -> Settings {
        Settings { sound: true, sets: 3, p1: "arrows".into(), p2: "computer".into(), disk_sound: None, disk_sets: None }
    }
}

impl Settings {
    pub fn load() -> Settings {
        let mut s = Settings::default();
        let Some(text) = read("settings") else { return s };
        for line in text.lines() {
            let Some((k, v)) = line.split_once('=') else { continue };
            let v = v.trim();
            match k.trim() {
                "sound" => s.sound = v != "off",
                "sets" => s.sets = v.parse().unwrap_or(3usize).clamp(1, 3),
                "player1" => s.p1 = v.to_string(),
                "player2" => s.p2 = v.to_string(),
                _ => {}
            }
        }
        s
    }

    pub fn save(&self) {
        let sound = self.disk_sound.unwrap_or(self.sound);
        let sets = self.disk_sets.unwrap_or(self.sets);
        write("settings", &format!("sound={}\nsets={}\nplayer1={}\nplayer2={}\n", if sound { "on" } else { "off" }, sets, self.p1, self.p2));
    }
}
