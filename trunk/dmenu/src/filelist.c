/*
 *  Copyright (C) 2009 Rookie1 <mr.rookie1@gmail.com>
 *
 *  Author: <mr.rookie1@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "conf.h"
#include "filelist.h"
#include "menu.h"

extern cfg_t *cfg;

extern SDL_Surface* background;
SDL_Surface* list_bg;
SDL_Surface* list_sel;
SDL_Surface* list_title;

SDL_Surface* list_dir_icon;
SDL_Surface* list_file_icon;

SDL_Surface* tmp_surface;

TTF_Font* list_font;

int status_changed;

struct dirent **namelist;
struct stat *statlist;
#define files_per_page 8 //todo - this should be in the conf file
SDL_Surface* list_filename[files_per_page]; //todo - if files_per_page is in conf, need to
                                           //dynamically allocate this
int num_of_files; // total number of files under current directory
int current_list_start; // index of the file at top of current page
int current_highlight; // index of the file being highlighted

char current_path[PATH_MAX];
char work_path[PATH_MAX];
char current_executable[PATH_MAX];

int filelist_theme; // indicate whether filelist is used to select theme
 
void filelist_fill()
{
    int i, j;
    SDL_Color color = {255,255,255,0};

    for (i=0,j=current_list_start; i<files_per_page; i++,j++) {
        list_filename[i] = NULL;
        if (j < num_of_files) {
            tmp_surface = TTF_RenderUTF8_Blended(list_font, namelist[j]->d_name, color);
            list_filename[i] = SDL_DisplayFormatAlpha(tmp_surface);
            SDL_FreeSurface(tmp_surface);
        }
    }
}

int list_filter(const struct dirent *dptr)
{
    if (dptr->d_name[0] == '.') return 0;
    else return 1;
}

int get_list(char* path)
{
    int i;

    num_of_files = scandir(path, &namelist, list_filter, alphasort);
    current_list_start = current_highlight = 0;

    if (num_of_files < 0) {
        num_of_files = 0;
        return -1;
    }
    else {
        statlist = malloc(sizeof(struct stat) * num_of_files);
        for (i=0;i<num_of_files;i++) {
            char *filename = (char*)malloc((strlen(path) + strlen(namelist[i]->d_name) + 2)
                              * sizeof(char));
            strcpy(filename, path);
            strcat(filename, "/");
            strcat(filename, namelist[i]->d_name);
            stat(filename, &statlist[i]);
            free(filename);
        }

        strcpy(current_path, path);

        filelist_fill();

        return 0;
    }
}

void clear_list()
{
    int i, j;
    for (i=0;i<num_of_files;i++) {
        free(namelist[i]);
    }
    free(namelist);
    namelist = NULL;
    free(statlist);
    statlist = NULL;

    for (i=0, j=current_list_start;
         (i < files_per_page) && (j < num_of_files);
         i++, j++) {
        SDL_FreeSurface(list_filename[i]);
        list_filename[i] = NULL;
    }

    current_list_start = current_highlight = num_of_files = 0;
}



int filelist_init(char* title, char* executable, char* path)
{
    SDL_Color color = {255,255,255,0};

    tmp_surface = IMG_Load(cfg_getstr(cfg, "ListBackground"));
    if (!tmp_surface) {
        printf("Failed to load %s: %s\n", cfg_getstr(cfg, "ListBackground"), IMG_GetError());
        return 1;
    }
    list_bg = SDL_DisplayFormatAlpha(tmp_surface);
    SDL_FreeSurface(tmp_surface);

    tmp_surface = IMG_Load(cfg_getstr(cfg, "ListSelector"));
    if (!tmp_surface) {
        printf("Failed to load %s: %s\n", cfg_getstr(cfg, "ListSelector"), IMG_GetError());
        return 1;
    }
    list_sel = SDL_DisplayFormatAlpha(tmp_surface);
    SDL_FreeSurface(tmp_surface);

    tmp_surface = IMG_Load(cfg_getstr(cfg, "ListDirIcon"));
    if (!tmp_surface) {
        printf("Failed to load %s: %s\n", cfg_getstr(cfg, "ListDirIcon"), IMG_GetError());
        return 1;
    }
    list_dir_icon = SDL_DisplayFormatAlpha(tmp_surface);
    SDL_FreeSurface(tmp_surface);

    tmp_surface = IMG_Load(cfg_getstr(cfg, "ListFileIcon"));
    if (!tmp_surface) {
        printf("Failed to load %s: %s\n", cfg_getstr(cfg, "ListFileIcon"), IMG_GetError());
        return 1;
    }
    list_file_icon = SDL_DisplayFormatAlpha(tmp_surface);
    SDL_FreeSurface(tmp_surface);

    // load font
    TTF_Init();
    list_font = TTF_OpenFont(cfg_getstr(cfg, "Font"), 18);

    tmp_surface = TTF_RenderUTF8_Blended(list_font, title, color);
    list_title = SDL_DisplayFormatAlpha(tmp_surface);
    SDL_FreeSurface(tmp_surface);

    status_changed = 1;

    // read files
    char real_path[PATH_MAX];
    if (!realpath(path, real_path)) {
        printf("Failed to get real path of directory %s\n", path);
        return 1;
    }
    if (get_list(real_path)) {
        printf("Failed to read directory %s\n", real_path);
        return 1;
    }

    strcpy(current_executable, executable);
    strcpy(work_path, path);

    if (strcmp(executable, COMMAND_THEMESELECT) == 0) filelist_theme = 1;
    else filelist_theme = 0;

    return 0;
}

void filelist_deinit()
{
    SDL_FreeSurface(list_bg);
    list_bg = NULL;
    SDL_FreeSurface(list_sel);
    list_sel = NULL;
    SDL_FreeSurface(list_title);
    list_title = NULL;
    SDL_FreeSurface(list_dir_icon);
    list_dir_icon= NULL;
    SDL_FreeSurface(list_file_icon);
    list_file_icon= NULL;

    if (list_font) {
        TTF_CloseFont(list_font);
        list_font = NULL;
        TTF_Quit();
    }

    if (namelist) clear_list();
}

void filelist_draw(SDL_Surface* screen)
{
    int i;
    SDL_Rect dstrect, txtrect;

    if (!status_changed) return;

    dstrect.x = 0;
    dstrect.y = 0;
    txtrect.x = 0;
    txtrect.y = 0;

    // clear screen
    //SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

    SDL_BlitSurface(background, 0, screen, &dstrect);
    SDL_BlitSurface(list_bg, 0, screen, &dstrect);

    SDL_BlitSurface(list_title, 0, screen, &txtrect);

    txtrect.y = 24;
    for (i=0; i<files_per_page; i++) {
        if (list_filename[i]) {
            if (i == current_highlight) SDL_BlitSurface(list_sel, 0, screen, &txtrect);
            if (S_ISDIR(statlist[current_list_start+i].st_mode)) {
                SDL_BlitSurface(list_dir_icon, 0, screen, &txtrect);
                txtrect.x += list_dir_icon->w;
            } else {
                SDL_BlitSurface(list_file_icon, 0, screen, &txtrect);
                txtrect.x += list_file_icon->w;
            }
            SDL_BlitSurface(list_filename[i], 0, screen, &txtrect);
            txtrect.x = 0;
            txtrect.y += 27;
        }
    }

    status_changed = 0;
}

void filelist_up()
{
    int i;
    SDL_Color color = {255,255,255,0};

    if (current_highlight > 0) {
        current_highlight--;
    } else if (current_list_start > 0) {
        SDL_FreeSurface(list_filename[files_per_page-1]);

        for (i=(files_per_page-1);i>0;i--) {
            list_filename[i] = list_filename[i-1];
        }

        current_list_start--;

        tmp_surface = TTF_RenderUTF8_Blended(list_font,
                      namelist[current_list_start]->d_name, color);
        list_filename[0] = SDL_DisplayFormatAlpha(tmp_surface);
        SDL_FreeSurface(tmp_surface);
    } else {
        for (i=0;i<files_per_page;i++) {
            if (list_filename[i]) SDL_FreeSurface(list_filename[i]);
        }
        current_list_start = num_of_files - files_per_page;
        if (current_list_start < 0) current_list_start = 0;
        current_highlight = files_per_page - 1;
        filelist_fill();
    }

    status_changed = 1;
}

void filelist_page_up()
{
    int i;

    for (i=0;i<files_per_page;i++) {
        if (list_filename[i]) SDL_FreeSurface(list_filename[i]);
    }

    current_list_start -= files_per_page;
    if (current_list_start < 0) current_list_start = 0;
    current_highlight = 0;
    filelist_fill();
    
    status_changed = 1;
}

void filelist_down()
{
    int i;
    SDL_Color color = {255,255,255,0};

    if (current_highlight < (files_per_page - 1) &&
        (current_list_start + current_highlight) < (num_of_files - 1)) {
        current_highlight++;
    } else if ((current_list_start + files_per_page) < num_of_files) {
        SDL_FreeSurface(list_filename[0]);

        for (i=0;i<(files_per_page-1);i++) {
            list_filename[i] = list_filename[i+1];
        }

        current_list_start++;

        tmp_surface = TTF_RenderUTF8_Blended(list_font,
                      namelist[current_list_start + files_per_page - 1]->d_name,
                      color);
        list_filename[files_per_page-1] = SDL_DisplayFormatAlpha(tmp_surface);
        SDL_FreeSurface(tmp_surface);
    } else {
        for (i=0;i<files_per_page;i++) {
            if (list_filename[i]) SDL_FreeSurface(list_filename[i]);
        }
        current_list_start = 0;
        current_highlight = 0;
        filelist_fill();
    }

    status_changed = 1;
}

void filelist_page_down()
{
    int i;

    for (i=0;i<files_per_page;i++) {
        if (list_filename[i]) SDL_FreeSurface(list_filename[i]);
    }

    current_list_start += files_per_page;
    if (current_list_start > (num_of_files - 1)) current_list_start = num_of_files - 1;
    current_highlight = 0;
    filelist_fill();
    
    status_changed = 1;
}

void filelist_right()
{
    char temp_path[PATH_MAX];
    int i = current_list_start+current_highlight;
    if (S_ISDIR(statlist[i].st_mode)) {
        strcpy(temp_path, current_path);
        strcat(temp_path, "/");
        strcat(temp_path, namelist[i]->d_name);
        clear_list();
        if (get_list(temp_path) != 0)
            get_list(current_path);
        status_changed = 1;
    }
}    

void filelist_left()
{
    int i = strlen(current_path);

    if ((i == 1) && (current_path[0] == '/')) {
        return;
    } else {
        char* last_separator = strrchr(current_path, '/');
        if (!last_separator) {
            printf("Unable to find path separator - %s\n", current_path);
            return;
        } else if (last_separator == current_path) {
            current_path[1] = '\0';
        } else {
            *last_separator = '\0';
        }
        clear_list();
        get_list(current_path);
        status_changed = 1;
    }

}

// Since filelist_run() has multiple purpose, the next state
// need to be returned from this function.
enum MenuState filelist_run()
{
    char file_name[PATH_MAX];
    int i = current_list_start+current_highlight;

    if (S_ISDIR(statlist[i].st_mode) && (!filelist_theme)) {
        filelist_right();
        return FILELIST;
    }

    if (filelist_theme) {
        strcpy(file_name, namelist[i]->d_name);
    } else {
        strcpy(file_name, current_path);
        strcat(file_name, "/");
        strcat(file_name, namelist[i]->d_name);
    }

    if (current_executable[0] != '\0')
        run_command(current_executable, file_name, work_path);
    else
        run_command(file_name, NULL, work_path);

    // If this is theme selection, we will return here from
    // run_command();
    // If it's running a program, it will not return.
    return MAINMENU;
}

enum MenuState filelist_keypress(SDLKey keysym)
{
    Uint8 *keystate = SDL_GetKeyState(NULL);

    if (keysym == SDLK_LALT) {
        filelist_deinit();
        return MAINMENU;
    }
    else if (keysym == SDLK_UP) {
        if (keystate[SDLK_LSHIFT]) filelist_page_up();
        else filelist_up();
    }
    else if (keysym == SDLK_DOWN) {
        if (keystate[SDLK_LSHIFT]) filelist_page_down();
        else filelist_down();
    }
    else if (keysym == SDLK_RIGHT) {
        if (!filelist_theme) filelist_right();
    }
    else if (keysym == SDLK_LEFT) {
        if (!filelist_theme) filelist_left();
    }
    else if (keysym == SDLK_LCTRL) {
        return filelist_run();
    }

    return FILELIST;
}
