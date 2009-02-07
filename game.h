
/**
 *
 * Tennix! SDL Port
 * Copyright (C) 2003, 2007, 2008, 2009 Thomas Perl <thp@thpinfo.com>
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
#include "sound.h"

#define SETS_TO_WIN 3

#define NGRAM_STEPS 6

#define MAXPLAYERS 2

#ifdef DELUXE_EDITION
#  define BALL_STATES 16
#else
#  define BALL_STATES 4
#endif

typedef unsigned char bool;
enum {
    false,
    true
};

#ifdef EXTENDED_REFEREE
enum {
    REFEREE_NORMAL,
    REFEREE_OUT,
    REFEREE_PLAYER1,
    REFEREE_PLAYER2,
    REFEREE_COUNT
};
#else
enum {
    REFEREE_NORMAL,
    REFEREE_PLAYER1,
    REFEREE_OUT,
    REFEREE_PLAYER2,
    REFEREE_COUNT
};
#endif

enum {
    WINNER_NONE,
    WINNER_PLAYER1,
    WINNER_PLAYER2
};

typedef struct {
    float x;
    float y;
    float z;
    float move_x;
    float move_y;
    float move_z;
    bool ground_hit;
    int last_hit_by;
    bool inhibit_gravity;
} Ball;

typedef struct {
    float x;
    float y;
    unsigned char state;
    unsigned int score;
    unsigned char desire;
    bool type; /* is this player ai-controlled or human? */
    float ball_dest; /* prospective y-position of ball */
    bool state_locked; /* enabled when user keeps pressing the "hit" key */
    int game; /* score for the current game */
    int sets[SETS_TO_WIN*2]; /* score for each set */
    int mouse_x; /* x position of mouse */
    int mouse_y; /* y position of mouse */
    float accelerate; /* a value [0..1] how fast the user accelerates */
    bool mouse_locked; /* on start, ignore unpressed mouse state */
} Player;

enum {
    PLAYER_TYPE_HUMAN,
    PLAYER_TYPE_AI,
};

enum {
    DESIRE_NORMAL,
    DESIRE_TOPSPIN,
    DESIRE_SMASH,
    DESIRE_MAX
};

/* wait 2 seconds before we score the game */
#define SCORING_DELAY 2000

enum {
    SCORE_UNDECIDED,
    SCORE_EVENT_NET,
    SCORE_EVENT_OUT,
    SCORE_EVENT_GROUND_INVALID,
    SCORE_EVENT_GROUND_VALID,
    SCORE_EVENT_OFFSCREEN,
    SCORE_EVENT_MAX
};

typedef struct {
    Ball ball;
    Player players[MAXPLAYERS];
    unsigned int time;
    bool was_stopped;
    unsigned int serving_player;
    char* status;
    char game_score_str[50];
    char sets_score_str[50];
    unsigned char referee;
    unsigned int current_set;
    int winner;
    bool is_over;
    unsigned int court_type;
    unsigned int old_court_type;
    unsigned int history[3];
    unsigned int history_size;
    bool history_is_locked;
    unsigned char ngram[NGRAM_STEPS][NGRAM_STEPS][NGRAM_STEPS];
    float ngram_prediction;
    sound_id play_sound;
    float joystick_y;
    float joystick_x;
    unsigned char joystick_a;
    unsigned int rain;
    unsigned int fog;
    bool night;
    int wind;
    unsigned int windtime;
    int timelimit;
    bool text_changed;
    unsigned char old_referee;
    int score_event;
    unsigned int score_time;
} GameState;

#define PI 3.1415

#define GAME_TICKS 15

#define GRAVITY -.03
#define RESTITUTION 0.6

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

/* net size and collision detection */
#define NET_X 320
#define NET_Y 107
#define NET_WIDTH 5
#define NET_HEIGHT 273
#define NET_TALLNESS 5
#define IN_NET_X(x) (x+BALL_X_MID >= NET_X && x-BALL_X_MID <= NET_X+NET_WIDTH)
#define IN_NET_Y(y) (y >= NET_Y && y <= NET_Y+NET_HEIGHT)
#define NET_COLLISION(x, y, z) (z<=NET_TALLNESS && IN_NET_X(x) && IN_NET_Y(y))
#define NET_COLLISION_BALL(b) (NET_COLLISION(b.x, b.y, b.z))

/* "in field" coordinates (left and right part of court) */
#define FIELD_LEFT_X 119
#define FIELD_LEFT_WIDTH 202
#define FIELD_LEFT_IS_VALID(x) (x >= FIELD_LEFT_X && x <= FIELD_LEFT_X+FIELD_LEFT_WIDTH)
#define FIELD_RIGHT_X 326
#define FIELD_RIGHT_WIDTH 202
#define FIELD_RIGHT_IS_VALID(x) (x >= FIELD_RIGHT_X && x <= FIELD_RIGHT_X+FIELD_RIGHT_WIDTH)

/* coordinates for one-vs-one game */
#define FIELD_SINGLE_Y 154
#define FIELD_SINGLE_HEIGHT 178
#define FIELD_SINGLE_IS_VALID(y) (y >= FIELD_SINGLE_Y && y <= FIELD_SINGLE_Y+FIELD_SINGLE_HEIGHT)

/* coordinates for two-vs-two game */
#define FIELD_DOUBLE_Y 115
#define FIELD_DOUBLE_HEIGHT 256
#define FIELD_DOUBLE_IS_VALID(y) (y >= FIELD_DOUBLE_Y && y <= FIELD_DOUBLE_Y+FIELD_DOUBLE_HEIGHT)

/* player = player that last hit the ball, */
#define GROUND_IS_VALID(player, x, y) (FIELD_SINGLE_IS_VALID(y) && ((player==1)?(FIELD_RIGHT_IS_VALID(x)):(FIELD_LEFT_IS_VALID(x))))
#define IS_OUT(x, y) (!(FIELD_SINGLE_IS_VALID(y) && (FIELD_RIGHT_IS_VALID(x) || FIELD_LEFT_IS_VALID(x))))

#define PLAYER_Y_MIN 0
#define PLAYER_Y_MAX HEIGHT

#define IS_OFFSCREEN_Y(y) (((y)<0-BALL_Y_MID*2) || ((y)>HEIGHT+BALL_Y_MID*2))
#define IS_OFFSCREEN_X(x) (((x)<0-BALL_X_MID*2) || ((x)>WIDTH+BALL_X_MID*2))
#define IS_OFFSCREEN(x,y) ((IS_OFFSCREEN_X(x)) || (IS_OFFSCREEN_Y(y)))

#define PLAYER_AREA_Y RACKET_Y_MID
#define PLAYER_AREA_X RACKET_X_MID
#define IS_NEAR_Y(py,by) (fabsf((py)-(by))<PLAYER_AREA_Y)
#define IS_NEAR_Y_AI(py,by) (fabsf((py)-(by))<PLAYER_AREA_Y/3.5)
#define IS_NEAR_X(px,bx) (fabsf((px)-(bx))<PLAYER_AREA_X)
#define IS_NEAR_X_AI(px,bx) (fabsf((px)-(bx))<PLAYER_AREA_X*2)

#define PLAYER_MOVE_Y 5.0
#define PLAYER_ACCEL_DEFAULT 0.2
#define PLAYER_ACCEL_INCREASE 1.2
#define PLAYER_STATE_MAX 7
#define PLAYER_POWERSHOT 6.2

#define MOVE_Y_SEED 3

#define PLAYER(s, num) (s->players[num-1])

/* Comment out the following #define to enable mouse control */
#define ENABLE_MOUSE

/* GameState handling*/
GameState *gamestate_new();


/* Game module functions */
void gameloop(GameState*);
void render( GameState*);
bool step( GameState*);
void limit_value( float*, float, float);
float get_move_y( GameState*, unsigned char);
void input_human( Player*, bool, bool, bool, bool, bool, bool, GameState*);
void input_ai( Player*, Ball*, Player*, GameState*);
void game_setup_serve( GameState*);
float ngram_predictor( GameState*);

void score_game(GameState*);
char* format_sets( GameState*);
char* format_game( GameState*);
char* format_status( GameState*);
int game_get_winner( GameState*);

#endif

