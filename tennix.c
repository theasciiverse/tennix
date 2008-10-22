
/**
 *
 * Tennix! SDL Port
 * Copyright (C) 2003, 2007, 2008 Thomas Perl <thp@perli.net>
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
#include <time.h>
#include <libgen.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "tennix.h"
#include "game.h"
#include "graphics.h"
#include "sound.h"
#include "input.h"

SDL_Surface *screen;

#ifdef WIN32

/* IDs from the resource file */
#define START_BUTTON 1
#define CHECKBOX_FULLSCREEN 2
#define QUIT_BUTTON 3

BOOL CALLBACK ConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static int checkbox_is_checked;

    switch (uMsg) {
        case WM_CLOSE:
            EndDialog(hwndDlg, IDCANCEL);
            break;
        case WM_COMMAND:
            switch (wParam) {
                case START_BUTTON:
                    EndDialog(hwndDlg, (checkbox_is_checked)?(IDYES):(IDNO));
                    break;
                case QUIT_BUTTON:
                    EndDialog(hwndDlg, IDCANCEL);
                    break;
                case CHECKBOX_FULLSCREEN:
                    checkbox_is_checked ^= 1;
                    break;
            }
            break;
        default:
            return FALSE;
    }
    return TRUE;
}
#endif

#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, int nCmdShow) {
#else
int main( int argc, char** argv) {
#endif
    int i, slide, slide_direction;
    int ticks;
    int mx, my;
    Uint8 *keys;
    Uint8 mb;
    SDL_Event e;
    int sdl_flags = SDL_SWSURFACE;
    int btn_hovering = 0, btn_hovering_old = 0;
    int slide_start;
    bool mouse_pressed = false;
    bool quit = false;
    GameState *current_game = NULL, *prepared_game = NULL;

    MenuButton btn_back = {
        "Back to main menu",
        MENU_OPTIONS_BORDER,
        HEIGHT-(MENU_OPTIONS_BORDER+MENU_OPTIONS_BUTTON_HEIGHT),
        MENU_OPTIONS_BUTTON_WIDTH, MENU_OPTIONS_BUTTON_HEIGHT,
        255, 0, 0
    };
    MenuButton btn_start = {
        "Start new game",
        WIDTH-(MENU_OPTIONS_BORDER+MENU_OPTIONS_BUTTON_WIDTH),
        HEIGHT-(MENU_OPTIONS_BORDER+MENU_OPTIONS_BUTTON_HEIGHT),
        MENU_OPTIONS_BUTTON_WIDTH, MENU_OPTIONS_BUTTON_HEIGHT,
        0, 255, 0
    };
    MenuButton btn_court_change = {
        "change",
        240,
        40,
        50, MENU_OPTIONS_BUTTON_HEIGHT,
        100, 100, 100
    };

    int state = MENU_STATE_STARTED;

#ifdef ENABLE_FPS_LIMIT
    Uint32 ft, frames; /* frame timer and frames */
#endif

#ifdef MAEMO
    sdl_flags |= SDL_FULLSCREEN;
#endif

#ifdef WIN32
    int mb_result;
    mb_result = DialogBox(hInstance, "CONFIG", 0, (DLGPROC)ConfigDialogProc);

    switch (mb_result) {
        case IDYES:
            sdl_flags |= SDL_FULLSCREEN;
            break;
        case IDCANCEL:
            return 0;
            break;
        default:
            break;
    }
#else
    fprintf( stderr, "Tennix %s\n%s\n%s\n\n", VERSION, COPYRIGHT, URL);

    bool do_help = false;
    i = 1;
    while (i < argc) {
        /* A poor/lazy man's getopt */
        #define OPTION_SET(longopt,shortopt) \
                (strcmp(argv[i], longopt)==0 || strcmp(argv[i], shortopt)==0)
        #define OPTION_VALUE \
                ((i+1 < argc)?(argv[i+1]):(NULL))
        #define OPTION_VALUE_PROCESSED \
                (i++)
        if (OPTION_SET("--fullscreen", "-f")) {
            sdl_flags |= SDL_FULLSCREEN;
        }
        else if (OPTION_SET("--help", "-h")) {
            do_help = true;
        }
        else if (OPTION_SET("--list-joysticks", "-J")) {
            SDL_Init(SDL_INIT_JOYSTICK);
            joystick_list();
            return 0;
        }
        else if (OPTION_SET("--joystick", "-j")) {
            SDL_Init(SDL_INIT_JOYSTICK);
            if (OPTION_VALUE == NULL) {
                fprintf(stderr, "Error: You need to specify the name of the joystick as parameter.\n");
                do_help = true;
                break;
            }
            if (joystick_open(OPTION_VALUE)==0) {
                fprintf(stderr, "Warning: Cannot find joystick \"%s\" - Ignored.\n", OPTION_VALUE);
                break;
            }
            OPTION_VALUE_PROCESSED;
        }
        else {
            fprintf(stderr, "Ignoring unknown option: %s\n", argv[i]);
        }
        i++;
    }

    if (do_help == true) {
        fprintf(stderr, "Usage: %s [--fullscreen|-f] [--help|-h] [--list-joysticks|-J] [--joystick|-j] [joystick name]\n", argv[0]);
        return 0;
    }
#endif

    srand( (unsigned)time( NULL));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == -1) {
        fprintf( stderr, "Can't init SDL:  %s\n", SDL_GetError());
        exit( 1);
    }

    SDL_VideoInfo* vi = (SDL_VideoInfo*)SDL_GetVideoInfo();
    if( (screen = SDL_SetVideoMode( WIDTH, HEIGHT, vi->vfmt->BitsPerPixel, sdl_flags)) == NULL) {
        fprintf( stderr, "Can't set video mode: %s\n", SDL_GetError());
        exit( 1);
    }   

    SDL_WM_SetCaption( "Tennix " VERSION, "Tennix");
    SDL_ShowCursor( SDL_DISABLE);
    SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, 1);

    init_sound();
    init_graphics();
    init_joystick();

#ifdef ENABLE_FPS_LIMIT
    frames = 0;
    ft = SDL_GetTicks();
#endif

    i = 0;
    /* Sliding initialization */
    ticks = SDL_GetTicks();
    slide = slide_start = get_image_width(GR_SIDEBAR);
    slide_direction = 0;
    while(!quit) {
        /* State transitions */
        switch (state) {
            case MENU_STATE_STARTED:
                state = MENU_STATE_SLIDE_TO_MAINMENU;
                break;
            case MENU_STATE_SLIDE_TO_MAINMENU:
                slide = slide_start;
                slide_direction = -1;
                state = MENU_STATE_SLIDE_TO_MAINMENU_IN_PROGRESS;
                break;
            case MENU_STATE_SLIDE_TO_MAINMENU_IN_PROGRESS:
                if (slide == 0) {
                    slide_direction = 0;
                    state = MENU_STATE_MAINMENU;
                }
                break;
            case MENU_STATE_MAINMENU:
                free(prepared_game);
                prepared_game = NULL;
                break;
            case MENU_STATE_SLIDE_TO_OPTIONS:
                slide = 1;
                slide_direction = 3;
                state = MENU_STATE_SLIDE_TO_OPTIONS_IN_PROGRESS;
                break;
            case MENU_STATE_SLIDE_TO_OPTIONS_IN_PROGRESS:
                if (slide == slide_start) {
                    start_fade();
                    state = MENU_STATE_OPTIONS;
                }
                break;
            case MENU_STATE_OPTIONS:
                /* Prepare a new game */
                if (prepared_game == NULL) {
                    prepared_game = gamestate_new();
                }
                break;
            case MENU_STATE_SLIDE_TO_GAME:
                /*slide = 1;
                slide_direction = 2;
                state = MENU_STATE_SLIDE_TO_GAME_IN_PROGRESS;
                break;
            case MENU_STATE_SLIDE_TO_GAME_IN_PROGRESS:
                if (slide == slide_start) {
                    state = MENU_STATE_GAME;
                }*/
                state = MENU_STATE_GAME;
                break;
            case MENU_STATE_SLIDE_TO_RESUME:
                slide = 1;
                slide_direction = 2;
                state = MENU_STATE_SLIDE_TO_RESUME_IN_PROGRESS;
                break;
            case MENU_STATE_SLIDE_TO_RESUME_IN_PROGRESS:
                if (slide == slide_start) {
                    state = MENU_STATE_RESUME;
                }
                break;
            case MENU_STATE_GAME:
                if (prepared_game == NULL) {
                    fprintf(stderr, "Game not yet prepared!\n");
                    exit(EXIT_FAILURE);
                }
                /* Cancel a possibly started game */
                free(current_game);
                current_game = prepared_game;
                prepared_game = NULL;
                /* no break - we are continuing with "resume" */
            case MENU_STATE_RESUME:
                if (current_game == NULL) {
                    fprintf(stderr, "Cannot resume game!\n");
                    exit(EXIT_FAILURE);
                }
                start_fade();
                gameloop(current_game);
                SDL_Delay(150);
                while(SDL_PollEvent(&e));
                start_fade();
                state = MENU_STATE_SLIDE_TO_MAINMENU;
                break;
            case MENU_STATE_SLIDE_TO_QUIT:
                slide = 1;
                slide_direction = 3;
                state = MENU_STATE_SLIDE_TO_QUIT_IN_PROGRESS;
                break;
            case MENU_STATE_SLIDE_TO_QUIT_IN_PROGRESS:
                if (slide == slide_start) {
                    state = MENU_STATE_QUIT;
                }
                break;
            case MENU_STATE_QUIT:
                quit = true;
                break;
            default:
                fprintf(stderr, "State error: %d\n", state);
                exit(EXIT_FAILURE);
        }

        /* Sliding */
        if (SDL_GetTicks() > ticks + 20) {
            if (slide >= 1 && slide <= slide_start) {
                slide += slide_direction+(slide_direction*slide/(sqrt(2*slide)));
                slide = MAX(0, MIN(slide_start, slide));
            } else if (slide_direction != 0) {
                slide_direction = 0;
            }
            ticks = SDL_GetTicks();
        }

        /* Graphics */
        show_image(GR_SIDEBAR, WIDTH-get_image_width(GR_SIDEBAR)+slide, 0, 255);
        show_image(GR_TENNIXLOGO, WIDTH-get_image_width(GR_SIDEBAR)-10, 20-slide, 255);
        if (state != MENU_STATE_OPTIONS) {
            /* Main Menu */
            show_image(GR_BTN_PLAY, WIDTH-get_image_width(GR_BTN_PLAY)+slide+(slide/7)+3-(3*(btn_hovering==MENU_START)), 150, 255);
            if (current_game != NULL) {
                show_image(GR_BTN_RESUME, WIDTH-get_image_width(GR_BTN_RESUME)+slide+(slide/7)+3-(3*(btn_hovering==MENU_RESUME)), 230, 255);
                if (current_game->player2.type == PLAYER_TYPE_AI) {
                    font_draw_string(GR_DKC2_FONT, "Game Against The Computer", 10, 10, 0, 0);
                } else {
                    font_draw_string(GR_DKC2_FONT, "Multiplayer Versus Game", 10, 10, 0, 0);
                }
                font_draw_string(GR_DKC2_FONT, current_game->sets_score_str, 10, 40, 0, 0);
            }
            show_image(GR_BTN_QUIT, WIDTH-get_image_width(GR_BTN_QUIT)+slide+(slide/7)+3-(3*(btn_hovering==MENU_QUIT)), 350, 255);
        } else {
            /* Options screen */
            show_image(GR_TENNIXLOGO, WIDTH-get_image_width(GR_SIDEBAR)-10, 20, 255);
            draw_button_object(btn_back, mx, my);
            draw_button_object(btn_start, mx, my);
            if (prepared_game != NULL) {
                fill_image(prepared_game->court_type, MENU_OPTIONS_BORDER, MENU_OPTIONS_BORDER*2, get_image_width(GR_STADIUM), get_image_height(GR_STADIUM));
                show_image(GR_STADIUM, MENU_OPTIONS_BORDER, MENU_OPTIONS_BORDER*2, 255);
                draw_button_object(btn_court_change, mx, my);
                font_draw_string(GR_DKC2_FONT, "Location", MENU_OPTIONS_BORDER, MENU_OPTIONS_BORDER, 0, 0);
            }
        }

        SDL_PollEvent( &e);
        if( e.type == SDL_QUIT) {
            state = MENU_STATE_SLIDE_TO_QUIT;
            /*break;*/
        }

        keys = SDL_GetKeyState( NULL);
        mb = SDL_GetMouseState( &mx, &my);

        btn_hovering_old = btn_hovering;
        if (state == MENU_STATE_MAINMENU) {
            btn_hovering = M_POS_DECODE(mx, my);
            if (current_game == NULL) {
                btn_hovering &= ~MENU_RESUME;
            }
        } else if (state == MENU_STATE_OPTIONS) {
            if (M_POS_BUTTON(btn_back, mx, my)) {
                btn_hovering = MENU_QUIT;
            } else if (M_POS_BUTTON(btn_start, mx, my)) {
                btn_hovering = MENU_START;
            } else if (M_POS_BUTTON(btn_court_change, mx, my)) {
                btn_hovering = MENU_COURT_CHANGE;
            } else {
                btn_hovering = 0;
            }
        } else {
            /* No menu screen - no hovering. */
            btn_hovering = 0;
        }
        if (btn_hovering_old != btn_hovering && btn_hovering != 0) {
            play_sample(SOUND_MOUSEOVER);
        }
   
        if( keys[SDLK_ESCAPE] || keys['q']) {
            /* FIXME: do the state thingie! */
            break;
        }

        if( keys['f']) {
            SDL_WM_ToggleFullScreen( screen);
        }
   
#ifndef MAEMO /* No mouse cursor on Maemo (we have a touchscreen) */
        if (state == MENU_STATE_MAINMENU || state == MENU_STATE_OPTIONS) {
            show_sprite( GR_RACKET, ((mb&SDL_BUTTON( SDL_BUTTON_LEFT))>0)+(((mb&SDL_BUTTON( SDL_BUTTON_RIGHT))>0)*2), 4, mx, my, 255);
        }
#endif

        /* Draw the "real" mouse coordinates */
        /*rectangle(mx-1, my-1, 2, 2, 255, 255, 255);*/

        /* Store the screen, because we are fading after this screen update */
        /*if (!(mb & SDL_BUTTON(SDL_BUTTON_LEFT)) && btn_hovering != MENU_NONE && mouse_pressed == true) store_screen();*/

        updatescr();

        if( mb & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            mouse_pressed = true;
        } else if (mouse_pressed == true) {
            /* Mouse button released */
            if (state == MENU_STATE_MAINMENU || state == MENU_STATE_OPTIONS) {
                play_sample(SOUND_MOUSECLICK);
            }
            if (state == MENU_STATE_MAINMENU) {
                switch (btn_hovering) {
                    case MENU_START:
                        state = MENU_STATE_SLIDE_TO_OPTIONS;
                        break;
                    case MENU_RESUME:
                        state = MENU_STATE_SLIDE_TO_RESUME;
                        break;
                    case MENU_QUIT:
                        state = MENU_STATE_SLIDE_TO_QUIT;
                        break;
                }
            } else if (state == MENU_STATE_OPTIONS) {
                switch (btn_hovering) {
                    case MENU_START:
                        state = MENU_STATE_SLIDE_TO_GAME;
                        break;
                    case MENU_QUIT:
                        state = MENU_STATE_SLIDE_TO_MAINMENU;
                        break;
                    case MENU_COURT_CHANGE:
                        prepared_game->court_type++;
                        if (prepared_game->court_type > GR_CTT_LAST) {
                            prepared_game->court_type = GR_CTT_FIRST;
                        }
                        break;
                }
            }
            mouse_pressed = false;
        }
        i++;
#ifdef ENABLE_FPS_LIMIT
        while (frames*1000.0/((float)(SDL_GetTicks()-ft+1))>(float)(DEFAULT_FPS)) {
            SDL_Delay(10);
        }
        frames++;
#endif
    }

    uninit_graphics();
    uninit_joystick();

    SDL_Quit();
    return 0;
}

