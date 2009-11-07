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

TTF_Font* get_theme_font(int size)
{
    return load_theme_font(get_user_attr("Font"), size);
}

SDL_Color* get_theme_font_color()
{
    SDL_Color* out = NULL;
    char* color = get_user_attr("FontColor");
    if (color == 0) 
    {
        out = load_global_color("fontcolor.ini");
    } 
    else {
        out = parse_color_string(color);
    }
    return out;
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

SDL_Surface* load_global_image( char* file ) {
    char* tmp = global_file(file);
    SDL_Surface* out = load_image_file(tmp);
    free(tmp); 
    return out;
}

SDL_Color* load_global_color ( char* file ) {
    char* tmp = global_file(file);
    SDL_Color* out = load_color_file(tmp);
    free(tmp);
    return out;
}

SDL_Surface* load_theme_background( char* file ) {
    char* tmp = theme_file(file);
    SDL_Surface* out = load_image_file_no_alpha(tmp);
    free(tmp); 
    return out;
}

SDL_Surface* load_theme_image( char* file ) {
    char* tmp = theme_file(file);
    SDL_Surface* out = load_image_file(tmp);
    free(tmp); 
    return out;
}

Mix_Music* load_theme_sound( char* file ) {
    char* tmp = theme_file(file);
    Mix_Music* out = Mix_LoadMUS(tmp);
    free(tmp); 
    return out;
}

TTF_Font* load_theme_font( char* file, int size ) {
    char* tmp = theme_file(file);
    TTF_Font* out = TTF_OpenFont(tmp, size);
    free(tmp); 
    return out;
}