//! The screens: the match with the score above it and the power bars
//! below, the menu over a match the computer plays with itself, the world
//! map of the tour, the controller setup.

use crate::app::{App, Ctl, Play, Screen};
use crate::game::{self, PLAYER_POWER_MAX};
use crate::locations::{self, LOCATIONS, Surface};
use crate::render::{self, Canvas, P1_COLOUR, P2_COLOUR, Rgb, Scene, View};
use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::style::{Color, Modifier, Style};

pub const MIN_COLS: u16 = 60;
pub const MIN_ROWS: u16 = 16;

const GOLD: Color = Color::Rgb(255, 214, 90);
const DIM: Color = Color::Rgb(150, 150, 150);
const PANEL: Color = Color::Rgb(18, 22, 30);

static WORLD: &[u8] = include_bytes!("../assets/worldmap.bits");
const WORLD_W: usize = 250;
const WORLD_H: usize = 110;

fn land(x: usize, y: usize) -> bool {
    x < WORLD_W && y < WORLD_H && WORLD[y * 32 + x / 8] >> (7 - x % 8) & 1 == 1
}

/// Text into the buffer, clipped to the area.
pub fn put(buf: &mut Buffer, area: Rect, x: i32, y: i32, text: &str, style: Style) {
    if y < area.y as i32 || y >= (area.y + area.height) as i32 {
        return;
    }
    let mut cx = x;
    for ch in text.chars() {
        if cx >= area.x as i32 && cx < (area.x + area.width) as i32 {
            let cell = &mut buf[(cx as u16, y as u16)];
            cell.set_char(ch);
            cell.set_style(style);
        }
        cx += 1;
    }
}

fn centre(buf: &mut Buffer, area: Rect, y: i32, text: &str, style: Style) {
    let x = area.x as i32 + (area.width as i32 - text.chars().count() as i32) / 2;
    put(buf, area, x, y, text, style);
}

fn colour(c: Rgb) -> Color {
    render::rgb(c)
}

pub fn draw(app: &App, area: Rect, buf: &mut Buffer) {
    buf.set_style(area, Style::new().bg(Color::Black).fg(Color::White));
    if area.width < MIN_COLS || area.height < MIN_ROWS {
        let y = area.y as i32 + area.height as i32 / 2;
        centre(buf, area, y - 1, "Tennix", Style::new().fg(GOLD).add_modifier(Modifier::BOLD));
        centre(buf, area, y, &format!("needs a terminal of at least {MIN_COLS}x{MIN_ROWS}"), Style::new().fg(Color::White));
        centre(buf, area, y + 1, &format!("(this one is {}x{})", area.width, area.height), Style::new().fg(DIM));
        return;
    }
    match app.screen {
        Screen::Play => match &app.play {
            Some(p) => draw_play(app, p, area, buf),
            None => {}
        },
        Screen::Menu => draw_menu(app, area, buf),
        Screen::Map => draw_map(app, area, buf),
        Screen::Setup => draw_setup(app, area, buf),
    }
}

fn scene_canvas(p: &Play, w: usize, h: usize) -> Canvas {
    let mut c = Canvas::new(w, h, [0.0; 3]);
    render::draw_scene(&mut c, &Scene { m: &p.m, loc: p.loc(), night: p.night, time_ms: p.time_ms, frame: p.frame });
    c
}

fn power_bar(power: f32, cells: usize) -> String {
    const EIGHTHS: [char; 8] = [' ', '▏', '▎', '▍', '▌', '▋', '▊', '▉'];
    let fill = (power / PLAYER_POWER_MAX).clamp(0.0, 1.0) * cells as f32;
    let full = fill.floor() as usize;
    let mut s: String = "█".repeat(full);
    if full < cells {
        s.push(EIGHTHS[((fill - full as f32) * 8.0) as usize]);
        s.push_str(&" ".repeat(cells - full - 1));
    }
    s
}

fn draw_play(app: &App, p: &Play, area: Rect, buf: &mut Buffer) {
    let m = &p.m;
    let court = Rect::new(area.x, area.y + 1, area.width, area.height - 2);
    let mut c = scene_canvas(p, court.width as usize, court.height as usize * 2);
    if m.winner != 0 {
        c.map(|_, _, px| [px[0] * 0.25, px[1] * 0.25, px[2] * 0.25]);
        // his winner screen: the winner's racket in the middle
        let v = View::fit(c.w, c.h);
        render::draw_player(&mut c, &v, m.winner, 320.0, 215.0, false);
    }
    c.blit(court, buf);

    // the score above the court: the game left, the sets right (his
    // text_game and text_sets), the venue between
    let top = area.y as i32;
    let bold = Style::new().fg(Color::White).add_modifier(Modifier::BOLD);
    put(buf, area, area.x as i32 + 1, top, &m.format_game(), bold);
    let sets = m.format_sets();
    put(buf, area, (area.x + area.width) as i32 - 1 - sets.chars().count() as i32, top, &sets, bold);
    let loc = p.loc();
    let venue = format!("{}, {}{}", loc.name, loc.city, if p.night { " (night)" } else { "" });
    if venue.chars().count() as i32 + 30 < area.width as i32 {
        centre(buf, area, top, &venue, Style::new().fg(DIM));
    }

    // the power bars and the status line below
    let bottom = (area.y + area.height - 1) as i32;
    let cells = ((area.width as usize).saturating_sub(34) / 2).clamp(6, 12);
    let bar_bg = Color::Rgb(50, 50, 50);
    let names = ["1", "2"];
    put(buf, area, area.x as i32 + 1, bottom, names[0], Style::new().fg(colour(P1_COLOUR)).add_modifier(Modifier::BOLD));
    put(buf, area, area.x as i32 + 3, bottom, &power_bar(m.players[0].power, cells), Style::new().fg(Color::Rgb(200, 200, 200)).bg(bar_bg));
    let rx = (area.x + area.width) as i32 - 3 - cells as i32;
    put(buf, area, rx, bottom, &power_bar(m.players[1].power, cells), Style::new().fg(Color::Rgb(200, 200, 200)).bg(bar_bg));
    put(buf, area, (area.x + area.width) as i32 - 2, bottom, names[1], Style::new().fg(colour(P2_COLOUR)).add_modifier(Modifier::BOLD));
    let status = m.format_status();
    centre(buf, area, bottom, &status, Style::new().fg(GOLD));

    let mid = court.y as i32 + court.height as i32 / 2;
    if m.winner != 0 {
        let who = if p.ctl[m.winner as usize - 1] == Ctl::Computer { game::AI_NAME.to_string() } else { format!("player {}", m.winner) };
        centre(buf, area, mid + 4, &format!("{who} wins the match"), Style::new().fg(GOLD).bg(Color::Black).add_modifier(Modifier::BOLD));
        centre(buf, area, mid + 5, &m.format_sets(), Style::new().fg(Color::White).bg(Color::Black));
        if !app.demo {
            centre(buf, area, mid + 7, "Enter for the menu", Style::new().fg(DIM).bg(Color::Black));
        }
    } else if p.paused {
        let mut lines = vec!["paused".to_string(), String::new()];
        for (i, ctl) in p.ctl.iter().enumerate() {
            match ctl {
                Ctl::Keys(l) => lines.push(format!("player {}: {}", i + 1, l.keys_help())),
                Ctl::Pad(_) => lines.push(format!("player {}: stick or cross to move, A hit, B/Y topspin, X smash", i + 1)),
                Ctl::Computer => lines.push(format!("player {}: {}", i + 1, game::AI_NAME)),
            }
        }
        lines.push(String::new());
        lines.push("hold the hit key to fill the bar, let go to swing".into());
        if !app.keys.releases {
            lines.push("(this terminal hides key releases: tap to wind up, tap again to swing)".into());
        }
        lines.push(String::new());
        lines.push("P play on   M sound   Esc menu".into());
        let w = lines.iter().map(|l| l.chars().count()).max().unwrap_or(0) as i32 + 4;
        let h = lines.len() as i32 + 2;
        let x0 = area.x as i32 + (area.width as i32 - w) / 2;
        let y0 = mid - h / 2;
        for y in 0..h {
            put(buf, area, x0, y0 + y, &" ".repeat(w as usize), Style::new().bg(PANEL));
        }
        for (i, l) in lines.iter().enumerate() {
            let st = if i == 0 { Style::new().fg(GOLD).bg(PANEL).add_modifier(Modifier::BOLD) } else { Style::new().fg(Color::White).bg(PANEL) };
            centre(buf, Rect::new(x0.max(0) as u16, area.y, (w as u16).min(area.width), area.height), y0 + 1 + i as i32, l, st);
        }
    }
}

/// TENNIX in a 5x7 pixel font.
fn title(c: &mut Canvas, x0: i32, y0: i32, scale: i32, col: Rgb) {
    const GLYPHS: [[u8; 7]; 4] = [
        [0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100], // T
        [0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111], // E
        [0b10001, 0b11001, 0b10101, 0b10101, 0b10011, 0b10001, 0b10001], // N
        [0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001], // X
    ];
    const I: [u8; 7] = [0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110];
    let word = [GLYPHS[0], GLYPHS[1], GLYPHS[2], GLYPHS[2], I, GLYPHS[3]];
    for (i, g) in word.iter().enumerate() {
        for (row, bits) in g.iter().enumerate() {
            for col_i in 0..5 {
                if bits >> (4 - col_i) & 1 == 1 {
                    for dy in 0..scale {
                        for dx in 0..scale {
                            let x = x0 + (i as i32 * 6 + col_i) * scale + dx;
                            let y = y0 + row as i32 * scale + dy;
                            let shade = 1.0 - 0.25 * (row as f32 / 6.0);
                            c.blend(x, y, [col[0] * shade, col[1] * shade, col[2] * shade], 1.0);
                            c.blend(x + 1, y + 1, [0.0, 0.0, 0.0], 0.0);
                        }
                    }
                }
            }
        }
    }
}

fn draw_menu(app: &App, area: Rect, buf: &mut Buffer) {
    let mut c = scene_canvas(&app.attract, area.width as usize, area.height as usize * 2);
    c.map(|_, _, p| [p[0] * 0.4, p[1] * 0.4, p[2] * 0.4]);
    // his sidebar on the right, with the logo
    let panel_w = 40.min(area.width as usize / 2 + 4);
    let px0 = c.w - panel_w;
    let (cw, ch) = (c.w as f32, c.h as f32);
    render::rect_px(&mut c, px0 as f32, 0.0, cw, ch, [12.0, 16.0, 24.0], 0.92);
    render::rect_px(&mut c, px0 as f32, 0.0, px0 as f32 + 1.0, ch, [255.0, 214.0, 90.0], 0.5);
    let scale = if panel_w >= 40 { 2 } else { 1 };
    let tw = 35 * scale;
    title(&mut c, px0 as i32 + (panel_w as i32 - tw as i32) / 2 + 1, 3, scale as i32, [255.0, 214.0, 90.0]);
    c.blit(area, buf);

    let pa = Rect::new(area.x + px0 as u16, area.y, panel_w as u16, area.height);
    let ty = area.y as i32 + (3 + 7 * scale as i32 + 1) / 2 + 1;
    centre(buf, pa, ty, "Classic Championship Tour", Style::new().fg(Color::White).bg(PANEL));
    let items = app.menu_items();
    let y0 = ty + 3;
    for (i, it) in items.iter().enumerate() {
        let sel = i == app.sel;
        let text = if sel { format!("▶ {it} ◀") } else { it.clone() };
        let st = if sel { Style::new().fg(Color::Black).bg(GOLD).add_modifier(Modifier::BOLD) } else { Style::new().fg(Color::White).bg(PANEL) };
        centre(buf, pa, y0 + i as i32 * 2, &text, st);
    }
    let hint = "↑↓ choose  Enter go  ←→ change";
    centre(buf, pa, (area.y + area.height) as i32 - 2, hint, Style::new().fg(DIM).bg(PANEL));
    if app.play.is_some() {
        put(buf, area, area.x as i32 + 1, area.y as i32, "match paused", Style::new().fg(GOLD));
    }
    put(buf, area, area.x as i32 + 1, (area.y + area.height) as i32 - 1, "Tennix by Thomas Perl, thp.io", Style::new().fg(DIM));
}

fn draw_map(app: &App, area: Rect, buf: &mut Buffer) {
    let (w, h) = (area.width as usize, area.height as usize * 2);
    let mut c = Canvas::new(w, h, [0.0; 3]);
    // the map in the room above the card
    let card_rows = 7usize;
    let map_h = h.saturating_sub(4 + card_rows * 2);
    let scale = (w as f32 / WORLD_W as f32).min(map_h as f32 / WORLD_H as f32);
    let (mw, mh) = (WORLD_W as f32 * scale, WORLD_H as f32 * scale);
    let (mx0, my0) = ((w as f32 - mw) / 2.0, 2.0 + (map_h as f32 - mh) / 2.0);
    let (ns, ne) = locations::day_night(app.clock, 500);
    for py in 0..mh as usize {
        for px in 0..mw as usize {
            let (sx, sy) = ((px as f32 / scale) as usize, (py as f32 / scale) as usize);
            let map_x = (sx * 2) as u32;
            let night = locations::is_night(map_x, (ns, ne));
            let col = if land(sx, sy) { [185.0, 185.0, 185.0] } else { [16.0, 28.0, 52.0] };
            let k = if night { 0.42 } else { 1.0 };
            c.blend(mx0 as i32 + px as i32, my0 as i32 + py as i32, [col[0] * k, col[1] * k, col[2] * k], 1.0);
        }
    }
    let blink = (app.menu_time_ms / 300) % 2 == 0;
    for (i, l) in LOCATIONS.iter().enumerate() {
        let x = mx0 + l.worldmap_x as f32 / 2.0 * scale;
        let y = my0 + l.worldmap_y as f32 / 2.0 * scale;
        if i == app.venue_sel {
            if blink {
                render::rect_px(&mut c, x - 2.0, y - 2.0, x + 2.0, y + 2.0, [255.0, 0.0, 255.0], 1.0);
            }
            render::rect_px(&mut c, x - 1.0, y - 1.0, x + 1.0, y + 1.0, [255.0, 255.0, 0.0], 1.0);
        } else {
            render::rect_px(&mut c, x - 1.0, y - 1.0, x + 1.0, y + 1.0, [255.0, 0.0, 0.0], 1.0);
        }
    }
    c.blit(area, buf);
    centre(buf, area, area.y as i32, "Pick a location", Style::new().fg(GOLD).add_modifier(Modifier::BOLD));

    // the card for the venue
    let l = &LOCATIONS[app.venue_sel];
    let night = app.night_override.unwrap_or_else(|| locations::is_night(l.worldmap_x, (ns, ne)));
    let surface = match l.court {
        Surface::Clay => "clay court",
        Surface::Grass => "grass court",
        Surface::Hard => "hard court",
    };
    let mut weather = vec![if night { "under the floodlights" } else { "in daylight" }];
    if l.rain > 0 {
        weather.push(if l.rain > 20 { "heavy rain" } else { "light rain" });
    }
    if l.fog > 0 {
        weather.push(if l.fog > 1 { "thick fog" } else { "mist" });
    }
    let crowd = if l.max_visitors > 100 { format!("{} seats", group(l.max_visitors)) } else { format!("{} seats, no umpire", l.max_visitors) };
    let lines = [
        (format!("{}  ({}/{})", l.name, app.venue_sel + 1, LOCATIONS.len()), Style::new().fg(GOLD).add_modifier(Modifier::BOLD)),
        (format!("{}, {}", l.area, l.city), Style::new().fg(Color::White)),
        (format!("{}: {}, {}", surface, l.court_type_name, crowd), Style::new().fg(Color::White)),
        (format!("Today: {}", weather.join(", ")), Style::new().fg(Color::White)),
        (String::new(), Style::new()),
        ("←→ another location   Enter choose players   Esc back".into(), Style::new().fg(DIM)),
    ];
    let y0 = (area.y + area.height) as i32 - lines.len() as i32 - 1;
    for (i, (t, st)) in lines.iter().enumerate() {
        centre(buf, area, y0 + i as i32, t, *st);
    }
}

fn group(n: u32) -> String {
    let s = n.to_string();
    let mut out = String::new();
    for (i, ch) in s.chars().enumerate() {
        if i > 0 && (s.len() - i) % 3 == 0 {
            out.push(',');
        }
        out.push(ch);
    }
    out
}

fn draw_setup(app: &App, area: Rect, buf: &mut Buffer) {
    let (w, h) = (area.width as usize, area.height as usize * 2);
    let mut c = Canvas::new(w, h, [0.0; 3]);
    // the two players, big, on their own patch of the venue's court
    let l = &LOCATIONS[app.venue_sel];
    let box_w = (w / 2).saturating_sub(4).min(40);
    let box_h = (h as f32 * 0.5) as usize;
    let by0 = 8usize;
    let surface = match l.court {
        Surface::Clay => [214.0, 74.0, 42.0],
        Surface::Grass => [86.0, 168.0, 22.0],
        Surface::Hard => [85.0, 107.0, 161.0],
    };
    for side in 0..2 {
        let bx0 = if side == 0 { (w / 2 - box_w) / 2 } else { w / 2 + (w / 2 - box_w) / 2 };
        render::rect_px(&mut c, bx0 as f32, by0 as f32, (bx0 + box_w) as f32, (by0 + box_h) as f32, surface, 1.0);
        let s = box_h as f32 / 110.0;
        let v = View { ox: bx0 as f32 + box_w as f32 / 2.0, oy: by0 as f32 + box_h as f32 * 0.45, sx: s, sy: s, wx0: 0.0, wy0: 0.0 };
        let winding = (app.menu_time_ms / 700) % 2 == 1;
        render::draw_player(&mut c, &v, side as u8 + 1, 0.0, 0.0, winding);
    }
    c.blit(area, buf);
    centre(buf, area, area.y as i32, "Controller setup", Style::new().fg(GOLD).add_modifier(Modifier::BOLD));
    centre(buf, area, area.y as i32 + 1, &format!("{}, {}", l.name, l.city), Style::new().fg(DIM));
    let ty = area.y as i32 + ((by0 + box_h) / 2) as i32 + 1;
    for side in 0..2 {
        let half = Rect::new(area.x + if side == 0 { 0 } else { area.width / 2 }, area.y, area.width / 2, area.height);
        let sel = app.setup_row == side;
        let name = app.setup[side].name(&app.pad_names);
        let text = format!("◀ {name} ▶");
        let st = if sel { Style::new().fg(Color::Black).bg(GOLD).add_modifier(Modifier::BOLD) } else { Style::new().fg(Color::White) };
        centre(buf, half, area.y as i32 + 3, &format!("Player {}", side + 1), Style::new().fg(colour(if side == 0 { P1_COLOUR } else { P2_COLOUR })).add_modifier(Modifier::BOLD));
        centre(buf, half, ty, &text, st);
        let help = match app.setup[side] {
            Ctl::Keys(l) => l.keys_help().to_string(),
            Ctl::Pad(_) => "stick moves, A hit, B/Y topspin, X smash".into(),
            Ctl::Computer => "the computer plays".into(),
        };
        centre(buf, half, ty + 1, &help, Style::new().fg(DIM));
    }
    let sel = app.setup_row == 2;
    let st = if sel { Style::new().fg(Color::Black).bg(GOLD).add_modifier(Modifier::BOLD) } else { Style::new().fg(Color::White) };
    centre(buf, area, ty + 4, if sel { "▶ Start the match ◀" } else { "Start the match" }, st);
    centre(buf, area, (area.y + area.height) as i32 - 1, "↑↓ choose  ←→ change  Enter start  Esc back", Style::new().fg(DIM));
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn the_map_has_land_where_it_should() {
        // Australia under Melbourne's marker, sea in the middle of the Pacific
        assert!((0..6).any(|d| land(422 / 2 - d, 187 / 2 - d)));
        assert!(!land(20, 60));
    }

    #[test]
    fn power_bars() {
        assert_eq!(power_bar(0.0, 4), "    ");
        assert_eq!(power_bar(100.0, 4), "████");
        assert_eq!(power_bar(50.0, 4).chars().count(), 4);
    }
}
