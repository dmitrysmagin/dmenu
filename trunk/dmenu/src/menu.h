#ifndef _MENU_H_
#define _MENU_H_

#include "common.h"

int menu_init();
void menu_deinit();
void menu_draw(SDL_Surface*);

void menu_next();
void menu_previous();
void menuitem_next();
void menuitem_previous();

enum MenuState menu_keypress(SDLKey);

void menuitem_run();

#endif
