
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

#ifndef __TENNIX_H
#define __TENNIX_H

#include <sys/param.h>

#include "SDL.h"


#define VERSION "SVN $Rev$"
#define COPYRIGHT "Copyright (c) 2003, 2007 Thomas Perl"
#define URL "http://thpinfo.com/2007/tennix/"


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
