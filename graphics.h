
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

#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include "tennix.h"

#include "SDL_image.h"

typedef struct {
    SDL_Surface* data;
} Image;

enum {
    GR_COURT = 0,
    GR_SHADOW,
    GR_RACKET,
    GR_GROUND,
    GR_BALL,
    GR_SCORE,
    GR_MENU,
    GR_ANIMATION,
    GR_COUNT
};

void init_graphics( const char*);
void uninit_graphics();
void show_sprite( unsigned int, int, int, int, int, int);
void show_image( unsigned int, int, int, int);
void show_digit( int, int, int, int);
void line_horiz( int y, Uint8 r, Uint8 g, Uint8 b);
void line_vert( int x, Uint8 r, Uint8 g, Uint8 b);
void introimage( unsigned int);
void clearscr();
void updatescr();

#endif

