/*
*  Copyright (C) 2009 Timothy Soehnlin <timothy.soehnlin@gmail.com>
*
*  Author: <timothy.soehnlin@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "env.h"
#include "conf.h"
#include "common.h"
#include "resource.h"

extern char THEME_PATH[];
extern cfg_t *cfg, *cfg_main;

char* get_user_attr(char* attr)
{
    char* val = cfg_getstr(cfg_main, attr);
    if (val == 0) {
        val = cfg_getstr(cfg, attr);
    }
    return val;
}

char** get_theme_previews() 
{
    struct dirent** dir_files;
    struct stat     file_stat;
    char name[PATH_MAX], *tmp;
    int i, pos=0, cnt=max(scandir(DMENU_THEMES, &dir_files, NULL, NULL), 0);
    char** out = new_str_arr(0);
    
    for (i=0;i<cnt;i++) {
        tmp = dir_files[i]->d_name;
        if (tmp != NULL && tmp[0] != '.') {
            sprintf(name, "%s%s/theme.png", DMENU_THEMES, tmp);
            
            if (stat(name, &file_stat)==0) { //If file is there
                append_str(out, pos, strdup(name));
            }
        }
        free_erase(dir_files[i]);
    }
    append_str(out, pos, NULL);
    
    free_erase(dir_files);
    return out;
}

TTF_Font* get_theme_font(int size)
{
    return load_theme_font(get_user_attr("Font"), size);
}

SDL_Surface* get_theme_background()
{
    return load_theme_background(get_user_attr("Background"));
}

char* get_theme_font_color_string()
{
    return get_user_attr("FontColor");
}

SDL_Color* get_theme_font_color()
{
    char* tmp = get_theme_font_color_string();
    return parse_color_string(tmp);
}

void show_menu_snapshot(SDL_Surface* screen)
{
    SDL_Surface* tmp = load_image_file_with_format(DMENU_SNAPSHOT,0,0);
    if (tmp) {
        SDL_BlitSurface(tmp, 0, screen, 0);
        SDL_Flip(screen);
        free_surface(tmp);
    }    
}

void save_menu_snapshot(SDL_Surface* screen, int blur)
{
    if (blur) {
        SDL_Surface* tmp = create_surface(screen->w, screen->h, 32, EXIT_TINT_COLOR);
        SDL_BlitSurface(tmp, 0, screen, 0);        
        SDL_Flip(screen);
        free_surface(tmp);
    }
    export_surface_as_bmp(DMENU_SNAPSHOT, screen);
}

char* dmenu_file(char* file) 
{
    return relative_file(DMENU_PATH, file);    
}

char* global_file(char* file) 
{
    return relative_file(GLOBAL_RESOURCE_PATH, file);
}

char* theme_file(char* file) 
{
    return relative_file(THEME_PATH, file);
}

SDL_Surface* load_osd_image( char* file ) {
    SDL_Surface* tmp = load_global_image(file);
    SDL_Surface* ret = tint_surface(tmp, DOSD_COLOR, 0xFF);
    free_surface(tmp);
    return ret;
}

SDL_Surface* load_global_image( char* file ) {
    char* tmp = global_file(file);
    SDL_Surface* out = load_image_file(tmp);
    free_erase(tmp); 
    return out;
}

SDL_Color* load_global_color ( char* file ) {
    char* tmp = global_file(file);
    SDL_Color* out = load_color_file(tmp);
    free_erase(tmp);
    return out;
}

TTF_Font* load_global_font( char* file, int size)
{   
    char* tmp = global_file(file);
    TTF_Font* out = TTF_OpenFont(tmp, size);
    free_erase(tmp);
    return out;
}

SDL_Surface* load_theme_background( char* file ) {
    char* tmp = theme_file(file);
    SDL_Surface* out = load_image_file_no_alpha(tmp);
    free_erase(tmp); 
    return out;
}

SDL_Surface* load_theme_image( char* file ) {
    char* tmp = theme_file(file);
    SDL_Surface* out = load_image_file(tmp);
    free_erase(tmp); 
    return out;
}

Mix_Chunk* load_theme_sound( char* file ) {
    char* tmp = theme_file(file);
    Mix_Chunk* out = Mix_LoadWAV(tmp);
    free_erase(tmp); 
    return out;
}

TTF_Font* load_theme_font( char* file, int size ) {
    char* tmp = theme_file(file);
    TTF_Font* out = TTF_OpenFont(tmp, size);
    free_erase(tmp); 
    return out;
}
