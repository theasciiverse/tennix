
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

#ifndef __NETWORK_H
#define __NETWORK_H

#include "game.h"

typedef struct {
    Uint32 x;
    Uint32 y;
    Uint32 z;
    Uint32 move_x;
    Uint32 move_y;
    Uint32 move_z;
    bool ground_hit;
    char last_hit_by;
    bool inhibit_gravity;
} NetworkBall;

typedef struct {
    Uint32 x;
    Uint32 y;
    Uint32 power;
    bool use_power;
    unsigned char score;
    unsigned char desire;
    char game; /* score for the current game */
    unsigned char sets[SETS_TO_WIN*2]; /* score for each set */
    Uint32 accelerate; /* a value [0..1] how fast the user accelerates */
} NetworkPlayer;

typedef struct {
    NetworkBall ball;
    NetworkPlayer players[MAXPLAYERS];
    unsigned char serving_player;
    referee_t referee;
    unsigned char current_set;
    winner_t winner;
    soundevent_t sound_events;
    scoreevent_t score_event;
    unsigned char score_time;
    eventcounter_t ec_game;
    eventcounter_t ec_sets;
    statusmessage_t status_message;
} NetworkGameState;

void
net_serialize_ball(const Ball* src, NetworkBall* dest);

void
net_unserialize_ball(const NetworkBall* src, Ball* dest);

void
net_serialize_player(const Player* src, NetworkPlayer* dest);

void
net_unserialize_player(const NetworkPlayer* src, Player* dest);

void
net_serialize_gamestate(const GameState* src, NetworkGameState* dest);

void
net_unserialize_gamestate(const NetworkGameState* src, GameState* dest);

#endif

