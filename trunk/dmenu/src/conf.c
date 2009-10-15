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
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <libgen.h>
#include "conf.h"

#define is_set(f, x) (((f) & (x)) == (f))

cfg_opt_t submenuitem_opts[] = {
    CFG_STR("Icon", 0, CFGF_NONE),
    CFG_STR("Name", 0, CFGF_NONE),
    CFG_STR("Executable", 0, CFGF_NONE),
    CFG_STR("WorkDir", ".", CFGF_NONE),
    CFG_BOOL("Selector", cfg_false, CFGF_NONE),
    CFG_END()
};

cfg_opt_t menuitem_opts[] = {
    CFG_STR("Icon", 0, CFGF_NONE),
    CFG_STR("Name", 0, CFGF_NONE),
    CFG_STR("Executable", 0, CFGF_NONE),
    CFG_STR("WorkDir", ".", CFGF_NONE),
    CFG_BOOL("Selector", cfg_false, CFGF_NONE),
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

cfg_t *cfg;
cfg_t *cfg_main;

cfg_opt_t* conf_dupopts(cfg_opt_t* opts);
int path_filter(const struct dirent *dptr);


int conf_load()
{
    int rc;
    struct dirent **namelist;
    int num_of_files;
    int i, j;


    // load main.cfg
    cfg_main = cfg_init(main_opts, 0);
    rc = cfg_parse(cfg_main, "main.cfg");
    if (rc != CFG_SUCCESS) {
        printf( "Unable to load main config file. rc = %d\n", rc);
        return rc;
    }

    // cd to theme directory
    char theme_dir[PATH_MAX] = "themes/";
    strcat(theme_dir, cfg_getstr(cfg_main, "Theme"));
    rc = chdir(theme_dir);
    if (rc != 0) {
        printf("Unable to change to theme directory %s\n", theme_dir);
        perror(0);
        return rc;
    }

    // load theme.cfg
    cfg = cfg_init(opts, 0);
    rc = cfg_parse(cfg, "theme.cfg");
    if (rc != CFG_SUCCESS) {
        printf( "Unable to load theme config file. rc = %d\n", rc);
        return rc;
    }

    // load dmenu.cfg files from SearchPath
    char search_path[PATH_MAX];
    char work_path[PATH_MAX];
    for (j=0;j<cfg_size(cfg_main, "SearchPath");j++) {
        char* search_path_in_conf = cfg_getnstr(cfg_main, "SearchPath", j);
        if (search_path_in_conf[0] == '/') { // SearchPath is absolute
            strcpy(search_path, search_path_in_conf);
        } else { // SearchPath is relative to dmenu directory
            getcwd(search_path, PATH_MAX);
            strcat(search_path, "/../../"); //cwd is the theme directory
            strcat(search_path, search_path_in_conf);
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

    // for internal command themeselect, set
    // Selector = Yes and
    // WorkDir = ".."
    int number_of_menu, number_of_menuitem;
    cfg_t *m;
    cfg_t *mi;
    number_of_menu = cfg_size(cfg, "Menu");
    for (i=0;i<number_of_menu;i++) {
        m = cfg_getnsec(cfg, "Menu", i);
        number_of_menuitem = cfg_size(m, "MenuItem");
        for (j=0;j<number_of_menuitem;j++) {
            mi = cfg_getnsec(m, "MenuItem", j);
            if (cfg_getstr(mi, "Executable") &&
                (strcmp(cfg_getstr(mi, "Executable"), COMMAND_THEMESELECT) == 0)) {
                cfg_setbool(mi, "Selector", cfg_true);
                cfg_setstr(mi, "WorkDir", "..");
            }
        }
    }

    return CFG_SUCCESS;
}

void conf_unload()
{
    cfg_free(cfg);
    cfg_free(cfg_main);

    if (chdir("../..")) { // go back to dmenu directory
        printf("Unable to change to dmenu directory\n");
        perror(0);
    }
}

int path_filter(const struct dirent *dptr)
{
    if (dptr->d_name[0] == '.') return 0;
    else return 1;
}

void conf_merge_standalone(char *conf_file)
{
    int rc;
    int i, j, number_of_menu, number_of_standalone_menu;
    cfg_t *standalone_cfg;
    cfg_t* m;
    cfg_t* standalone_m;
    char* conf_file_dir;

    standalone_cfg = cfg_init(standalone_opts, 0);
    rc = cfg_parse(standalone_cfg, conf_file);
    if (rc != CFG_SUCCESS) {
        if (rc != CFG_FILE_ERROR) {
            printf("Error parsing cfg file %s, rc=%d\n", conf_file, rc);
        }
        return;
    }

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
                    printf("Not supported - dmenu.cfg contains SubMenuItems\n");
                    printf("in directory %s\n", conf_file_dir);
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
                    mi_opt->values[mi_opt->nvalues] = malloc(sizeof(cfg_value_t));
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

extern void menu_deinit();
extern void menu_init();

void conf_themeselect(char* themedir)
{
    if (cfg_getbool(cfg_main, "AllowDynamicThemeChange")) {
        FILE *fp;
        fp = fopen("../../main.cfg", "w");
        if(fp == 0) {
            printf( "Unable to open main config file.\n");
            perror(0);
            return;
        }

        cfg_setstr(cfg_main, "Theme", themedir);
        cfg_print(cfg_main, fp);
        fclose(fp);
    
        menu_deinit();
        conf_unload();
        conf_load();
        menu_init();
    }
}
