
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
#include <time.h>
#include <libgen.h>

#include "tennix.h"
#include "game.h"
#include "graphics.h"
#include "sound.h"

SDL_Surface *screen;

int main( int argc, char** argv) {
    int i = 0;
    int mx, my;
    Uint8 *keys;
    Uint8 mb;
    SDL_Event e;
    SDL_VideoInfo* vi;
    int sdl_flags = SDL_SWSURFACE | SDL_DOUBLEBUF;
    char datadir[MAXPATHLEN];
    int highlight_y = 0;
    char copyright_line[100];
    int copyright_line_width;

    Point elektrons[ELEKTRONS];
    int el;

    fprintf( stderr, "Tennix %s\n%s\n%s\n\n", VERSION, COPYRIGHT, URL);

    sprintf( copyright_line, "Tennix %s -- %s", VERSION, COPYRIGHT);

    srand( (unsigned)time( NULL));

    for( el=0; el<ELEKTRONS; el++) {
        elektrons[el].x = elektrons[el].y = 0;
        elektrons[el].w = 50+(rand()%120);
        elektrons[el].h = 10+(rand()%10);
        elektrons[el].current_x = WIDTH/2;
        elektrons[el].current_y = HEIGHT/2;
        elektrons[el].new_x = rand()%WIDTH;
        elektrons[el].new_y = rand()%HEIGHT;
        elektrons[el].phase = 4*2*PI*((rand()%1000)/1000.0);
        elektrons[el].r = 100+rand()%155;
        elektrons[el].g = 100+rand()%155;
        elektrons[el].b = 0;
        elektrons[el].size = 0;
        elektrons[el].new_size = 5+rand()%10;
    }
   
    if( SDL_Init( SDL_INIT_VIDEO) == -1) {
        fprintf( stderr, "Can't init SDL:  %s\n", SDL_GetError());
        exit( 1);
    }

    atexit( SDL_Quit);

    vi = (SDL_VideoInfo*)SDL_GetVideoInfo();
   
    if( (screen = SDL_SetVideoMode( WIDTH, HEIGHT, vi->vfmt->BitsPerPixel, sdl_flags)) == NULL) {
        fprintf( stderr, "Can't set video mode: %s\n", SDL_GetError());
        exit( 1);
    }   

    SDL_WM_SetCaption( "Tennix! SDL", "Tennix");
    SDL_ShowCursor( SDL_DISABLE);
    SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, 1);

    strcpy( datadir, argv[0]);
    strcpy( (char*)(datadir+strlen( dirname( datadir))), DATADIR);

    init_sound( datadir);
    init_graphics( datadir);

    copyright_line_width = font_get_string_width( GR_DKC2_FONT, copyright_line);
   
    while( 1) {
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
   
        clearscr();
        if( highlight_y) {
            for( el=0; el<ELEKTRONS; el++) {
                elektrons[el].new_x = WIDTH/2 - ELEKTRONS_WIDTH;
                elektrons[el].new_y = highlight_y;
                if( elektrons[el].new_size >= 5) {
                    elektrons[el].new_size = 1+rand()%4;
                }
            }
        }
        
        for( el=0; el<ELEKTRONS; el++) {
            elektrons[el].current_x += (1.0*(elektrons[el].new_x-elektrons[el].current_x)/ELEKTRON_LAZYNESS);
            elektrons[el].current_y += (1.0*(elektrons[el].new_y-elektrons[el].current_y)/ELEKTRON_LAZYNESS);
            if( i%4==0 && elektrons[el].size < elektrons[el].new_size) {
                elektrons[el].size++;
            }
            if( i%4==0 && elektrons[el].size > elektrons[el].new_size) {
                elektrons[el].size--;
            }
            elektrons[el].phase += 0.02+(rand()%20/100.0);
            elektrons[el].x = elektrons[el].current_x + ELEKTRONS_WIDTH + elektrons[el].w*cosf(elektrons[el].phase/4);
            elektrons[el].y = elektrons[el].current_y + elektrons[el].h*sinf(elektrons[el].phase);
        }

        if( M_POS_START_GAME(mx,my)) {
            highlight_y = HIGHLIGHT_START_GAME;
        } else if( M_POS_START_MULTI(mx,my)) {
            highlight_y = HIGHLIGHT_START_MULTI;
        /*} else if( M_POS_CREDITS(mx,my)) {
            highlight_y = HIGHLIGHT_CREDITS;
        */} else if( M_POS_QUIT(mx,my)) {
            highlight_y = HIGHLIGHT_QUIT;
        } else if( highlight_y == 0) {
            if( i%20 == 0) {
                for( el=0; el<ELEKTRONS; el++) {
                    elektrons[el].new_size = 5+rand()%10;
                }
            }
        } else {
            for( el=0; el<ELEKTRONS; el++) {
                elektrons[el].new_x = rand()%(WIDTH-ELEKTRONS_WIDTH-elektrons[el].w/2);
                elektrons[el].new_y = rand()%HEIGHT;
                elektrons[el].new_size = 5+rand()%10;
            }
            highlight_y = 0;
        }

        for( el=0; el<ELEKTRONS; el++) {
            if( elektrons[el].size > 0) {
                rectangle( elektrons[el].x, elektrons[el].y, elektrons[el].size, elektrons[el].size, elektrons[el].r, elektrons[el].g, elektrons[el].b);
                if( elektrons[el].size > 4) {
                    if( highlight_y != 0 || rand()%10 == 0) {
                        draw_line_faded( elektrons[el].x+elektrons[el].size/2, elektrons[el].y+elektrons[el].size/2, mx+15, my+24, elektrons[el].r/3, elektrons[el].g/3, 0, 0 ,0 ,0);
                    }
                    rectangle( elektrons[el].x+2, elektrons[el].y+2, elektrons[el].size-4, elektrons[el].size-4, 0, 0, 0);
                }
            }
        }

        show_image( GR_MENU, WIDTH/2-get_image_width( GR_MENU)/2, 0, 255);
        font_draw_string( GR_DKC2_FONT, copyright_line, (WIDTH-copyright_line_width)/2, HEIGHT-35, i, ((i/150)%ANIMATION_COUNT));
        show_sprite( GR_RACKET, (mb&SDL_BUTTON( SDL_BUTTON_LEFT)) > 0, 4, mx, my, 255);
        updatescr();

        if( mb & SDL_BUTTON( SDL_BUTTON_LEFT)) {
            if( M_POS_START_GAME(mx,my)) {
                start_fade();
                game( true);
                SDL_Delay( 150);
                start_fade();
                while( SDL_PollEvent( &e));
                fade_out( CH_AUDIENCE);
            }   
            if( M_POS_START_MULTI(mx,my)) {
                start_fade();
                game( false);
                SDL_Delay( 150);
                start_fade();
                while( SDL_PollEvent( &e));
                fade_out( CH_AUDIENCE);
            }
            if( M_POS_CREDITS(mx,my)) {
                //introimage( "data/credits.bmp");
            }
            if( M_POS_QUIT(mx,my)) {
                break;
            }
        }
        i++;
        SDL_Delay( 10);
    }

    start_fade();
    while( is_fading()) {
        clearscr();
        updatescr();
        SDL_Delay( 10);
    }
   
    uninit_graphics();

    SDL_Quit();
    return 0;
}

