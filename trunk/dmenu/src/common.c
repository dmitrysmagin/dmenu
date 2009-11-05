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
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <sys/mman.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "env.h"
#include "filelist.h"
#include "conf.h"
#include "main.h"
#include "menu.h"

extern char THEME_PATH[];
SDL_Surface* tmp_surface;

void run_internal_command(char* command, char* args, char* workdir);
/*
void fill_fb(Uint16* source)
{
    int fbdev;
    Uint16 *fb;
    
    fbdev=open("/dev/fb0", O_RDWR);
    fb=(Uint16 *)mmap(0, 320*240*sizeof(Uint16), PROT_WRITE, MAP_SHARED, fbdev, 0);
    memcpy(fb, source, 320*240*sizeof(Uint16));
    }
*/

/*
void run_command(char* executable, char* args, char* workdir)
{
    char exe[512];
    char cwd[512];
    int rc;
    
    int exe_len = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    exe[exe_len] = '\0';
    int cwd_len = readlink("/proc/self/cwd", cwd, sizeof(cwd) - 1);
    cwd[cwd_len] = '\0';
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s %s;cd %s;exec %s", executable, args, cwd, exe);
    
    rc = chdir(workdir);
    if (rc != 0) {
   printf("Unable to change to directory %s\n", workdir);
   perror(0);
   }
   
   execlp("/bin/sh", "/bin/sh", "-c", cmd, NULL);
   
   // it should not return, otherwise it means we are not able to execute the application
   rc = chdir(cwd);
   if (rc != 0) {
   printf("Unable to change to directory %s\n", cwd);
   perror(0);
   }
   execlp(exe, exe, NULL);
   }
*/

void clear_last_command()
{
    remove(DMENU_COMMAND_FILE);
}

/* this is a simplified version when init is used to spawn dmenu */
void run_command(char* executable, char* args, char* workdir)
{   
    log_debug("Running Command: %s, %s, %s", executable, args, workdir);
    
    if (internal_command(executable)) {
        run_internal_command(executable, args, workdir);
        return;
    }
    
    char** args_list = build_arg_list(executable, args);
    //Make local copy of work dir
    
    char tmp_work[PATH_MAX]; strcpy(tmp_work, "");
    if (workdir != NULL) strcpy(tmp_work, workdir);
    
    // launch the program
    execute_next_command(tmp_work, args_list);
    //execute_command(tmp_work, args_list);
}

void run_internal_command(char* command, char* args, char* workdir)
{
    if (!args) return;

    switch (get_command(command)) 
    {
        case THEMESELECT:
            conf_themeselect(args);
            break;
        case BACKGROUNDSELECT:
            conf_backgroundselect(args);
            break;
        default: break;
    }
}

void free_arg_list(char** args)
{
    int i =0;
    while (*(args + i)) i++;
    while (i--) free(args[i]);
    free(args);
}

char** build_arg_list(char* commandline, char* args) 
{
    // build the args list for exec()
    char** args_list = NULL;
    char* token, *temp;
    int len = 0;
    const char delimeter[] = " ";
    
    while ((token = strsep(&commandline, delimeter))) {
        if (token[0] != '\0') {
            copy_str(temp, token);
            append_str(args_list, len, temp);
        }
    }
    
    // filelist.c will pass the selected filename in args. filename
    // may contain spaces.
    char *filename = NULL;
    if (args) {
        copy_str(filename, args);
        append_str(args_list, len, filename);
    }
    
    // add null pointer as the last arg
    append_str(args_list, len, NULL);
    
    return args_list;
}

void execute_next_command(char* dir, char** args) 
{   
    //Write next command to commandfile
    FILE* out = load_file(DMENU_COMMAND_FILE, "w");
    
    //Write change dir
    if (dir!=NULL && strlen(dir) > 0)
    {
        fprintf(out, "cd %s\n", dir);
    }
    
    //Write command
    int i = 0;
    for (i=0;*(args+i);i++) 
    {
        fprintf(out, "\"%s\" ", *(args+i)); 
    }    
    fprintf(out, "\n");
    fclose(out);

    free_arg_list(args);
    
    //Exit program, and let shell script call DMENU_COMMAND_FILE
    quit();
}

void execute_command(char* dir, char** args) 
{    
    deinit();
    SDL_Quit();
    
    int i = 0;
    
    if (dir!=NULL && strlen(dir) > 0)
    {
        change_dir(dir);
    }
    
    // launch the program and it should not return, 
    // otherwise it means we are not able to execute the application
    execvp(args[0], args);
    
    log_message("Unable to execute command - ");
    for (i=0;*(args+i);i++) printf("\"%s\" ", *(args+i)); printf("\n");

    // it should not return, otherwise it means we are not able to execute the application
    free_arg_list(args);
    
    quick_quit();
}

#define IF_CMD_THEN(c, k) if (strcmp(c, COMMAND_##k)==0) return k;
enum InternalCommand get_command(char* cmd) {
    IF_CMD_THEN(cmd, THEMESELECT);
    IF_CMD_THEN(cmd, BACKGROUNDSELECT);
    return 0;
}

int bound(int val, int low, int high) {
    return val < low ? low : ( val > high ? high : val);
}

int wrap(int val, int low, int high) {
    return val < low ? high : (val > high ? low : val);
}

void _foreach(void** array, void* (f)(void*), int len) {
    int i=-1;
    while (++i<len) f(array[i]);
}

char* relative_file(char* root, char* file) 
{    
    char* out = new_str(PATH_MAX); out[0] = '\0';
    if (file[0] != '/') strcat(out, root); //If it really is relative
    strcat(out, file);
    return out;
}

char* dmenu_file(char* file) 
{
    return relative_file(DMENU_PATH, file);    
}

char* home_file(char* file) 
{
    return relative_file(HOME_PATH, file);    
}

char* user_file(char* file) 
{
    return relative_file(USER_PATH, file);
}

char* theme_file(char* file) 
{
    return relative_file(THEME_PATH, file);
}

int change_dir(char* path) 
{
    int rc = chdir(path);
    if (rc) {
        log_error("Unable to change to directory %s", path);
    }
    return rc;
}

FILE* load_file_and_handle_fail(char* file, char* mode, int die_on_fail) {
    log_debug("Opening file: %s", file);
    
    FILE* out = fopen(file, mode);    
    if (out == NULL) {
        log_error("Failed to open %s", file);
        if (die_on_fail) exit(EXIT_FAILURE);
    }
    return out;
}

FILE* load_file_or_die(char* file, char* mode) {
    return load_file_and_handle_fail(file,mode,1);
}

FILE* load_file(char* file, char* mode) {
    return load_file_and_handle_fail(file,mode,0);
}

SDL_Surface* load_image_file_with_format( char* file , int alpha ) {
    SDL_Surface* out = NULL;

    tmp_surface = IMG_Load(file);
    if (tmp_surface == NULL) {
        log_error("Failed to load %s: %s", file, IMG_GetError());
        exit(EXIT_FAILURE);
    }
    
    if ( alpha ) {
        out = SDL_DisplayFormatAlpha(tmp_surface);
    } else {
        out = SDL_DisplayFormat(tmp_surface);
    } 
    
    SDL_FreeSurface(tmp_surface);
    return out;
}

SDL_Surface* load_image_file( char* file ) {
    return load_image_file_with_format(file, 1);
}

SDL_Surface* load_image_file_no_alpha( char* file ) {
    return load_image_file_with_format(file, 0);
}

SDL_Surface* load_user_image( char* file ) {
    char* tmp = user_file(file);
    SDL_Surface* out = load_image_file(tmp);
    free(tmp); 
    return out;
}

SDL_Color* load_user_color ( char* file ) {
    char* tmp = user_file(file);
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

SDL_Surface* render_text(char* text, TTF_Font* font, SDL_Color* color, int solid) {
    SDL_Surface* out = NULL;
    if (solid == 0) {
        tmp_surface = TTF_RenderUTF8_Blended(font, text, *color);
    } else {
        tmp_surface = TTF_RenderUTF8_Solid(font, text, *color);
    }
    out = SDL_DisplayFormatAlpha(tmp_surface);
    SDL_FreeSurface(tmp_surface);
    return out;
}

SDL_Surface* draw_text(char* text, TTF_Font* font, SDL_Color* color) {
    return render_text(text, font, color, 0);
}

SDL_Color* parse_color_string(char* str) {
    SDL_Color* out = new_item(SDL_Color);
    SDL_Color color = {255,255,255,0};
    int r = 0, g = 0, b = 0;
    char* format;

    switch (str[0]) {
        case '#':
            format = "#%2x%2x%2x"; // #RRGGBB
            break;
        case '(':
            format = "(%d,%d,%d)"; // (R,G,B)
            break;
        default:
            format = "%d,%d,%d"; // R,G,B
    }
    
    sscanf(str, format, &r, &g, &b);
    color.r = (Uint8)r;
    color.g = (Uint8)g;
    color.b = (Uint8)b;
    memcpy(out, &color, sizeof(SDL_Color));
    return out;
}

SDL_Color* load_color_file(char* filename) {
    char color[12];
    FILE* fd = load_file_or_die(filename,"r");
    int i = fscanf(fd, "%s", color);
    fclose(fd);
    return (i == 0 || i == EOF) ? NULL : parse_color_string(color);
}

void init_rect_pos(SDL_Rect* rect, int x, int y) {
    init_rect(rect, x, y, -1, -1);
}

void  init_rect(SDL_Rect* rect, int x, int y, int w, int h) {
    rect->x = x;
    rect->y = y;
    if (w>=0) rect->w = w;
    if (h>=0) rect->h = h;
}

/**
 * Optimized for 16bit images.  Will not work any any others
 */
SDL_Surface* shrink_surface(SDL_Surface *src, double factor)
{
    if(!src || factor > 1 || factor <= 0) return NULL;
    
    double fp = 1/factor;
    int y,x,w = (int)(src->w * factor), h = (int)(src->h * factor);
    
    SDL_Surface *dst = SDL_CreateRGBSurface(
        src->flags, w, h, SCREEN_COLOR_DEPTH,
        src->format->Rmask, src->format->Gmask, 
        src->format->Bmask, src->format->Amask);

    register int src_pitch = src->pitch, dst_pitch = dst->pitch;
    register int src_y_off = 0, dst_y_off = 0, dst_x_off=0;
    Uint16 *src_pixels = (Uint16*)src->pixels, *dst_pixels = (Uint16*)dst->pixels;
    int max_w = SCREEN_BPP*w;
    
    y = h;
    while (y--) {
        
        dst_y_off = dst_pitch*y;
        src_y_off = src_pitch*(int)(y*fp);
        dst_x_off = max_w;
        
        x= w;
        while (x--) {
            dst_x_off -= SCREEN_BPP;
            *(Uint16*)((int)dst_pixels + dst_y_off+dst_x_off) = 
                *(Uint16*)((int)src_pixels + src_y_off+(int)(x*fp)*SCREEN_BPP);
        }
    }
    
    return dst;
}

SDL_Surface* create_surface(int w, int h, int r, int g, int b, int a)
{
    SDL_Surface *tmp;
    Uint32 rmask, gmask, bmask, amask;
    
    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
    on the endianness (byte order) of the machine */
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
    #else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
    #endif
    
    tmp =  SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, rmask, gmask, bmask, a < 0 ? 0 : amask);
    
    if (a >= 0) 
    {
        SDL_FillRect(tmp, 0, SDL_MapRGBA(tmp->format, r,g,b,a));
    } 
    else 
    {
        SDL_FillRect(tmp, 0, SDL_MapRGB(tmp->format, r,g,b));
    }
    return tmp;
}
 
