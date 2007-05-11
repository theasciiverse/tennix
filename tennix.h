#ifndef __TENNIX_H
#define __TENNIX_H

#include <SDL/SDL.h>

#define WIDTH 320
#define HEIGHT 200

#define SPONSOR_BMP "data/sponsor.bmp"
#define SPONSOR_COUNT 6

#define RACKET_BMP "data/player-racket.bmp"

#define M_POS_XPOS(x) (x>170 && x<280)

#define M_POS_START_GAME(x,y)  (M_POS_XPOS(x) && y>60 && y<70)
#define M_POS_HIGH_SCORES(x,y) (M_POS_XPOS(x) && y>80 && y<90)
#define M_POS_CREDITS(x,y)     (M_POS_XPOS(x) && y>100 && y<115)
#define M_POS_QUIT(x,y)        (M_POS_XPOS(x) && y>140 && y<150)

extern SDL_Surface *screen;

#endif
