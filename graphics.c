
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
#include <math.h>

#include "tennix.h"
#include "graphics.h"
#include "input.h"

static Image* images;

static SDL_Surface *buffer;

static Uint32 fading_start = 0;

static const char* filenames[] = {
    "court.png",
    "shadow.png",
    "player-racket.png",
    "ground.png",
    "ball.png",
    "score.png",
    "menu.png",
    "animation.png",
    "smallish-font.png"
};

static const char* smallish_font_chars = " !\"#----()'+,-./0123456789:;<=>?@abcdefghijklmnopqrstuvwxyz";

void init_graphics( const char *data_dir) {
    int i;
    SDL_Surface* data;
    SDL_Surface* tmp;
    char temp[MAXPATHLEN];

#ifndef MACOSX
    strcpy( temp, data_dir);
    strcat( temp, "icon.png");
    tmp = IMG_Load( temp);
    if( tmp != NULL) {
        SDL_WM_SetIcon( tmp, NULL);
        SDL_FreeSurface( tmp);
    }
#endif

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

    buffer = SDL_CreateRGBSurface( SDL_HWSURFACE, WIDTH, HEIGHT, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);

    if( buffer == NULL) {
        fprintf( stderr, "Cannot create buffer surface: %s\n", SDL_GetError());
    }
}

void uninit_graphics() {
    int i;

    for( i=0; i<GR_COUNT; i++) {
        SDL_FreeSurface( images[i].data);
    }

    if( buffer != NULL) {
        SDL_FreeSurface( buffer);
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
    int ticks = SDL_GetTicks();

    if( ticks < fading_start+FADE_DURATION) {
        SDL_SetAlpha( buffer, SDL_SRCALPHA | SDL_RLEACCEL, 255-255*(ticks-fading_start)/FADE_DURATION);
        SDL_BlitSurface( buffer, NULL, screen, NULL);
    }

    SDL_UpdateRect( screen, 0, 0, 0, 0);
    SDL_Flip( screen);
}

void start_fade() {
    SDL_BlitSurface( screen, NULL, buffer, NULL);
    fading_start = SDL_GetTicks();
}

int is_fading() {
    return SDL_GetTicks() < fading_start+FADE_DURATION;
}

int font_get_metrics( unsigned int id, char ch, int* xp, int* wp) {
    SDL_Surface *bitmap;
    SDL_PixelFormat *fmt;
    int pos, x = -1, w = 0;
    int search_pos = 0, search_x = 0;
    Uint8 red, green, blue;
    Uint32 pixel;

    if( id > GR_COUNT) return 0;

    ch = tolower( ch);

    if( ch == ' ') {
        w = 5;
    } else {
        for( pos = 0; pos < strlen( smallish_font_chars); pos++) {
            if( smallish_font_chars[pos] == ch)
                break;
        }

        if( pos == strlen( smallish_font_chars)) {
            return 0;
        }

        bitmap = images[id].data;
        fmt = bitmap->format;

        SDL_LockSurface( bitmap);
        while( search_x < bitmap->w) {
            pixel = *( (Uint32*)(bitmap->pixels + search_x*fmt->BytesPerPixel));
            red = (Uint8)(((pixel & fmt->Rmask) >> fmt->Rshift) << fmt->Rloss);
            green = (Uint8)(((pixel & fmt->Gmask) >> fmt->Gshift) << fmt->Gloss);
            blue = (Uint8)(((pixel & fmt->Bmask) >> fmt->Bshift) << fmt->Bloss);

            /* Increase pos counter if we have a "marker" pixel (255,0,255) */
            if( red == 255 && green == 0 && blue == 255) {
                search_pos++;
                if( search_pos == pos) {
                    x = search_x;
                } else if( search_pos == pos + 1) {
                    w = search_x - x;
                    break;
                }
            }

            search_x++;
        }
        SDL_UnlockSurface( bitmap);
    }

    if( wp != NULL) (*wp) = w;
    if( xp != NULL) (*xp) = x;

    return w;
}

int font_draw_char( unsigned int id, char ch, int x_offset, int y_offset) {
    SDL_Surface *bitmap;
    SDL_Rect src, dst;
    int x = -1, w = 0;

    font_get_metrics( id, ch, &x, &w);
    if( x == -1) return w;

    bitmap = images[id].data;

    dst.w = src.w = w;
    dst.h = src.h = bitmap->h - 1;
    src.x = x;
    src.y = 1;
    dst.x = x_offset;
    dst.y = y_offset;

    SDL_BlitSurface( bitmap, &src, screen, &dst);

    return src.w;
}

void font_draw_string( unsigned int id, char* s, int x_offset, int y_offset, int start, int animation) {
    int y = y_offset;
    int x = x_offset;
    int i, xw;

    if( animation & ANIMATION_SPACING) {
        xw = (int)fabsf((15.0*sinf( start/10.0 + x_offset/30.0)));
        x -= xw;
    }

    if( animation & ANIMATION_SKIP_TWO) {
        x -= (strlen(s)*(int)(2.0*sinf( start / 10.0)))/2;
    }

    for( i=0; i<strlen(s); i++) {
        if( animation & ANIMATION_WAVE) {
            y = y_offset + (int)(3.0*sinf( start/10.0 + x/30.0));
        }
        x += font_draw_char( id, s[i], x, y);
        if( animation & ANIMATION_SKIP_TWO) {
            x += (int)(2.0*sinf( start / 10.0));
        }
        if( animation & ANIMATION_SPACING) {
            x += (int)(xw/strlen(s));
        }
    }
}

int font_get_string_width( unsigned int id, char* s) {
    int w = 0, i;

    for( i=0; i<strlen(s); i++) {
        w += font_get_metrics( id, s[i], NULL, NULL);
    }

    return w;
}

