/*
 *  Copyright (C) 2009 sca <scahoo <at> gmail <dot> com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __IMAGES_H__
#define __IMAGES_H__

#include <SDL.h>

typedef struct {
    int w, h;
    unsigned char data[];
} image_data_t;

typedef struct {
    int w, h;
    SDL_Surface *sfc;
} image_t;

typedef enum {
    IMG_BATTERY = 0,
    IMG_BATTERY_1,
    IMG_BATTERY_2,
    IMG_BATTERY_3,
	IMG_BATTERY_C,
    IMG_LOCK,
    IMG_MAX
} image_e;

const image_data_t* const c_images[IMG_MAX];

#endif//__IMAGES_H__
