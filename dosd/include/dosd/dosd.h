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

#define DOSD_UPDATE_INTERVAL (500)
#define DOSD_PADDING         (4)

int dosd_init(uint32_t color);
void dosd_deinit();
void dosd_show(SDL_Surface *surface);

inline bool dosd_is_locked();

#endif//__DOSD_H__
