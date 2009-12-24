#ifndef _WATCH_H_
#define _WATCH_H_

#include <stdio.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "conf.h"
#include "common.h"
#include "env.h"

int  watch_enabled();
void watch_init();
void watch_show(SDL_Surface *surface);
void watch_deinit();

#endif
