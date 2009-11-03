#ifndef _SOUND_H_
#define _SOUND_H_

#include <SDL_mixer.h>

enum MenuSound { MENU_MOVE = 0, MENUITEM_MOVE = 1, DECIDE = 2, CANCEL = 3, OUT = 4, TEST = 5};

void SE_Init();
void SE_out(enum MenuSound seNum); 
void SE_deInit();
#endif
