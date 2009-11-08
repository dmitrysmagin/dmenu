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
#include "common.h"
#include "imageviewer.h"
#include "resource.h"
#include "menu.h"
#include "sound.h"
#include "dingoo.h"

extern SDL_Surface* background;
extern cfg_t *cfg;

SDL_Surface* imageviewer_preview;
SDL_Surface* imageviewer_background;
SDL_Surface* imageviewer_highlight;
SDL_Surface* imageviewer_title_bg;
SDL_Surface* imageviewer_title;
TTF_Font*    imageviewer_font;
SDL_Color*   imageviewer_font_color;

typedef struct ImageViewerGlobal {
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
} ImageViewerGlobal;

ImageViewerGlobal iv_global;

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
    iv_global.total_size = max(scandir(path, &iv_global.files, filter, alphasort), 0);
    return iv_global.total_size == 0 ? 1 : 0;
}

#define get_root_file(buff, i)\
    strcpy(buff, iv_global.root);\
    strcat(buff,"/");\
    strcat(buff, iv_global.files[i]->d_name)

void reset_pagination()
{
    iv_global.set_size = IMAGE_THUMBS_PER_PAGE;
    iv_global.is_ready = 0;
    iv_global.total_size = 0;
    iv_global.relative_pos = 0;
    iv_global.absolute_pos = 0;
    iv_global.page = 0;
    iv_global.title[0] = '\0';
    iv_global.executable[0] = '\0';
}

int imageviewer_init(char* title, char* executable, char* path)
{
    log_debug("Initializing");
    
    reset_pagination();
    
    // try to read files before we do anything else
    if (!realpath(path, iv_global.root)) {
        log_error("Failed to get real path of directory %s", path);
        return 1;
    }
    
    if (get_imagelist(iv_global.root)) {
        log_error("Failed to read directory %s", iv_global.root);
        return 1;
    }
    
    if (title != NULL) strcpy(iv_global.title, title);
    if (executable != NULL) strcpy(iv_global.executable, executable);
    
    //Setup UI
    if (strlen(iv_global.title) > 0) 
    {
        // load font
        imageviewer_font       = get_theme_font(14);
        imageviewer_font_color = get_theme_font_color();
        imageviewer_title      = render_imageviewer_text(title);
        imageviewer_title_bg = create_surface(
            SCREEN_WIDTH, SELECT_TITLE_HEIGHT, 32, 
            SELECT_TITLE_COLOR, 
            SELECT_TITLE_ALPHA);
    }
    
    imageviewer_highlight = create_surface(
        IMAGE_THUMB_WIDTH,  IMAGE_THUMB_HEIGHT, 32,
        IMAGE_SELECT_COLOR, IMAGE_SELECT_ALPHA);

    imageviewer_background = create_surface(
        SCREEN_WIDTH, SCREEN_HEIGHT, 24, SELECT_BG_COLOR, 0);

    iv_global.entries = new_array(SDL_Surface*, iv_global.total_size);
    imageviewer_update_list();
    imageviewer_update_preview();
    
    iv_global.state_changed = 1;
    iv_global.is_ready = 1;

    return 0;
}

void imageviewer_deinit()
{
    int i = 0;
    if (!iv_global.is_ready) return;
    log_debug("De-initializing");
    
    free_surface(imageviewer_preview);
    free_surface(imageviewer_background);
    free_surface(imageviewer_highlight);
    free_surface(imageviewer_title_bg);
    free_surface(imageviewer_title);
    free_color(imageviewer_font_color);
    free_font(imageviewer_font);
    
    for (i=0;i<iv_global.total_size;i++) free_erase(iv_global.files[i]);
    for (i=0;i<iv_global.set_size;i++) free_surface(iv_global.entries[i]);
    
    free_erase(iv_global.files);
    free_erase(iv_global.entries);
    
    reset_pagination();
    iv_global.is_ready = 0;
}

void imageviewer_draw(SDL_Surface* screen)
{
    if (!iv_global.state_changed) return;
    
    int i;
    SDL_Rect dstrect, txtrect;

    SDL_BlitSurface(imageviewer_background, NULL, screen, NULL);
    
    //Draw thumbnails
    init_rect_pos(&dstrect, IMAGE_THUMB_PAD_X,IMAGE_THUMB_TOP+IMAGE_THUMB_PAD_Y);
    for (i = 0; i < iv_global.set_size;i++) 
    {
        if (iv_global.entries[i]) {
            SDL_BlitSurface(iv_global.entries[i],   0, screen, &dstrect);
        }
        dstrect.x += IMAGE_THUMB_WIDTH;
    }
    
    //Draw highlight for active item
    init_rect_pos(&dstrect, (iv_global.relative_pos)*IMAGE_THUMB_WIDTH, IMAGE_THUMB_TOP);
    SDL_BlitSurface(imageviewer_highlight, 0, screen, &dstrect);
    
    //Draw imageviewer_preview
    init_rect_pos(&dstrect, screen->w/2-imageviewer_preview->w/2, (screen->h-IMAGE_THUMB_HEIGHT)/2-imageviewer_preview->h/2);
    SDL_BlitSurface(imageviewer_preview, 0, screen, &dstrect);

    //Draw top message
    init_rect_pos(&txtrect, 0,0);
    SDL_BlitSurface(imageviewer_title_bg, 0, screen, &txtrect);
    txtrect.x += DOSD_PADDING;
    SDL_BlitSurface(imageviewer_title, 0, screen, &txtrect);
    
    iv_global.state_changed = 0;
}


void imageviewer_update_list()
{
    int i,j;
    int size = iv_global.set_size;
    
    for (i=0;i<size;i++)
    {
        if (iv_global.entries[i] && iv_global.is_ready)
        {
            SDL_FreeSurface(iv_global.entries[i]);
        }
        iv_global.entries[i] = NULL;
    }
    
    int start = iv_global.page * size;
    char tmp[PATH_MAX];
    
    for (i=0,j=start;j<iv_global.total_size&&i<size;i++,j++) 
    {
        get_root_file(tmp, j);
        iv_global.entries[i] = load_resized_image(tmp, IMAGE_THUMB_RATIO_INNER, IMAGE_THUMB_RATIO_INNER);
    }
}

void imageviewer_update_preview() 
{
    if (imageviewer_preview) SDL_FreeSurface(imageviewer_preview);
    char tmp[PATH_MAX];
    get_root_file(tmp, iv_global.absolute_pos);
    imageviewer_preview = load_resized_image(tmp, IMAGE_PREVIEW_RATIO, IMAGE_PREVIEW_RATIO);
}

void imageviewer_move_page(enum Direction dir)
{
    int delta = (dir == PREV) ? -1 : 1;
    int prev_p = iv_global.page;
    iv_global.page = bound(iv_global.page + delta, 0, iv_global.total_size/iv_global.set_size);
    
    SE_out( MENU_MOVE );
    
    if (prev_p != iv_global.page)
    {
        iv_global.absolute_pos  = iv_global.page*iv_global.set_size;
        iv_global.relative_pos  = 0;
        iv_global.state_changed = 1;
        imageviewer_update_list();
        imageviewer_update_preview();
    }
}

void imageviewer_move(enum Direction dir) 
{
    int delta = (dir == PREV) ? -1 : 1;
    int prev = iv_global.absolute_pos;
    int prev_p = iv_global.page;
    
    iv_global.absolute_pos = bound(iv_global.absolute_pos + delta, 0, iv_global.total_size-1);
    iv_global.state_changed = prev != iv_global.absolute_pos;
    iv_global.page = iv_global.absolute_pos/iv_global.set_size;
 
    SE_out( MENUITEM_MOVE );
    
    if (prev_p == iv_global.page) {
        iv_global.relative_pos = iv_global.absolute_pos % iv_global.set_size;
    } else {
        iv_global.relative_pos = (dir == PREV ? iv_global.set_size-1 : 0);
        imageviewer_update_list();
    }
    
    if (iv_global.state_changed) {
        imageviewer_update_preview();
    }
}

enum MenuState imageviewer_select()
{
    SE_out( DECIDE );
    
    char tmp[PATH_MAX];
    get_root_file(tmp, iv_global.absolute_pos);

    if (strlen(iv_global.executable) > 0) 
    {
        run_command(iv_global.executable, tmp, iv_global.root);
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