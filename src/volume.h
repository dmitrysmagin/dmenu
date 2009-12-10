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

int  vol_enabled();
void vol_init();
void vol_change(Direction dir);
void vol_set(int vol);
void vol_show(SDL_Surface *surface);
void vol_set_text(int);
void vol_deinit();

#endif