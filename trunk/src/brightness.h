#ifndef _BRIGHTNESS_H_
#define _BRIGHTNESS_H_

#include <stdio.h>
#include <fcntl.h>
#include <SDL.h>
#include <SDL_image.h>
#include <unistd.h>

#include "env.h"
#include "conf.h"
#include "menu.h"

int  brightness_enabled();
int  brightness_is_dimmed();
void brightness_init();
void brightness_change(Direction dir);
void brightness_dim(int on);
void brightness_set(int bright);
void brightness_show(SDL_Surface *surface);
void brightness_deinit();

#endif
