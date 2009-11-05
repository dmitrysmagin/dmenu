/*
 *  Copyright (C) 2009 Rookie1 <mr.rookie1@gmail.com>
 *
 *  Author: <mr.rookie1@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <libgen.h>
#include "common.h"
#include "conf.h"
#include "main.h"
#include "menu.h"

#define is_set(f, x) (((f) & (x)) == (f))
cfg_opt_t submenuitem_opts[] = {
    CFG_STR("Icon", 0, CFGF_NONE),
    CFG_STR("Name", 0, CFGF_NONE),
    CFG_STR("Executable", 0, CFGF_NONE),
    CFG_STR("WorkDir", ".", CFGF_NONE),
    CFG_BOOL("Selector", cfg_false, CFGF_NONE),
    CFG_STR("SelectorDir", 0, CFGF_NONE),
    CFG_END()
};

cfg_opt_t menuitem_opts[] = {
    CFG_STR("Icon", 0, CFGF_NONE),
    CFG_STR("Name", 0, CFGF_NONE),
    CFG_STR("Executable", 0, CFGF_NONE),
    CFG_STR("WorkDir", ".", CFGF_NONE),
    CFG_BOOL("Selector", cfg_false, CFGF_NONE),
    CFG_STR("SelectorDir", 0, CFGF_NONE),
    CFG_SEC("SubMenuItem", submenuitem_opts, CFGF_MULTI | CFGF_TITLE),
    CFG_FUNC("include", cfg_include),
    CFG_END()
};

cfg_opt_t menu_opts[] = {

    CFG_STR("Icon", 0, CFGF_NONE),
    CFG_STR("Name", 0, CFGF_NONE),
    CFG_SEC("MenuItem", menuitem_opts, CFGF_MULTI | CFGF_TITLE),
    CFG_FUNC("include", cfg_include),
    CFG_END()
};

cfg_opt_t opts[] = {
    CFG_STR("Background", 0, CFGF_NONE),
    CFG_STR("Cursor", 0, CFGF_NONE),
    CFG_STR("Font", 0, CFGF_NONE),

    CFG_BOOL("VolDisp", cfg_false, CFGF_NONE),
    CFG_BOOL("BrightDisp", cfg_false, CFGF_NONE),

    CFG_STR("ListBackground", 0, CFGF_NONE),
    CFG_STR("ListSelector", 0, CFGF_NONE),
    CFG_STR("ListDirIcon", 0, CFGF_NONE),
    CFG_STR("ListFileIcon", 0, CFGF_NONE),
    CFG_STR("LoadingScreen", 0, CFGF_NONE),

    CFG_STR("MenuSE", 0, CFGF_NONE),
    CFG_STR("MenuItemSE", 0, CFGF_NONE),
    CFG_STR("DecideSE", 0, CFGF_NONE),
    CFG_STR("CancelSE", 0, CFGF_NONE),
    CFG_STR("OutSE", 0, CFGF_NONE),
    CFG_STR("TestSE", 0, CFGF_NONE),

    CFG_SEC("Menu", menu_opts, CFGF_MULTI | CFGF_TITLE),

    CFG_FUNC("include", cfg_include),

    CFG_END()
};

cfg_opt_t standalone_menu_opts[] = {
    CFG_SEC("MenuItem", menuitem_opts, CFGF_TITLE),
    CFG_END()
};

cfg_opt_t standalone_opts[] = {
    CFG_SEC("Menu", standalone_menu_opts, CFGF_MULTI | CFGF_TITLE),
    CFG_END()
};

cfg_opt_t main_opts[] = {
    CFG_STR("Theme", "default", CFGF_NONE),
    CFG_STR_LIST("SearchPath", "{}", CFGF_NONE),
    CFG_BOOL("AllowDynamicThemeChange", cfg_false, CFGF_NONE),
    CFG_END()
};

cfg_opt_t value_opts[] = {
    CFG_INT("SndVol", 50, CFGF_NONE),
    CFG_INT("Bright", 3, CFGF_NONE),
    CFG_END()
};

cfg_t *cfg;
cfg_t *cfg_main;
cfg_t *cfg_value;

char* theme_path;

cfg_opt_t* conf_dupopts(cfg_opt_t* opts);
int path_filter(const struct dirent *dptr);

cfg_t* conf_from_file(cfg_opt_t* opts, char* file)
{
    cfg_t* out = cfg_init(opts, 0);
    int rc = cfg_parse(out, file);
    if (rc != CFG_SUCCESS) {
        char* action = rc == CFG_FILE_ERROR ? "load" : "parse";
        log_error( "Unable to %s config file:%s . rc = %d", action, file, rc);
        out = NULL;
    }
    return out;
}

int conf_to_file(cfg_t* cfg, char* file) {
    FILE *fp;
    int file_no;

    log_debug("Saving conf file: %s", file);
    
    fp = load_file(file, "w");
    if (fp == NULL) return 0;

    cfg_print(cfg, fp);
    file_no = fileno(fp);
    fsync(file_no);
    fclose(fp);
    
    return 1;
}

char* THEME_CONF_FILE;

int conf_load_theme() 
{
    int rc = 0;
    
    // build theme_dir
    char theme_dir[PATH_MAX]; theme_dir[0] = '\0';
    char* _theme_path = cfg_getstr(cfg_main, "Theme");
    
    if (_theme_path[0] != '/')  //If relative
    {
        strcat(theme_dir, DMENU_THEMES);
    }
    
    strcat(theme_dir, _theme_path);
    
    rc = change_dir(theme_dir);
    if (rc) return rc;

    //Build theme path
    strcpy(THEME_PATH, theme_dir);
    strcat(THEME_PATH, "/");
    
    //Build theme conf file
    THEME_CONF_FILE = theme_file("theme.cfg");

    // load theme.cfg
    change_dir(THEME_PATH);    
    cfg = conf_from_file(opts, THEME_CONF_FILE);
    if (cfg == NULL) return CFG_PARSE_ERROR;
    change_dir(DMENU_PATH);
    
    return 0;
}

int conf_load()
{
    log_debug("Initializing");

    struct dirent **namelist;
    int num_of_files, rc, i, j;

    // load main.cfg
    cfg_main = conf_from_file(main_opts, DMENU_CONF_FILE);
    if (cfg_main == NULL) return CFG_PARSE_ERROR;
    
    // load dmenu.ini
    cfg_value = conf_from_file(value_opts, USER_CONF_FILE);
    if (cfg_value == NULL) return CFG_PARSE_ERROR;

    //Find theme path
    rc = conf_load_theme();
    if (rc) return rc;
    
    // load dmenu.cfg files from SearchPath
    char search_path[PATH_MAX];
    char work_path[PATH_MAX];
    for (j=0;j<cfg_size(cfg_main, "SearchPath");j++) {
        char* search_path_in_conf = cfg_getnstr(cfg_main, "SearchPath", j);
        if (search_path_in_conf[0] == '/') { // SearchPath is absolute
            strcpy(search_path, search_path_in_conf);
        } else { // SearchPath is relative to dmenu directory
            char * tmp = dmenu_file(search_path_in_conf);
            strcpy(search_path, tmp);
            free(tmp);
        }

        num_of_files = scandir(search_path, &namelist, path_filter, alphasort);
        if (num_of_files > 0) {
            for (i=0;i<num_of_files;i++) {
                strcpy(work_path, search_path);
                strcat(work_path, "/");
                strcat(work_path, namelist[i]->d_name);
                strcat(work_path, "/dmenu.cfg");
                conf_merge_standalone(work_path);
            }
        }
    }

    return CFG_SUCCESS;
}

void conf_unload()
{
    log_debug("De-initializing");
    
    free(THEME_CONF_FILE);
    
    cfg_free(cfg);
    cfg_free(cfg_main);

    // Write to dmenu.ini
    conf_to_file(cfg_value, USER_CONF_FILE);
    change_dir(DMENU_PATH);
}

int path_filter(const struct dirent *dptr)
{
    if (dptr->d_name[0] == '.') return 0;
    else return 1;
}

void conf_merge_standalone(char *conf_file)
{
    int i, j, number_of_menu, number_of_standalone_menu;
    cfg_t *standalone_cfg;
    cfg_t* m;
    cfg_t* standalone_m;
    char* conf_file_dir;

    standalone_cfg = conf_from_file(standalone_opts, conf_file);
    if (standalone_cfg == NULL) return;

    conf_file_dir = dirname(conf_file);

    number_of_standalone_menu = cfg_size(standalone_cfg, "Menu");
    number_of_menu = cfg_size(cfg, "Menu");

    for (i=0;i<number_of_standalone_menu;i++) {
        standalone_m = cfg_getnsec(standalone_cfg, "Menu", i);
        for (j=0;j<number_of_menu;j++) {
            m = cfg_getnsec(cfg, "Menu", j);
            if (strcmp(cfg_title(m), cfg_title(standalone_m)) == 0) {

                cfg_t* standalone_mi = cfg_getsec(standalone_m, "MenuItem");
                // Don't include this MenuItem if it includes SubMenuItems,
                // Our dupopts routine does not handle sections, hence doesn't
                // doesn't work for SubMenuItems
                if (cfg_size(standalone_mi, "SubMenuItem") > 0) {
                    log_error("Not supported - dmenu.cfg contains SubMenuItems " 
                                "in directory %s", conf_file_dir);
                }
                else {
                    char* icon_path = cfg_getstr(standalone_mi, "Icon");
                    if (icon_path[0] != '/') { // Icon path is relative
                        char work_path[PATH_MAX];
                        strcpy(work_path, conf_file_dir);
                        strcat(work_path, "/");
                        strcat(work_path, icon_path);
                        cfg_setstr(standalone_mi, "Icon", work_path);
                    }

                    cfg_opt_t* standalone_mi_opt = cfg_getopt(standalone_m, "MenuItem");
                    cfg_opt_t* mi_opt = cfg_getopt(m, "MenuItem");
                    mi_opt->values = realloc(mi_opt->values,
                                          (mi_opt->nvalues+1) * sizeof(cfg_value_t *));
                    mi_opt->values[mi_opt->nvalues] = (cfg_value_t*) malloc(sizeof(cfg_value_t));
                    mi_opt->values[mi_opt->nvalues]->section = calloc(1, sizeof(cfg_t));
                    cfg_t* mi_sec = mi_opt->values[mi_opt->nvalues]->section;
                    mi_opt->nvalues++;
                    cfg_t* s_mi_sec = standalone_mi_opt->values[0]->section;
                    mi_sec->name = strdup(s_mi_sec->name);
                    mi_sec->flags = s_mi_sec->flags;
                    mi_sec->filename = s_mi_sec->filename ? strdup(s_mi_sec->filename) : 0; 
                    mi_sec->line = s_mi_sec->line;
                    mi_sec->errfunc = s_mi_sec->errfunc;
                    mi_sec->title = s_mi_sec->title; 
                    mi_sec->opts = conf_dupopts(s_mi_sec->opts);
                }
            }
        }
    }

    cfg_free(standalone_cfg);
}

cfg_opt_t* conf_dupopts(cfg_opt_t* opts)
{
    int i, j;
    cfg_opt_t *dupopts;
    int n;
    for(n = 0; opts[n].name; n++) // to get number of opts
        /* do nothing */ ;

    dupopts = calloc(n+1, sizeof(cfg_opt_t));
    memcpy(dupopts, opts, n * sizeof(cfg_opt_t));

    for(i = 0; i < n; i++)
    {
        dupopts[i].name = strdup(opts[i].name);

        dupopts[i].values = calloc(dupopts[i].nvalues, sizeof(cfg_value_t*));
        for(j = 0; j < dupopts[i].nvalues; j++) {
            dupopts[i].values[j] = calloc(1, sizeof(cfg_value_t));
            switch (dupopts[i].type) {
              case CFGT_INT:
                dupopts[i].values[j]->number = opts[i].values[j]->number;
                break;
              case CFGT_FLOAT:
                dupopts[i].values[j]->fpnumber = opts[i].values[j]->fpnumber;
                break;
              case CFGT_BOOL:
                dupopts[i].values[j]->boolean = opts[i].values[j]->boolean;
                break;
              case CFGT_STR:
                if (opts[i].values[j]->string)
                    dupopts[i].values[j]->string = strdup(opts[i].values[j]->string);
                else
                    dupopts[i].values[j]->string = NULL;
                break;
              default: //we are not handling the other types
                break;
            }
        }

        if(opts[i].type == CFGT_SEC && opts[i].subopts)
            dupopts[i].subopts = conf_dupopts(opts[i].subopts);

        if(is_set(CFGF_LIST, opts[i].flags) || opts[i].type == CFGT_FUNC)
            dupopts[i].def.parsed = opts[i].def.parsed ? strdup(opts[i].def.parsed) : 0;
        else if(opts[i].type == CFGT_STR)
            dupopts[i].def.string = opts[i].def.string ? strdup(opts[i].def.string) : 0;
    }

    return dupopts;
}

int conf_root_item(cfg_t* menu_item, cfg_t* root_item, char* root_name) {

    char *filename, *out_root_name = NULL;
    int number_of_menu, number_of_menuitem, i, j;
    cfg_t *m, *mi, *out_root_item = NULL;
  
    filename = menu_item->filename;

    if (strcmp(cfg->filename, filename) == 0) {
       out_root_item = cfg;
       out_root_name = "";
    } else {
        number_of_menu = cfg_size(cfg, "Menu");
        for (i=0;i<number_of_menu;i++) {
            m = cfg_getnsec(cfg, "Menu", i);
            if (strcmp(filename,m->filename)==0) {
                out_root_name = "Menu";
                out_root_item = m;
                break;
            } 
          
            number_of_menuitem = cfg_size(m, "MenuItem");
            for (j=0;j<number_of_menuitem;j++) {
                mi = cfg_getnsec(m, "MenuItem", j);
                if (strcmp(filename, mi->filename)==0) {
                    out_root_name = "MenuItem";
                    out_root_item = mi;
                    break;
                }
            }
            if (out_root_item != NULL) break;
        }
    }

    if (out_root_item != NULL) {
        memcpy(root_item, out_root_item, sizeof(struct cfg_t));
        strcpy(root_name, out_root_name);
    }
    return out_root_item!=NULL;
}

void conf_persist_item(cfg_t* menu_item) {

    cfg_t *root_item = NULL;
    char *root_name = NULL;
    FILE *fp = load_file(menu_item->filename, "w");
    if (fp == NULL) return;
    
    root_name = new_str(100);
    root_item = new_item(cfg_t);
    strcpy(root_name, "");

    if (!conf_root_item(menu_item, root_item, root_name)) {
        log_error("Unable to find config file for: %s", menu_item->name);
        return;
    }

    if (strlen(root_name)>0) {
        fprintf(fp, "%s %s {\n", root_name, root_item->title);
    }
        
    cfg_print(root_item, fp);

    if (strlen(root_name)>0) {
        fprintf(fp, "}\n");
    }
 
    free(root_name);
    free(root_item);
    fclose(fp);
}

void conf_themeselect(char* themedir)
{
    char* path = strrchr(themedir, '/'); 
    if (path == NULL) path = themedir;
    else {
        path++;
    }
    
    if (cfg_getbool(cfg_main, "AllowDynamicThemeChange")) {
        cfg_setstr(cfg_main, "Theme", path);
        
        if (!conf_to_file(cfg_main, DMENU_CONF_FILE)) return;
        reload();
    }
}

void conf_selectordir(cfg_t* menu_item, char* dir) 
{
    cfg_setstr(menu_item, "SelectorDir", dir);
    //conf_persist_item(menu_item);
}

void conf_backgroundselect(char* bgimage)
{
    cfg_setstr(cfg, "Background", bgimage);
    //if (!conf_to_file(cfg, THEME_CONF_FILE)) return;
    menu_reload_background();
}
