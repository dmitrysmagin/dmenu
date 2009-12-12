#ifndef _SOUND_H_
#define _SOUND_H_

#include <SDL_mixer.h>

typedef enum { MENU_MOVE = 0, MENUITEM_MOVE = 1, DECIDE = 2, CANCEL = 3, OUT = 4, GLOBAL_KEY = 5} MenuSound;

void sound_init();
void sound_out(MenuSound seNum);
void sound_deinit();
#endif
