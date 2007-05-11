#include <SDL/SDL.h>

#include "input.h"

void wait_keypress() {
    SDL_Event e;
    unsigned char done = 0;

    SDL_Delay( 100);

    /* clean up all events */
    while( SDL_PollEvent( &e));

    while( !done) {
        if( SDL_PollEvent( &e)) {
            done = (e.type == SDL_KEYUP || e.type == SDL_MOUSEBUTTONUP);
        } else {
            SDL_Delay( 50);
        }
    }
}

