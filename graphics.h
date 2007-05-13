#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include <SDL/SDL.h>

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

void init_graphics();
void uninit_graphics();
void show_sprite( unsigned int, int, int, int, int, int);
void show_image( unsigned int, int, int, int);
void show_digit( int, int, int, int);
void introimage( unsigned int);
void clearscr();
void updatescr();

#endif

