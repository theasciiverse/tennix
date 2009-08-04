
/**
 *
 * Tennix! SDL Port
 * Copyright (C) 2003, 2007, 2008, 2009 Thomas Perl <thp@thpinfo.com>
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

#include "game.h"
#include "network.h"

#include <assert.h>
#include <SDL/SDL_net.h>

/* HELPER FUNCTIONS */

Uint32
pack_float(float v, float min, float max)
{
    assert(v >= min && v < max);
    return (Uint32)((1U<<31) * (v-min) / (max-min));
}

float
unpack_float(Uint32 v, float min, float max)
{
    assert(v < (1U<<31));
    return v * (max-min) / (1U<<31) + min;
}


void
net_serialize_ball(const Ball* src, NetworkBall* dest)
{
    SDLNet_Write32(pack_float(src->x, -WIDTH, WIDTH), &(dest->x));
    SDLNet_Write32(pack_float(src->y, -HEIGHT, HEIGHT), &(dest->y));
    SDLNet_Write32(pack_float(src->z, -50, 50), &(dest->z));
    SDLNet_Write32(pack_float(src->move_x, -50, 50), &(dest->move_x));
    SDLNet_Write32(pack_float(src->move_y, -50, 50), &(dest->move_y));
    SDLNet_Write32(pack_float(src->move_z, -50, 50), &(dest->move_z));
    dest->ground_hit = src->ground_hit;
    dest->last_hit_by = src->last_hit_by;
    dest->inhibit_gravity = src->inhibit_gravity;
}

void
net_unserialize_ball(const NetworkBall* src, Ball* dest)
{
    dest->x = unpack_float(SDLNet_Read32(&(src->x)), -WIDTH, WIDTH);
    dest->y = unpack_float(SDLNet_Read32(&(src->y)), -HEIGHT, HEIGHT);
    dest->z = unpack_float(SDLNet_Read32(&(src->z)), -50, 50);
    dest->move_x = unpack_float(SDLNet_Read32(&(src->move_x)), -50, 50);
    dest->move_y = unpack_float(SDLNet_Read32(&(src->move_y)), -50, 50);
    dest->move_z = unpack_float(SDLNet_Read32(&(src->move_z)), -50, 50);
    dest->ground_hit = src->ground_hit;
    dest->last_hit_by = src->last_hit_by;
    dest->inhibit_gravity = src->inhibit_gravity;
}

void
net_serialize_player(const Player* src, NetworkPlayer* dest)
{
    SDLNet_Write32(pack_float(src->x, 0, WIDTH), &(dest->x));
    SDLNet_Write32(pack_float(src->y, 0, HEIGHT), &(dest->y));
    SDLNet_Write32(pack_float(src->power, 0, 110), &(dest->power));
    dest->use_power = src->use_power;
    dest->score = src->score;
    dest->desire = src->desire;
    dest->game = src->game;
    memcpy(dest->sets, src->sets, sizeof(unsigned char)*(SETS_TO_WIN*2));
    SDLNet_Write32(pack_float(src->accelerate, 0, 1), &(dest->accelerate));
}

void
net_unserialize_player(const NetworkPlayer* src, Player* dest)
{
    dest->x = unpack_float(SDLNet_Read32(&(src->x)), 0, WIDTH);
    dest->y = unpack_float(SDLNet_Read32(&(src->y)), 0, HEIGHT);
    dest->power = unpack_float(SDLNet_Read32(&(src->power)), 0, 110);
    dest->use_power = src->use_power;
    dest->score = src->score;
    dest->desire = src->desire;
    dest->game = src->game;
    memcpy(dest->sets, src->sets, sizeof(unsigned char)*(SETS_TO_WIN*2));
    dest->accelerate = unpack_float(SDLNet_Read32(&(src->accelerate)), 0, 1);
}

void
net_serialize_gamestate(const GameState* src, NetworkGameState* dest)
{
    int p;

    net_serialize_ball(&(src->ball), &(dest->ball));
    for (p=0; p<MAXPLAYERS; p++) {
        net_serialize_player(&(src->players[p]), &(dest->players[p]));
    }
    dest->serving_player = src->serving_player;
    dest->referee = src->referee;
    dest->current_set = src->current_set;
    dest->winner = src->winner;
    dest->sound_events = src->sound_events;
    dest->score_event = src->score_event;
    dest->ec_game = src->ec_game;
    dest->ec_sets = src->ec_sets;
    dest->status_message = src->status_message;

    fprintf(stderr, "size = %lu\n", sizeof(NetworkGameState));
}

void
net_unserialize_gamestate(const NetworkGameState* src, GameState* dest)
{
    int p;

    net_unserialize_ball(&(src->ball), &(dest->ball));
    for (p=0; p<MAXPLAYERS; p++) {
        net_unserialize_player(&(src->players[p]), &(dest->players[p]));
    }
    dest->serving_player = src->serving_player;
    dest->referee = src->referee;
    dest->current_set = src->current_set;
    dest->winner = src->winner;
    dest->sound_events = src->sound_events;
    dest->score_event = src->score_event;
    dest->ec_game = src->ec_game;
    dest->ec_sets = src->ec_sets;
    dest->status_message = src->status_message;
}

