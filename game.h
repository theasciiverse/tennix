
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

enum {
    REFEREE_NORMAL,
    REFEREE_PLAYER1,
    REFEREE_OUT,
    REFEREE_PLAYER2
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
    bool responsible; /* responsible for the next fault (if any) */
    unsigned char desire; /* what the player aims to do (0=normal, 1=upper edge, 2=lower edge)*/
    bool type; /* is this player ai-controlled or human? */
    float ball_dest; /* prospective y-position of ball */
} Player;

enum {
    PLAYER_TYPE_HUMAN,
    PLAYER_TYPE_AI,
};

enum {
    DESIRE_NORMAL,
    DESIRE_MAX
};

typedef struct {
    Ball ball;
    Ball ground;
    Player player1;
    Player player2;
    float phase;
    unsigned int time;
    bool was_stopped;
    bool player1_serves;
    char* status;
    char score_str[10];
    unsigned char referee;
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

#define GAME_Y_MIN 155.0
#define GAME_Y_MAX 330.0
#define GAME_Y_MID ((GAME_Y_MIN+GAME_Y_MAX)/2)

#define GAME_EDGE_AREA 20.0
#define GAME_EDGE_UPPER GAME_Y_MIN+GAME_EDGE_AREA
#define GAME_EDGE_LOWER GAME_Y_MAX-GAME_EDGE_AREA

#define PLAYER_Y_MIN 0.0
#define PLAYER_Y_MAX 1.0*HEIGHT

#define IS_OFFSCREEN_Y(y) (((y)<0-BALL_Y_MID*2) || ((y)>HEIGHT+BALL_Y_MID*2))
#define IS_OFFSCREEN_X(x) (((x)<0-BALL_X_MID*2) || ((x)>WIDTH+BALL_X_MID*2))
#define IS_OFFSCREEN(x,y) ((IS_OFFSCREEN_X(x)) || (IS_OFFSCREEN_Y(y)))
#define IS_OUT_Y(y) (y<GAME_Y_MIN || y>GAME_Y_MAX)
#define IS_OUT_X(x) (x<GAME_X_MIN || x>GAME_X_MAX)

#define PLAYER_AREA_Y RACKET_Y_MID
#define PLAYER_AREA_X RACKET_X_MID
#define IS_NEAR_Y(py,by) (fabsf(py-by)<PLAYER_AREA_Y)
#define IS_NEAR_Y_AI(py,by) (fabsf(py-by)<PLAYER_AREA_Y/2.5)
#define IS_NEAR_X(px,bx) (fabsf(px-bx)<PLAYER_AREA_X)
#define IS_NEAR_X_AI(px,bx) (fabsf(px-bx)<PLAYER_AREA_X*2)

#define PLAYER_STATE_MAX 5
#define PLAYER_POWERSHOT 6.2

#define MOVE_Y_SEED 3

void game( bool);
void render( GameState*);
bool step( GameState*);
void limit_value( float*, float, float);
float get_phase( GameState*);
float get_move_y( GameState*, unsigned char);
void input_human( Player*, bool, bool, bool);
void input_ai( Player*, Ball*, Player*);
void game_setup_serve( GameState*);

#endif

