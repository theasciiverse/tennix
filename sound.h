#ifndef __SOUND_H
#define __SOUND_H

#include "SDL_mixer.h"

#define FADE_OUT_MS 1000

enum {
    CH_RACKET = 0,
    CH_GROUND,
    CH_AUDIENCE,
    CH_APPLAUSE,
    CH_MAX,
};

void init_sound( const char *);
void play_sample( const char*, unsigned char, int);
void fade_out( unsigned char);

void sound_ground();
void sound_racket( unsigned char);
void sound_audience();
void sound_applause();
void sound_applause_stop();

#endif
