
/**
 *
 * Tennix! SDL Port
 * Copyright (C) 2003, 2007, 2008 Thomas Perl <thp@perli.net>
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

#ifndef __TENNIX_H
#define __TENNIX_H

#include <sys/param.h>

#include "SDL.h"


#define COPYRIGHT "Copyright 2003, 2007, 2008 Thomas Perl"
#define URL "http://icculus.org/tennix/"

#define WIDTH 640
#define HEIGHT 480

#define MENU_BUTTON_HEIGHT 62
#define MENU_BUTTON_WIDTH 185

#define MENU_START_YPOS 150
#define MENU_RESUME_YPOS 230
#define MENU_QUIT_YPOS 350

#define M_POS_XPOS(x)    (x>WIDTH-MENU_BUTTON_WIDTH)
#define M_POS_START(y)   (y>MENU_START_YPOS && y<MENU_START_YPOS+MENU_BUTTON_HEIGHT)
#define M_POS_RESUME(y) (y>MENU_RESUME_YPOS && y<MENU_RESUME_YPOS+MENU_BUTTON_HEIGHT)
#define M_POS_QUIT(y)    (y>MENU_QUIT_YPOS && y<MENU_QUIT_YPOS+MENU_BUTTON_HEIGHT)

enum {
    MENU_NONE = 0,
    MENU_START = 1,
    MENU_RESUME = 2,
    MENU_QUIT = 4
};

#define M_POS_DECODE(x,y) (M_POS_XPOS(x)?((M_POS_START(y))?(MENU_START):((M_POS_QUIT(y))?(MENU_QUIT):((M_POS_RESUME(y))?(MENU_RESUME):(MENU_NONE)))):(MENU_NONE))


/* Comment the following #define to disable FPS limiting */
#define ENABLE_FPS_LIMIT
#define DEFAULT_FPS 33


extern SDL_Surface *screen;

typedef struct {
    const char* data;
    unsigned int size;
} ResourceData;

#define RESOURCE(x) ((ResourceData){ x, sizeof( x) })

typedef struct {
    int x;
    int y;
    float current_x;
    float current_y;
    int new_x;
    int new_y;
    int w;
    int h;
    float phase;
    int r;
    int g;
    int b;
    int size;
    int new_size;
} Point;

#endif
