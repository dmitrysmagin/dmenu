#ifndef _COLORPICKER_H_
#define _COLORPICKER_H_

#include "common.h"

int  colorpicker_init(char* title, char* executable,  char* path, char* color, char* background);
void colorpicker_deinit();
int  colorpicker_draw(SDL_Surface* screen);
int  colorpicker_animate(SDL_Surface* screen);
void colorpicker_update_preview();
void colorpicker_update_color();
enum MenuState colorpicker_keypress(SDLKey keysym);

#endif