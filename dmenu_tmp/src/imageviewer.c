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
#include <limits.h>
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "conf.h"
#include "env.h"
#include "imageviewer.h"
#include "resource.h"
#include "menu.h"
#include "sound.h"
#include "dingoo.h"

#define IMAGE_VIEWER_TITLE_COLOR 152,152,152
#define IMAGE_VIEWER_SELECT_COLOR 255,255,255
#define IMAGE_VIEWER_BACKGROUND_COLOR 48,48,48
#define IMAGE_VIEWER_TITLE_ALPHA 152
#define IMAGE_VIEWER_SELECT_ALPHA 64

extern SDL_Surface* background;
extern cfg_t *cfg;

SDL_Surface* image_preview;

SDL_Surface* imageviewer_highlight;
SDL_Surface* imageviewer_osd_highlight;
SDL_Surface* imageviewer_title;
TTF_Font*    imageviewer_font;
SDL_Color*   imageviewer_font_color;

typedef struct Paginate {
    int total_size;
    int set_size;
    int page;
    int is_ready;
    
    int absolute_pos;
    int relative_pos;
    int new;
    
    int state_changed;
    char title[PATH_MAX];
    char executable[PATH_MAX];
    char root[PATH_MAX];
    struct dirent **files;
    SDL_Surface** entries;
} Paginate;

Paginate iv_paginate;

SDL_Surface* render_imageviewer_text(char* text) {
    return draw_text(text, imageviewer_font, imageviewer_font_color);
}

char* IMAGE_TYPES[3] = {".jpg", ".png", ".bmp"};
int filter(const struct dirent *dptr)
{
    char *name = (char*)dptr->d_name;
    return strstr(name, IMAGE_TYPES[0]) != NULL 
        || strstr(name, IMAGE_TYPES[1]) != NULL 
        || strstr(name, IMAGE_TYPES[2]) != NULL;
}

int get_imagelist(char* path)
{
    iv_paginate.total_size = max(scandir(path, &iv_paginate.files, filter, alphasort), 0);
    return iv_paginate.total_size == 0 ? 1 : 0;
}

#define get_root_file(buff, i)\
    strcpy(buff, iv_paginate.root);\
    strcat(buff,"/");\
    strcat(buff, iv_paginate.files[i]->d_name)

void reset_pagination()
{
    iv_paginate.set_size = IMAGE_THUMBS_PER_PAGE;
    iv_paginate.is_ready = 0;
    iv_paginate.total_size = 0;
    iv_paginate.relative_pos = 0;
    iv_paginate.absolute_pos = 0;
    iv_paginate.page = 0;
    iv_paginate.title[0] = '\0';
    iv_paginate.executable[0] = '\0';
}

int imageviewer_init(char* title, char* executable, char* path)
{
    log_debug("Initializing");
    
    reset_pagination();
    
    // try to read files before we do anything else
    if (!realpath(path, iv_paginate.root)) {
        log_error("Failed to get real path of directory %s", path);
        return 1;
    }
    
    if (get_imagelist(iv_paginate.root)) {
        log_error("Failed to read directory %s", iv_paginate.root);
        return 1;
    }
    
    if (title != NULL) strcpy(iv_paginate.title, title);
    if (executable != NULL) strcpy(iv_paginate.executable, executable);
    
    //Setup UI
    if (strlen(iv_paginate.title) > 0) 
    {
        // load font
        imageviewer_font       = get_theme_font(14);
        imageviewer_font_color = get_theme_font_color();
        
        imageviewer_title     = render_imageviewer_text(title);
        imageviewer_osd_highlight = create_surface(
                SCREEN_WIDTH, 20, 
                IMAGE_VIEWER_TITLE_COLOR, 
                IMAGE_VIEWER_TITLE_ALPHA);
    }
    
    imageviewer_highlight = create_surface(
                IMAGE_THUMB_WIDTH, IMAGE_THUMB_HEIGHT, 
                IMAGE_VIEWER_SELECT_COLOR,
                IMAGE_VIEWER_SELECT_ALPHA);
                    
    iv_paginate.entries = new_array(SDL_Surface*, iv_paginate.total_size);
    imageviewer_update_list();
    imageviewer_update_preview();
    
    iv_paginate.state_changed = 1;
    iv_paginate.is_ready = 1;

    return 0;
}

void imageviewer_deinit()
{
    int i = 0;
    if (!iv_paginate.is_ready) return;
    log_debug("De-initializing");
    
    free_surface(image_preview);
    free_surface(imageviewer_highlight);
    free_surface(imageviewer_osd_highlight);
    free_surface(imageviewer_title);
    free_color(imageviewer_font_color);
    free_font(imageviewer_font);
    
    for (i=0;i<iv_paginate.total_size;i++) free_erase(iv_paginate.files[i]);
    for (i=0;i<iv_paginate.set_size;i++) free_surface(iv_paginate.entries[i]);
    
    free_erase(iv_paginate.files);
    free_erase(iv_paginate.entries);
    
    reset_pagination();
    iv_paginate.is_ready = 0;
}

void imageviewer_draw(SDL_Surface* screen)
{
    if (!iv_paginate.state_changed) return;
    
    int i;
    SDL_Rect dstrect, txtrect;

    // clear screen
    SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, IMAGE_VIEWER_BACKGROUND_COLOR));
    
    //Draw thumbnails
    init_rect_pos(&dstrect, IMAGE_THUMB_PAD_X,IMAGE_THUMB_TOP+IMAGE_THUMB_PAD_Y);
    for (i = 0; i < iv_paginate.set_size;i++) 
    {
        if (iv_paginate.entries[i]) {
            SDL_BlitSurface(iv_paginate.entries[i],   0, screen, &dstrect);
        }
        dstrect.x += IMAGE_THUMB_WIDTH;
    }
    
    //Draw highlight for active item
    init_rect_pos(&dstrect, (iv_paginate.relative_pos)*IMAGE_THUMB_WIDTH, IMAGE_THUMB_TOP);
    SDL_BlitSurface(imageviewer_highlight, 0, screen, &dstrect);
    
    //Draw image_preview
    init_rect_pos(&dstrect, screen->w/2-image_preview->w/2, (screen->h-IMAGE_THUMB_HEIGHT)/2-image_preview->h/2);
    SDL_BlitSurface(image_preview, 0, screen, &dstrect);

    //Draw top message
    init_rect_pos(&txtrect, 0,0);
    SDL_BlitSurface(imageviewer_osd_highlight, 0, screen, &txtrect);
    SDL_BlitSurface(imageviewer_title, 0, screen, &txtrect);
    
    iv_paginate.state_changed = 0;
}


void imageviewer_update_list()
{
    int i,j;
    int size = iv_paginate.set_size;
    
    for (i=0;i<size;i++)
    {
        if (iv_paginate.entries[i] && iv_paginate.is_ready)
        {
            SDL_FreeSurface(iv_paginate.entries[i]);
        }
        iv_paginate.entries[i] = NULL;
    }
    
    int start = iv_paginate.page * size;
    char tmp[PATH_MAX];
    SDL_Surface *tmp_surf;
    
    for (i=0,j=start;j<iv_paginate.total_size&&i<size;i++,j++) 
    {
        get_root_file(tmp, j);
        tmp_surf = load_image_file_no_alpha(tmp);
        iv_paginate.entries[i] = shrink_surface(tmp_surf, IMAGE_THUMB_RATIO_INNER);
        SDL_FreeSurface(tmp_surf);
    }
}

void imageviewer_update_preview() 
{
    if (image_preview) SDL_FreeSurface(image_preview);
    char str[PATH_MAX];
    get_root_file(str, iv_paginate.absolute_pos);
    SDL_Surface* tmp = load_image_file_no_alpha(str);
    image_preview = shrink_surface(tmp, IMAGE_PREVIEW_RATIO);
    SDL_FreeSurface(tmp);
}

void imageviewer_move_page(enum Direction dir)
{
    int delta = (dir == PREV) ? -1 : 1;
    int prev_p = iv_paginate.page;
    iv_paginate.page = bound(iv_paginate.page + delta, 0, iv_paginate.total_size/iv_paginate.set_size);
    
    SE_out( MENU_MOVE );
    
    if (prev_p != iv_paginate.page)
    {
        iv_paginate.absolute_pos  = iv_paginate.page*iv_paginate.set_size;
        iv_paginate.relative_pos  = 0;
        iv_paginate.state_changed = 1;
        imageviewer_update_list();
        imageviewer_update_preview();
    }
}

void imageviewer_move(enum Direction dir) 
{
    int delta = (dir == PREV) ? -1 : 1;
    int prev = iv_paginate.absolute_pos;
    int prev_p = iv_paginate.page;
    
    iv_paginate.absolute_pos = bound(iv_paginate.absolute_pos + delta, 0, iv_paginate.total_size-1);
    iv_paginate.state_changed = prev != iv_paginate.absolute_pos;
    iv_paginate.page = iv_paginate.absolute_pos/iv_paginate.set_size;
 
    SE_out( MENUITEM_MOVE );
    
    if (prev_p == iv_paginate.page) {
        iv_paginate.relative_pos = iv_paginate.absolute_pos % iv_paginate.set_size;
    } else {
        iv_paginate.relative_pos = (dir == PREV ? iv_paginate.set_size-1 : 0);
        imageviewer_update_list();
    }
    
    if (iv_paginate.state_changed) {
        imageviewer_update_preview();
    }
}

enum MenuState imageviewer_select()
{
    SE_out( DECIDE );
    
    char tmp[PATH_MAX];
    get_root_file(tmp, iv_paginate.absolute_pos);

    if (strlen(iv_paginate.executable) > 0) 
    {
        run_command(iv_paginate.executable, tmp, iv_paginate.root);
    }
    
    imageviewer_deinit();
    
    return MAINMENU;
}

enum MenuState imageviewer_keypress(SDLKey keysym)
{
    switch (keysym) {
        case DINGOO_BUTTON_B:
            SE_out( CANCEL );
            imageviewer_deinit();
            return MAINMENU;
        case DINGOO_BUTTON_A:
            return imageviewer_select();
        case DINGOO_BUTTON_RIGHT:
            imageviewer_move(NEXT);
            break;
        case DINGOO_BUTTON_LEFT:
            imageviewer_move(PREV);
            break;
        case DINGOO_BUTTON_L:
            imageviewer_move_page(PREV);
            break;
        case DINGOO_BUTTON_R:
            imageviewer_move_page(NEXT);
            break;            
        default: break;
    }
    
    return IMAGEVIEWER;
}