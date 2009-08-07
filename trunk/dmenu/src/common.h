#ifndef _COMMON_H_
#define _COMMON_H_

enum MenuState { MAINMENU, FILELIST };

void fill_fb(Uint16* source);

void run_command (char* cmd, char* args, char* workdir);

#endif
