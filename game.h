
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

#ifndef __GAME_H
#define __GAME_H

#include <math.h>
#include "tennix.h"

typedef unsigned char bool;
enum {
    false,
    true
};

typedef struct {
    float x;
    float y;
    float move_x;
    float move_y;
    float jump;
} Ball;

typedef struct {
    float x;
    float y;
    unsigned char state;
    unsigned int score;
    unsigned char responsible; /* responsible for the next fault (if any) */
} Player;

typedef struct {
    Ball ball;
    Ball ground;
    Player player1;
    Player player2;
    float phase;
    unsigned int time;
    bool was_stopped;
} GameState;

#define PI 3.1415

#define GAME_TICKS 15

#define GROUND_PHASE 0.4
#define PHASE_AMP 3.2

#define BALL_JUMP_MIN 4
#define BALL_JUMP_MAX 10
#define BALL_DEFAULT_SPEED 3.0

#define RACKET_X_MID 15
#define RACKET_Y_MID 24

#define BALL_X_MID 9
#define BALL_Y_MID 9

#define GAME_X_MIN 41.0*2
#define GAME_X_MAX 270.0*2
#define GAME_X_MID ((GAME_X_MIN+GAME_X_MAX)/2)

#define GAME_Y_MIN 20.0*2+75
#define GAME_Y_MAX 150.0*2+75
#define GAME_Y_MID ((GAME_Y_MIN+GAME_Y_MAX)/2)

#define PLAYER_Y_MIN 0.0
#define PLAYER_Y_MAX 1.0*HEIGHT

#define IS_OFFSCREEN_Y(y) (((y)<0) || ((y)>HEIGHT))
#define IS_OFFSCREEN_X(x) (((x)<0) || ((x)>WIDTH))
#define IS_OFFSCREEN(x,y) ((IS_OFFSCREEN_X(x)) || (IS_OFFSCREEN_Y(y)))
#define IS_OUT_Y(y) (y<GAME_Y_MIN || y>GAME_Y_MAX)
#define IS_OUT_X(x) (x<GAME_X_MIN || x>GAME_X_MAX)

#define PLAYER_AREA_Y RACKET_Y_MID
#define PLAYER_AREA_X RACKET_X_MID
#define IS_NEAR_Y(py,by) (fabsf(py-by)<PLAYER_AREA_Y)
#define IS_NEAR_X(px,bx) (fabsf(px-bx)<PLAYER_AREA_X)

#define PLAYER_STATE_MAX 5
#define PLAYER_POWERSHOT 8.2

void game();
void render( GameState*);
bool step( GameState*);
void limit_value( float*, float, float);
float get_phase( GameState*);
float get_move_y( GameState*, unsigned char);

#endif

