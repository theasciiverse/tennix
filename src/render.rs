//! Drawing the court in half blocks: two square pixels a cell, 24-bit
//! colour, after his render(): the court surface of the venue, the lines
//! and the net, the umpire in the chair, the two players with their
//! rackets, the ball and its shadow, then the weather over it all (rain,
//! fog, floodlights at night).

use crate::game::{self, Match, Referee};
use crate::locations::{Location, Surface};
use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::style::Color;

pub type Rgb = [f32; 3];

pub struct Canvas {
    pub w: usize,
    pub h: usize,
    px: Vec<Rgb>,
}

impl Canvas {
    pub fn new(w: usize, h: usize, bg: Rgb) -> Canvas {
        Canvas { w, h, px: vec![bg; w * h] }
    }

    pub fn get(&self, x: usize, y: usize) -> Rgb {
        self.px[y * self.w + x]
    }

    /// Blends a colour into one pixel.
    pub fn blend(&mut self, x: i32, y: i32, c: Rgb, a: f32) {
        if x < 0 || y < 0 || x as usize >= self.w || y as usize >= self.h || a <= 0.0 {
            return;
        }
        let p = &mut self.px[y as usize * self.w + x as usize];
        let a = a.min(1.0);
        for i in 0..3 {
            p[i] += (c[i] - p[i]) * a;
        }
    }

    pub fn map(&mut self, f: impl Fn(usize, usize, Rgb) -> Rgb) {
        for y in 0..self.h {
            for x in 0..self.w {
                let i = y * self.w + x;
                self.px[i] = f(x, y, self.px[i]);
            }
        }
    }

    /// Writes the pixels into the buffer, two a cell ("▀", top pixel as
    /// the foreground, bottom as the background).
    pub fn blit(&self, area: Rect, buf: &mut Buffer) {
        for cy in 0..area.height as usize {
            for cx in 0..area.width as usize {
                if cx >= self.w || cy * 2 + 1 >= self.h {
                    continue;
                }
                let (t, b) = (self.get(cx, cy * 2), self.get(cx, cy * 2 + 1));
                let cell = &mut buf[(area.x + cx as u16, area.y + cy as u16)];
                cell.set_symbol("▀").set_fg(rgb(t)).set_bg(rgb(b));
            }
        }
    }
}

pub fn rgb(c: Rgb) -> Color {
    let q = |v: f32| (v.clamp(0.0, 255.0) + 0.5) as u8;
    Color::Rgb(q(c[0]), q(c[1]), q(c[2]))
}

/// From his 640x480 world to canvas pixels.
#[derive(Clone, Copy, Debug)]
pub struct View {
    pub ox: f32,
    pub oy: f32,
    pub sx: f32,
    pub sy: f32,
    /// world point at the canvas origin
    pub wx0: f32,
    pub wy0: f32,
}

/// The part of his screen the match uses: everything left to right, and
/// from the umpire's head down to just below the court.
pub const VIEW_TOP: f32 = 4.0;
pub const VIEW_BOTTOM: f32 = 420.0;

impl View {
    /// Fits the match into a canvas of w x h pixels; the picture may be
    /// stretched sideways a little to use a wide terminal.
    pub fn fit(w: usize, h: usize) -> View {
        let (ww, wh) = (game::WIDTH, VIEW_BOTTOM - VIEW_TOP);
        let sy = (h as f32 / wh).min(w as f32 / ww / 0.8);
        let sx = (w as f32 / ww).min(sy * 1.3);
        let ox = (w as f32 - ww * sx) / 2.0;
        let oy = (h as f32 - wh * sy) / 2.0;
        View { ox, oy, sx, sy, wx0: 0.0, wy0: VIEW_TOP }
    }

    pub fn x(&self, wx: f32) -> f32 {
        self.ox + (wx - self.wx0) * self.sx
    }
    pub fn y(&self, wy: f32) -> f32 {
        self.oy + (wy - self.wy0) * self.sy
    }
}

/// Fills a world rectangle, with partly covered pixels blended.
pub fn rect(c: &mut Canvas, v: &View, x0: f32, y0: f32, x1: f32, y1: f32, col: Rgb, a: f32) {
    let (px0, px1, py0, py1) = (v.x(x0), v.x(x1), v.y(y0), v.y(y1));
    rect_px(c, px0, py0, px1, py1, col, a);
}

pub fn rect_px(c: &mut Canvas, px0: f32, py0: f32, px1: f32, py1: f32, col: Rgb, a: f32) {
    for py in py0.floor() as i32..py1.ceil() as i32 {
        let cy = (py1.min(py as f32 + 1.0) - py0.max(py as f32)).clamp(0.0, 1.0);
        for px in px0.floor() as i32..px1.ceil() as i32 {
            let cx = (px1.min(px as f32 + 1.0) - px0.max(px as f32)).clamp(0.0, 1.0);
            c.blend(px, py, col, a * cx * cy);
        }
    }
}

/// An ellipse in pixel space, 4x4 samples a pixel.
pub fn ellipse_px(c: &mut Canvas, cx: f32, cy: f32, rx: f32, ry: f32, col: Rgb, a: f32) {
    let (rx, ry) = (rx.max(0.3), ry.max(0.3));
    for py in (cy - ry).floor() as i32..=(cy + ry).ceil() as i32 {
        for px in (cx - rx).floor() as i32..=(cx + rx).ceil() as i32 {
            let mut n = 0;
            for sy in 0..4 {
                for sx in 0..4 {
                    let x = px as f32 + (sx as f32 + 0.5) / 4.0 - cx;
                    let y = py as f32 + (sy as f32 + 0.5) / 4.0 - cy;
                    if (x / rx).powi(2) + (y / ry).powi(2) <= 1.0 {
                        n += 1;
                    }
                }
            }
            c.blend(px, py, col, a * n as f32 / 16.0);
        }
    }
}

/// A ring (an ellipse's outline) of the given thickness in pixels.
pub fn ring_px(c: &mut Canvas, cx: f32, cy: f32, rx: f32, ry: f32, t: f32, col: Rgb, a: f32) {
    for py in (cy - ry - 1.0).floor() as i32..=(cy + ry + 1.0).ceil() as i32 {
        for px in (cx - rx - 1.0).floor() as i32..=(cx + rx + 1.0).ceil() as i32 {
            let mut n = 0;
            for sy in 0..4 {
                for sx in 0..4 {
                    let x = px as f32 + (sx as f32 + 0.5) / 4.0 - cx;
                    let y = py as f32 + (sy as f32 + 0.5) / 4.0 - cy;
                    let d = ((x / rx).powi(2) + (y / ry).powi(2)).sqrt();
                    // distance from the outline, roughly, in pixels
                    let dist = (d - 1.0).abs() * rx.min(ry);
                    if dist <= t / 2.0 {
                        n += 1;
                    }
                }
            }
            c.blend(px, py, col, a * n as f32 / 16.0);
        }
    }
}

/// A line of the given width in pixels, from (x0,y0) to (x1,y1) in pixels.
pub fn line_px(c: &mut Canvas, x0: f32, y0: f32, x1: f32, y1: f32, width: f32, col: Rgb, a: f32) {
    let r = width / 2.0;
    let (minx, maxx) = (x0.min(x1) - r - 1.0, x0.max(x1) + r + 1.0);
    let (miny, maxy) = (y0.min(y1) - r - 1.0, y0.max(y1) + r + 1.0);
    let (dx, dy) = (x1 - x0, y1 - y0);
    let len2 = (dx * dx + dy * dy).max(1e-6);
    for py in miny.floor() as i32..=maxy.ceil() as i32 {
        for px in minx.floor() as i32..=maxx.ceil() as i32 {
            let mut n = 0;
            for sy in 0..4 {
                for sx in 0..4 {
                    let x = px as f32 + (sx as f32 + 0.5) / 4.0;
                    let y = py as f32 + (sy as f32 + 0.5) / 4.0;
                    let t = (((x - x0) * dx + (y - y0) * dy) / len2).clamp(0.0, 1.0);
                    let (qx, qy) = (x0 + t * dx - x, y0 + t * dy - y);
                    if qx * qx + qy * qy <= r * r {
                        n += 1;
                    }
                }
            }
            c.blend(px, py, col, a * n as f32 / 16.0);
        }
    }
}

/// A small hash for textures and weather that must not flicker.
pub fn hash(x: i32, y: i32, seed: u32) -> f32 {
    let mut h = (x as u32).wrapping_mul(374_761_393) ^ (y as u32).wrapping_mul(668_265_263) ^ seed.wrapping_mul(2_246_822_519);
    h = (h ^ (h >> 13)).wrapping_mul(1_274_126_177);
    h ^= h >> 16;
    (h & 0xffff) as f32 / 65535.0
}

/// Smooth value noise, for drifting fog.
fn noise(x: f32, y: f32, seed: u32) -> f32 {
    let (xi, yi) = (x.floor() as i32, y.floor() as i32);
    let (fx, fy) = (x - xi as f32, y - yi as f32);
    let s = |t: f32| t * t * (3.0 - 2.0 * t);
    let (u, w) = (s(fx), s(fy));
    let a = hash(xi, yi, seed);
    let b = hash(xi + 1, yi, seed);
    let c = hash(xi, yi + 1, seed);
    let d = hash(xi + 1, yi + 1, seed);
    a + (b - a) * u + (c - a) * w + (a - b - c + d) * u * w
}

pub const P1_COLOUR: Rgb = [70.0, 110.0, 235.0];
pub const P2_COLOUR: Rgb = [230.0, 60.0, 60.0];
const WHITE: Rgb = [250.0, 250.0, 250.0];
const SKIN: Rgb = [235.0, 190.0, 150.0];

/// His court textures (ctt_*.png), by their average colour, with grain.
fn surface_colour(s: Surface) -> Rgb {
    match s {
        Surface::Clay => [214.0, 74.0, 42.0],
        Surface::Grass => [86.0, 168.0, 22.0],
        Surface::Hard => [85.0, 107.0, 161.0],
    }
}

pub struct Scene<'a> {
    pub m: &'a Match,
    pub loc: &'a Location,
    pub night: bool,
    /// milliseconds since the match started (his SDL_GetTicks for the
    /// umpire's animation and the fog)
    pub time_ms: u64,
    /// frame counter, for the rain
    pub frame: u64,
}

/// The whole match picture into a canvas.
pub fn draw_scene(c: &mut Canvas, s: &Scene) {
    let v = View::fit(c.w, c.h);
    draw_court(c, &v, s.loc);
    if s.loc.has_referee {
        draw_referee(c, &v, s.m.referee, s.time_ms);
    }
    for p in 1..=2u8 {
        let pl = s.m.p(p);
        draw_player(c, &v, p, pl.x, pl.y, !pl.use_power);
    }
    draw_ball(c, &v, s.m);
    weather(c, &v, s);
}

pub fn draw_court(c: &mut Canvas, v: &View, loc: &Location) {
    // his fill_image(court_type, 120, 120, 400, 250)
    let base = surface_colour(loc.court);
    let (x0, x1, y0, y1) = (v.x(120.0), v.x(520.0), v.y(120.0), v.y(370.0));
    for py in y0.floor() as i32..y1.ceil() as i32 {
        for px in x0.floor() as i32..x1.ceil() as i32 {
            let grain = hash(px, py, 7) * 0.12 - 0.06;
            let mut k = 1.0 + grain;
            if loc.court == Surface::Grass {
                // mowing stripes across the court
                let wx = (px as f32 - v.ox) / v.sx;
                if ((wx - 120.0) / 40.0).floor() as i32 % 2 == 0 {
                    k += 0.07;
                }
            }
            let col = [base[0] * k, base[1] * k, base[2] * k];
            let cx = (x1.min(px as f32 + 1.0) - x0.max(px as f32)).clamp(0.0, 1.0);
            let cy = (y1.min(py as f32 + 1.0) - y0.max(py as f32)).clamp(0.0, 1.0);
            c.blend(px, py, col, cx * cy);
        }
    }
    // the lines of his court.png
    let line = |c: &mut Canvas, x0: f32, y0: f32, x1: f32, y1: f32| rect(c, v, x0, y0, x1, y1, WHITE, 0.92);
    line(c, 119.0, 115.0, 526.0, 121.0); // doubles side line, far
    line(c, 119.0, 365.0, 526.0, 371.0); // doubles side line, near
    line(c, 119.0, 154.0, 526.0, 160.0); // singles side line, far
    line(c, 119.0, 326.0, 526.0, 332.0); // singles side line, near
    line(c, 119.0, 115.0, 125.0, 371.0); // baseline, left
    line(c, 520.0, 115.0, 526.0, 371.0); // baseline, right
    line(c, 215.0, 154.0, 221.0, 332.0); // service line, left
    line(c, 424.0, 154.0, 430.0, 332.0); // service line, right
    line(c, 215.0, 239.0, 430.0, 246.0); // centre service line
    line(c, 119.0, 240.0, 129.0, 245.0); // centre marks
    line(c, 516.0, 240.0, 526.0, 245.0);
    // the net, its shadow and posts
    rect(c, v, 324.0, 109.0, 331.0, 380.0, [0.0, 0.0, 0.0], 0.25);
    rect(c, v, 319.0, 107.0, 326.0, 380.0, [150.0, 150.0, 158.0], 0.95);
    rect(c, v, 321.0, 107.0, 323.5, 380.0, [235.0, 235.0, 240.0], 0.9);
    ellipse_px(c, v.x(322.5), v.y(107.0), 3.5 * v.sx, 3.5 * v.sy, [60.0, 60.0, 66.0], 1.0);
    ellipse_px(c, v.x(322.5), v.y(380.0), 3.5 * v.sx, 3.5 * v.sy, [60.0, 60.0, 66.0], 1.0);
}

/// The umpire's chair and the umpire (his referee.png at 250,10):
/// watching the rally, arms up for an out, pointing at who scored.
pub fn draw_referee(c: &mut Canvas, v: &View, r: Referee, time_ms: u64) {
    let period = match r {
        Referee::Normal => 1000,
        Referee::Out => 200,
        Referee::Player1 | Referee::Player2 => 400,
    };
    // his frames 0, 1, 0, 2
    let frame = [0, 1, 0, 2][((time_ms / period) % 4) as usize];
    let (x, y) = (v.x(291.0), v.y(0.0));
    let s = v.sy;
    let px = |dx: f32| x + dx * v.sx;
    let py = |dy: f32| y + dy * s;
    let grey = [200.0, 200.0, 205.0];
    // legs of the chair, the seat
    line_px(c, px(-12.0), py(70.0), px(-14.0), py(112.0), (3.0 * v.sx).max(0.6), grey, 0.9);
    line_px(c, px(12.0), py(70.0), px(14.0), py(112.0), (3.0 * v.sx).max(0.6), grey, 0.9);
    line_px(c, px(-13.0), py(92.0), px(13.0), py(92.0), (2.0 * v.sx).max(0.5), grey, 0.7);
    rect_px(c, px(-16.0), py(56.0), px(16.0), py(74.0), [215.0, 40.0, 40.0], 1.0);
    // the umpire: blazer, head turning, arms
    let blazer = [30.0, 45.0, 90.0];
    rect_px(c, px(-9.0), py(34.0), px(9.0), py(60.0), blazer, 1.0);
    let turn = match frame {
        1 => -2.5,
        2 => 2.5,
        _ => 0.0,
    };
    ellipse_px(c, px(turn), py(26.0), 7.0 * v.sx, 8.0 * s, SKIN, 1.0);
    ellipse_px(c, px(turn), py(21.0), 7.0 * v.sx, 4.0 * s, [70.0, 50.0, 35.0], 1.0);
    let arm = (3.0 * v.sx).max(0.6);
    let (lx, ly, rx, ry) = match r {
        Referee::Normal => (-11.0, 58.0, 11.0, 58.0),
        Referee::Out => {
            let lift = if frame == 1 { 6.0 } else { 0.0 };
            (-18.0, 12.0 + lift, 18.0, 12.0 + lift)
        }
        Referee::Player1 => (-34.0, 30.0 + frame as f32 * 2.0, 11.0, 58.0),
        Referee::Player2 => (-11.0, 58.0, 34.0, 30.0 + frame as f32 * 2.0),
    };
    line_px(c, px(-9.0), py(37.0), px(lx), py(ly), arm, blazer, 1.0);
    line_px(c, px(9.0), py(37.0), px(rx), py(ry), arm, blazer, 1.0);
    ellipse_px(c, px(lx), py(ly), arm * 0.7, arm * 0.7, SKIN, 1.0);
    ellipse_px(c, px(rx), py(ry), arm * 0.7, arm * 0.7, SKIN, 1.0);
}

/// A player: the figure behind the baseline and the racket (his
/// player-racket.png, blue for player 1, red for player 2), tilted back
/// while the swing winds up.
pub fn draw_player(c: &mut Canvas, v: &View, p: u8, x: f32, y: f32, winding: bool) {
    let col = if p == 1 { P1_COLOUR } else { P2_COLOUR };
    let side = if p == 1 { -1.0 } else { 1.0 }; // the figure stands outside the racket
    let w = |dx: f32| v.x(x + dx * side);
    let h = |dy: f32| v.y(y + dy);
    let t = (2.5 * v.sx).max(0.55);
    // shadow at the feet
    ellipse_px(c, w(14.0), h(36.0), 13.0 * v.sx, 4.0 * v.sy, [0.0, 0.0, 0.0], 0.3);
    // legs, shorts, shirt, head
    line_px(c, w(11.0), h(14.0), w(9.0), h(34.0), t * 1.2, SKIN, 1.0);
    line_px(c, w(18.0), h(14.0), w(20.0), h(34.0), t * 1.2, SKIN, 1.0);
    rect_px(c, w(8.0).min(w(21.0)), h(8.0), w(8.0).max(w(21.0)), h(16.0), WHITE, 1.0);
    rect_px(c, w(7.0).min(w(22.0)), h(-12.0), w(7.0).max(w(22.0)), h(9.0), col, 1.0);
    ellipse_px(c, w(14.5), h(-19.0), 6.5 * v.sx, 7.0 * v.sy, SKIN, 1.0);
    ellipse_px(c, w(14.5), h(-23.0), 6.5 * v.sx, 3.5 * v.sy, [60.0, 40.0, 25.0], 1.0);
    // the racket: head centred on the player's point, handle to the hand
    let tilt: f32 = if winding { 0.55 } else { 0.12 };
    let (sin, cos) = (tilt.sin() * side, tilt.cos());
    let (hx, hy) = (x + 2.0 * -side, y - 4.0);
    let hand = (hx + sin * 34.0, hy + cos * 34.0);
    line_px(c, w(5.0), h(-8.0), v.x(hand.0), v.y(hand.1), t, SKIN, 1.0);
    line_px(c, v.x(hx + sin * 18.0), v.y(hy + cos * 18.0), v.x(hand.0), v.y(hand.1), t, [40.0, 40.0, 40.0], 1.0);
    let (rx, ry) = (12.0 * v.sx, 18.0 * v.sy);
    let (cx, cy) = (v.x(hx - sin * 2.0), v.y(hy));
    ellipse_px(c, cx, cy, rx, ry, [235.0, 235.0, 235.0], 0.45);
    ring_px(c, cx, cy, rx, ry, (3.0 * v.sx).max(0.6), col, 1.0);
}

pub fn draw_ball(c: &mut Canvas, v: &View, m: &Match) {
    let b = &m.ball;
    // his shadow: smaller as the ball goes up
    let zs = ((30.0 - b.z) / 10.0).clamp(0.5, 1.0);
    let r = (9.0 * zs * v.sx).max(0.8);
    ellipse_px(c, v.x(b.x), v.y(b.y + 3.0), r, (r * 0.75).max(0.6), [28.0, 28.0, 28.0], 0.5);
    // the ball: a little bigger the higher it flies
    let zb = 1.0 + (b.z - 10.0).max(0.0) / 100.0;
    let r = (9.0 * zb * v.sx).max(0.9);
    let (bx, by) = (v.x(b.x), v.y(b.y - b.z));
    ellipse_px(c, bx, by, r, r, [215.0, 200.0, 40.0], 1.0);
    ellipse_px(c, bx - r * 0.25, by - r * 0.3, r * 0.45, r * 0.45, [250.0, 245.0, 150.0], 0.8);
}

/// Rain, fog and night over the match (the end of his render()).
fn weather(c: &mut Canvas, v: &View, s: &Scene) {
    let t = s.time_ms as f32;
    // rain: his streaks of 10 by 30 pixels, fading from blue to pale blue
    for i in 0..s.loc.rain {
        let x = hash(i as i32, s.frame as i32, 11) * game::WIDTH;
        let y = hash(i as i32, s.frame as i32, 12) * game::HEIGHT;
        line_px(c, v.x(x), v.y(y), v.x(x + 10.0), v.y(y + 30.0), 0.5, [100.0, 200.0, 255.0], 0.55);
    }
    // fog: drifting banks, one more for every level (his fog.png layers)
    if s.loc.fog > 0 {
        let layers = s.loc.fog.min(4);
        let speeds = [(t / 200.0, 0.0, 1u32), (-t / 180.0, 80.0, 2), (-t / 100.0, 20.0, 3), (-t / 150.0, 0.0, 4)];
        c.map(|x, y, p| {
            let wx = (x as f32 - v.ox) / v.sx;
            let wy = (y as f32 - v.oy) / v.sy + v.wy0;
            let mut out = p;
            for &(off, dy, seed) in speeds.iter().take(layers as usize) {
                let n = noise((wx + off) / 70.0, (wy + dy) / 50.0, seed);
                let a = ((n - 0.45) * 0.7).clamp(0.0, 0.2);
                for (o, w) in out.iter_mut().zip([235.0, 238.0, 240.0]) {
                    *o += (w - *o) * a;
                }
            }
            out
        });
    }
    // night: dark but for two floodlit ovals (his night.png)
    if s.night {
        c.map(|x, y, p| {
            let wx = (x as f32 + 0.5 - v.ox) / v.sx;
            let wy = (y as f32 + 0.5 - v.oy) / v.sy + v.wy0;
            let lit = |cx: f32, cy: f32, rx: f32, ry: f32| {
                let d = (((wx - cx) / rx).powi(2) + ((wy - cy) / ry).powi(2)).sqrt();
                (1.0 - (d - 0.85) / 0.6).clamp(0.0, 1.0)
            };
            let l = lit(187.0, 257.0, 95.0, 135.0).max(lit(450.0, 247.0, 93.0, 140.0));
            let dark = 0.82 * (1.0 - l);
            [p[0] * (1.0 - dark), p[1] * (1.0 - dark), p[2] * (1.0 - dark) + 6.0 * dark]
        });
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::locations::LOCATIONS;

    #[test]
    fn the_ball_is_drawn_where_it_flies() {
        let mut m = Match::new(3);
        m.ball.x = 400.0;
        m.ball.y = 240.0;
        m.ball.z = 0.0;
        let (w, h) = (100, 54);
        let mut c = Canvas::new(w, h, [0.0; 3]);
        draw_scene(&mut c, &Scene { m: &m, loc: &LOCATIONS[3], night: false, time_ms: 0, frame: 0 });
        let v = View::fit(w, h);
        let p = c.get(v.x(400.0) as usize, v.y(240.0) as usize);
        assert!(p[0] > 150.0 && p[1] > 150.0 && p[2] < p[0] - 60.0, "ball yellow, got {p:?}");
    }

    #[test]
    fn view_fits_100x30_and_80x24() {
        for (w, h) in [(100usize, 54usize), (80, 42)] {
            let v = View::fit(w, h);
            assert!(v.x(0.0) >= -0.01 && v.x(640.0) <= w as f32 + 0.01);
            assert!(v.y(VIEW_TOP) >= -0.01 && v.y(VIEW_BOTTOM) <= h as f32 + 0.01);
        }
    }
}
