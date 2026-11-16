//! The match: Thomas Perl's game.cc and game.h from Tennix 1.3.5, ported
//! line by line. The court is his 640x480 screen, x from left to right,
//! y from top to bottom, z the ball's height above the ground. Player 1
//! plays from the left, player 2 from the right; each moves only up and
//! down along the baseline, as in the original.
//!
//! Copyright (C) 2003, 2007, 2008, 2009 Thomas Perl; Rust port 2026
//! Asciiverse. GPL-2.0-or-later.

pub const WIDTH: f32 = 640.0;
pub const HEIGHT: f32 = 480.0;

/// Sets a player needs to win the match (his SETS_TO_WIN: best of five).
pub const SETS_TO_WIN: usize = 3;

pub const BALL_RESTITUTION: f64 = 0.6;
/// Simulation step in milliseconds (GAME_TICKS).
pub const GAME_TICKS: u32 = 15;
pub const GRAVITY: f64 = -0.03;

pub const RACKET_X_MID: f32 = 15.0;
pub const RACKET_Y_MID: f32 = 24.0;
pub const BALL_X_MID: f32 = 9.0;
pub const BALL_Y_MID: f32 = 9.0;

pub const GAME_X_MIN: f32 = 41.0 * 2.0;
pub const GAME_X_MAX: f32 = 270.0 * 2.0;
pub const GAME_X_MID: f32 = (GAME_X_MIN + GAME_X_MAX) / 2.0;
pub const GAME_Y_MIN: f32 = 155.0;
pub const GAME_Y_MAX: f32 = 330.0;
pub const GAME_Y_MID: f32 = (GAME_Y_MIN + GAME_Y_MAX) / 2.0;

pub const NET_X: f32 = 320.0;
pub const NET_Y: f32 = 107.0;
pub const NET_WIDTH: f32 = 5.0;
pub const NET_HEIGHT: f32 = 273.0;
pub const NET_TALLNESS: f32 = 5.0;

pub const FIELD_LEFT_X: f32 = 119.0;
pub const FIELD_LEFT_WIDTH: f32 = 202.0;
pub const FIELD_RIGHT_X: f32 = 326.0;
pub const FIELD_RIGHT_WIDTH: f32 = 202.0;
pub const FIELD_SINGLE_Y: f32 = 154.0;
pub const FIELD_SINGLE_HEIGHT: f32 = 178.0;

pub const PLAYER_Y_MIN: f32 = 0.0;
pub const PLAYER_Y_MAX: f32 = HEIGHT;
pub const PLAYER_AREA_Y: f32 = RACKET_Y_MID;
pub const PLAYER_AREA_X: f32 = RACKET_X_MID;
pub const PLAYER_MOVE_Y: f32 = 5.0;
pub const PLAYER_ACCEL_DEFAULT: f32 = 0.2;
pub const PLAYER_ACCEL_INCREASE: f64 = 1.2;
pub const POWER_UP_FACTOR: f64 = 1.1;
pub const POWER_DOWN_FACTOR: f64 = 0.9;
pub const PLAYER_POWER_MAX: f32 = 100.0;

/// Wait before a point is scored (SCORING_DELAY), in ticks.
pub const SCORING_TICKS: u32 = 1000 / GAME_TICKS;

/// The computer player's name, as on his controller setup screen.
pub const AI_NAME: &str = "Carl Van Court";

fn in_net_x(x: f32) -> bool {
    x + BALL_X_MID >= NET_X && x - BALL_X_MID <= NET_X + NET_WIDTH
}
fn in_net_y(y: f32) -> bool {
    (NET_Y..=NET_Y + NET_HEIGHT).contains(&y)
}
pub fn net_collision(x: f32, y: f32, z: f32) -> bool {
    z <= NET_TALLNESS && in_net_x(x) && in_net_y(y)
}
fn field_left_valid(x: f32) -> bool {
    (FIELD_LEFT_X..=FIELD_LEFT_X + FIELD_LEFT_WIDTH).contains(&x)
}
fn field_right_valid(x: f32) -> bool {
    (FIELD_RIGHT_X..=FIELD_RIGHT_X + FIELD_RIGHT_WIDTH).contains(&x)
}
fn field_single_valid(y: f32) -> bool {
    (FIELD_SINGLE_Y..=FIELD_SINGLE_Y + FIELD_SINGLE_HEIGHT).contains(&y)
}
/// The ball landed where the player who hit it had to put it.
pub fn ground_is_valid(player: i8, x: f32, y: f32) -> bool {
    field_single_valid(y) && if player == 1 { field_right_valid(x) } else { field_left_valid(x) }
}
pub fn is_out(x: f32, y: f32) -> bool {
    !(field_single_valid(y) && (field_right_valid(x) || field_left_valid(x)))
}
fn is_offscreen(x: f32, y: f32) -> bool {
    let oy = y < -BALL_Y_MID * 2.0 || y > HEIGHT + BALL_Y_MID * 2.0;
    let ox = x < -BALL_X_MID * 2.0 || x > WIDTH + BALL_X_MID * 2.0;
    ox || oy
}
fn is_near_y(py: f32, by: f32) -> bool {
    (py - by).abs() < PLAYER_AREA_Y
}
fn is_near_y_ai(py: f32, by: f32) -> bool {
    ((py - by).abs() as f64) < PLAYER_AREA_Y as f64 / 3.5
}
fn is_near_x(px: f32, bx: f32) -> bool {
    (px - bx).abs() < PLAYER_AREA_X
}
fn is_near_x_ai(px: f32, bx: f32) -> bool {
    (px - bx).abs() < PLAYER_AREA_X * 2.0
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Referee {
    Normal,
    Out,
    Player1,
    Player2,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Desire {
    Normal,
    Topspin,
    Smash,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ScoreEvent {
    Undecided,
    Net,
    Out,
    GroundInvalid,
    GroundValid,
    Offscreen,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Status {
    #[allow(dead_code)]
    None,
    Welcome,
    Default,
    Net,
    Out,
    Fault,
    P1Scores,
    P2Scores,
    Volley,
    DidntCatch,
}

/// What happened during a step, for sound and the log (his sound_events
/// bits, kept as a list instead of flipped bits).
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum Event {
    /// the ball bounced; how hard (|move_z| after the bounce) and where (x)
    Ground { speed: f32, x: f32 },
    Out,
    Applause,
    Racket { player: u8, x: f32 },
    /// a point was scored for this player
    Point(u8),
}

#[derive(Clone, Copy, Debug, Default)]
pub struct Ball {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub move_x: f32,
    pub move_y: f32,
    pub move_z: f32,
    pub ground_hit: bool,
    pub last_hit_by: i8,
    pub inhibit_gravity: bool,
}

/// What a player wants this tick: up/down axis (-1..1) and the three keys.
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Controls {
    pub axis: f32,
    pub hit: bool,
    pub topspin: bool,
    pub smash: bool,
}

#[derive(Clone, Copy, Debug)]
pub struct Player {
    pub x: f32,
    pub y: f32,
    pub power: f32,
    pub use_power: bool,
    pub desire: Desire,
    pub ai: bool,
    /// points in the current game (0, 1, 2, 3 are love to forty; deuce and
    /// advantage go on counting, as his do)
    pub game: i32,
    pub sets: [u8; SETS_TO_WIN * 2],
    pub accelerate: f32,
}

impl Player {
    fn new(x: f32) -> Player {
        Player {
            x,
            y: GAME_Y_MID,
            power: 0.0,
            use_power: true,
            desire: Desire::Normal,
            ai: true,
            game: 0,
            sets: [0; SETS_TO_WIN * 2],
            accelerate: PLAYER_ACCEL_DEFAULT,
        }
    }
}

#[derive(Clone, Debug)]
pub struct Match {
    pub ball: Ball,
    pub players: [Player; 2],
    /// 1 or 2
    pub serving_player: u8,
    pub referee: Referee,
    pub current_set: usize,
    /// 0 none, else 1 or 2
    pub winner: u8,
    pub score_event: ScoreEvent,
    pub score_time: u32,
    pub status: Status,
    /// sets a player needs (his 3; shorter matches are an option of ours)
    pub sets_to_win: usize,
    /// ticks played, for the log
    pub ticks: u64,
    /// racket hits in the current rally
    pub rally: u32,
    /// Ours, not his: when the computer plays itself his players never
    /// miss and a rally never ends (his own --benchmark plays one rally for
    /// ever). With this set (a random seed), each computer player aims a
    /// little off the ball for every return, so shots go wide and balls
    /// are missed, and points get won. Off in every match a person plays.
    pub variation: Option<u64>,
    aim: [f32; 2],
}

impl Match {
    pub fn new(sets_to_win: usize) -> Match {
        let mut m = Match {
            ball: Ball { last_hit_by: -1, ..Ball::default() },
            players: [Player::new(GAME_X_MIN - RACKET_X_MID * 2.0), Player::new(GAME_X_MAX + RACKET_X_MID * 2.0)],
            serving_player: 1,
            referee: Referee::Normal,
            current_set: 0,
            winner: 0,
            score_event: ScoreEvent::Undecided,
            score_time: 0,
            status: Status::Welcome,
            sets_to_win: sets_to_win.clamp(1, SETS_TO_WIN),
            ticks: 0,
            rally: 0,
            variation: None,
            aim: [0.0; 2],
        };
        m.setup_serve();
        m
    }

    pub fn p(&self, n: u8) -> &Player {
        &self.players[n as usize - 1]
    }
    fn pm(&mut self, n: u8) -> &mut Player {
        &mut self.players[n as usize - 1]
    }

    /// One tick of the game (his step()), then the players' input (his
    /// handle_input()), as his game loop runs them. `controls` is used for
    /// human players; computer players think for themselves.
    pub fn tick(&mut self, controls: [Controls; 2], events: &mut Vec<Event>) {
        self.tick_by(|_, p| controls[p as usize - 1], events);
    }

    /// As tick(), with each human's controls read after the ball has moved
    /// (where his game reads the keyboard).
    pub fn tick_by(&mut self, controls: impl Fn(&Match, u8) -> Controls, events: &mut Vec<Event>) {
        self.step(events);
        if self.winner == 0 {
            for p in 1..=2u8 {
                if self.p(p).ai {
                    self.input_ai(p);
                } else {
                    let c = controls(self, p);
                    self.input_human(p, c);
                }
                let pl = self.pm(p);
                pl.y = pl.y.clamp(PLAYER_Y_MIN, PLAYER_Y_MAX);
            }
        }
        self.ticks += 1;
    }

    fn step(&mut self, events: &mut Vec<Event>) {
        let mut ground_event = false;
        let b = &mut self.ball;
        b.z += b.move_z;
        // his constants are C doubles: the sums are done in double and
        // stored back in float, as here, so the port follows his exactly
        if !b.inhibit_gravity {
            b.move_z = (b.move_z as f64 + GRAVITY) as f32;
        }
        b.x += b.move_x;
        b.y += b.move_y;

        for pl in &mut self.players {
            if pl.use_power {
                pl.power = ((pl.power as f64 * POWER_DOWN_FACTOR) as f32).min(PLAYER_POWER_MAX).max(0.0);
            }
        }

        let b = &mut self.ball;
        if b.z < 0.0 {
            ground_event = true;
            self.referee = Referee::Normal;
            if b.move_z.abs() > 0.3 {
                b.move_z = (b.move_z as f64 * -BALL_RESTITUTION) as f32;
                events.push(Event::Ground { speed: b.move_z.abs(), x: b.x });
            } else {
                b.move_z = 0.0;
            }
            b.z = 0.0;
        }

        if net_collision(b.x, b.y, b.z) {
            // the net blocks the ball: put it back outside the net
            while net_collision(b.x, b.y, b.z) {
                if b.move_x < 0.0 {
                    b.x += 1.0;
                } else {
                    b.x -= 1.0;
                }
            }
            b.move_x = 0.0;
            b.move_y = 0.0;
        }

        if self.score_event == ScoreEvent::Undecided {
            let b = self.ball;
            if net_collision(b.x, b.y, b.z) {
                self.score_event = ScoreEvent::Net;
                events.push(Event::Out);
                self.status = Status::Net;
            } else if is_offscreen(b.x, b.y) {
                self.score_event = ScoreEvent::Offscreen;
                events.push(Event::Out);
                self.status = Status::Out;
            } else if ground_event {
                if is_out(b.x, b.y) {
                    self.score_event = ScoreEvent::Out;
                    events.push(Event::Out);
                    self.status = Status::Out;
                } else if ground_is_valid(b.last_hit_by, b.x, b.y) {
                    if b.ground_hit {
                        self.score_event = ScoreEvent::GroundValid;
                        events.push(Event::Out);
                        self.status = Status::DidntCatch;
                    } else {
                        self.ball.ground_hit = true;
                    }
                } else {
                    self.score_event = ScoreEvent::GroundInvalid;
                    events.push(Event::Out);
                    self.status = Status::Fault;
                }
            }
        }

        if self.score_event != ScoreEvent::Undecided {
            if self.score_time < SCORING_TICKS {
                self.score_time += 1;
                self.referee = Referee::Out;
            } else {
                let w = self.score_point();
                self.status = if w == 1 { Status::P1Scores } else { Status::P2Scores };
                self.referee = if w == 1 { Referee::Player1 } else { Referee::Player2 };
                events.push(Event::Point(w));
                self.score_time = 0;
                self.rally = 0;
                self.setup_serve();
                events.push(Event::Applause);
            }
        } else {
            for p in 1..=2u8 {
                let pl = *self.p(p);
                let b = self.ball;
                if is_near_x(pl.x, b.x) && is_near_y(pl.y, b.y - b.z) && pl.use_power && pl.power > 30.0 && b.last_hit_by != p as i8 {
                    // racket hit
                    self.status = if !b.ground_hit && b.move_x != 0.0 { Status::Volley } else { Status::Default };
                    let pw = pl.power as f64;
                    let (mx, mz) = match pl.desire {
                        Desire::Normal => (2.7 + 2.0 * pw / 100.0, 1.2 * pw / 100.0),
                        Desire::Topspin => (1.1 + 2.2 * pw / 100.0, 2.5 * pw / 100.0),
                        Desire::Smash => (4.0 + 3.0 * pw / 100.0, 1.1 * pw / 100.0),
                    };
                    self.ball.move_x = mx as f32;
                    self.ball.move_z = mz as f32;
                    self.ball.move_y = self.get_move_y(p);
                    events.push(Event::Racket { player: p, x: self.ball.x });
                    self.ball.ground_hit = false;
                    self.ball.inhibit_gravity = false;
                    self.ball.last_hit_by = p as i8;
                    self.rally += 1;
                    self.vary_aim(if p == 1 { 2 } else { 1 });
                    if p == 2 {
                        self.ball.move_x *= -1.0;
                    }
                }
            }
        }
    }

    /// Where the hit sends the ball across: off-centre on the racket sends
    /// it wide (his get_move_y()).
    fn get_move_y(&self, player: u8) -> f32 {
        let py = self.p(player).y;
        let by = self.ball.y - self.ball.z;
        let pa = RACKET_Y_MID * 2.0;
        let move_x = self.ball.move_x;
        let pct = ((by - py) / (pa / 2.0)).clamp(-1.0, 1.0);
        let dest = ((155.0f64 + 330.0) / 2.0 + pct as f64 * (330.0 - 155.0)) as f32;
        let x_len = if player == 1 { (540.0 - self.ball.x as f64) as f32 } else { (self.ball.x as f64 - 82.0) as f32 };
        let y_len = dest - by;
        (y_len * move_x) / x_len
    }

    fn input_human(&mut self, player: u8, c: Controls) {
        let pl = self.pm(player);
        let mut move_y = (PLAYER_MOVE_Y as f64 * c.axis as f64) as f32;
        if move_y != 0.0 {
            if move_y.abs() > (move_y * pl.accelerate).abs() {
                move_y *= pl.accelerate;
            }
            pl.y += move_y;
            pl.accelerate = (pl.accelerate as f64 * PLAYER_ACCEL_INCREASE) as f32;
            if pl.accelerate > 190.0 {
                pl.accelerate = 190.0;
            }
        } else {
            pl.accelerate = PLAYER_ACCEL_DEFAULT;
        }
        if c.hit || c.topspin || c.smash {
            pl.desire = if c.smash {
                Desire::Smash
            } else if c.topspin {
                Desire::Topspin
            } else {
                Desire::Normal
            };
            pl.power = power_up(pl.power);
            pl.use_power = false;
        } else {
            pl.use_power = true;
        }
    }

    /// Carl Van Court, his computer player (input_ai()).
    fn input_ai(&mut self, player: u8) {
        let b = self.ball;
        let aim = self.aim[player as usize - 1];
        let pl = self.pm(player);
        let mut fact = 1.7f32;
        let approaching = (pl.x < GAME_X_MID && b.move_x <= 0.0) || (pl.x > GAME_X_MID && b.move_x >= 0.0);
        let by = b.y - b.z;
        // where the player heads for: the ball, or a little off it (variation)
        let goal = by + aim;
        if (pl.y - by).abs() > RACKET_Y_MID * 5.0 {
            fact = 3.5;
        }
        if pl.desire == Desire::Normal && !is_near_y_ai(pl.y, goal) && approaching {
            if pl.y < goal {
                pl.y += (2.0 * fact).min(goal - pl.y);
            } else if pl.y > goal {
                pl.y -= (2.0 * fact).min(pl.y - goal);
            }
        }
        if is_near_y(pl.y, by) && is_near_x_ai(pl.x, b.x) && pl.power > 90.0 {
            pl.use_power = true;
        } else if approaching {
            pl.power = power_up(pl.power);
            pl.use_power = false;
        }
    }

    fn setup_serve(&mut self) {
        self.aim = [0.0; 2];
        let b = &mut self.ball;
        b.z = 10.0;
        b.y = GAME_Y_MID;
        b.move_x = 0.0;
        b.move_y = 0.0;
        b.move_z = 0.0;
        b.last_hit_by = -1;
        b.inhibit_gravity = true;
        b.x = if self.serving_player == 1 { GAME_X_MIN - RACKET_X_MID * 1.5 } else { GAME_X_MAX + RACKET_X_MID * 1.5 };
    }

    /// Scores the point (his score_game()); returns the player who won it.
    fn score_point(&mut self) -> u8 {
        let (last, other) = if self.ball.last_hit_by == 1 { (1u8, 2u8) } else { (2, 1) };
        let winner = match self.score_event {
            ScoreEvent::Net => other,
            ScoreEvent::Out | ScoreEvent::GroundInvalid | ScoreEvent::Offscreen | ScoreEvent::GroundValid => {
                if self.ball.ground_hit {
                    last
                } else {
                    other
                }
            }
            ScoreEvent::Undecided => other,
        };
        let loser = if winner == last { other } else { last };
        let set = self.current_set;
        let (w, l) = (winner as usize - 1, loser as usize - 1);
        self.players[w].game += 1;
        if self.players[l].game < self.players[w].game - 1 && self.players[w].game >= 4 {
            self.players[w].game = 0;
            self.players[l].game = 0;
            self.players[w].sets[set] += 1;
            // the serve changes when the game is over
            self.serving_player = if self.serving_player == 1 { 2 } else { 1 };
            let (ws, ls) = (self.players[w].sets[set], self.players[l].sets[set]);
            if (ws == 6 && ls < 5) || ws == 7 {
                self.current_set += 1;
                self.winner = self.match_winner();
            }
        }
        self.score_event = ScoreEvent::Undecided;
        winner
    }

    fn match_winner(&self) -> u8 {
        let mut sets = [0usize; 2];
        for i in 0..self.current_set {
            if self.players[0].sets[i] > self.players[1].sets[i] {
                sets[0] += 1;
            } else {
                sets[1] += 1;
            }
        }
        if sets[0] == self.sets_to_win {
            1
        } else if sets[1] == self.sets_to_win {
            2
        } else {
            0
        }
    }

    /// "6:4, 3:2" (his format_sets()).
    pub fn format_sets(&self) -> String {
        let mut max = self.current_set as i64;
        if self.winner != 0 {
            max -= 1;
        }
        (0..=max.max(0) as usize).map(|i| format!("{}:{}", self.players[0].sets[i], self.players[1].sets[i])).collect::<Vec<_>>().join(", ")
    }

    /// "15 - 30", "deuce", "advantage player 1" (his format_game()).
    pub fn format_game(&self) -> String {
        const SCORING: [i32; 4] = [0, 15, 30, 40];
        let (a, b) = (self.players[0].game, self.players[1].game);
        if a < 4 && b < 4 {
            format!("{} - {}", SCORING[a as usize], SCORING[b as usize])
        } else if a > b {
            "advantage player 1".into()
        } else if a < b {
            "advantage player 2".into()
        } else {
            "deuce".into()
        }
    }

    /// The line under the court (his format_status()).
    pub fn format_status(&self) -> String {
        const SET_NAMES: [&str; 5] = ["first", "second", "third", "fourth", "fifth"];
        match self.status {
            Status::None => String::new(),
            Status::Welcome => format!("welcome to tennix {}", crate::UPSTREAM_VERSION),
            Status::Default => {
                let s = self.current_set.min(SETS_TO_WIN * 2 - 1);
                format!("{}:{} in {} set", self.players[0].sets[s], self.players[1].sets[s], SET_NAMES[s.min(4)])
            }
            Status::Net => "net!".into(),
            Status::Out => "out!".into(),
            Status::Fault => "fault!".into(),
            Status::P1Scores => "player 1 scores.".into(),
            Status::P2Scores => "player 2 scores.".into(),
            Status::Volley => "volley!".into(),
            Status::DidntCatch => "did not catch the ball.".into(),
        }
    }

    /// Plain text of the whole state, for saving a paused match.
    pub fn save(&self) -> String {
        let b = &self.ball;
        let mut s = format!(
            "ball {} {} {} {} {} {} {} {} {}\n",
            b.x, b.y, b.z, b.move_x, b.move_y, b.move_z, b.ground_hit as u8, b.last_hit_by, b.inhibit_gravity as u8
        );
        for p in &self.players {
            let sets: Vec<String> = p.sets.iter().map(|v| v.to_string()).collect();
            s += &format!("player {} {} {} {} {} {} {} {}\n", p.x, p.y, p.power, p.use_power as u8, desire_n(p.desire), p.ai as u8, p.game, sets.join(","));
        }
        s += &format!("state {} {} {} {} {}\n", self.serving_player, self.current_set, self.winner, self.sets_to_win, self.ticks);
        s
    }

    /// Reads what save() wrote; None when it does not parse.
    pub fn load(text: &str) -> Option<Match> {
        let mut m = Match::new(SETS_TO_WIN);
        let mut players = 0;
        for line in text.lines() {
            let f: Vec<&str> = line.split_whitespace().collect();
            match f.first().copied() {
                Some("ball") if f.len() == 10 => {
                    let n = |i: usize| f[i].parse::<f32>().ok();
                    m.ball = Ball {
                        x: n(1)?,
                        y: n(2)?,
                        z: n(3)?,
                        move_x: n(4)?,
                        move_y: n(5)?,
                        move_z: n(6)?,
                        ground_hit: f[7] == "1",
                        last_hit_by: f[8].parse().ok()?,
                        inhibit_gravity: f[9] == "1",
                    };
                }
                Some("player") if f.len() == 9 && players < 2 => {
                    let n = |i: usize| f[i].parse::<f32>().ok();
                    let p = &mut m.players[players];
                    p.x = n(1)?;
                    p.y = n(2)?;
                    p.power = n(3)?;
                    p.use_power = f[4] == "1";
                    p.desire = match f[5] {
                        "1" => Desire::Topspin,
                        "2" => Desire::Smash,
                        _ => Desire::Normal,
                    };
                    p.ai = f[6] == "1";
                    p.game = f[7].parse().ok()?;
                    for (i, v) in f[8].split(',').enumerate().take(SETS_TO_WIN * 2) {
                        p.sets[i] = v.parse().ok()?;
                    }
                    players += 1;
                }
                Some("state") if f.len() == 6 => {
                    m.serving_player = f[1].parse().ok().filter(|v| *v == 1 || *v == 2)?;
                    m.current_set = f[2].parse().ok().filter(|v| *v < SETS_TO_WIN * 2)?;
                    m.winner = f[3].parse().ok()?;
                    m.sets_to_win = f[4].parse::<usize>().ok()?.clamp(1, SETS_TO_WIN);
                    m.ticks = f[5].parse().ok()?;
                }
                _ => {}
            }
        }
        (players == 2).then(|| {
            m.status = Status::Default;
            m
        })
    }

    /// A new aim for the player about to receive the ball (variation only).
    fn vary_aim(&mut self, player: u8) {
        let Some(r) = self.variation.as_mut() else { return };
        let mut next = || {
            *r ^= *r << 13;
            *r ^= *r >> 7;
            *r ^= *r << 17;
            (*r >> 11) as f32 / (1u64 << 53) as f32
        };
        let (a, b) = (next(), next());
        // mostly a touch off centre, which sends the ball wide now and
        // then; one return in ten reached for too late and missed
        let off = if b < 0.1 { (a - 0.5).signum() * (26.0 + 10.0 * a) } else { (a - 0.5) * 26.0 };
        self.aim[player as usize - 1] = off;
    }

    pub fn snapshot(&self) -> String {
        let b = &self.ball;
        format!(
            "ball ({:.1},{:.1},{:.1}) v ({:.2},{:.2},{:.2}) bounced {} last {} | p1 y {:.1} power {:.0}{} | p2 y {:.1} power {:.0}{} | game {} sets {} serve {} rally {} status {:?}",
            b.x,
            b.y,
            b.z,
            b.move_x,
            b.move_y,
            b.move_z,
            b.ground_hit,
            b.last_hit_by,
            self.players[0].y,
            self.players[0].power,
            if self.players[0].use_power { "" } else { " winding" },
            self.players[1].y,
            self.players[1].power,
            if self.players[1].use_power { "" } else { " winding" },
            self.format_game(),
            self.format_sets(),
            self.serving_player,
            self.rally,
            self.status
        )
    }
}

/// The bar fills by a tenth a tick while the key is held, from at least 10.
fn power_up(power: f32) -> f32 {
    ((power as f64 * POWER_UP_FACTOR) as f32).min(PLAYER_POWER_MAX).max(10.0)
}

fn desire_n(d: Desire) -> u8 {
    match d {
        Desire::Normal => 0,
        Desire::Topspin => 1,
        Desire::Smash => 2,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn win_point(m: &mut Match, p: u8) {
        // a point for p: the other player put the ball out
        m.ball.last_hit_by = if p == 1 { 2 } else { 1 };
        m.ball.ground_hit = false;
        m.score_event = ScoreEvent::Out;
        assert_eq!(m.score_point(), p);
    }

    #[test]
    fn points_go_love_fifteen_thirty_forty_game() {
        let mut m = Match::new(3);
        assert_eq!(m.format_game(), "0 - 0");
        win_point(&mut m, 1);
        assert_eq!(m.format_game(), "15 - 0");
        win_point(&mut m, 1);
        win_point(&mut m, 2);
        assert_eq!(m.format_game(), "30 - 15");
        win_point(&mut m, 1);
        assert_eq!(m.format_game(), "40 - 15");
        win_point(&mut m, 1);
        assert_eq!(m.format_game(), "0 - 0");
        assert_eq!(m.format_sets(), "1:0");
        assert_eq!(m.serving_player, 2, "the serve changes after a game");
    }

    #[test]
    fn deuce_and_advantage() {
        let mut m = Match::new(3);
        for _ in 0..3 {
            win_point(&mut m, 1);
            win_point(&mut m, 2);
        }
        assert_eq!(m.format_game(), "40 - 40");
        win_point(&mut m, 2);
        assert_eq!(m.format_game(), "advantage player 2");
        win_point(&mut m, 1);
        assert_eq!(m.format_game(), "deuce");
        win_point(&mut m, 1);
        assert_eq!(m.format_game(), "advantage player 1");
        win_point(&mut m, 1);
        assert_eq!(m.format_sets(), "1:0");
    }

    fn win_game(m: &mut Match, p: u8) {
        for _ in 0..4 {
            win_point(m, p);
        }
    }

    #[test]
    fn a_set_is_six_games_by_two_or_seven_with_no_tiebreak() {
        let mut m = Match::new(3);
        for _ in 0..5 {
            win_game(&mut m, 1);
            win_game(&mut m, 2);
        }
        win_game(&mut m, 1);
        assert_eq!(m.format_sets(), "6:5", "6:5 is not a set yet");
        assert_eq!(m.current_set, 0);
        win_game(&mut m, 2);
        assert_eq!(m.format_sets(), "6:6");
        win_game(&mut m, 2);
        assert_eq!(m.current_set, 1, "7:6 takes the set, game by game as he has it");
        assert_eq!(m.format_sets(), "6:7, 0:0");
    }

    #[test]
    fn best_of_five_takes_three_sets() {
        let mut m = Match::new(3);
        for set in 0..3 {
            for _ in 0..6 {
                win_game(&mut m, 2);
            }
            assert_eq!(m.current_set, set + 1);
        }
        assert_eq!(m.winner, 2);
        assert_eq!(m.format_sets(), "0:6, 0:6, 0:6");
    }

    #[test]
    fn a_ball_dropped_in_play_bounces_lower_and_lower() {
        let mut m = Match::new(3);
        m.ball = Ball { x: 200.0, y: 200.0, z: 50.0, last_hit_by: 2, ..Ball::default() };
        let mut ev = Vec::new();
        let mut bounces = Vec::new();
        for _ in 0..400 {
            m.step(&mut ev);
            for e in ev.drain(..) {
                if let Event::Ground { speed, .. } = e {
                    bounces.push(speed);
                }
            }
        }
        assert!(bounces.len() >= 2);
        assert!(bounces.windows(2).all(|w| w[1] < w[0]));
        // first landing in player 2's target court, valid, then the second bounce scores
        assert_eq!(m.score_event, ScoreEvent::Undecided);
        assert_eq!(m.p(2).sets[0] + m.p(1).sets[0], 0);
        assert_eq!(m.p(2).game, 1, "the second bounce is a point for the hitter");
    }

    #[test]
    fn the_net_stops_a_low_ball() {
        let mut m = Match::new(3);
        m.ball = Ball { x: 300.0, y: 240.0, z: 2.0, move_x: 3.0, last_hit_by: 1, inhibit_gravity: true, ..Ball::default() };
        let mut ev = Vec::new();
        for _ in 0..20 {
            m.step(&mut ev);
        }
        assert_eq!(m.ball.move_x, 0.0);
        assert!(m.ball.x + BALL_X_MID < NET_X);
    }

    #[test]
    fn out_and_in() {
        assert!(is_out(100.0, 240.0));
        assert!(is_out(400.0, 140.0), "the doubles alleys are out in singles");
        assert!(!is_out(400.0, 240.0));
        assert!(ground_is_valid(1, 400.0, 240.0));
        assert!(!ground_is_valid(1, 200.0, 240.0));
    }

    #[test]
    fn a_held_hit_key_fills_the_power_bar_and_letting_go_swings() {
        let mut m = Match::new(3);
        m.players[0].ai = false;
        m.players[1].ai = true;
        let hold = Controls { hit: true, ..Controls::default() };
        let mut ev = Vec::new();
        // walk to the ball floating at the serve spot, holding hit
        for _ in 0..40 {
            m.tick([hold, Controls::default()], &mut ev);
        }
        assert!((m.p(1).power - 100.0).abs() < 0.01, "power {}", m.p(1).power);
        assert!(ev.iter().all(|e| !matches!(e, Event::Racket { .. })));
        // move up to the ball (it floats at z 10, so its picture is 10 higher)
        m.players[0].y = m.ball.y - m.ball.z;
        m.tick([Controls::default(), Controls::default()], &mut ev);
        m.tick([Controls::default(), Controls::default()], &mut ev);
        assert!(ev.iter().any(|e| matches!(e, Event::Racket { player: 1, .. })), "{ev:?}");
        assert!(m.ball.move_x > 4.0);
    }

    #[test]
    fn save_and_load_round_trip() {
        let mut m = Match::new(2);
        win_game(&mut m, 1);
        win_point(&mut m, 2);
        m.players[0].ai = false;
        let l = Match::load(&m.save()).unwrap();
        assert_eq!(l.format_game(), m.format_game());
        assert_eq!(l.format_sets(), m.format_sets());
        assert_eq!(l.sets_to_win, 2);
        assert!(!l.players[0].ai && l.players[1].ai);
        assert_eq!(l.serving_player, m.serving_player);
    }
}

#[cfg(test)]
mod ai_tests {
    use super::*;

    /// The computer against itself, headless: rallies happen and points
    /// are won, as in his --benchmark mode.
    #[test]
    fn his_computer_against_itself_never_misses() {
        // as his C does (checked tick for tick against it): one endless rally
        let mut m = Match::new(1);
        let mut ev = Vec::new();
        for _ in 0..100_000 {
            m.tick([Controls::default(); 2], &mut ev);
        }
        assert!(ev.iter().filter(|e| matches!(e, Event::Racket { .. })).count() > 800);
        assert!(!ev.iter().any(|e| matches!(e, Event::Point(_))));
    }

    #[test]
    fn computer_rallies_and_wins_points() {
        let mut m = Match::new(1);
        m.variation = Some(12345);
        let mut ev = Vec::new();
        let mut hits = 0;
        let mut points = [0u32; 2];
        let mut longest = 0;
        let mut rally = 0;
        for _ in 0..200_000 {
            ev.clear();
            m.tick([Controls::default(); 2], &mut ev);
            for e in &ev {
                match e {
                    Event::Racket { .. } => {
                        hits += 1;
                        rally += 1;
                        longest = longest.max(rally);
                    }
                    Event::Point(w) => {
                        points[*w as usize - 1] += 1;
                        rally = 0;
                    }
                    _ => {}
                }
            }
            if m.winner != 0 {
                break;
            }
        }
        eprintln!("hits {hits} points {points:?} longest rally {longest} sets {} winner {} ticks {}", m.format_sets(), m.winner, m.ticks);
        assert!(m.winner != 0, "a one-set match ends");
        assert!(points[0] > 5 && points[1] > 5, "{points:?}");
        assert!(longest >= 6, "{longest}");
        let mean = hits as f32 / (points[0] + points[1]) as f32;
        assert!((2.0..12.0).contains(&mean), "{mean} hits a point");
    }
}
