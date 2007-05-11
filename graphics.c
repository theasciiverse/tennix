#include <stdio.h>

#include <SDL/SDL.h>

#include "tennix.h"
#include "graphics.h"
#include "input.h"

void show_sprite( const char* name, int pos, int items, int x_offset, int y_offset, int opacity) {
    SDL_Surface *bitmap;
    SDL_Rect src, dst;

    bitmap = SDL_LoadBMP( name);
    if( bitmap == NULL) {
        fprintf( stderr, "Error: %s\n", SDL_GetError());
        return;
    }

    SDL_SetAlpha( bitmap, SDL_SRCALPHA | SDL_RLEACCEL, opacity);
    SDL_SetColorKey( bitmap, SDL_SRCCOLORKEY, SDL_MapRGB( bitmap->format, 0, 0, 0)); 

    dst.w = src.w = bitmap->w/items;
    dst.h = src.h = bitmap->h;
    src.x = src.w*pos;
    src.y = 0;
    dst.x = x_offset;
    dst.y = y_offset;

    SDL_BlitSurface( bitmap, &src, screen, &dst);
    SDL_FreeSurface( bitmap);
}

void show_bmp( const char *name, int x_offset, int y_offset, int opacity) {
    show_sprite( name, 0, 1, x_offset, y_offset, opacity);
}

void show_digit( int zahl, int x_offset, int y_offset, int opacity) {
    show_sprite( "data/score.bmp", zahl, 11, x_offset, y_offset, opacity);
}

void introimage( char *name) {
    int i;
   
    for( i=0; i<256; i+=10) {
        clearscr();
        show_bmp( name, 0, 0, i);
        updatescr();
        SDL_Delay( 30);
    }
   
    //SDL_Delay( 500);
    wait_keypress();
   
    for( i=255; i>=0; i-=10) {
        clearscr();
        show_bmp( name, 0, 0, i);
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

