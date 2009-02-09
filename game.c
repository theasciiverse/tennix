
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "tennix.h"
#include "game.h"
#include "graphics.h"
#include "input.h"
#include "sound.h"


GameState *gamestate_new() {
    int x, y, z;
    GameState *s;

    GameState template = {
        NULL,
        { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.6, false, -1, false },
        {
            { NULL, -1, GAME_X_MIN-RACKET_X_MID*2, GAME_Y_MID, 0, POWER_UP_FACTOR, POWER_DOWN_FACTOR, true, 0, DESIRE_NORMAL, PLAYER_TYPE_AI, GAME_Y_MID, false, 0, {0}, 0, 0, PLAYER_ACCEL_DEFAULT, true },
            { NULL, -1, GAME_X_MAX+RACKET_X_MID*2, GAME_Y_MID, 0, POWER_UP_FACTOR, POWER_DOWN_FACTOR, true, 0, DESIRE_NORMAL, PLAYER_TYPE_AI, GAME_Y_MID, false, 0, {0}, 0, 0, PLAYER_ACCEL_DEFAULT, true },
        },
        0,
        0,
        1,
        "welcome to tennix " VERSION,
        { 0 },
        { 0 },
        REFEREE_NORMAL,
        0,
        WINNER_NONE,
        false,
        GR_COUNT,
        { 0 },
        0,
        false,
        { { { 0 } } },
        0.0,
        0, /* sound events */
        0.0,
        0.0,
        0,
        0,
        0,
        false,
        0,
        0,
        0,
        false,
        REFEREE_COUNT,
        SCORE_UNDECIDED,
        0,
    };

    s = (GameState*)malloc(sizeof(GameState));
    if (s == NULL) abort();

    memcpy(s, &template, sizeof(GameState));

    game_setup_serve(s);

    /* smoothen n-gram */
    for( x = 0; x<NGRAM_STEPS; x++) {
        for( y = 0; y<NGRAM_STEPS; y++) {
            for( z = 0; z<NGRAM_STEPS; z++) {
                s->ngram[x][y][z] = 1;
            }
        }
    }

    return s;
}


void gameloop(GameState *s) {
    Uint32 ot = SDL_GetTicks();
    Uint32 nt;
    Uint32 dt = GAME_TICKS;
    Uint32 diff;
    Uint32 accumulator = 0;
    bool quit = false;

#ifdef ENABLE_FPS_LIMIT
    Uint32 ft, frames; /* frame timer and frames */
#endif

    strcpy(s->game_score_str, format_game(s));
    strcpy(s->sets_score_str, format_sets(s));
    s->text_changed = true;

    if (s->rain > 0) {
        play_sample_background(SOUND_RAIN);
    }
    if (s->location->max_visitors > 100) {
        play_sample_loop(SOUND_AUDIENCE);
    }
    /* Reset the court type, so it is redrawn on first display */
    s->displayed_court_type = GR_COUNT;

#ifdef ENABLE_FPS_LIMIT
    frames = 0;
    ft = SDL_GetTicks();
#endif
    while( !quit) {
        nt = SDL_GetTicks();
        diff = nt-ot;
        if( diff > 2000) {
            diff = 0;
        }

        accumulator += diff;
        ot = nt;

        while( accumulator >= dt) {
            quit = step(s);
            s->time += dt;
            s->windtime += s->wind*dt;
            accumulator -= dt;

            if( s->was_stopped) {
                ot = SDL_GetTicks();
                s->was_stopped = false;
            }
        }
        if (s->timelimit != 0 && s->time >= s->timelimit) {
            quit = 1;
        }

#ifdef ENABLE_FPS_LIMIT
        while (frames*1000.0/((float)(SDL_GetTicks()-ft+1))>(float)(DEFAULT_FPS)) {
            SDL_Delay(10);
        }
        frames++;
#endif

        render(s);
    }

    clear_screen();
    store_screen();

    stop_sample(SOUND_AUDIENCE);
    stop_sample(SOUND_RAIN);
}

bool step( GameState* s) {
    Uint8 *keys;
    SDL_Event e;
    bool ground_event = false;
    int p;

    if (s->ball.z < 0) {
        /* ground event */
        ground_event = true;

        s->referee = REFEREE_NORMAL;

        /* bounce from the ground */
        if (fabsf(s->ball.move_z) > 0.3) {
            s->sound_events |= SOUND_EVENT_GROUND;
            sample_volume_group(SOUND_GROUND_FIRST, SOUND_GROUND_LAST, MAX(0.0, MIN(1.0, fabsf(s->ball.move_z)/2)));
            pan_sample_group(SOUND_GROUND_FIRST, SOUND_GROUND_LAST, MAX(0.0, MIN(1.0, 0.5+0.8*(s->ball.x-GAME_X_MID)/WIDTH)));
            s->ball.move_z *= -s->ball.restitution;
        } else {
            s->ball.move_z = 0;
        }
        s->ball.z = 0;
    }

    if (NET_COLLISION_BALL(s->ball)) {
        /* the net blocks movement of the ball */
        while (NET_COLLISION_BALL(s->ball)) {
            /* make sure the ball appears OUTSIDE of the net */
            if (s->ball.move_x < 0) {
                s->ball.x += 1;
            } else {
                s->ball.x -= 1;
            }
        }
        s->ball.move_x = 0;
        s->ball.move_y = 0;
    }

    /* see if we have something to score */
    if (s->score_event == SCORE_UNDECIDED) {
        if (NET_COLLISION_BALL(s->ball)) {
            /* the ball "fell" into the net  */
            s->score_event = SCORE_EVENT_NET;
            s->sound_events |= SOUND_EVENT_OUT;
            s->status = "net!";
        } else if (IS_OFFSCREEN(s->ball.x, s->ball.y)) {
            /* ball flew offscreen */
            s->score_event = SCORE_EVENT_OFFSCREEN;
            s->sound_events |= SOUND_EVENT_OUT;
            s->status = "out!";
        } else if (ground_event) {
            /* the ball hit the ground on the screen */
            if (IS_OUT(s->ball.x, s->ball.y)) {
                /* the ball bounced in the OUT area */
                s->score_event = SCORE_EVENT_OUT;
                s->sound_events |= SOUND_EVENT_OUT;
                s->status = "out!";
            } else if (GROUND_IS_VALID(s->ball.last_hit_by, s->ball.x, s->ball.y)) {
                if (s->ball.ground_hit) {
                    s->score_event = SCORE_EVENT_GROUND_VALID;
                    s->sound_events |= SOUND_EVENT_OUT;
                    s->status = "did not catch the ball!";
                } else {
                    /* first ground hit in valid area */
                    s->ball.ground_hit = true;
                }
            } else {
                /* ball hit somewhere invalid */
                s->score_event = SCORE_EVENT_GROUND_INVALID;
                s->sound_events |= SOUND_EVENT_OUT;
                s->status = "fault!";
            }
        }
    }

    if (s->score_event != SCORE_UNDECIDED) {
        /* we have some scoring to do */
        if (s->score_time == 0) {
            /* schedule scoring in the future */
            s->score_time = s->time + SCORING_DELAY;
            s->referee = REFEREE_OUT;
        } else if (s->time >= s->score_time) {
            /* time has ran out - score now */
            switch (score_game(s)) {
                case WINNER_PLAYER1:
                    s->status = "player 1 scores";
                    s->referee = REFEREE_PLAYER1;
                    break;
                case WINNER_PLAYER2:
                    s->status = "player 2 scores";
                    s->referee = REFEREE_PLAYER2;
                    break;
                default:
                    assert(0);
                    break;
            }
            s->score_time = 0;

            strcpy(s->game_score_str, format_game(s));
            strcpy(s->sets_score_str, format_sets(s));
            s->text_changed = true;

            game_setup_serve(s);
            if (s->location->max_visitors > 100) {
                s->sound_events |= SOUND_EVENT_APPLAUSE;
            }
            /*FIXME n-gram predictor broken
            s->history_size = 0;
            s->history_is_locked = 0;
            s->ngram_prediction = 0.0;*/
        }
    } else {
        /* score is still undecided - do the racket swing thing */
        for (p=1; p<=2; p++) {
            if (IS_NEAR_X(PLAYER(s, p).x, s->ball.x) && IS_NEAR_Y(PLAYER(s, p).y, s->ball.y-s->ball.z) && PLAYER(s, p).use_power && PLAYER(s, p).power > 30 && s->ball.last_hit_by != p) {
                /* RACKET HIT */
                if (!s->ball.ground_hit && s->ball.move_x != 0.0) {
                    s->status = "volley!";
                } else {
                    s->status = format_status(s);
                }
                switch (PLAYER(s, p).desire) {
                    case DESIRE_NORMAL:
                        /* normal swing */
                        s->ball.move_x = 2.7 + 2.0*PLAYER(s, p).power/PLAYER_POWER_MAX;
                        s->ball.move_z = 1.2*PLAYER(s, p).power/PLAYER_POWER_MAX;
                        break;
                    case DESIRE_TOPSPIN:
                        /* top spin */
                        s->ball.move_x = 1.1 + 2.2*PLAYER(s, p).power/PLAYER_POWER_MAX;
                        s->ball.move_z = 2.5*PLAYER(s, p).power/PLAYER_POWER_MAX;
                        break;
                    case DESIRE_SMASH:
                        /* smash */
                        s->ball.move_x = 4.0 + 3.0*PLAYER(s, p).power/PLAYER_POWER_MAX;
                        s->ball.move_z = 1.1*PLAYER(s, p).power/PLAYER_POWER_MAX;
                        break;
                }
                s->ball.move_y = get_move_y( s, p);
                s->sound_events |= SOUND_EVENT_RACKET;
                s->ball.ground_hit = false;
                s->ball.inhibit_gravity = false;
                s->ball.last_hit_by = p;
                if (p==1) {
                    pan_sample_group(SOUND_RACKET_FIRST, SOUND_RACKET_LAST, 0.3);
                } else {
                    pan_sample_group(SOUND_RACKET_FIRST, SOUND_RACKET_LAST, 0.7);
                    s->ball.move_x *= -1;
                }
            }
        }
    }

#if 0
    FIXME n-gram predictor broken
    if( IS_OUT_X(s->ball.x) || IS_OFFSCREEN_Y(s->ball.y)) {
        if( IS_OUT_X(s->ball.x)) {
            /*if( !s->history_is_locked && s->referee != REFEREE_OUT) {
                s->history[s->history_size] = (int)(NGRAM_STEPS*s->ball.y/HEIGHT);
                s->history_size++;
                if( s->history_size == 3) {
                    s->ngram[s->history[0]][s->history[1]][s->history[2]] += 10;
                    s->ngram_prediction = ngram_predictor( s);
                    s->history[0] = s->history[1];
                    s->history[1] = s->history[2];
                    s->history_size--;
                }
                s->history_is_locked = true;
            }*/
        }
    } else {
        /*s->history_is_locked = false;*/
    }
#endif

    SDL_PumpEvents();
    keys = SDL_GetKeyState(NULL);

    if( s->time%50==0) {
        if (keys['r']) {
            if (s->rain == 0) {
                play_sample_background(SOUND_RAIN);
            }
            s->rain += 10;
        }
        if (keys['t']) {
            s->fog++;
        }
        if (keys['1']) {
            s->wind++;
        }
        if (keys['2']) {
            s->wind--;
        }
    }

    if(!(SDL_GetTicks() < fading_start+FADE_DURATION) && !s->is_over) {
        for (p=1; p<=MAXPLAYERS; p++) {
            if( PLAYER(s, p).type == PLAYER_TYPE_HUMAN) {
                input_human(s, p);
            } else {
                input_ai(s, p);
            }
        }
    }
    
    /* Maemo: The "F6" button is the "Fullscreen" button */
    /*if( keys['f'] || keys[SDLK_F6]) SDL_WM_ToggleFullScreen( screen);*/
    if( keys['y']) SDL_SaveBMP( screen, "screenshot.bmp");

    /* Maemo: The "F4" button is the "Open menu" button */
    if( keys['p'] || keys[SDLK_F4]) {
        while( keys['p'] || keys[SDLK_F4]) {
            SDL_PollEvent( &e);
            keys = SDL_GetKeyState( NULL);
            SDL_Delay( 10);
        }
        while( (keys['p'] || keys[SDLK_F4]) == 0) {
            SDL_PollEvent( &e);
            keys = SDL_GetKeyState( NULL);
            SDL_Delay( 10);
        }
        while( keys['p'] || keys[SDLK_F4]) {
            SDL_PollEvent( &e);
            keys = SDL_GetKeyState( NULL);
            SDL_Delay( 10);
        }
        s->was_stopped = true;
    }
     
    if( keys[SDLK_ESCAPE] || keys['q']) return true;
    
    limit_value( &PLAYER(s, 1).y, PLAYER_Y_MIN, PLAYER_Y_MAX);
    limit_value( &PLAYER(s, 2).y, PLAYER_Y_MIN, PLAYER_Y_MAX);
    
    if (s->ball.move_x > 0 && s->wind > 0) {
        s->ball.x += fabsf(s->wind)/2;
    } else if (s->ball.move_x < 0 && s->wind < 0) {
        s->ball.x -= fabsf(s->wind)/2;
    }

    s->ball.z += s->ball.move_z;
    if (!s->ball.inhibit_gravity) {
        s->ball.move_z += GRAVITY;
    }

    s->ball.x += s->ball.move_x;
    s->ball.y += s->ball.move_y;

    for (p=1; p<=MAXPLAYERS; p++) {
        if (PLAYER(s, p).use_power) {
            PLAYER(s, p).power = MAX(0, MIN(PLAYER(s, p).power*PLAYER(s, p).power_down_factor, PLAYER_POWER_MAX));
        }
    }

    return false;
}

void render( GameState* s) {
    int x, y, b;
    unsigned int i;
#ifdef EXTENDED_REFEREE
    int t=1000;
#endif
    if (s->sound_events != 0) {
        if (s->sound_events & SOUND_EVENT_GROUND) {
            play_sample(SOUND_GROUND);
        }
        if (s->sound_events & SOUND_EVENT_OUT) {
            play_sample(SOUND_OUT);
        }
        if (s->sound_events & SOUND_EVENT_APPLAUSE) {
            play_sample(SOUND_APPLAUSE);
        }
        if (s->sound_events & SOUND_EVENT_RACKET) {
            play_sample(SOUND_RACKET);
        }
        s->sound_events = 0;
    }
    if( s->winner != WINNER_NONE) {
        if( !s->is_over) {
            start_fade();
            s->is_over = true;
        }
        clear_screen();
        store_screen();
        show_sprite( GR_RACKET, 2*(s->winner-1), 4, WIDTH/2 - get_image_width( GR_RACKET)/8, HEIGHT/2 - get_image_height( GR_RACKET), 255);
        sprintf( s->game_score_str, "player %d wins the match with %s", s->winner, format_sets( s));
        font_draw_string( GR_DKC2_FONT, s->game_score_str, (WIDTH-font_get_string_width( GR_DKC2_FONT, s->game_score_str))/2, HEIGHT/2 + 30, s->time/20, ANIMATION_WAVE | ANIMATION_BUNGEE);
        updatescr();
        return;
    }
    if (s->displayed_court_type != s->location->court_type || s->text_changed || s->old_referee != s->referee) {
        clear_screen();
        fill_image(s->location->court_type, 120, 120, 400, 250);
        show_image(GR_COURT, 0, 0, 255);
        font_draw_string( GR_DKC2_FONT, s->game_score_str, 14, 14, 0, ANIMATION_NONE);
        font_draw_string( GR_DKC2_FONT, s->sets_score_str, (WIDTH-font_get_string_width( GR_DKC2_FONT, s->sets_score_str))-14, 14, 0, ANIMATION_NONE);
        if (s->location->has_referee) {
#ifdef EXTENDED_REFEREE
            switch (s->referee) {
                case REFEREE_NORMAL:
                    t = 1000;
                    break;
                case REFEREE_OUT:
                    t = 200;
                    break;
                case REFEREE_PLAYER1:
                case REFEREE_PLAYER2:
                    t = 400;
                    break;
            }
            t = (s->time/t)%4;
            switch (t) {
                case 0:
                    t=0;
                    break;
                case 1:
                    t=1;
                    break;
                case 2:
                    t=0;
                    break;
                case 3:
                    t=2;
                    break;
            }
            show_sprite( GR_REFEREE, s->referee*3+t, 12, 250, 10, 255);
            if (voice_finished_flag == 0) {
                show_sprite(GR_TALK, (s->time/150)%2, 2, 280, 45, 255);
            }
#else
            show_sprite( GR_REFEREE, s->referee, 4, 250, 10, 255);
#endif
        }
        s->displayed_court_type = s->location->court_type;
        s->text_changed = false;
        s->old_referee = s->referee;
        store_screen();
    }
    
   /* show_sprite( GR_RACKET, (!PLAYER(s, 1).state), 4, PLAYER(s, 1).x-RACKET_X_MID, PLAYER(s, 1).y-RACKET_Y_MID, 255);
    show_sprite( GR_RACKET, (!PLAYER(s, 2).state)+2, 4, PLAYER(s, 2).x-RACKET_X_MID, PLAYER(s, 2).y-RACKET_Y_MID, 255);*/
    show_sprite( GR_RACKET, !PLAYER(s, 1).use_power, 4, PLAYER(s, 1).x-RACKET_X_MID, PLAYER(s, 1).y-RACKET_Y_MID, 255);
    show_sprite( GR_RACKET, !PLAYER(s, 2).use_power + 2, 4, PLAYER(s, 2).x-RACKET_X_MID, PLAYER(s, 2).y-RACKET_Y_MID, 255);
    
    rectangle(10, HEIGHT-30, PLAYER_POWER_MAX, 10, 50, 50, 50);
    rectangle(WIDTH-PLAYER_POWER_MAX-10, HEIGHT-30, PLAYER_POWER_MAX, 10, 50, 50, 50);

    rectangle(10, HEIGHT-30, PLAYER(s, 1).power, 10, 200, 200, 200);
    rectangle(WIDTH-PLAYER_POWER_MAX-10, HEIGHT-30, PLAYER(s, 2).power, 10, 200, 200, 200);
    
    if( s->ball.move_x > 0) {
        b = (s->time/100)%BALL_STATES;
    } else if( s->ball.move_x < 0) {
        b = BALL_STATES-1-(s->time/100)%BALL_STATES;
    } else {
        b = 0;
    }
    show_image(GR_SHADOW,                s->ball.x-BALL_X_MID, s->ball.y-4, 255);
    show_sprite(GR_BALL, b, BALL_STATES, s->ball.x-BALL_X_MID, s->ball.y-BALL_Y_MID-s->ball.z, 255);

    /* Player 1's mouse rectangle */
    if (!(PLAYER(s, 1).mouse_locked)) {
        rectangle(PLAYER(s, 1).x-2+5, PLAYER(s, 1).mouse_y-2, 4, 4, 255, 255, 255);
        rectangle(PLAYER(s, 1).x-1+5, PLAYER(s, 1).mouse_y-1, 2, 2, 0, 0, 0);
    }
    

    font_draw_string( GR_DKC2_FONT, s->status, (WIDTH-font_get_string_width( GR_DKC2_FONT, s->status))/2, HEIGHT-50, s->time/30, ANIMATION_WAVE);

    for (i=0; i<s->rain; i++) {
        x = rand()%WIDTH;
        y = rand()%HEIGHT;
        draw_line_faded(x, y, x+10+s->wind*5, y+30, 0, 0, 255, 100, 200, 255);
    }
    if (s->rain) {
        /**
         * Cheap-ish update of the whole screen. This can
         * probably be optimized.
         **/
        update_rect(0, 0, WIDTH, HEIGHT);
    }

#ifdef DEBUG
    line_horiz( PLAYER(s, 1).y, 255, 0, 0);
    line_horiz( PLAYER(s, 2).y, 0, 0, 255);
    line_horiz( s->ball.y, 0, 255, 0);

    line_vert( PLAYER(s, 1).x, 255, 0, 0);
    line_vert( PLAYER(s, 2).x, 0, 0, 255);
    line_vert( s->ball.x, 0, 255, 0);

    line_horiz( GAME_Y_MIN, 100, 100, 100);
    line_horiz( GAME_Y_MAX, 100, 100, 100);
#endif
    switch (s->fog) {
        default:
        case 4:
            fill_image_offset(GR_FOG2, 0, 0, WIDTH, HEIGHT, -s->time/150-s->windtime/200, 0);
        case 3:
            fill_image_offset(GR_FOG, 0, 0, WIDTH, HEIGHT, -s->time/100-s->windtime/150, 20);
        case 2:
            fill_image_offset(GR_FOG2, 0, 0, WIDTH, HEIGHT, -s->time/180-s->windtime/180, 80);
        case 1:
            fill_image_offset(GR_FOG, 0, 0, WIDTH, HEIGHT, s->time/200-s->windtime/100, 0);
        case 0:
            break;
    }
    if (s->night) {
        show_image(GR_NIGHT, 0, 0, 255);
    }

    updatescr();
}

void limit_value( float* value, float min, float max) {
    if( *value < min) {
        *value = min;
    } else if( *value > max) {
        *value = max;
    }
}

float get_move_y( GameState* s, unsigned char player) {
    float pct, dest, x_len, y_len;
    float py, by, pa, move_x;

    py = (player==1)?(PLAYER(s, 1).y):(PLAYER(s, 2).y);
    by = s->ball.y - s->ball.z;
    pa = RACKET_Y_MID*2;
    move_x = s->ball.move_x;

    /* -1.0 .. 1.0 for racket hit position */
    pct = MAX(-1.0, MIN(1.0, (by-py)/(pa/2)));

    /* Y destination for ball */
    dest = GAME_Y_MID + pct*(GAME_Y_MAX-GAME_Y_MIN);

    /* lengths for the ball's journey */
    if( player == 1) {
        x_len = GAME_X_MAX - s->ball.x;
    } else {
        x_len = s->ball.x - GAME_X_MIN;
    }
    y_len = dest - by + MOVE_Y_SEED-rand()%MOVE_Y_SEED*2;

    /* return the should-be value for move_y */
    return (y_len*move_x)/(x_len);
}

void input_human(GameState* s, int player) {
    bool hit, topspin, smash;
    float move_y;

    move_y = PLAYER_MOVE_Y*input_device_get_axis(PLAYER(s, player).input, INPUT_AXIS_Y);

    hit = input_device_get_key(PLAYER(s, player).input, INPUT_KEY_HIT);
    topspin = input_device_get_key(PLAYER(s, player).input, INPUT_KEY_TOPSPIN);
    smash = input_device_get_key(PLAYER(s, player).input, INPUT_KEY_SMASH);

    if (move_y != 0) {
        if (fabsf(move_y) > fabsf(move_y*PLAYER(s, player).accelerate)) {
            move_y *= PLAYER(s, player).accelerate;
        }
        PLAYER(s, player).y += move_y;
        PLAYER(s, player).accelerate *= PLAYER_ACCEL_INCREASE;
    } else {
        PLAYER(s, player).accelerate = PLAYER_ACCEL_DEFAULT;
    }

    if(hit || topspin || smash) {
        PLAYER(s, player).desire = (topspin)?(DESIRE_TOPSPIN):(DESIRE_NORMAL);
        PLAYER(s, player).desire = (smash)?(DESIRE_SMASH):(PLAYER(s, player).desire);

        PLAYER(s, player).power = MAX(10, MIN(PLAYER(s, player).power*PLAYER(s, player).power_up_factor, PLAYER_POWER_MAX));
        PLAYER(s, player).use_power = false;
    } else {
        PLAYER(s, player).use_power = true;
    }
}

void input_ai(GameState* s, int player) {
    float fact = 1.7;
    float target;
    int ball_approaching = 0;

    if ((PLAYER(s, player).x < GAME_X_MID && s->ball.move_x <= 0) ||
        (PLAYER(s, player).x > GAME_X_MID && s->ball.move_x >= 0)) {
        ball_approaching = 1;
    }

    /* FIXME - this is broken since the new physics model has been introduced */

    if( fabsf( PLAYER(s, player).y - (s->ball.y-s->ball.z)) > RACKET_Y_MID*5) {
        fact = 3.5;
    }

    if(1) {
        if( PLAYER(s, player).desire == DESIRE_NORMAL && !IS_NEAR_Y_AI(PLAYER(s, player).y, (s->ball.y-s->ball.z)) && ball_approaching) {
            if( PLAYER(s, player).y < (s->ball.y-s->ball.z)) {
                PLAYER(s, player).y += fmin( 2*fact, (s->ball.y-s->ball.z) - PLAYER(s, player).y);
            } else if( PLAYER(s, player).y > (s->ball.y-s->ball.z)) {
                PLAYER(s, player).y -= fmin( 2*fact, PLAYER(s, player).y - (s->ball.y-s->ball.z));
            }
        }

        if (IS_NEAR_Y(PLAYER(s, player).y, (s->ball.y-s->ball.z)) && IS_NEAR_X_AI(PLAYER(s, player).x, s->ball.x) && PLAYER(s, player).power > 90) {
            PLAYER(s, player).use_power = true;
        } else if (ball_approaching) {
            PLAYER(s, player).power = MAX(10, MIN(PLAYER(s, player).power*PLAYER(s, player).power_up_factor, PLAYER_POWER_MAX));
            PLAYER(s, player).use_power = false;
        }
    } else if( s->ngram_prediction > 0.0) {
        target = s->ngram_prediction*((float)HEIGHT)/((float)(NGRAM_STEPS));
        target = GAME_Y_MID + (target-GAME_Y_MID)*1.5;

        if( PLAYER(s, player).desire == DESIRE_NORMAL && !IS_NEAR_Y_AI( PLAYER(s, player).y, target)) {
            if( PLAYER(s, player).y < target) {
                PLAYER(s, player).y += fmin( fact, (target-PLAYER(s, player).y)/40.0);
            } else if( PLAYER(s, player).y > target) {
                PLAYER(s, player).y -= fmin( fact, (PLAYER(s, player).y-target)/40.0);
            }
        }
    }
}

void game_setup_serve( GameState* s) {
    s->ball.z = 10.0;
    s->ball.y = GAME_Y_MID;
    s->ball.move_x = 0.0;
    s->ball.move_y = 0.0;
    s->ball.move_z = 0.0;
    s->ball.last_hit_by = -1;
    s->ball.inhibit_gravity = true;

    if( s->serving_player == 1) {
        s->ball.x = GAME_X_MIN-RACKET_X_MID*1.5;
    } else {
        s->ball.x = GAME_X_MAX+RACKET_X_MID*1.5;
    }
}

float ngram_predictor( GameState* s) {
    unsigned int count = 0;
    unsigned long sum = 0;
    int x, y, z;
    float result;

    if( s->history_size < 3) {
        return 0.0;
    }

    x = s->history[1];
    y = s->history[2];

    for( z = 0; z<NGRAM_STEPS; z++) {
        count += s->ngram[x][y][z];
        sum += z * s->ngram[x][y][z];
    }

    result = ((float)(sum))/((float)(count));
#ifdef DEBUG
    printf( "predicting next = %.2f\n", result);
#endif

    return result;
}

int score_game(GameState* s) {
    Player *last_hit_by = NULL;
    Player *other = NULL;

    Player* winner = NULL;
    Player* loser = NULL;

    /* determine "last hit by" and "other" */
    if (s->ball.last_hit_by == 1) {
        last_hit_by = &(PLAYER(s, 1));
        other = &(PLAYER(s, 2));
    } else {
        last_hit_by = &(PLAYER(s, 2));
        other = &(PLAYER(s, 1));
    }

    switch (s->score_event) {
        case SCORE_EVENT_NET:
            winner = other;
            break;
        case SCORE_EVENT_OUT:
        case SCORE_EVENT_GROUND_INVALID:
        case SCORE_EVENT_OFFSCREEN:
        case SCORE_EVENT_GROUND_VALID:
            if (s->ball.ground_hit) {
                winner = last_hit_by;
            } else {
                winner = other;
            }
            break;
        default:
            break;
    }

    /* determine loser based on winner */
    if (winner == last_hit_by) {
        loser = other;
    } else {
        loser = last_hit_by;
    }

    /* we cannot be in an "impossibly high" set */
    assert(s->current_set < SETS_TO_WIN*2-1);

    winner->game++;
    if( loser->game < winner->game-1) {
        if( winner->game >= 4) {
            winner->game = loser->game = 0;
            winner->sets[s->current_set]++;

            /* serving is changed when the "game" is over */
            s->serving_player = (s->serving_player==1)?(2):(1);

#ifdef HAVE_VOICE_FILES
            /* speak the current score */
            voice_say_list(4, VOICE_ZERO_IN + (PLAYER(s, 1).sets[s->current_set])*2, VOICE_TO, VOICE_ZERO_OUT + (PLAYER(s, 2).sets[s->current_set])*2, VOICE_IN_THE_FIRST_SET+s->current_set);
#endif

            /* scoring the set.. */
            if( (winner->sets[s->current_set] == 6 && loser->sets[s->current_set] < 5) ||
                winner->sets[s->current_set] == 7) {
                s->current_set++;
                s->winner = game_get_winner( s);
            }
        }
    }

    /* forget this event - we've handled it */
    s->score_event = SCORE_UNDECIDED;

    if (winner == &PLAYER(s, 1)) {
        return WINNER_PLAYER1;
    } else {
        return WINNER_PLAYER2;
    }
}

char* format_sets( GameState* s) {
    static char sets[100];
    static char tmp[100];
    int i, max = s->current_set;

    sets[0] = '\0';

    if( s->winner != WINNER_NONE) {
        max--;
    }
    for( i=0; i<=max; i++) {
        sprintf( tmp, "%d:%d, ", PLAYER(s, 1).sets[i], PLAYER(s, 2).sets[i]);
        strcat( sets, tmp);
    }

    sets[strlen(sets)-2] = '\0';

    return sets;
}

char* format_game( GameState* s) {
    static char game[100];
    static const int game_scoring[] = { 0, 15, 30, 40 };

    if( PLAYER(s, 1).game < 4 && PLAYER(s, 2).game < 4) {
#ifdef HAVE_VOICE_FILES
        if (PLAYER(s, 1).game > 0 || PLAYER(s, 2).game > 0) {
            if (PLAYER(s, 1).game == PLAYER(s, 2).game) {
                voice_say_list(2, VOICE_LOVE_IN + 2*(PLAYER(s, 1).game), VOICE_ALL);
            } else {
                voice_say_list(2, VOICE_LOVE_IN + 2*(PLAYER(s, 1).game), VOICE_LOVE_OUT + 2*(PLAYER(s, 2).game));
            }
        }
#endif
        sprintf( game, "%d - %d", game_scoring[PLAYER(s, 1).game], game_scoring[PLAYER(s, 2).game]);
    } else if( PLAYER(s, 1).game > PLAYER(s, 2).game) {
#ifdef HAVE_VOICE_FILES
        voice_say_list(1, VOICE_ADVANTAGE_PLAYER_ONE);
#endif
        strcpy( game, "advantage player 1");
    } else if( PLAYER(s, 1).game < PLAYER(s, 2).game) {
#ifdef HAVE_VOICE_FILES
        voice_say_list(1, VOICE_ADVANTAGE_PLAYER_TWO);
#endif
        strcpy( game, "advantage player 2");
    } else {
#ifdef HAVE_VOICE_FILES
        voice_say_list(1, VOICE_DEUCE);
#endif
        strcpy( game, "deuce");
    }

    return game;
}

char* format_status( GameState* s) {
    static char status[100];
    static const char* set_names[] = { "first", "second", "third", "fourth", "fifth" };

    sprintf( status, "%d:%d in %s set", PLAYER(s, 1).sets[s->current_set], PLAYER(s, 2).sets[s->current_set], set_names[s->current_set]);

    return status;
}

int game_get_winner( GameState* s) {
    unsigned int i;
    int sets[2] = {0};

    for( i=0; i<s->current_set; i++) {
        if( PLAYER(s, 1).sets[i] > PLAYER(s, 2).sets[i]) {
            sets[0]++;
        } else {
            sets[1]++;
        }
    }

    if( sets[0] == SETS_TO_WIN) return WINNER_PLAYER1;
    if( sets[1] == SETS_TO_WIN) return WINNER_PLAYER2;

    return WINNER_NONE;
}

