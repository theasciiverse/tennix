
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

#include "tennix.h"
#include "sound.h"

#include <stdarg.h>

static Sound* sounds;

#include "data/sounds_data.c"
static const ResourceData samples[] = {
    RESOURCE(racket1),
    RESOURCE(racket2),
    RESOURCE(rain),
    RESOURCE(ground1),
    RESOURCE(ground2),
    RESOURCE(audience),
    RESOURCE(applause),
    RESOURCE(out),
    RESOURCE(background),
    RESOURCE(mouseover),
    RESOURCE(mouseclick),

#ifdef HAVE_VOICE_FILES
    RESOURCE(to),
    RESOURCE(all),
    RESOURCE(love_in),
    RESOURCE(love_out),
    RESOURCE(fifteen_in),
    RESOURCE(fifteen_out),
    RESOURCE(thirty_in),
    RESOURCE(thirty_out),
    RESOURCE(forty_in),
    RESOURCE(forty_out),
    RESOURCE(deuce),
    RESOURCE(advantage_player_one),
    RESOURCE(advantage_player_two),

    RESOURCE(zero_in),
    RESOURCE(zero_out),
    RESOURCE(one_in),
    RESOURCE(one_out),
    RESOURCE(two_in),
    RESOURCE(two_out),
    RESOURCE(three_in),
    RESOURCE(three_out),
    RESOURCE(four_in),
    RESOURCE(four_out),
    RESOURCE(five_in),
    RESOURCE(five_out),
    RESOURCE(six_in),
    RESOURCE(six_out),
    RESOURCE(seven_in),
    RESOURCE(seven_out),

    RESOURCE(in_the_first_set),
    RESOURCE(in_the_second_set),
    RESOURCE(in_the_third_set),
    RESOURCE(in_the_fourth_set),
    RESOURCE(in_the_fifth_set),

    RESOURCE(quit_it1),
    RESOURCE(quit_it2),
    RESOURCE(quit_it3),
    RESOURCE(quit_it4),

    RESOURCE(new_game1),
    RESOURCE(new_game2),
    RESOURCE(new_game3),
    RESOURCE(new_game4),
    RESOURCE(new_game5),
    RESOURCE(new_game6),

    RESOURCE(lets_go1),
    RESOURCE(lets_go2),
    RESOURCE(lets_go3),
    RESOURCE(lets_go4),
#endif
};

static sound_id voice_queue[VOICE_QUEUE_MAX];
static int voice_queue_size = 0;
static int voice_queue_position = 0;

int voice_finished_flag = 1;

void init_sound() {
    int i;
    Mix_Chunk* data;

    if( Mix_OpenAudio( 44100, AUDIO_S16SYS, 2, 1024) < 0) {
        fprintf( stderr, "Error initializing SDL_mixer: %s\n", Mix_GetError());
    }

    sounds = (Sound*)calloc(SOUND_MAX, sizeof(Sound));

    for( i=0; i<SOUND_MAX; i++) {
        data = Mix_LoadWAV_RW(SDL_RWFromConstMem(samples[i].data, samples[i].size), 1);
        if( !data) {
            fprintf( stderr, "Error: %s\n", SDL_GetError());
            continue;
        }

        sounds[i].data = data;
    }

    /* for Voice Queue processing */
    Mix_ChannelFinished(voice_channel_finished);
}

void play_sample_n(sound_id id, int n)
{
    if (id >= SOUND_MAX) {
        fprintf(stderr, "Cannot play sound #%d.\n", id);
        return;
    }

    if (n == -2) {
        Mix_FadeInChannel(CHANNEL_BY_ID(id), sounds[id].data, -1, FADE_IN_MS);
    } else {
        Mix_PlayChannel(CHANNEL_BY_ID(id), sounds[id].data, n);
    }

    /* Audience stops clapping when ball is served */
    if (id >= SOUND_RACKET_FIRST && id <= SOUND_RACKET_LAST) {
        stop_sample(SOUND_APPLAUSE);
    }
}

void stop_sample(sound_id id)
{
    Mix_FadeOutChannel(CHANNEL_BY_ID(id), FADE_OUT_MS);
}

void pan_sample(sound_id id, float position)
{
    if (position == 0.5) {
        Mix_SetPanning(CHANNEL_BY_ID(id), 255, 255);
    }
    else {
        Mix_SetPanning(CHANNEL_BY_ID(id), 255*(1.0-position), 255*(position));
    }
}

void voice_clear()
{
    voice_queue_size = 0;
    Mix_HaltChannel(CHANNEL_VOICE);
}

void voice_enqueue(sound_id id)
{
    if (voice_queue_size < VOICE_QUEUE_MAX) {
        voice_queue[voice_queue_size++] = id;
    } else {
        fprintf(stderr, "Voice queue overflow. Skipping: %d\n", id);
    }
}

void voice_say()
{
    if (voice_queue_size > 0) {
        voice_queue_position = 0;
        voice_finished_flag = 0;
        Mix_PlayChannel(CHANNEL_VOICE, sounds[voice_queue[voice_queue_position]].data, 0);
        voice_queue_position++;
    }
}

void voice_say_list(int n, ...)
{
    va_list ap;
    int i;
    sound_id id;

    voice_clear();

    va_start(ap, n);
    for (i=0; i<n; i++) {
        id = (sound_id)va_arg(ap, int);
        voice_enqueue(id);
    }
    va_end(ap);
    voice_say();
}

void voice_channel_finished(int channel)
{
    if (channel == CHANNEL_VOICE) {
        if (voice_queue_position < voice_queue_size) {
            Mix_PlayChannel(CHANNEL_VOICE, sounds[voice_queue[voice_queue_position]].data, 0);
            voice_queue_position++;
        } else {
            voice_finished_flag = 1;
            voice_queue_position = 0;
        }
    }
}

