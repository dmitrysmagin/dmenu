#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

char* get_user_attr(char*attr);
TTF_Font* get_theme_font(int size);
char** get_theme_previews();
SDL_Color* get_theme_font_color();
char* get_theme_font_color_string();
SDL_Surface* get_theme_background();
void show_menu_snapshot(SDL_Surface* screen);
void save_menu_snapshot(SDL_Surface* screen, int blur);

char* dmenu_file(char* file);
char* global_file(char* file);
char* theme_file(char* file);

SDL_Surface* load_global_image( char* file );
SDL_Color*   load_global_color( char* file );
TTF_Font*    load_global_font( char* file, int size);
SDL_Surface* load_theme_image( char* file );
SDL_Surface* load_theme_background( char* file );
TTF_Font*    load_theme_font( char* file, int size );
Mix_Chunk*   load_theme_sound( char* file );

#endif
