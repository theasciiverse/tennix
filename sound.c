
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

void sound_out() {
    play_sample( "out.wav", CH_OUT, 0);
}

