
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

#include <stdio.h>

#include "tennix.h"
#include "graphics.h"
#include "input.h"

static Image* images;

static const char* filenames[] = {
    "court.png",
    "shadow.png",
    "player-racket.png",
    "ground.png",
    "ball.png",
    "score.png",
    "menu.png",
    "animation.png"
};

void init_graphics( const char *data_dir) {
    int i;
    SDL_Surface* data;
    SDL_Surface* tmp;
    char temp[MAXPATHLEN];

    images = (Image*)calloc( GR_COUNT, sizeof( Image));

    for( i=0; i<GR_COUNT; i++) {
        strcpy( temp, data_dir);
        strcat( temp, filenames[i]);
        tmp = IMG_Load( temp);
        if( !tmp) {
            fprintf( stderr, "Error: %s\n", SDL_GetError());
            continue;
        }

        SDL_SetColorKey( tmp, SDL_SRCCOLORKEY, SDL_MapRGB( tmp->format, 0, 0, 0)); 
        data = SDL_DisplayFormatAlpha( tmp);
        SDL_FreeSurface( tmp);

        if( !data) {
            fprintf( stderr, "Error: %s\n", SDL_GetError());
            continue;
        }
        images[i].data = data;
    }
}

void uninit_graphics() {
    int i;

    for( i=0; i<GR_COUNT; i++) {
        SDL_FreeSurface( images[i].data);
    }

    free( images);
}

void show_sprite( unsigned int id, int pos, int items, int x_offset, int y_offset, int opacity) {
    SDL_Surface *bitmap;
    SDL_Rect src, dst;

    bitmap = images[id].data;

    if( !bitmap) return;

    SDL_SetAlpha( bitmap, SDL_SRCALPHA | SDL_RLEACCEL, opacity);

    dst.w = src.w = bitmap->w/items;
    dst.h = src.h = bitmap->h;
    src.x = src.w*pos;
    src.y = 0;
    dst.x = x_offset;
    dst.y = y_offset;

    SDL_BlitSurface( bitmap, &src, screen, &dst);
}

void line_horiz( int y, Uint8 r, Uint8 g, Uint8 b) {
    Uint32 color = SDL_MapRGB( screen->format, r, g, b);
    SDL_Rect rect;

    rect.x = 0;
    rect.w = screen->w;
    rect.y = y;
    rect.h = 1;

    SDL_FillRect( screen, &rect, color);
}

void line_vert( int x, Uint8 r, Uint8 g, Uint8 b) {
    Uint32 color = SDL_MapRGB( screen->format, r, g, b);
    SDL_Rect rect;

    rect.x = x;
    rect.w = 1;
    rect.y = 0;
    rect.h = screen->h;

    SDL_FillRect( screen, &rect, color);
}

void rectangle( int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b) {
    Uint32 color = SDL_MapRGB( screen->format, r, g, b);
    SDL_Rect rect;

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    SDL_FillRect( screen, &rect, color);
}

void show_image( unsigned int id, int x_offset, int y_offset, int opacity) {
    show_sprite( id, 0, 1, x_offset, y_offset, opacity);
}

void show_digit( int zahl, int x_offset, int y_offset, int opacity) {
    show_sprite( GR_SCORE, zahl, 11, x_offset, y_offset, opacity);
}

void introimage( unsigned int id) {
    int i;
   
    for( i=0; i<256; i+=10) {
        clearscr();
        show_image( id, 0, 0, i);
        updatescr();
        SDL_Delay( 30);
    }
   
    //SDL_Delay( 500);
    wait_keypress();
   
    for( i=255; i>=0; i-=10) {
        clearscr();
        show_image( id, 0, 0, i);
        updatescr();
        SDL_Delay( 30);
    }
   
    clearscr();
}

void clearscr() {
    SDL_Rect rect;

    rect.x = 0;
    rect.y = 0;
    rect.w = WIDTH;
    rect.h = HEIGHT;

    SDL_FillRect( screen, &rect, SDL_MapRGB( screen->format, 0, 0, 0));
}

void updatescr() {
    SDL_UpdateRect( screen, 0, 0, 0, 0);
    SDL_Flip( screen);
}

