#ifndef __TENNIX_H
#define __TENNIX_H

#include <sys/param.h>

#include "SDL.h"

#define DATADIR "/data/"
#ifdef MACOSX
#undef DATADIR
#define DATADIR "/../Resources/"
#endif

#define WIDTH 640
#define HEIGHT 480

#define SPONSOR_BMP "data/sponsor.bmp"
#define SPONSOR_COUNT 6

#define RACKET_BMP "data/player-racket.bmp"

#define M_POS_XPOS(x) (x>170*2 && x<280*2)

#define M_POS_START_GAME(x,y)  (M_POS_XPOS(x) && y>60*2 && y<70*2)
#define M_POS_HIGH_SCORES(x,y) (M_POS_XPOS(x) && y>80*2 && y<90*2)
#define M_POS_CREDITS(x,y)     (M_POS_XPOS(x) && y>100*2 && y<115*2)
#define M_POS_QUIT(x,y)        (M_POS_XPOS(x) && y>140*2 && y<150*2)

extern SDL_Surface *screen;

#endif
