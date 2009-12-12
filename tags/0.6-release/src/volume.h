#ifndef _VOLUME_H_
#define _VOLUME_H_

#include <stdio.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "conf.h"
#include "common.h"
#include "env.h"

int  volume_enabled();
void volume_init();
void volume_change(Direction dir);
void volume_set(int vol);
void volume_show(SDL_Surface *surface);
void volume_set_text(int);
void volume_deinit();

#endif