#ifndef _IMAGEVIEWER_H_
#define _IMAGEVIEWER_H_

#include "common.h"

typedef struct {
    char* title;
    char* file;
    char* value;
} ImageEntry;

int  imageviewer_init(char* title, char* executable, char* path, ImageEntry** files);
int  imageviewer_init_theme(char* title, char* executable, char* path);
void imageviewer_deinit();
int  imageviewer_draw(SDL_Surface* screen);
void imageviewer_animate(SDL_Surface* screen);
void imageviewer_osd(SDL_Surface* screen);
MenuState imageviewer_keypress(SDLKey keysym);
void imageviewer_update_preview();
void imageviewer_update_list();

#endif
