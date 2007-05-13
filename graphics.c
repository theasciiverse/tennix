#include <stdio.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "tennix.h"
#include "graphics.h"
#include "input.h"

static Image* images;

static const char* filenames[] = {
    "data/court.bmp",
    "data/shadow.png",
    "data/player-racket.bmp",
    "data/ground.png",
    "data/ball.png",
    "data/score.png",
    "data/menu.bmp",
    "data/animation.bmp"
};

void init_graphics() {
    int i;
    SDL_Surface* data;

    images = (Image*)calloc( GR_COUNT, sizeof( Image));

    for( i=0; i<GR_COUNT; i++) {
        data = IMG_Load( filenames[i]);
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
    SDL_SetColorKey( bitmap, SDL_SRCCOLORKEY, SDL_MapRGB( bitmap->format, 0, 0, 0)); 

    dst.w = src.w = bitmap->w/items;
    dst.h = src.h = bitmap->h;
    src.x = src.w*pos;
    src.y = 0;
    dst.x = x_offset;
    dst.y = y_offset;

    SDL_BlitSurface( bitmap, &src, screen, &dst);
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
}

