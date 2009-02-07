
/**
 *
 * Tennix! SDL Port
 * Copyright (C) 2003, 2007, 2008, 2009 Thomas Perl <thp@thpinfo.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA  02110-1301, USA.
 *
 **/

#ifndef __LOCATIONS_H
#define __LOCATIONS_H

#include "game.h"
#include "graphics.h"

static Location locations[] = {
    {
        /* Australian Open, http://en.wikipedia.org/wiki/Australian_Open */
        "Margaret Court Arena",
        "Melbourne Park",
        "Melbourne",
        GR_CTT_GRASS,
        6000,
        "Plexicushion",
        GR_LOC_MARGARET_COURT_ARENA,
        1,
        422,
        187
    },
    {
        /* French Open, http://en.wikipedia.org/wiki/French_Open */
        "Stade Roland-Garros",
        "16th arrondissement",
        "Paris",
        GR_CTT_CLAY,
        8000,
        "terre battue",
        GR_LOC_STADE_ROLAND_GARROS,
        1,
        227,
        51,
        true
    },
    {
        /* http://en.wikipedia.org/wiki/The_Championships,_Wimbledon */
        "Court No 1",
        "Wimbledon",
        "London",
        GR_CTT_GRASS,
        14000,
        "ryegrass",
        GR_LOC_COURT_NO_1,
        1,
        225,
        46,
        true
    },
    {
        /* US Open, http://en.wikipedia.org/wiki/US_Open_(tennis) */
        "Arthur Ashe Stadium",
        "Flushing Meadows",
        "New York City",
        GR_CTT_HARD,
        22547,
        "acrylic hard",
        GR_LOC_ARTHUR_ASHE_STADIUM,
        1,
        125,
        64,
        true
    },

#ifdef NONFREE_LOCATIONS
    {
        /* Training Camp Lung-Katah */
        "Training Camp Bunabhainneadar",
        "Husinish",
        "North Harris",
        GR_CTT_GRASS,
        30,
        "artificial grass",
        GR_LOC_TRAINING_CAMP,
        1,
        218,
        37,
        false
    },
#endif

    /* End marker */
    { NULL, NULL, NULL, 0, 0, NULL, 0, 0, 0, 0 }
};

int location_count()
{
    static int location_count = 0;

    if (location_count == 0) {
        while (locations[location_count].name != NULL) {
            location_count++;
        }
    }

    return location_count;
}


#endif

