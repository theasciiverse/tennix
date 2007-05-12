#include <stdio.h>
#include <time.h>
#include <SDL/SDL.h>

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
    int sdl_flags = SDL_HWSURFACE;

    srand( (unsigned)time( NULL));
   
    if( SDL_Init( SDL_INIT_VIDEO) == -1) {
        fprintf( stderr, "Can't init SDL:  %s\n", SDL_GetError());
        exit( 1);
    }

    init_sound();
   
    atexit( SDL_Quit);

#ifdef WIN32
    sdl_flags = SDL_SWSURFACE;
#endif
   
    if( (screen = SDL_SetVideoMode( WIDTH, HEIGHT, 16, sdl_flags)) == NULL) {
        fprintf( stderr, "Can't set video mode: %s\n", SDL_GetError());
        exit( 1);
    }   

    SDL_WM_SetCaption( "Tennix! SDL", "Tennix");
    SDL_ShowCursor( SDL_DISABLE);
    SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, 1);
   
    introimage( "data/title.bmp");
   
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
        show_bmp( "data/menu.bmp", 180, 0, 255);
        if( i/12%4 == 0) {
            sound_racket( 0);
        }
        show_sprite( "data/animation.bmp", (i++/12%4), 4, 20, 30, 255);
        show_sprite( RACKET_BMP, (mb&SDL_BUTTON( SDL_BUTTON_LEFT)) > 0, 4, mx, my, 255);
        updatescr();
        
        if( mb & SDL_BUTTON( SDL_BUTTON_LEFT)) {
            if( M_POS_START_GAME(mx,my)) {
                game();
                SDL_Delay( 50);
                while( SDL_PollEvent( &e));
            }   
            if( M_POS_HIGH_SCORES(mx,my)) {
                introimage( "data/hiscores.bmp");
            }
            if( M_POS_CREDITS(mx,my)) {
                introimage( "data/credits.bmp");
            }
            if( M_POS_QUIT(mx,my)) {
                break;
            }
        }
        SDL_Delay( 10);
    }
   
    SDL_Quit();
    return 0;
}

