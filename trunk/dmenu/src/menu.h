#ifndef _MENU_H_
#define _MENU_H_

#include "common.h"

int menu_init();
void menu_reload_background();
void menu_deinit();
void menu_draw(SDL_Surface*);
void menu_move( enum Direction dir);
void menuitem_move( enum Direction dir);

enum MenuState menu_keypress(SDLKey);
enum MenuState menuitem_run();

#endif
