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

    srand( (unsigned)time( NULL));
   
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
   
    while( 1) {
        SDL_PollEvent( &e);
        keys = SDL_GetKeyState( NULL);
        mb = SDL_GetMouseState( &mx, &my);
   
        if( keys[SDLK_ESCAPE] || keys['q']) {
            break;
        }

        if( keys['f']) {
            SDL_WM_ToggleFullScreen( screen);
        }
   
        clearscr();
        show_image( GR_MENU, 180*2, 0, 255);
        if( i%(12*4) == 0) {
            sound_racket( 0);
        }
        show_sprite( GR_ANIMATION, (i++/12%4), 4, 20*2, 30*2, 255);
        show_sprite( GR_RACKET, (mb&SDL_BUTTON( SDL_BUTTON_LEFT)) > 0, 4, mx, my, 255);
        updatescr();
        
        if( mb & SDL_BUTTON( SDL_BUTTON_LEFT)) {
            if( M_POS_START_GAME(mx,my)) {
                game();
                SDL_Delay( 50);
                while( SDL_PollEvent( &e));
            }   
            if( M_POS_HIGH_SCORES(mx,my)) {
                //introimage( "data/hiscores.bmp");
            }
            if( M_POS_CREDITS(mx,my)) {
                //introimage( "data/credits.bmp");
            }
            if( M_POS_QUIT(mx,my)) {
                break;
            }
        }
        SDL_Delay( 10);
    }
   
    uninit_graphics();

    SDL_Quit();
    return 0;
}

