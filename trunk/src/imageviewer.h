#ifndef _IMAGEVIEWER_H_
#define _IMAGEVIEWER_H_

#include "common.h"

typedef struct {
    char* title;
    char* file;
} ImageEntry;

int  imageviewer_init(char* title, char* executable, char* path, ImageEntry** files);
void imageviewer_deinit();
int  imageviewer_draw(SDL_Surface* screen);
int  imageviewer_animate(SDL_Surface* screen);
enum MenuState imageviewer_keypress(SDLKey keysym);
void imageviewer_update_preview();
void imageviewer_update_list();

#endif
