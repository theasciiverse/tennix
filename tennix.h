
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

#define M_POS_XPOS(x) (x>WIDTH-185)

#define M_POS_START_GAME(x,y)  (M_POS_XPOS(x) && y>150 && y<150+62)
#define M_POS_START_MULTI(x,y) (M_POS_XPOS(x) && y>230 && y<230+62)
#define M_POS_QUIT(x,y)        (M_POS_XPOS(x) && y>350 && y<350+62)


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
