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

char* global_file(char* file) 
{
    return relative_file(GLOBAL_RESOURCE_PATH, file);
}

char* dmenu_file(char* file) 
{
    if (strstr(file, GLOBAL_RESOURCE_PRE) != NULL) {
        return global_file(strchr(file, ':')+1);
    } else {    
        return relative_file(DMENU_PATH, file);
    }
}

char* theme_file(char* file) 
{
    if (strstr(file, GLOBAL_RESOURCE_PRE) != NULL) {
        return global_file(strchr(file, ':')+1);
    } else {    
        return relative_file(THEME_PATH, file);
    }
}

int get_theme_list(char* path, char*** files) 
{
    struct dirent** dir_files;
    struct stat file_stat;
    char name[PATH_MAX], *tmp;
    int i, pos=0, cnt=max(scandir(path, &dir_files, NULL, NULL), 0);
    char* themes[100];

    for (i=0;i<cnt;i++) {
        tmp = dir_files[i]->d_name;
        if (tmp != NULL && tmp[0] != '.') {
            strcpy(name, DMENU_THEMES);
            strcat(name, tmp);
            strcat(name, "/theme.cfg");

            if (stat(name, &file_stat) == 0) {
                strcpy(name, DMENU_THEMES);
                strcat(name, tmp);
                themes[pos++] = strdup(name);
            }
        }
        free_erase(dir_files[i]);
    }
    free_erase(dir_files);
    
    char** out = new_array(char*, pos);
    for (i=0;i<pos;i++) out[i] = themes[i];
    *files = out;
    
    return pos;
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

void make_menu_snapshot(SDL_Surface* screen, int blur) 
{
    if (blur) {
        SDL_Surface* tmp = create_surface(screen->w, screen->h, 32, 0, 255);
        SDL_BlitSurface(tmp, 0, screen, 0);        
        SDL_Flip(screen);
        free_surface(tmp);
    }
}

void save_menu_snapshot(SDL_Surface* screen)
{
    export_surface_as_bmp(DMENU_SNAPSHOT, screen);
}

SDL_Surface* load_osd_image( char* file ) {
	SDL_Color* dosd_color = get_theme_font_color();
	int color = 0;
	color += (dosd_color->r) << 16;
	color += (dosd_color->g) << 8;
	color += dosd_color->b;
	SDL_Surface* tmp = load_global_image(file);
	SDL_Surface* ret = tint_surface(tmp, color, 0xFF);
	free_surface(tmp);
	free_color(dosd_color);
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
