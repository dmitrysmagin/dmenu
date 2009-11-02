#ifndef _CONF_H_
#define _CONF_H_

#include "confuse.h"
#include "env.h"

int  conf_load();
void conf_unload();
void conf_merge_standalone(char *conf_file);
int  conf_root_item(cfg_t* menu_item, cfg_t* root_item, char* root_name);
void conf_persist_item(cfg_t* menu_item);
void conf_selectordir(cfg_t* menu_item, char* dir);
void conf_themeselect(char* themedir);
void conf_backgroundselect(char* bgimage);
int  conf_to_file(cfg_t* cfg, char* file);
cfg_t* conf_from_file(cfg_opt_t* opts, char* file);
#endif
