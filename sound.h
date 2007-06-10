
/**
 *
 * Tennix! SDL Port
 * Copyright (C) 2003, 2007 Thomas Perl <thp@perli.net>
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

#ifndef __SOUND_H
#define __SOUND_H

#include "SDL_mixer.h"

#define FADE_OUT_MS 1000

enum {
    CH_RACKET = 0,
    CH_GROUND,
    CH_AUDIENCE,
    CH_APPLAUSE,
    CH_OUT,
    CH_MAX,
};

void init_sound( const char *);
void play_sample( const char*, unsigned char, int);
void fade_out( unsigned char);

void sound_ground();
void sound_racket( unsigned char);
void sound_audience();
void sound_applause();
void sound_applause_stop();
void sound_out();

#endif
