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

#define GAME_X_MIN 41.0*2
#define GAME_X_MAX 270.0*2
#define GAME_X_MID ((GAME_X_MIN+GAME_X_MAX)/2)

#define GAME_Y_MIN 20.0*2+75
#define GAME_Y_MAX 150.0*2+75
#define GAME_Y_MID ((GAME_Y_MIN+GAME_Y_MAX)/2)

#define PLAYER_Y_MIN 0.0
#define PLAYER_Y_MAX 1.0*HEIGHT

#define IS_OUT_Y(y) (y<GAME_Y_MIN || y>GAME_Y_MAX)
#define IS_OUT_X(x) (x<GAME_X_MIN || x>GAME_X_MAX)

#define PLAYER_AREA 10.0*2
#define IS_NEAR_Y(py,by) (fabsf(py+(PLAYER_AREA/2)-by)<PLAYER_AREA)

#define PLAYER_STATE_MAX 10
#define PLAYER_POWERSHOT 8.2

void game();
void render( GameState*);
bool step( GameState*);
void limit_value( float*, float, float);
float get_phase( GameState*);
float get_move_y( GameState*, unsigned char);

#endif

