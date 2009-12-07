#ifndef _BRIGHTNESS_H_
#define _BRIGHTNESS_H_

#include <stdio.h>
#include <fcntl.h>
#include <SDL.h>
#include <SDL_image.h>
#include <unistd.h>

#include "env.h"
#include "conf.h"

int  bright_enabled();
void bright_init(int color);
void bright_set(int bright);
void bright_show(SDL_Surface *surface);
void bright_deinit();

#endif