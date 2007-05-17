#include "tennix.h"
#include "sound.h"

static char datadir[MAXPATHLEN];

void init_sound( const char *data_dir) {
    strcpy( datadir, data_dir);

    if( Mix_OpenAudio( 44100, AUDIO_S16SYS, 2, 1024) < 0) {
        fprintf( stderr, "Error initializing SDL_mixer: %s\n", Mix_GetError());
    }
}

void play_sample( const char* filename, unsigned char channel, int loop) {
    static Mix_Chunk* chunks[CH_MAX] = { 0 };
    char fn[MAXPATHLEN];

    strcpy( fn, datadir);
    strcat( fn, filename);

    if( chunks[channel] != NULL) {
        Mix_FreeChunk( chunks[channel]);
        chunks[channel] = NULL;
    }

    chunks[channel] = Mix_LoadWAV( fn);
    if( chunks[channel] != NULL) {
        Mix_PlayChannel( channel, chunks[channel], loop);
    } else {
        fprintf( stderr, "Error: %s\n", Mix_GetError());
    }
}

void fade_out( unsigned char channel) {
    Mix_FadeOutChannel( channel, FADE_OUT_MS);
}

void sound_ground() {
    const char* sounds[] = { "ground1.wav", "ground2.wav" };
    play_sample( sounds[rand()%2], CH_GROUND, 0);
}

void sound_racket( unsigned char powershot) {
    const char* sounds[] = { "racket1.wav", "racket2.wav" };
    if( powershot) {
        play_sample( "powershot.wav", CH_RACKET, 0);
    } else {
        play_sample( sounds[rand()%2], CH_RACKET, 0);
    }
}

void sound_audience() {
    play_sample( "audience.wav", CH_AUDIENCE, -1);
}

void sound_applause() {
    play_sample( "applause.wav", CH_APPLAUSE, 0);
}

void sound_applause_stop() {
    fade_out( CH_APPLAUSE);
}

