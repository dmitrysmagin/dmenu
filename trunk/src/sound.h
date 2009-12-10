#ifndef _SOUND_H_
#define _SOUND_H_

#include <SDL_mixer.h>

enum MenuSound { MENU_MOVE = 0, MENUITEM_MOVE = 1, DECIDE = 2, CANCEL = 3, OUT = 4, GLOBAL_KEY = 5};

void sound_init();
void sound_out(enum MenuSound seNum); 
void sound_deinit();
#endif
