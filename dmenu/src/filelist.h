#ifndef _FILELIST_H_
#define _FILELIST_H_

#include "common.h"

int filelist_init(char* title, char* executable, char* path, int can_change_dirs);
void filelist_deinit();
void filelist_draw(SDL_Surface* screen);

enum MenuState filelist_keypress(SDLKey keysym);

#endif
