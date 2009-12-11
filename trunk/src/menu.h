#ifndef _MENU_H_
#define _MENU_H_

#include "common.h"

#define item_alpha(act, cur) (MENU_ACTIVE_ALPHA >> abs(cur-act))

int  menu_init();
void menu_get_position(int* menu_index, int* menu_item_index);
void menu_set_position(int menu_index, int menu_item_index);
void menu_state_changed();
void menu_reload_background();
void menu_deinit();
int  menu_draw(SDL_Surface* screen);
void menu_animate(SDL_Surface* screen);
void menu_osd(SDL_Surface* screen);
void menu_move( Direction dir);
void menu_move_item( Direction dir);

MenuState menu_keypress(SDLKey);
MenuState menu_run_item();
void menu_close_sub();
void menu_open_sub();
#endif
