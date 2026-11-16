//! Thomas Perl's sounds from Tennix (data/*.ogg), played as his
//! sound.cc plays them: the ball on the ground louder the harder it lands,
//! rackets panned to the side that hit, the crowd murmuring through the
//! match at venues with a crowd, applause after a point that stops when
//! the next ball is served, rain where it rains.

use rodio::buffer::SamplesBuffer;
use rodio::{Decoder, MixerDeviceSink, Player, Source};
use std::io::Cursor;
use std::num::NonZero;
use std::time::Duration;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Sound {
    Racket1,
    Racket2,
    Ground1,
    Ground2,
    Out,
    Applause,
    Audience,
    Rain,
    MouseOver,
    MouseClick,
}

const FILES: [(Sound, &[u8]); 10] = [
    (Sound::Racket1, include_bytes!("../assets/sounds/racket1.ogg")),
    (Sound::Racket2, include_bytes!("../assets/sounds/racket2.ogg")),
    (Sound::Ground1, include_bytes!("../assets/sounds/ground1.ogg")),
    (Sound::Ground2, include_bytes!("../assets/sounds/ground2.ogg")),
    (Sound::Out, include_bytes!("../assets/sounds/out.ogg")),
    (Sound::Applause, include_bytes!("../assets/sounds/applause.ogg")),
    (Sound::Audience, include_bytes!("../assets/sounds/audience.ogg")),
    (Sound::Rain, include_bytes!("../assets/sounds/rain.ogg")),
    (Sound::MouseOver, include_bytes!("../assets/sounds/mouseover.ogg")),
    (Sound::MouseClick, include_bytes!("../assets/sounds/mouseclick.ogg")),
];

/// A decoded sound, mixed down to mono.
struct Clip {
    rate: u32,
    samples: Vec<f32>,
}

fn decode(bytes: &'static [u8]) -> Option<Clip> {
    let d = Decoder::new(Cursor::new(bytes)).ok()?;
    let ch = d.channels().get() as usize;
    let rate = d.sample_rate().get();
    let all: Vec<f32> = d.collect();
    let samples = all.chunks(ch).map(|f| f.iter().sum::<f32>() / ch as f32).collect();
    Some(Clip { rate, samples })
}

pub struct Audio {
    sink: Option<MixerDeviceSink>,
    clips: Vec<(Sound, Clip)>,
    pub muted: bool,
    applause: Option<Player>,
    audience: Option<Player>,
    rain: Option<Player>,
}

impl Audio {
    pub fn new(muted: bool) -> Audio {
        let sink = match rodio::DeviceSinkBuilder::open_default_sink() {
            Ok(mut s) => {
                s.log_on_drop(false);
                Some(s)
            }
            Err(e) => {
                log::warn!("no audio: {e}");
                None
            }
        };
        let clips = if sink.is_some() {
            FILES
                .iter()
                .filter_map(|(s, b)| match decode(b) {
                    Some(c) => Some((*s, c)),
                    None => {
                        log::warn!("cannot decode {s:?}");
                        None
                    }
                })
                .collect()
        } else {
            Vec::new()
        };
        Audio { sink, clips, muted, applause: None, audience: None, rain: None }
    }

    /// No device at all (tests, --shot).
    pub fn none() -> Audio {
        Audio { sink: None, clips: Vec::new(), muted: true, applause: None, audience: None, rain: None }
    }

    pub fn available(&self) -> bool {
        self.sink.is_some()
    }

    /// A stereo buffer of a clip at a volume (0..1) and pan (0 left, 1 right).
    fn buffer(&self, s: Sound, volume: f32, pan: f32) -> Option<SamplesBuffer> {
        let clip = &self.clips.iter().find(|(k, _)| *k == s)?.1;
        // SDL_mixer's Mix_SetPanning: full volume on the near side
        let pan = pan.clamp(0.0, 1.0);
        let (l, r) = ((2.0 * (1.0 - pan)).min(1.0), (2.0 * pan).min(1.0));
        let mut v = Vec::with_capacity(clip.samples.len() * 2);
        for x in &clip.samples {
            v.push(x * volume * l);
            v.push(x * volume * r);
        }
        Some(SamplesBuffer::new(NonZero::new(2).unwrap(), NonZero::new(clip.rate).unwrap(), v))
    }

    pub fn play(&self, s: Sound, volume: f32, pan: f32) {
        if self.muted {
            return;
        }
        let (Some(sink), Some(buf)) = (&self.sink, self.buffer(s, volume, pan)) else { return };
        sink.mixer().add(buf);
    }

    fn player(&self, s: Sound, looped: bool, fade_in: Duration) -> Option<Player> {
        let sink = self.sink.as_ref()?;
        let buf = self.buffer(s, 1.0, 0.5)?;
        let p = Player::connect_new(sink.mixer());
        if looped {
            p.append(buf.repeat_infinite().fade_in(fade_in));
        } else {
            p.append(buf);
        }
        Some(p)
    }

    /// The crowd claps after a point (only where there is a crowd).
    pub fn applause(&mut self) {
        if self.muted {
            return;
        }
        self.applause = self.player(Sound::Applause, false, Duration::ZERO);
    }

    /// The audience stops clapping when the ball is served.
    pub fn stop_applause(&mut self) {
        if let Some(p) = self.applause.take() {
            p.stop();
        }
    }

    /// Background loops for a match: the murmur of the crowd, the rain.
    pub fn start_match(&mut self, crowd: bool, rain: bool) {
        self.stop_match();
        if self.muted {
            return;
        }
        if crowd {
            self.audience = self.player(Sound::Audience, true, Duration::ZERO);
        }
        if rain {
            // his rain fades in over three seconds (FADE_IN_MS)
            self.rain = self.player(Sound::Rain, true, Duration::from_secs(3));
        }
    }

    pub fn stop_match(&mut self) {
        for p in [self.audience.take(), self.rain.take(), self.applause.take()].into_iter().flatten() {
            p.stop();
        }
    }
}
