//! The Classic Championship Tour: Thomas Perl's locations.h. His four
//! tour venues and the two he had to leave switched off carry names of
//! our own here (his are real stadiums); the cities, surfaces, crowds and
//! weather are his. The training camp in the Hebrides is his own.

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Surface {
    Grass,
    Clay,
    Hard,
}

#[derive(Clone, Copy, Debug)]
pub struct Location {
    pub name: &'static str,
    pub area: &'static str,
    pub city: &'static str,
    /// which of his three court textures the court is drawn with
    pub court: Surface,
    pub max_visitors: u32,
    /// what the surface is called there
    pub court_type_name: &'static str,
    /// on his 500x219 world map
    pub worldmap_x: u32,
    pub worldmap_y: u32,
    pub has_referee: bool,
    /// raindrops drawn each frame
    pub rain: u32,
    /// layers of drifting fog
    pub fog: u32,
}

pub const LOCATIONS: [Location; 7] = [
    Location {
        name: "Eucalyptus Arena",
        area: "Riverbank Park",
        city: "Melbourne",
        court: Surface::Grass,
        max_visitors: 6000,
        court_type_name: "Plexicushion",
        worldmap_x: 422,
        worldmap_y: 187,
        has_referee: true,
        rain: 0,
        fog: 0,
    },
    Location {
        name: "Stade Terre-Rouge",
        area: "Porte de l'Ouest",
        city: "Paris",
        court: Surface::Clay,
        max_visitors: 8000,
        court_type_name: "terre battue",
        worldmap_x: 227,
        worldmap_y: 51,
        has_referee: true,
        rain: 10,
        fog: 0,
    },
    Location {
        name: "The Lawn Court",
        area: "Thameside Common",
        city: "London",
        court: Surface::Grass,
        max_visitors: 14000,
        court_type_name: "ryegrass",
        worldmap_x: 225,
        worldmap_y: 46,
        has_referee: true,
        rain: 50,
        fog: 1,
    },
    Location {
        name: "Hudson Stadium",
        area: "Riverside Meadows",
        city: "New York City",
        court: Surface::Hard,
        max_visitors: 22547,
        court_type_name: "acrylic hard",
        worldmap_x: 125,
        worldmap_y: 64,
        has_referee: true,
        rain: 0,
        fog: 2,
    },
    Location {
        name: "Tennis Club Bunabhainneadar",
        area: "Husinish",
        city: "North Harris",
        court: Surface::Grass,
        max_visitors: 30,
        court_type_name: "artificial grass",
        worldmap_x: 218,
        worldmap_y: 37,
        has_referee: false,
        rain: 0,
        fog: 0,
    },
    Location {
        name: "Alpine Center Court",
        area: "Valley Sportpark",
        city: "Kitzbühel",
        court: Surface::Clay,
        max_visitors: 4500,
        court_type_name: "clay",
        worldmap_x: 243,
        worldmap_y: 53,
        has_referee: true,
        rain: 0,
        fog: 0,
    },
    Location {
        name: "Jade Court",
        area: "North Park Tennis Centre",
        city: "Beijing",
        court: Surface::Hard,
        max_visitors: 10000,
        court_type_name: "deco turf",
        worldmap_x: 384,
        worldmap_y: 64,
        has_referee: true,
        rain: 0,
        fog: 0,
    },
];

/// Where night falls on the 500-pixel world map right now (his
/// day_night()): the night runs from `start` to `end`, wrapping round.
/// His C casts a negative difference to an unsigned int before wrapping,
/// which on 64-bit systems moves the night about 14 hours round the
/// world; this is the sum his comment means ("fits for London").
pub fn day_night(unix_secs: u64, width: u32) -> (u32, u32) {
    let offset: i64 = 60;
    let w = width as i64;
    let minutes = (unix_secs / 60) as i64;
    let start = (offset - minutes * w / (60 * 24)).rem_euclid(w);
    let end = (start + w / 2) % w;
    (start as u32, end as u32)
}

/// Is it night at this map column now (his location menu's test)?
pub fn is_night(worldmap_x: u32, (start, end): (u32, u32)) -> bool {
    if start < end { worldmap_x > start && worldmap_x < end } else { worldmap_x < end || worldmap_x > start }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn night_moves_west_through_the_day() {
        // twelve hours later the night has moved half way round
        let a = day_night(1_000_000_020, 500);
        let b = day_night(1_000_000_020 + 12 * 3600, 500);
        assert_eq!((a.0 + 250) % 500, b.0);
        // London (x 225) by day at noon UTC, by night at midnight
        let noon = 20_000 * 86_400 + 12 * 3600;
        assert!(!is_night(225, day_night(noon, 500)));
        assert!(is_night(225, day_night(noon + 12 * 3600, 500)));
        // London and Melbourne are never both dark at a moment far from dawn and dusk
        let mut both = 0;
        for h in 0..24 {
            let dn = day_night(1_000_000_000 + h * 3600, 500);
            if is_night(225, dn) && is_night(422, dn) {
                both += 1;
            }
        }
        assert!(both < 12);
    }
}
