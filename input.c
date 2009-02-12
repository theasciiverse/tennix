
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

#include <string.h>

#include "tennix.h"
#include "graphics.h"
#include "game.h"
#include "input.h"
#include "util.h"

static InputDevice devices[MAX_INPUT_DEVICES];
static int devices_count;

void init_input()
{
    int n, x;

    SDL_JoystickEventState(SDL_ENABLE);

#ifndef MAEMO
    /* keyboard presets */
    devices[devices_count].type = INPUT_TYPE_KEYBOARD;
    devices[devices_count].up_key = 'w';
    devices[devices_count].down_key = 's';
    devices[devices_count].input_keys[INPUT_KEY_HIT]= 'd';
    devices[devices_count].input_keys[INPUT_KEY_TOPSPIN] = 'e';
    devices[devices_count].input_keys[INPUT_KEY_SMASH] = 'f';
    devices[devices_count].icon = GR_INPUT_KEYBOARD_WS;
    devices[devices_count].exclusive_to_player = 1;
    strcpy(devices[devices_count].name, "Keyboard (WS/DEF)");
    devices_count++;

    /* keyboard presets */
    devices[devices_count].type = INPUT_TYPE_KEYBOARD;
    devices[devices_count].up_key = 'o';
    devices[devices_count].down_key = 'l';
    devices[devices_count].input_keys[INPUT_KEY_HIT]= 'k';
    devices[devices_count].input_keys[INPUT_KEY_TOPSPIN] = 'i';
    devices[devices_count].input_keys[INPUT_KEY_SMASH] = 'j';
    devices[devices_count].icon = GR_INPUT_KEYBOARD_OL;
    devices[devices_count].exclusive_to_player = 2;
    strcpy(devices[devices_count].name, "Keyboard (OL/KIJ)");
    devices_count++;
#endif

    /* keyboard presets */
    devices[devices_count].type = INPUT_TYPE_KEYBOARD;
    devices[devices_count].up_key = SDLK_UP;
    devices[devices_count].down_key = SDLK_DOWN;
    devices[devices_count].input_keys[INPUT_KEY_HIT]= SDLK_SPACE;
    devices[devices_count].input_keys[INPUT_KEY_TOPSPIN] = SDLK_LCTRL;
    devices[devices_count].input_keys[INPUT_KEY_SMASH] = SDLK_LALT;
#ifdef MAEMO
    devices[devices_count].icon = GR_INPUT_MAEMO_DPAD;
    strcpy(devices[devices_count].name, "D-Pad");
    devices[devices_count].input_keys[INPUT_KEY_HIT]= SDLK_RETURN;
    /* FIXME: No TOPSPIN and SMASH keys on Maemo yet with d-pad */
#else
    devices[devices_count].icon = GR_INPUT_KEYBOARD_ARROWS;
    strcpy(devices[devices_count].name, "Keyboard (arrows + space, lctrl, lalt)");
#endif
    devices_count++;

    /* mouse */
    devices[devices_count].type = INPUT_TYPE_MOUSE;
    devices[devices_count].input_keys[INPUT_KEY_HIT]= SDL_BUTTON(1);
    devices[devices_count].input_keys[INPUT_KEY_TOPSPIN] = SDL_BUTTON(2);
    devices[devices_count].input_keys[INPUT_KEY_SMASH] = SDL_BUTTON(3);
#ifdef MAEMO
    devices[devices_count].icon = GR_INPUT_TOUCHSCREEN;
    strcpy(devices[devices_count].name, "Touchscreen");
#else
    devices[devices_count].icon = GR_INPUT_MOUSE;
    strcpy(devices[devices_count].name, "Mouse");
#endif
    devices_count++;

    /* joysticks */
    n = SDL_NumJoysticks();
    for (x=0; x<n && devices_count<MAX_INPUT_DEVICES; x++) {
        strcpy(devices[devices_count].name, SDL_JoystickName(x));
        devices[devices_count].type = INPUT_TYPE_JOYSTICK;
        devices[devices_count].joystick = SDL_JoystickOpen(x);
        devices[devices_count].x_axis = 0;
        devices[devices_count].y_axis = 0;
        devices[devices_count].input_keys[INPUT_KEY_HIT]= 0;
        devices[devices_count].input_keys[INPUT_KEY_TOPSPIN] = 1;
        devices[devices_count].input_keys[INPUT_KEY_SMASH] = 2;
        devices[devices_count].icon = GR_INPUT_GAMEPAD;
        devices_count++;
    }
}

void uninit_input()
{
    int i;
    SDL_JoystickEventState(SDL_IGNORE);

    for (i=0; i<devices_count; i++) {
        if (devices[i].type == INPUT_TYPE_JOYSTICK) {
            SDL_JoystickClose(devices[i].joystick);
        }
    }
    devices_count = 0;
}


InputDevice* find_input_devices(unsigned int* count)
{
    *count = devices_count;
    return devices;
}

const char* input_device_get_name(InputDevice* d)
{
    return d->name;
}

float input_device_get_axis(InputDevice* d, unsigned const char axis) {
    Uint8 *keystate;
    Uint8 mb;

    SDL_PumpEvents();

    if (d->type == INPUT_TYPE_KEYBOARD) {
        keystate = SDL_GetKeyState(NULL);
        if (axis == INPUT_AXIS_X) {
            return 1.0*keystate[d->right_key] + -1.0*keystate[d->left_key];
        } else {
            return 1.0*keystate[d->down_key] + -1.0*keystate[d->up_key];
        }
    } else if (d->type == INPUT_TYPE_JOYSTICK) {
        if (axis == INPUT_AXIS_X) {
            return JOYSTICK_PERCENTIZE(SDL_JoystickGetAxis(d->joystick, d->x_axis*2));
        } else {
            return JOYSTICK_PERCENTIZE(SDL_JoystickGetAxis(d->joystick, 1+d->y_axis*2));
        }
    } else if (d->type == INPUT_TYPE_MOUSE) {
        mb = SDL_GetMouseState(&d->mx, &d->my);
        if (axis == INPUT_AXIS_X) {
            /* Not x-movement yet (PLAYER_MOVE_X is not defined!) */
            /*if (absf(d->mx - d->player_x) > PLAYER_MOVE_X) {
                if (d->mx > d->player_x)  {
                    return 1.0;
                } else if (d->mx < d->player_x) {
                    return -1.0;
                }
            }*/
        } else {
            if (absf(d->my - d->player_y) > PLAYER_MOVE_Y) {
                if (d->my > d->player_y)  {
                    return 1.0;
                } else if (d->my < d->player_y) {
                    return -1.0;
                }
            }
        }
    } else {
        /* unimplemented */
    }
    return 0.0;
}

char input_device_get_key(InputDevice* d, unsigned const char key) {
    Uint8 mb;
    SDL_PumpEvents();

    if (d->type == INPUT_TYPE_KEYBOARD) {
        return SDL_GetKeyState(NULL)[d->input_keys[key]];
    } else if (d->type == INPUT_TYPE_JOYSTICK) {
        return SDL_JoystickGetButton(d->joystick, d->input_keys[key]);
    } else if (d->type == INPUT_TYPE_MOUSE) {
        mb = SDL_GetMouseState(NULL, NULL);
        return (mb & d->input_keys[key]) != 0;
    } else {
        /* unimplemented */
    }
    return 0;
}


