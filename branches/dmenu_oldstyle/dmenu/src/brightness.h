#include <stdio.h>
#include <fcntl.h>
#include <SDL.h>
#include <SDL_image.h>
#include <unistd.h>

#include "conf.h"

void bright_init();
void bright_set(int bright);
void bright_show(SDL_Surface *surface);
void bright_deinit();
