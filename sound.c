#include <SDL/SDL_mixer.h>

#include "tennix.h"
#include "sound.h"

void init_sound() {
    if( Mix_OpenAudio( 44100, AUDIO_S16SYS, 2, 1024) < 0) {
        fprintf( stderr, "Error initializing SDL_mixer: %s\n", Mix_GetError());
    }
}

void play_sample( const char* filename, unsigned char channel, int loop) {
    static Mix_Chunk* chunks[CH_MAX] = { 0 };

    if( chunks[channel] != NULL) {
        Mix_FreeChunk( chunks[channel]);
        chunks[channel] = NULL;
    }

    chunks[channel] = Mix_LoadWAV( filename);
    if( chunks[channel] != NULL) {
        Mix_PlayChannel( channel, chunks[channel], loop);
    }
}

void fade_out( unsigned char channel) {
    Mix_FadeOutChannel( channel, FADE_OUT_MS);
}

void sound_ground() {
    const char* sounds[] = { "data/ground1.wav", "data/ground2.wav" };
    play_sample( sounds[rand()%2], CH_GROUND, 0);
}

void sound_racket( unsigned char powershot) {
    const char* sounds[] = { "data/racket1.wav", "data/racket2.wav" };
    if( powershot) {
        play_sample( "data/powershot.wav", CH_RACKET, 0);
    } else {
        play_sample( sounds[rand()%2], CH_RACKET, 0);
    }
}

void sound_audience() {
    play_sample( "data/audience.wav", CH_AUDIENCE, -1);
}

void sound_applause() {
    play_sample( "data/applause.wav", CH_APPLAUSE, 0);
}

void sound_applause_stop() {
    fade_out( CH_APPLAUSE);
}

