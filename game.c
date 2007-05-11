#include <stdio.h>
#include <math.h>

#include <SDL/SDL.h>

#include "tennix.h"
#include "game.h"
#include "graphics.h"
#include "input.h"

typedef struct {
    float x;
    float y;
    float move_x;
    float move_y;
} Ball;

typedef struct {
    float x;
    float y;
    unsigned char state;
    unsigned int score;
} Player;

void game() {
    int i, w;
    int werbung[4];
   
    Ball ball = { GAME_X_MID, GAME_Y_MID, 2.0, 0.2 };
    Player player1 = { GAME_X_MIN-4, GAME_Y_MID, 0, 0 };
    Player player2 = { GAME_X_MAX, GAME_Y_MID, 0, 0 };
   
    Uint8 *keys;
    SDL_Event e;
   
    for( w=0; w<4; w++) {
        werbung[w] = rand()%SPONSOR_COUNT;
    }
   
    clearscr();
    updatescr();
    wait_keypress();
   
    while( 1) {
     clearscr();
     show_bmp( "data/court.bmp", 0, 0, 255);
   
     for( w=0; w<4; w++) {
         show_sprite( SPONSOR_BMP, werbung[w], SPONSOR_COUNT, 60+50*w+5*(w>1), 7, 255);
     }
   
     show_sprite( RACKET_BMP, (!player1.state), 4, player1.x, player1.y, 255);
     show_sprite( RACKET_BMP, (!player2.state)+2, 4, player2.x, player2.y, 255);
   
     show_bmp( "data/ball.bmp", ball.x, ball.y, 255);
   
     show_bmp( "data/standings.bmp", 115, 160, 255);
     show_digit( player1.score/10%10, 140, 180, 100);
     show_digit( player1.score%10, 148, 180, 100);
     show_digit( 10, 156, 180, 100);
     show_digit( player2.score/10%10, 164, 180, 100);
     show_digit( player2.score%10, 172, 180, 100);
   
     updatescr();
   
     if( i%2) {
         if( IS_OUT_X(ball.x) || IS_OUT_Y(ball.y)) {
             if( (IS_OUT_Y(ball.y) && ball.move_y > 0) || (!(player1.state && IS_NEAR_Y(player1.y, ball.y)) && ball.x < GAME_X_MIN)) {
                 player2.score++;
                 ball.x = GAME_X_MID;
                 ball.y = GAME_Y_MID;
                 ball.move_x = fabsf( ball.move_x);
                 ball.move_y = -0.1;
                 wait_keypress();
             }

             if( (IS_OUT_Y(ball.y) && ball.move_y < 0) || (!(player2.state && IS_NEAR_Y(player2.y, ball.y)) && ball.x > GAME_X_MAX)) {
                 player1.score++;
                 ball.x = GAME_X_MID;
                 ball.y = GAME_Y_MID;
                 ball.move_x = -fabsf( ball.move_x);
                 ball.move_y = 0.1;
                 wait_keypress();
             }

             if( IS_OUT_X(ball.x)) {
                 if( ball.move_x < 0) {
                     ball.x = GAME_X_MIN;
                     ball.move_x = (2.0+(2.0*(rand()%10))/10.0);
                     ball.move_y = GET_MOVE(player1.y,ball.y);
                 } else {
                     ball.x = GAME_X_MAX;
                     ball.move_x = -(2.0+(2.0*(rand()%10))/10.0);
                     ball.move_y = GET_MOVE(player2.y,ball.y);
                 }
             }
         }

         ball.x += ball.move_x;
         ball.y += ball.move_y;

         if( player1.state) player1.state--;
         if( player2.state) player2.state--;
     }
   
     limit_value( &player1.y, GAME_Y_MIN, GAME_Y_MAX);
     limit_value( &player2.y, GAME_Y_MIN, GAME_Y_MAX);
    
     SDL_PollEvent( &e);
     keys = SDL_GetKeyState( NULL);
   
     if( keys['2']) player1.y-=3;
     if( keys['w']) player1.y-=2;
     if( keys['s']) player1.y+=2;
     if( keys['x']) player1.y+=3;
     if( keys['d']) player1.state = 10;
   
     if( keys['9']) player2.y-=3;
     if( keys['o']) player2.y-=2;
     if( keys['l']) player2.y+=2;
     if( keys['.']) player2.y+=3;
     if( keys['k']) player2.state = 10;

     if( keys['f']) SDL_WM_ToggleFullScreen( screen);
      
     if( keys[SDLK_ESCAPE] || keys['q']) break;
   
   
     SDL_Delay( 10);
     i++;
    }
   
}

void limit_value( float* value, float min, float max) {
    if( *value < min) {
        *value = min;
    } else if( *value > max) {
        *value = max;
    }
}

