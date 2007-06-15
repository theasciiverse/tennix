
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

#include <stdio.h>
#include <math.h>

#include "tennix.h"
#include "game.h"
#include "graphics.h"
#include "input.h"
#include "sound.h"

void game( bool singleplayer) {
    GameState s = {
        { GAME_X_MID, GAME_Y_MID, BALL_DEFAULT_SPEED, 0.2, 7.5 },
        { 0, 0, 0, 0, 0 },
        { GAME_X_MIN-RACKET_X_MID*2, GAME_Y_MID, 0, 0, 1, DESIRE_NORMAL, PLAYER_TYPE_HUMAN },
        { GAME_X_MAX+RACKET_X_MID*2, GAME_Y_MID, 0, 0, 0, DESIRE_NORMAL, PLAYER_TYPE_HUMAN },
        0,
        0,
        0,
    };

    Uint32 ot = SDL_GetTicks();
    Uint32 nt;
    Uint32 dt = GAME_TICKS;
    Uint32 diff;
    Uint32 accumulator = 0;
    bool quit = false;

    if( singleplayer) {
        s.player1.type = PLAYER_TYPE_AI;
        s.player2.type = PLAYER_TYPE_AI;
    }

    sound_audience();
   
    while( !quit) {
        nt = SDL_GetTicks();
        diff = nt-ot;
        if( diff > 2000) {
            diff = 0;
        }

        accumulator += diff;
        ot = nt;

        while( accumulator >= dt) {
            quit = step( &s);
            s.time += dt;
            accumulator -= dt;

            if( s.was_stopped) {
                ot = SDL_GetTicks();
                s.was_stopped = false;
            }
        }

        render( &s);
    }
   
}

bool step( GameState* s) {
    Uint8 *keys;
    SDL_Event e;
   
    if( get_phase( s) < 1.0) {
        if( !s->ground.jump) {
            sound_ground();

            if( IS_OUT_Y( s->ball.y)) {
                /* out - responsibilities stay the same */
                sound_out();
            } else {
                /* not out - responsibilities change */
                s->player1.responsible = !(s->player2.responsible = !s->player2.responsible);
            }
        }
        s->ground.jump = 3;
        s->ground.x = s->ball.x;
        s->ground.y = s->ball.y;
    } else {
        if( s->ground.jump && !(s->time%5)) s->ground.jump--;
    }

    if( IS_OUT_X(s->ball.x) || IS_OFFSCREEN_Y(s->ball.y)) {
        if( IS_OFFSCREEN( s->ball.x, s->ball.y) && s->player1.responsible) {
            s->player2.score++;
            sound_applause();
            s->player2.responsible = !(s->player1.responsible = 1);
            s->ball.x = GAME_X_MID;
            s->ball.y = GAME_Y_MID;
            s->ball.move_x = fabsf( s->ball.move_x);
            s->player1.ball_dest = 0.0;
            limit_value( &s->ball.move_x, s->ball.move_x, BALL_DEFAULT_SPEED);
            s->ball.move_y = -0.1;
            wait_keypress();
            s->was_stopped = true;
            sound_applause_stop();
        }
    
        if( IS_OFFSCREEN( s->ball.x, s->ball.y) && s->player2.responsible) {
            s->player1.score++;
            sound_applause();
            s->player1.responsible = !(s->player2.responsible = 1);
            s->ball.x = GAME_X_MID;
            s->ball.y = GAME_Y_MID;
            s->ball.move_x = -fabsf( s->ball.move_x);
            s->player2.ball_dest = 0.0;
            limit_value( &s->ball.move_x, -BALL_DEFAULT_SPEED, s->ball.move_x);
            s->ball.move_y = 0.1;
            wait_keypress();
            s->was_stopped = true;
            sound_applause_stop();
        }
    
        if( IS_OUT_X(s->ball.x)) {
            if( s->ball.move_x < 0 && IS_NEAR_X( s->player1.x, s->ball.x) && IS_NEAR_Y( s->player1.y, s->ball.y) && s->player1.state) {
                s->ball.x = GAME_X_MIN;
                if( s->player1.state == PLAYER_STATE_MAX) {
                    s->ball.move_x = PLAYER_POWERSHOT;
                } else {
                    s->ball.move_x = 2.5 + 2.0*s->player1.state/PLAYER_STATE_MAX;
                }
                s->ball.move_y = get_move_y( s, 1);
                s->player2.responsible = !(s->player1.responsible = 1);
                s->ball.jump += 1.0-2.0*(s->player1.state<5);
                sound_racket( s->player1.state == PLAYER_STATE_MAX);
            } else if( IS_NEAR_X( s->player2.x, s->ball.x) && IS_NEAR_Y( s->player2.y, s->ball.y) && s->player2.state) {
                s->ball.x = GAME_X_MAX;
                if( s->player2.state == PLAYER_STATE_MAX) {
                    s->ball.move_x = -PLAYER_POWERSHOT;
                } else {
                    s->ball.move_x = -(2.5 + 2.0*s->player2.state/PLAYER_STATE_MAX);
                }
                s->ball.move_y = get_move_y( s, 2);
                s->player1.responsible = !(s->player2.responsible = 1);
                s->ball.jump += 1.0-2.0*(s->player2.state<5);
                sound_racket( s->player2.state == PLAYER_STATE_MAX);
            }
        }
    }

#ifdef DEBUG
    /* Update ball_dest for debugging purposes */
    if( s->ball.move_x < 0) {
        get_move_y( s, 1);
    } else {
        get_move_y( s, 2);
    }
#endif

    s->ball.x += s->ball.move_x;
    s->ball.y += s->ball.move_y;
    
    if( s->player1.state) s->player1.state--;
    if( s->player2.state) s->player2.state--;

    SDL_PollEvent( &e);
    keys = SDL_GetKeyState( NULL);
    
    if( s->player1.type == PLAYER_TYPE_HUMAN) {
        input_human( &s->player1, keys['w'], keys['s'], keys['d']);
    } else {
        input_ai( &s->player1, &s->ball);
    }

    if( s->player2.type == PLAYER_TYPE_HUMAN) {
        input_human( &s->player2, keys['o'], keys['l'], keys['k']);
    } else {
        input_ai( &s->player2, &s->ball);
    }
    
    if( keys['f']) SDL_WM_ToggleFullScreen( screen);
    if( keys['y']) SDL_SaveBMP( screen, "screenshot.bmp");
     
    if( keys[SDLK_ESCAPE] || keys['q']) return true;
    
    limit_value( &s->player1.y, PLAYER_Y_MIN, PLAYER_Y_MAX);
    limit_value( &s->player2.y, PLAYER_Y_MIN, PLAYER_Y_MAX);
    limit_value( &s->ball.jump, BALL_JUMP_MIN, BALL_JUMP_MAX);
    
    return false;
}

void render( GameState* s) {
    clearscr();
    show_image( GR_COURT, 0, 0, 255);
    show_image( GR_SHADOW, s->ball.x-BALL_X_MID, s->ball.y + get_phase( s) - BALL_Y_MID, 255);
    
    show_sprite( GR_RACKET, (!s->player1.state), 4, s->player1.x-RACKET_X_MID, s->player1.y-RACKET_Y_MID, 255);
    show_sprite( GR_RACKET, (!s->player2.state)+2, 4, s->player2.x-RACKET_X_MID, s->player2.y-RACKET_Y_MID, 255);
    
    if( s->ground.jump) {
        show_sprite( GR_GROUND, s->ground.jump-1, 3, s->ground.x - BALL_X_MID, s->ground.y - BALL_Y_MID, 128);
    }

    if( s->ball.move_x > 0) {
        show_sprite( GR_BALL, (s->time/1000)%4, 4, s->ball.x-BALL_X_MID, s->ball.y-BALL_Y_MID, 255);
    } else {
        show_sprite( GR_BALL, 3-(s->time/1000)%4, 4, s->ball.x-BALL_X_MID, s->ball.y-BALL_Y_MID, 255);
    }
    
    show_digit( s->player1.score/10%10, 140*2, 14, 100);
    show_digit( s->player1.score%10, 148*2, 14, 100);
    show_digit( 10, 156*2, 14, 100);
    show_digit( s->player2.score/10%10, 164*2, 14, 100);
    show_digit( s->player2.score%10, 172*2, 14, 100);

#ifdef DEBUG
    line_horiz( s->player1.y, 255, 0, 0);
    line_horiz( s->player2.y, 0, 0, 255);
    line_horiz( s->ball.y, 0, 255, 0);

    line_vert( s->player1.x, 255, 0, 0);
    line_vert( s->player2.x, 0, 0, 255);
    line_vert( s->ball.x, 0, 255, 0);

    line_horiz( s->player1.ball_dest, 255, 0, 255);
    line_horiz( s->player2.ball_dest, 0, 255, 255);

    line_horiz( GAME_Y_MIN, 100, 100, 100);
    line_horiz( GAME_Y_MAX, 100, 100, 100);
#endif

    updatescr();
}

void limit_value( float* value, float min, float max) {
    if( *value < min) {
        *value = min;
    } else if( *value > max) {
        *value = max;
    }
}

float get_phase( GameState* s) {
    float pos, fract;
    float x, min, max, direction;

    x = s->ball.x;
    min = GAME_X_MIN;
    max = GAME_X_MAX;
    direction = s->ball.move_x;

    pos = (direction>0)?(1-GROUND_PHASE):(GROUND_PHASE);

    fract = (x-min)/(max-min);

    if( fract < pos) {
        fract = fract/pos;
        return fabsf( cosf(PI*fract/2))*PHASE_AMP*s->ball.jump;
    } else {
        fract = (pos-fract)/(1-pos);
        return fabsf( sinf(PI*fract/2))*PHASE_AMP*s->ball.jump;
    }
}

float get_move_y( GameState* s, unsigned char player) {
    float pct, dest, x_len, y_len;
    float py, by, pa, move_x;

    py = (player==1)?(s->player1.y):(s->player2.y);
    by = s->ball.y;
    pa = RACKET_Y_MID*2;
    move_x = s->ball.move_x;

    /* -1.0 .. 1.0 for racket hit position */
    pct = (by-py)/(pa/2);
    limit_value( &pct, -1.0, 1.0);

    /* Y destination for ball */
    dest = GAME_Y_MID + pct*(GAME_Y_MAX-GAME_Y_MIN);
    if( player == 1) {
        s->player1.ball_dest = dest;
    } else {
        s->player2.ball_dest = dest;
    }

    /* lengths for the ball's journey */
    if( player == 1) {
        x_len = fabsf(GAME_X_MAX - s->ball.x);
        y_len = dest - by;
    } else {
        x_len = s->ball.x - GAME_X_MIN;
        y_len = by - dest;
    }

    /* return the should-be value for move_y */
    return (y_len*move_x)/(x_len);
}

void input_human( Player* player, bool up, bool down, bool hit) {
    if( up) {
        player->y -= 6;
    }

    if( down) {
        player->y += 6;
    }

    if( hit && !player->state) {
        player->state = PLAYER_STATE_MAX;
    }
}

void input_ai( Player* player, Ball* ball) {
    float fact;

    if( player->responsible) {
        if( fabsf( player->y - ball->y) > RACKET_Y_MID*5) {
            fact = 4;
        } else {
            fact = 1.5;
        }

        if( player->desire == DESIRE_NORMAL && !IS_NEAR_Y_AI( player->y, ball->y)) {
            if( player->y < ball->y) {
                player->y += fmin( 2*fact, ball->y - player->y);
            } else if( player->y > ball->y) {
                player->y -= fmin( 2*fact, player->y - ball->y);
            }
        }

        if( IS_NEAR_X_AI( player->x, ball->x) && !player->state && rand()%4) {
            player->state = PLAYER_STATE_MAX;
        }
    }
}

