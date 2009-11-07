#ifndef _COMMON_H_
#define _COMMON_H_

#include "confuse.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <png.h>

#define COMMAND_THEMESELECT "!themeselect"
#define COMMAND_BACKGROUNDSELECT "!backgroundselect"

#define new_array(t, len)      (t*)malloc(sizeof(t) * len)
#define new_str(len)           new_array(char, len)
#define new_item(t)            new_array(t, 1)
#define copy_item(dst, src, t) memcpy(dst,src,sizeof(t))
#define copy_str(tmp, str)     tmp = new_str(strlen(str)+1); strcpy(tmp, str)
#define clean_erase(e,f)       if (e) { f(e); e = NULL; } 
#define free_surface(sfc)      clean_erase(sfc, SDL_FreeSurface)
#define free_font(fnt)         clean_erase(fnt, TTF_CloseFont);
#define free_erase(e)          clean_erase(e, free);
#define free_color(c)          free_erase(c);

#define in_bounds(v, l, h) ((v>=l) && (v<h))
#define min(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)
#define internal_command(s) (s[0] == '!')

#define _log(dst, pre, fmt, args...) fprintf(dst, "%-20s(%4d)[%s]: "fmt "\n", strstr(__FILE__, "src"), __LINE__, pre, ##args)
#define log_error(fmt, args...)      _log(stderr, "err", fmt, ##args); perror(0)
#define log_message(fmt, args...)    _log(stdout, "msg", fmt, ##args)
#if DEBUG==1
    #define log_debug(fmt, args...) _log(stdout, "dbg", fmt, ##args)
#else
    #define log_debug(args...)
#endif

#define foreach(arr, fn, len) _foreach((void*)arr, (void*)fn, len)

#define new_str_arr(l) new_array(char*, l)
void free_str_arr(char** arr);
#define append_str(lst, len, val)\
    len++;\
    lst = realloc(lst, len*sizeof(char*));\
    lst[len-1] = val;
    
enum MenuState { MAINMENU, FILELIST, IMAGEVIEWER };
enum InternalCommand { THEMESELECT, BACKGROUNDSELECT }; 
enum Direction { PREV, NEXT, UP, DOWN, RIGHT, LEFT };

//void fill_fb(Uint16* source);
enum InternalCommand get_command(char* cmd);

int bound(int val, int low, int high);
int wrap(int val, int low, int high);

void _foreach(void** array, void* (f)(void*), int len);

void run_command (char* cmd, char* args, char* workdir);
void run_internal_command (char* cmd, char* args, char* workdir);
void execute_command(char* dir, char** args);
void execute_next_command(char* dir, char** args);
void clear_last_command();
char** build_arg_list(char* commandline, char* args); 
void free_arg_list(char** args);
int change_dir(char* path);

FILE* load_file_and_handle_fail ( char* file, char* mode, int die_on_fail );
FILE* load_file( char* file, char* mode );
FILE* load_file_or_die( char* file, char* mode );
char* relative_file(char* path, char* file);

SDL_Surface* load_image_file( char* file );
SDL_Surface* load_image_file_no_alpha( char* file );
SDL_Surface* load_image_file_with_format( char* file , int alpha, int fail_on_notfound );
SDL_Surface* load_resized_image(char* path, char* file, float ratio);

SDL_Surface* render_text( char* text, TTF_Font* font, SDL_Color* color, int solid );
SDL_Surface* draw_text( char* text, TTF_Font* font, SDL_Color* color);
SDL_Color*   parse_color_string( char* color );
SDL_Color*   load_color_file( char* file );

void  init_rect(SDL_Rect* rect, int x, int y, int w, int h);
void  init_rect_pos(SDL_Rect* rect, int x, int y);
SDL_Surface* create_surface(int w, int h, int r, int g, int b, int a);
SDL_Surface* shrink_surface(SDL_Surface *src, double factor);
int export_surface_as_bmp(char* file_name, SDL_Surface* surface);
#endif
