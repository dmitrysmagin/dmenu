/*
 *  Copyright (C) 2009 sca <scahoo <at> gmail <dot> com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
 
#ifndef __DOSD_H__
#define __DOSD_H__

#include <SDL.h>
#include <stdint.h>
#include <stdbool.h>

int dosd_init(uint32_t color);
void dosd_deinit();
void dosd_show(SDL_Surface *surface);

inline bool dosd_is_locked();

enum BatteryLevel {
    BEST_LEVEL=3739,
    FAIR_LEVEL=3675,
    LOW_LEVEL=3611
};
#endif//__DOSD_H__
