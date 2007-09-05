
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

#define FADE_DURATION 500
#define BUTTON_HIGHLIGHT 0.4
#define BUTTON_BORDER 2

#define GET_PIXEL_DATA(surface,x,y) (*((Uint32*)(surface->pixels + x * surface->format->BytesPerPixel + y * surface->pitch)))

#define GET_PIXEL_RGB(surface,x,y,r,g,b) (SDL_GetRGB( GET_PIXEL_DATA(surface,x,y), surface->format, r, g, b))
#define SET_PIXEL_RGB(surface,x,y,r,g,b) (GET_PIXEL_DATA(surface,x,y)=SDL_MapRGB(surface->format, r, g, b))

typedef struct {
    SDL_Surface* data;
} Image;

enum {
    GR_COURT = 0,
    GR_SHADOW,
    GR_RACKET,
    GR_GROUND,
    GR_BALL,
    GR_MENU,
    GR_SMALLISH_FONT,
    GR_DKC2_FONT,
    GR_COUNT
};

enum {
    ANIMATION_NONE = 0,
    ANIMATION_WAVE = 1,
    ANIMATION_BUNGEE = 2,
    ANIMATION_PENDULUM = 4,
    ANIMATION_COUNT = 8
};

void init_graphics( const char*);
void uninit_graphics();
int get_image_width( unsigned int);
int get_image_height( unsigned int);
int get_sprite_width( unsigned int, int);
void show_sprite( unsigned int, int, int, int, int, int);
void show_image( unsigned int, int, int, int);
void line_horiz( int y, Uint8 r, Uint8 g, Uint8 b);
void line_vert( int x, Uint8 r, Uint8 g, Uint8 b);
void rectangle( int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b);
void draw_button( int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, char pressed);
void draw_button_text( char* s, int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, char pressed);
void introimage( unsigned int);
void clearscr();
void updatescr();
void start_fade();
int is_fading();

int font_get_metrics( unsigned int id, char ch, int* xp, int* wp);
int font_draw_char( unsigned int id, char ch, int x_offset, int y_offset);
void font_draw_string( unsigned int id, char* s, int x_offset, int y_offset, int start, int animation);
int font_get_string_width( unsigned int id, char* s);

void draw_line_faded( int x1, int y1, int x2, int y2, int r, int g, int b, int r2, int g2, int b2);
#define draw_line(x1,y1,x2,y2,r,g,b) draw_line_faded(x1,y1,x2,y2,r,g,b,r,g,b)

#endif

