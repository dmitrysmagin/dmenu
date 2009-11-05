#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

char* get_user_attr(char*attr);
TTF_Font* get_theme_font(int size);
SDL_Color* get_theme_font_color();

char* dmenu_file(char* file);
//	char* home_file(char* file);
char* user_file(char* file);
char* theme_file(char* file);

SDL_Surface* load_image_file( char* file );
SDL_Surface* load_image_file_no_alpha( char* file );
SDL_Surface* load_user_image( char* file );
SDL_Color*   load_user_color( char* file );
SDL_Surface* load_theme_image( char* file );
SDL_Surface* load_theme_background( char* file );
TTF_Font*    load_theme_font( char* file, int size );
Mix_Music*   load_theme_sound( char* file );

#endif
