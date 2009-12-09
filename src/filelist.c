/*
 *  Copyright (C) 2009 Rookie1 <mr.rookie1@gmail.com>
 *
 *  Author: <mr.rookie1@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "env.h"
#include "conf.h"
#include "filelist.h"
#include "resource.h"
#include "menu.h"

#include "sound.h"
#include "dingoo.h"

#define ends_with_slash(str) (str[strlen(str)-1] == '/')

//Get access to the current menu item
extern cfg_t* mi;
extern cfg_t *cfg;
extern SDL_Surface* background;
SDL_Surface* list_bg;
SDL_Surface* list_sel;
SDL_Surface* list_title;

SDL_Surface* list_dir_icon;
SDL_Surface* list_file_icon;

TTF_Font* list_font;
SDL_Color* list_font_color;
int status_changed;

struct dirent **namelist;
struct stat *statlist;
SDL_Surface* list_filename[FILES_PER_PAGE]; 
int num_of_files; // total number of files under current directory
int current_list_start; // index of the file at top of current page
int current_highlight; // index of the file being highlighted
int at_root; //Whether or not the listing is at the root of the file system
int can_change_dir = 0;
int is_initted = 0;

char current_path[PATH_MAX];
char work_path[PATH_MAX];
char current_executable[PATH_MAX];

SDL_Surface* render_list_text(char* text) {
    return draw_text(text, list_font, list_font_color);
}

void filelist_fill()
{
    int i, j;
    //Clear old entries
    for (i=0;i<FILES_PER_PAGE;i++) {
        if (list_filename[i]) {
            SDL_FreeSurface(list_filename[i]);
            list_filename[i] = NULL;
        }
    }
    
    //Write new entries
    for (i=0,j=current_list_start; i<FILES_PER_PAGE; i++,j++) {
        if (j < num_of_files) {
            list_filename[i] = render_list_text(namelist[j]->d_name);
        }
    }
}

// Determines if entry references the parent dir as ".."
int is_back_dir(const struct dirent *dr) {
    return !strcmp(dr->d_name, "..");
}

int list_filter(const struct dirent *dptr)
{
    return (can_change_dir && !at_root && is_back_dir(dptr)) 
            || dptr->d_name[0] != '.';
}

int sort_files(const struct dirent **a, const struct dirent **b) 
{
    //Note this fails for symlinks of dirs, but because the dingux runs on a Fat32 drive, this
    // shouldn't be an issue.
    if ((*a)->d_name[0] == '.' || (*b)->d_name[0] == '.') 
    {
        return is_back_dir(*a) ? -1 : 1;
    }

    int c = (*a)->d_type==DT_DIR, d=(*b)->d_type==DT_DIR;
    return c==d ? alphasort(a,b) : 
                  c ? -1 : 1;
}

int get_list(char* path)
{
    int i = 0;

    at_root = !strcmp(path,"/");
    num_of_files = scandir(path, &namelist, list_filter, (void*)sort_files);
    current_list_start = current_highlight = 0;

    if (num_of_files < 0) {
        num_of_files = 0;
        return -1;
    }
    else {
        statlist = new_array(struct stat, num_of_files);
        for (i=0;i<num_of_files;i++) {
            char *filename = new_str(strlen(path) + strlen(namelist[i]->d_name) + 2);
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
    free_erase(statlist);
    
    if (namelist) {
        int i, j;
        for (i=0;i<num_of_files;i++) free_erase(namelist[i]);
        
        free_erase(namelist);
    
        for (i=0, j=current_list_start;
            (i < FILES_PER_PAGE) && (j < num_of_files);
            i++, j++) {
            free_surface(list_filename[i]);
        }
    }
    
    current_list_start = current_highlight = num_of_files = 0;
}



int filelist_init(char* title, char* executable, char *exec_path, char* select_path, int can_change_dirs)
{
    log_debug("Initializing");
    
    //Make sure it is something
    if (executable == 0) executable = "";
    
    // load font
    list_font       = get_theme_font(18);
    list_font_color = get_theme_font_color();

    //Setup UI
    list_bg        = load_theme_image(cfg_getstr(cfg, "ListBackground"));
    list_sel       = load_theme_image(cfg_getstr(cfg, "ListSelector"));
    list_dir_icon  = load_theme_image(cfg_getstr(cfg, "ListDirIcon"));
    list_file_icon = load_theme_image(cfg_getstr(cfg, "ListFileIcon"));
    list_title     = render_list_text(title);
    status_changed = 1;

    // Prep path/executable vars, determine filelist_theme status
    strcpy(current_executable, executable);
    strcpy(work_path, exec_path);

    can_change_dir = can_change_dirs;
    
    // read files
    char real_path[PATH_MAX];
    if (!realpath(select_path, real_path)) {
        log_error("Failed to get real path of directory %s", select_path);
        strcpy(real_path, "/"); //Default to root when real_path fails
        //return 1;
    }
    if (get_list(real_path)) {
        log_error("Failed to read directory %s", real_path);
        return 1;
    }

    is_initted = 1;
    
    return 0;
}

void filelist_deinit()
{
    if (!is_initted) return;
    log_debug("De-initializing");
    
    is_initted = 0;

    free_surface(list_bg);
    free_surface(list_sel);
    free_surface(list_title);
    free_surface(list_dir_icon);
    free_surface(list_file_icon);
    free_font(list_font);
    free_color(list_font_color);

    clear_list();
}

int filelist_draw(SDL_Surface* screen)
{
    int i;
    SDL_Rect dstrect, txtrect;

    if (!status_changed) return 0;
    init_rect_pos(&dstrect, 0,0);
    init_rect_pos(&txtrect, 0,0);

    // clear screen
    //SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

    SDL_BlitSurface(background, 0, screen, &dstrect);
    SDL_BlitSurface(list_title, 0, screen, &txtrect);
    SDL_BlitSurface(list_bg,    0, screen, &dstrect);

    txtrect.y = FILE_LIST_OFFSET;
    for (i=0; i<FILES_PER_PAGE; i++) {
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
            txtrect.y += FILE_ENTRY_HEIGHT;
        }
    }

    status_changed = 0;
    return 1;
}

int filelist_animate(SDL_Surface* screen)
{
    return 0;
}

void shift_highlight(enum Direction dir) 
{
    current_highlight += (PREV==dir ? -1 : 1);
}

void shift_page_surfaces(enum Direction dir) 
{
    int i, 
        start = 0, 
        end = FILES_PER_PAGE-1, 
        delta = 1;

    if (dir == PREV) 
    {
        start = FILES_PER_PAGE-1;
        end = 0;
        delta = -1;
    }
        
    SDL_FreeSurface(list_filename[start]);
    for (i=start;i!=end;i+=delta) 
    {
        list_filename[i] = list_filename[i+delta];
    }
    
    current_list_start += delta;
    list_filename[end] = render_list_text(namelist[current_list_start+end]->d_name);
}

void wrap_page_surfaces(enum Direction dir) 
{
    int size = FILES_PER_PAGE;

    if (dir != PREV) 
    { //start at top
        current_list_start = 0; 
        current_highlight  = 0;
    } 
    else {
        current_list_start = max(num_of_files - size, 0);
        current_highlight  = min(num_of_files - 1, size - 1); 
    }
    filelist_fill();
}

void filelist_move_single(enum Direction dir) 
{
    int delta    = (dir == PREV) ? -1 : 1;
    int size     = FILES_PER_PAGE;
    int max      = num_of_files;
    int next_pos = current_highlight + delta;
    int next_abs_pos = current_list_start + next_pos; 
    
    SE_out( MENUITEM_MOVE );
    
    if (in_bounds(next_pos,0,size) && next_pos < max) //If moving within page
    {
        shift_highlight(dir);
    } 
    else if (in_bounds(next_abs_pos, 0, max)) //Slide page
    {
        shift_page_surfaces(dir);
    }
    else  //Wrap Around
    {
        wrap_page_surfaces(dir);
    }    
    
    status_changed = 1;
}


void filelist_move_page(enum Direction dir)
{
    int size = FILES_PER_PAGE, delta = (dir==PREV?-size:size);;
    int start = current_list_start;
    int next = start + delta;
    SE_out( MENU_MOVE );
    current_list_start =  bound(next, 0, max(num_of_files-size, 0));
    current_highlight = 0;
    
    filelist_fill();
    
    status_changed = 1;
}

void filelist_right()
{
    char temp_path[PATH_MAX];
    int i = current_list_start+current_highlight;

    SE_out( DECIDE );

    if (S_ISDIR(statlist[i].st_mode) && !is_back_dir(namelist[i])) {
        strcpy(temp_path, current_path);
        if (!ends_with_slash(current_path)) 
        {
            strcat(temp_path, "/");
        }
        strcat(temp_path, namelist[i]->d_name);
        clear_list();
        if (get_list(temp_path) != 0)
            get_list(current_path);
        status_changed = 1;
    }
}    

void filelist_left()
{
    SE_out( CANCEL );

    if (at_root) {
        return;
    } else {
        char* last_separator = strrchr(current_path, '/');
        if (!last_separator) {
            log_error("Unable to find path separator - %s", current_path);
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

    if (can_change_dir) {
        if (S_ISDIR(statlist[i].st_mode)) {
            if (is_back_dir(namelist[i])) {
                filelist_left();
            } else { 	 
                filelist_right();
            }
            return FILELIST;
        }
    }
    
    strcpy(file_name, current_path);
    if (!ends_with_slash(current_path)) 
    {
        strcat(file_name, "/");
    }
    strcat(file_name, namelist[i]->d_name);
    
    // If this is theme selection, we will return here from run_command();
    // If it's running a program, it will not return.
    if (current_executable[0] != '\0') 
    {
        run_command(current_executable, file_name, work_path);
    }
    else {
        run_command(file_name, NULL, current_path);
    }
    return MAINMENU;
}

enum MenuState filelist_keypress(SDLKey key)
{
    Uint8 *keystate = SDL_GetKeyState(NULL);
    enum Direction dir = getKeyDir(key);
    
    switch (key) {

        case DINGOO_BUTTON_B:
            filelist_deinit();
            return MAINMENU;
        case DINGOO_BUTTON_A:
            return filelist_run();
        case DINGOO_BUTTON_START:
            if (can_change_dir) 
                conf_selectordir(mi, current_path);
            break;
        case DINGOO_BUTTON_L:
        case DINGOO_BUTTON_R:
            filelist_move_page(dir);
            break;
        case DINGOO_BUTTON_UP:
        case DINGOO_BUTTON_DOWN:
            if (keystate[DINGOO_BUTTON_Y]) 
                filelist_move_page(dir);
            else 
                filelist_move_single(dir);
            break;
        case DINGOO_BUTTON_RIGHT:
            if (can_change_dir) filelist_right();
            break;
        case DINGOO_BUTTON_LEFT:
            if (can_change_dir) filelist_left();
            break;

        default:break;
    }

    return FILELIST;
}
