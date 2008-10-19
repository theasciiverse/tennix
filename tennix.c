
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
    int i, slide, slide_direction = -1;
    int ticks;
    int mx, my;
    Uint8 *keys;
    Uint8 mb;
    SDL_Event e;
    int sdl_flags = SDL_SWSURFACE;
    int copyright_line_width;
    int btn_offset;
    int btn_hovering = 0;
    int slide_start;

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

    play_sample_background(SOUND_BACKGROUND);

    copyright_line_width = font_get_string_width( GR_DKC2_FONT, COPYRIGHT);

    clear_screen();
    start_fade();

    i = 0;
    ticks = SDL_GetTicks();
    slide_start = get_image_width(GR_SIDEBAR);
    slide = slide_start;
    while( 1) {
        if (SDL_GetTicks() > ticks + 20) {
            if (slide > 0 && slide <= slide_start) {
                slide += slide_direction*(1+slide/25);
            } else if (slide >= slide_start) {
                slide = slide_start;
                slide_direction = 0;
            } else {
                slide = 0;
                slide_direction = 1;
            }
            ticks = SDL_GetTicks();
        }
        clear_screen();
        btn_offset = slide/7;
        show_image(GR_SIDEBAR, WIDTH-get_image_width(GR_SIDEBAR)+slide, 0, 255);
        show_image(GR_TENNIXLOGO, WIDTH-get_image_width(GR_SIDEBAR)-10+slide/2, 20, 255);
        show_image(GR_BTN_PLAY, WIDTH-get_image_width(GR_BTN_PLAY)+slide+btn_offset+3-(3*(btn_hovering==1)), 150, 255);
        show_image(GR_BTN_OPTIONS, WIDTH-get_image_width(GR_BTN_OPTIONS)+slide+btn_offset+3-(3*(btn_hovering==2)), 230, 255);
        show_image(GR_BTN_QUIT, WIDTH-get_image_width(GR_BTN_QUIT)+slide+btn_offset+3-(3*(btn_hovering==3)), 350, 255);

        SDL_PollEvent( &e);
        if( e.type == SDL_QUIT) {
            break;
        }
        keys = SDL_GetKeyState( NULL);
        mb = SDL_GetMouseState( &mx, &my);
   
        if( keys[SDLK_ESCAPE] || keys['q']) {
            break;
        }

        if( keys['f']) {
            SDL_WM_ToggleFullScreen( screen);
        }
   
        if( M_POS_START_GAME(mx,my)) {
            if (btn_hovering != 1) play_sample(SOUND_MOUSEOVER);
            btn_hovering = 1;
        } else if( M_POS_START_MULTI(mx,my)) {
            if (btn_hovering != 2) play_sample(SOUND_MOUSEOVER);
            btn_hovering = 2;
        } else if( M_POS_QUIT(mx,my)) {
            if (btn_hovering != 3) play_sample(SOUND_MOUSEOVER);
            btn_hovering = 3;
        } else {
            btn_hovering = 0;
        }

        font_draw_string( GR_SMALLISH_FONT, COPYRIGHT, (WIDTH-copyright_line_width)/2-80, HEIGHT-35, i*3, (((i*3)/150)%ANIMATION_COUNT));
#ifndef MAEMO /* No mouse cursor on Maemo (we have a touchscreen) */
        show_sprite( GR_RACKET, ((mb&SDL_BUTTON( SDL_BUTTON_LEFT))>0)+(((mb&SDL_BUTTON( SDL_BUTTON_RIGHT))>0)*2), 4, mx, my, 255);
#endif
        font_draw_string( GR_SMALLISH_FONT, "FYI: Currently, the options menu is the two-player mode :)", 10, 10, 0, 0);

#ifdef DEBUG
        /* Draw the "real" mouse coordinates */
        rectangle(mx-1, my-1, 2, 2, 255, 255, 255);
#endif
        store_screen();
        updatescr();

        if( mb & SDL_BUTTON( SDL_BUTTON_LEFT)) {
            play_sample(SOUND_MOUSECLICK);
            if( M_POS_START_GAME(mx,my)) {
                stop_sample(SOUND_BACKGROUND);
                start_fade();
                game( true);
                SDL_Delay( 150);
                start_fade();
                while( SDL_PollEvent( &e));
                play_sample_background(SOUND_BACKGROUND);
                slide = slide_start;
                slide_direction = -1;
            }   
            if( M_POS_START_MULTI(mx,my)) {
                stop_sample(SOUND_BACKGROUND);
                start_fade();
                game( false);
                SDL_Delay( 150);
                start_fade();
                while( SDL_PollEvent( &e));
                play_sample_background(SOUND_BACKGROUND);
            }
            if( M_POS_QUIT(mx,my)) {
                stop_sample(SOUND_BACKGROUND);
                break;
            }
        }
        i++;
        SDL_Delay(10);
    }

    start_fade();
    store_screen();
    while( is_fading()) {
        clear_screen();
        updatescr();
        SDL_Delay( 10);
    }
   
    uninit_graphics();
    uninit_joystick();

    SDL_Quit();
    return 0;
}

