#ifndef _IMAGEVIEWER_H_
#define _IMAGEVIEWER_H_

#include "common.h"

int  imageviewer_init(char* title, char* executable,  char* path);
void imageviewer_deinit();
void imageviewer_draw(SDL_Surface* screen);
enum MenuState imageviewer_keypress(SDLKey keysym);
void imageviewer_update_preview();
void imageviewer_update_list();

#endif
