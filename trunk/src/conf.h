#ifndef _CONF_H_
#define _CONF_H_

#include "confuse.h"
#include "env.h"

int  conf_load();
void conf_unload();
void conf_merge_standalone(char *conf_file);
void conf_dirselect(cfg_t* menu_item, char* dir);
void conf_themeselect(char* themedir);
void conf_backgroundselect(char* bgimage);
void conf_colorselect(char* color);
int  conf_to_file(cfg_t* cfg, char* file);
int  conf_set_item_path(cfg_t* config, char** names, char* attr, char* val);
char** conf_get_item_path(cfg_t* item);
cfg_t* conf_from_file(cfg_opt_t* opts, char* file);

#endif
