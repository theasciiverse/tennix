#ifndef __GAME_H
#define __GAME_H

#include <math.h>

#define GAME_X_MIN 41.0
#define GAME_X_MAX 270.0
#define GAME_X_MID ((GAME_X_MIN+GAME_X_MAX)/2)

#define GAME_Y_MIN 20.0
#define GAME_Y_MAX 150.0
#define GAME_Y_MID ((GAME_Y_MIN+GAME_Y_MAX)/2)

#define IS_OUT_Y(y) (y<GAME_Y_MIN || y>GAME_Y_MAX)
#define IS_OUT_X(x) (x<GAME_X_MIN || x>GAME_X_MAX)

#define PLAYER_AREA 10.0
#define IS_NEAR_Y(py,by) (fabsf(py+(PLAYER_AREA/2)-by)<PLAYER_AREA)
#define GET_MOVE(py,by) (-(py+(PLAYER_AREA/2)-by)/4)

void game();
void limit_value( float*, float, float);

#endif

