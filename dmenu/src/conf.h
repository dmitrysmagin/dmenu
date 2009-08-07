#ifndef _CONF_H_
#define _CONF_H_

#include "confuse.h"

#define COMMAND_THEMESELECT "!themeselect"

int conf_load();
void conf_unload();
void conf_merge_standalone(char *conf_file);
void conf_themeselect(char* themedir);

#endif
