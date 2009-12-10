#ifndef _COMMON_H_
#define _COMMON_H_

#include "basec.h"
#include "confuse.h"
#include <string.h>
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <png.h>

#define COMMAND_THEMESELECT "!themeselect"
#define COMMAND_BACKGROUNDSELECT "!backgroundselect"
#define COMMAND_COLORSELECT "!colorselect"

#define free_sound(snd)    clean_erase(snd, Mix_FreeChunk)
#define free_surface(sfc)  clean_erase(sfc, SDL_FreeSurface)
#define free_font(fnt)     clean_erase(fnt, TTF_CloseFont)
#define free_erase(e)      clean_erase(e, free)
#define free_color(c)      free_erase(c)

#define internal_command(s) (s[0] == '!')

#define getKeyDir(k) ((k==DINGOO_BUTTON_L || k==DINGOO_BUTTON_Y || k==DINGOO_BUTTON_LEFT || k==DINGOO_BUTTON_UP)?PREV:NEXT)

enum MenuState { MAINMENU, FILELIST, IMAGEVIEWER, COLORPICKER };
enum InternalCommand { THEMESELECT, BACKGROUNDSELECT, COLORSELECT }; 
enum Direction { PREV, NEXT, UP, DOWN, RIGHT, LEFT };

enum InternalCommand get_command(char* cmd);

typedef struct {
    char* file;
    struct stat* orig_stat;
    SDL_Surface *surface;
} ImageExportJob;


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
char* read_first_line( char* file);

SDL_Surface* load_image_file( char* file );
SDL_Surface* load_image_file_no_alpha( char* file );
SDL_Surface* load_image_file_with_format( char* file , int alpha, int fail_on_notfound );
SDL_Surface* load_resized_image(char* file, int width, int height);
void export_image(ImageExportJob* job);

SDL_Surface* render_text( char* text, TTF_Font* font, SDL_Color* color, int solid );
SDL_Surface* draw_text( char* text, TTF_Font* font, SDL_Color* color);
SDL_Color*   parse_color_string( char* color );
SDL_Color*   load_color_file( char* file );

void  init_rect(SDL_Rect* rect, int x, int y, int w, int h);
void  init_rect_pos(SDL_Rect* rect, int x, int y);
SDL_Surface* create_surface(int w, int h, int depth, int r, int g, int b, int a);
void alphaBlendSurface( SDL_Surface* s, int alpha );
void blitSurfaceAlpha ( SDL_Surface* src, SDL_Rect* src_rect, SDL_Surface* dst, SDL_Rect* dst_rect, int alpha);
SDL_Surface* shrink_surface(SDL_Surface *src, int width, int height);
int export_surface_as_bmp(char* file_name, SDL_Surface* surface);
SDL_Surface* tint_surface(SDL_Surface* src, int color, int alpha);
SDL_Surface* copy_surface(SDL_Surface* src);
#endif
