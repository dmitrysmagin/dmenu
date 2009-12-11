#ifndef _MENU_H_
#define _MENU_H_

#include "common.h"

#define item_alpha(act, cur) (MENU_ACTIVE_ALPHA >> abs(cur-act))
#define copyname_trim(str, conf) if (conf != NULL) { c = cfg_getstr(conf, "Name"); while (*c==' ') c++; strcpy(str, c); }
#define if_names_equal(m, s) c = cfg_getstr(m, "Name"); while (*c==' ') c++; if (strcmp(c, s)==0)
    
int  menu_init();
void menu_force_redraw(SDL_Surface* screen);
void menu_get_position(char* menu_title, char* menu_item_title, char* menu_subitem_title);
void menu_set_position(const char* menu_title, const char* menu_item_title, const char* menu_subitem_title);
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
