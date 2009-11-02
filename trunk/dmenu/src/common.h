#ifndef _COMMON_H_
#define _COMMON_H_

#include "confuse.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#define COMMAND_THEMESELECT "!themeselect"
#define COMMAND_BACKGROUNDSELECT "!backgroundselect"

#define new_array(t, len) (t*)malloc(sizeof(t) * len)
#define new_str(len) new_array(char, len)
#define new_item(t) new_array(t, 1)
#define copy_item(dst, src, t) memcpy(dst,src,sizeof(t))
#define copy_str(tmp, str) tmp = new_str(strlen(str)); strcpy(tmp, str)

#define in_bounds(v, l, h) ((v>=l) && (v<h))
#define min(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)
#define internal_command(s) (s[0] == '!')

#define _log(dst, fmt, args...)   fprintf(dst, "%-15s(%4d): "fmt "\n", strstr(__FILE__, "src"), __LINE__, ##args)
#define log_error(fmt, args...)   _log(stderr, fmt, ##args); perror(0)
#define log_message(fmt, args...) _log(stdout, fmt, ##args)
#define foreach(arr, fn, len) _foreach((void*)arr, (void*)fn, len)
#define append_str(lst, len, val)\
    len++;\
    lst = realloc(lst, len*sizeof(char*));\
    lst[len-1] = val;
    

#define get_pixel(s,x,y) *(Uint16 *)(s->pixels + y * s->pitch + x*SCREEN_BPP)
#define put_pixel(s,x,y,c) get_pixel(s,x,y) = c

enum MenuState { MAINMENU, FILELIST, IMAGEVIEWER };
enum InternalCommand { THEMESELECT, BACKGROUNDSELECT }; 
enum Direction { PREV, NEXT, UP, DOWN, RIGHT, LEFT };

//void fill_fb(Uint16* source);
enum InternalCommand get_command(char* cmd);

int bound(int val, int low, int high);
int wrap(int val, int low, int high);

char* relative_file(char* path, char* file);
char* dmenu_file(char* file);
char* home_file(char* file);
char* user_file(char* file);
char* theme_file(char* file);
void _foreach(void** array, void* (f)(void*), int len);

void run_command (char* cmd, char* args, char* workdir);
void run_internal_command (char* cmd, char* args, char* workdir);
void execute_command(char** args);
char** build_arg_list(char* commandline, char* args); 
void free_arg_list(char** args);
int change_dir(char* path);

FILE* open_file_and_handle_fail ( char* file, char* mode, int die_on_fail );
FILE* open_file( char* file, char* mode );
FILE* open_file_or_die( char* file, char* mode );

SDL_Surface* load_image_file( char* file );
SDL_Surface* load_image_file_no_alpha( char* file );
SDL_Surface* load_user_image( char* file );
SDL_Color*   load_user_color( char* file );
SDL_Surface* load_theme_image( char* file );
SDL_Surface* load_theme_background( char* file );
TTF_Font*    load_theme_font( char* file, int size );
Mix_Music*   load_theme_sound( char* file );

SDL_Surface* render_text( char* text, TTF_Font* font, SDL_Color* color, int solid );
SDL_Surface* draw_text( char* text, TTF_Font* font, SDL_Color* color);
SDL_Color*   parse_color_string( char* color );
SDL_Color*   load_color_file( char* file );

void  init_rect(SDL_Rect* rect, int x, int y, int w, int h);
void  init_rect_pos(SDL_Rect* rect, int x, int y);
SDL_Surface* create_alpha_surface(int w, int h, int r, int g, int b, int a);
SDL_Surface* shrink_surface(SDL_Surface *src, double factor);
#endif