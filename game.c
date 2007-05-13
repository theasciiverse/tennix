#include <stdio.h>
#include <math.h>

#include <SDL/SDL.h>

#include "tennix.h"
#include "game.h"
#include "graphics.h"
#include "input.h"
#include "sound.h"

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

void game() {
    int i;
    float phase;
    int ground_state = 0;
   
    Ball ball = { GAME_X_MID, GAME_Y_MID, BALL_DEFAULT_SPEED, 0.2, 7.5 };
    Ball ground = { 0, 0, 0, 0, 0 };
    Player player1 = { GAME_X_MIN-4, GAME_Y_MID, 0, 0, 1 };
    Player player2 = { GAME_X_MAX, GAME_Y_MID, 0, 0, 0 };
   
    Uint8 *keys;
    SDL_Event e;
   
    clearscr();
    updatescr();
    wait_keypress();

    sound_audience();
   
    while( 1) {
        clearscr();
        show_image( GR_COURT, 0, 0, 255);
        show_image( GR_SHADOW, ball.x, ball.y, 255);
        
        show_sprite( GR_RACKET, (!player1.state), 4, player1.x, player1.y, 255);
        show_sprite( GR_RACKET, (!player2.state)+2, 4, player2.x, player2.y, 255);
        
        phase = 2*1.6*ball.jump*get_phase( ball.x, GAME_X_MIN, GAME_X_MAX, ball.move_x);
        if( phase < 1.0) {
            if( !ground_state) {
                sound_ground();
                /* after the ball hit the ground, responsibilities change */
                player1.responsible = !(player2.responsible = !player2.responsible);
            }
            ground_state = 1;
            ground.jump = 3;
            ground.x = ball.x;
            ground.y = ball.y;
        } else {
            ground_state = 0;
            if( ground.jump && !(i%5)) ground.jump--;
        }

        if( ground.jump) {
            show_sprite( GR_GROUND, ground.jump-1, 3, ground.x, ground.y, 128);
        }

        if( ball.move_x > 0) {
            show_sprite( GR_BALL, (i/5)%4, 4, ball.x, ball.y-phase, 255);
        } else {
            show_sprite( GR_BALL, 3-(i/5)%4, 4, ball.x, ball.y-phase, 255);
        }
        
        show_digit( player1.score/10%10, 140*2, 14, 100);
        show_digit( player1.score%10, 148*2, 14, 100);
        show_digit( 10, 156*2, 14, 100);
        show_digit( player2.score/10%10, 164*2, 14, 100);
        show_digit( player2.score%10, 172*2, 14, 100);
        
        updatescr();
        
        if( IS_OUT_X(ball.x) || IS_OUT_Y(ball.y)) {
            if( (IS_OUT_Y(ball.y) && player1.responsible) || (!(player1.state && IS_NEAR_Y(player1.y, ball.y+phase)) && ball.x < GAME_X_MIN)) {
                player2.score++;
                sound_applause();
                player2.responsible = !(player1.responsible = 1);
                ball.x = GAME_X_MID;
                ball.y = GAME_Y_MID;
                ball.move_x = fabsf( ball.move_x);
                limit_value( &ball.move_x, ball.move_x, BALL_DEFAULT_SPEED);
                ball.move_y = -0.1;
                wait_keypress();
                sound_applause_stop();
            }
        
            if( (IS_OUT_Y(ball.y) && player2.responsible) || (!(player2.state && IS_NEAR_Y(player2.y, ball.y+phase)) && ball.x > GAME_X_MAX)) {
                player1.score++;
                sound_applause();
                player1.responsible = !(player2.responsible = 1);
                ball.x = GAME_X_MID;
                ball.y = GAME_Y_MID;
                ball.move_x = -fabsf( ball.move_x);
                limit_value( &ball.move_x, -BALL_DEFAULT_SPEED, ball.move_x);
                ball.move_y = 0.1;
                wait_keypress();
                sound_applause_stop();
            }
        
            if( IS_OUT_X(ball.x)) {
                if( ball.move_x < 0) {
                    ball.x = GAME_X_MIN;
                    if( player1.state == PLAYER_STATE_MAX) {
                        ball.move_x = PLAYER_POWERSHOT;
                    } else {
                        ball.move_x = 2.5 + 2.0*player1.state/PLAYER_STATE_MAX;
                    }
                    ball.move_y = get_move_y( player1.y, ball.y-phase, PLAYER_AREA, ball.move_x, phase);
                    ball.jump += 1.0-2.0*(player1.state<5);
                    sound_racket( player1.state == PLAYER_STATE_MAX);
                } else {
                    ball.x = GAME_X_MAX;
                    if( player2.state == PLAYER_STATE_MAX) {
                        ball.move_x = -PLAYER_POWERSHOT;
                    } else {
                        ball.move_x = -(2.5 + 2.0*player2.state/PLAYER_STATE_MAX);
                    }
                    ball.move_y = get_move_y( player2.y, ball.y-phase, PLAYER_AREA, ball.move_x, phase);
                    ball.jump += 1.0-2.0*(player2.state<5);
                    sound_racket( player2.state == PLAYER_STATE_MAX);
                }
            }
        }
        
        ball.x += ball.move_x*2;
        ball.y += ball.move_y*2;
        
        if( player1.state) player1.state--;
        if( player2.state) player2.state--;

        SDL_PollEvent( &e);
        keys = SDL_GetKeyState( NULL);
        
        if( keys['2']) player1.y-=8;
        if( keys['w']) player1.y-=6;
        if( keys['s']) player1.y+=6;
        if( keys['x']) player1.y+=8;
        if( keys['d'] && !player1.state) player1.state = PLAYER_STATE_MAX;
        
        if( keys['9']) player2.y-=8;
        if( keys['o']) player2.y-=6;
        if( keys['l']) player2.y+=6;
        if( keys['.']) player2.y+=8;
        if( keys['k'] && !player2.state) player2.state = PLAYER_STATE_MAX;
        
        if( keys['f']) SDL_WM_ToggleFullScreen( screen);
         
        if( keys[SDLK_ESCAPE] || keys['q']) break;
        
        limit_value( &player1.y, PLAYER_Y_MIN, PLAYER_Y_MAX);
        limit_value( &player2.y, PLAYER_Y_MIN, PLAYER_Y_MAX);
        limit_value( &ball.jump, BALL_JUMP_MIN, BALL_JUMP_MAX);
        
        
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

float get_phase( float x, float min, float max, float direction) {
    float pos, fract;
    
    pos = (direction>0)?(1-GROUND_PHASE):(GROUND_PHASE);

    fract = (x-min)/(max-min);

    if( fract < pos) {
        fract = fract/pos;
        return fabsf( cosf(PI*fract/2));
    } else {
        fract = (pos-fract)/(1-pos);
        return fabsf( sinf(PI*fract/2));
    }
}

float get_move_y( float py, float by, float pa, float move_x, float phase) {
    float pct, dest, x_len, y_len;

    /* 0.0 .. 1.0 for racket hit position */
    pct = ((py+pa)-by)/(2*pa);
    limit_value( &pct, 0.0, 1.0);

    /* Y destination for ball */
    dest = GAME_Y_MIN + pct*(GAME_Y_MAX-GAME_Y_MIN);

    /* lengths for the ball's journey */
    x_len = GAME_X_MAX - GAME_X_MIN;
    y_len = dest - py + phase;

    /* return the should-be value for move_y */
    return (y_len*move_x)/(x_len);
}

