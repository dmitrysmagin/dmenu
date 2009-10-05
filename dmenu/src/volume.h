#include <stdio.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

void vol_init();
void vol_set(int vol);
void vol_show(SDL_Surface *surface);
void vol_deinit();

