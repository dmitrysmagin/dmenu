#include <SDL_mixer.h>

#define MENU_MOVE	0
#define MENUITEM_MOVE	1
#define DECIDE		2
#define CANCEL		3
#define OUT		4
#define TEST		5

void SE_Init();
void SE_out(int seNum); 
void SE_deInit();
