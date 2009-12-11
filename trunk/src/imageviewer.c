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

SDL_Surface* imageviewer_preview;
SDL_Surface* imageviewer_preview_title;
SDL_Surface* imageviewer_background;
SDL_Surface* imageviewer_highlight;
SDL_Surface* imageviewer_title_bg;
SDL_Surface* imageviewer_title;
TTF_Font*    imageviewer_font;
SDL_Color*   imageviewer_font_color;

typedef struct ImageViewerGlobal 
{
    int total_size;
    int max_pages;
    int set_size;
    int page;
    int is_ready;
    
    int absolute_pos;
    int relative_pos;
    int new;
    int title_ticks;
    
    int  state_changed;
    char* title;
    char* executable;
    char* root;
    SDL_Surface** entries;
    ImageEntry** images;    
} ImageViewerGlobal;

ImageViewerGlobal iv_global;

#define get_root_file(buff, i)\
    strcpy(buff, "");\
    if (iv_global.images[i]->file[0] != '/') {\
        strcat(buff, iv_global.root);\
        strcat(buff,"/");\
    };\
    strcat(buff, iv_global.images[i]->file);

SDL_Surface* imageviewer_render_text(char* text) 
{
    return draw_text(text, imageviewer_font, imageviewer_font_color);
}

char* IMAGE_TYPES[3] = {".jpg", ".png", ".bmp"};
int imageviewer_file_filter(const struct dirent *dptr)
{
    char *name = (char*)dptr->d_name;
    return strstr(name, IMAGE_TYPES[0]) != NULL 
        || strstr(name, IMAGE_TYPES[1]) != NULL 
        || strstr(name, IMAGE_TYPES[2]) != NULL;
}

int imageviewer_getlist(char* path, ImageEntry** images)
{
    int i = 0;
    if (images == NULL) {
        struct dirent** dir_files;
        iv_global.total_size = max(scandir(path, &dir_files, imageviewer_file_filter, alphasort), 0);
        if (iv_global.total_size > 0) {
            iv_global.images = new_array(ImageEntry*, iv_global.total_size);
            for (i=0;i<iv_global.total_size;i++) {
                iv_global.images[i] = new_item(ImageEntry);
                iv_global.images[i]->file = strdup(dir_files[i]->d_name);
                iv_global.images[i]->title = NULL;
                iv_global.images[i]->value = NULL;
                free_erase(dir_files[i]);
            }
        }
        free_erase(dir_files);
    } else {
        while (images[i] != NULL) i++;
        iv_global.total_size = i;
        if (i > 0) {
            iv_global.images = new_array(ImageEntry*, iv_global.total_size);
            for (i=0;i<iv_global.total_size;i++) {
                iv_global.images[i] = new_item(ImageEntry);
                iv_global.images[i]->file = strdup(images[i]->file);
                iv_global.images[i]->title = strdup(images[i]->title);
                iv_global.images[i]->value = strdup(images[i]->value);
                if (strlen(iv_global.images[i]->title) == 0) {
                    iv_global.images[i]->title = NULL;
                }
                if (strlen(iv_global.images[i]->value)==0) {
                    iv_global.images[i]->value = NULL;
                }
            }
        }
    }
    
    //Calculate page max, remember to remove 1 page when number 
    //  of entries is perfect mod of set size
    iv_global.max_pages = iv_global.total_size/iv_global.set_size -
        (iv_global.total_size % iv_global.set_size == 0 ? 1 : 0);
        
    return iv_global.total_size == 0 ? 1 : 0;
}

void imageviewer_reset_pagination()
{
    iv_global.set_size = IMAGE_THUMBS_PER_PAGE;
    iv_global.is_ready = 0;
    iv_global.total_size = 0;
    iv_global.relative_pos = 0;
    iv_global.absolute_pos = 0;
    iv_global.page = 0;
    iv_global.title_ticks = 0;
    free_erase(iv_global.root);
    free_erase(iv_global.title);
    free_erase(iv_global.executable);
}

int  imageviewer_init(char* title, char* executable, char* path, ImageEntry** images)
{
    log_debug("Initializing");
    
    imageviewer_reset_pagination();
    
    // try to read files before we do anything else
    char root[PATH_MAX];
    if (!realpath(path, root)) 
    {
        log_error("Failed to get real path of directory %s", path);
        return 1;
    }
    iv_global.root = strdup(root);
    
    if (imageviewer_getlist(iv_global.root, images)) 
    {
        log_error("Failed to read/find files in %s", iv_global.root);
        return 1;
    }
    
    if (title != NULL) iv_global.title = strdup(title);
    if (executable != NULL) iv_global.executable = strdup(executable);
    
    //Setup UI
    if (strlen(iv_global.title) > 0) 
    {
        // load font
        imageviewer_font       = get_theme_font(14);
        imageviewer_font_color = get_theme_font_color();
        imageviewer_title      = imageviewer_render_text(title);
        imageviewer_title_bg   = create_surface(
            SCREEN_WIDTH, SELECT_TITLE_HEIGHT, 32, 
            SELECT_TITLE_COLOR, 
            SELECT_TITLE_ALPHA);
    }
    
    imageviewer_highlight = create_surface(
        IMAGE_THUMB_WIDTH,  IMAGE_THUMB_HEIGHT, 32,
        IMAGE_SELECT_COLOR, IMAGE_SELECT_ALPHA);

    imageviewer_background = create_surface(
        SCREEN_WIDTH, SCREEN_HEIGHT, 24, SELECT_BG_COLOR, 0);

    int i;
    iv_global.entries = new_array(SDL_Surface*, iv_global.set_size);
    for (i=0;i<iv_global.set_size;i++) iv_global.entries[i] = NULL;
    
    imageviewer_update_list();
    imageviewer_update_preview();
    
    iv_global.state_changed = 1;
    iv_global.is_ready = 1;

    return 0;
}

int imageviewer_init_theme(char* title, char* executable, char* path)
{
    char tmp_name[200];
    char **files;
    ImageEntry** images;
    int rc=0, i=0, count=0;
    struct stat file_stat;
    
    //A lot of string manipulation happens to convert the file names
    // Of the theme directories to the proper paths for the thumbnails
    // and the theme titles.  This is why we handle so much malloc
    // and free in this case.  The goal is to ensure no memory
    // is leaked.
    files = get_theme_previews();
    while (files[count] != NULL) count++;
    images = new_array(ImageEntry*, count+1);
    
    for (i=0;i<count;i++) 
    {
        images[i] = new_item(ImageEntry); 
        {
            images[i]->title = strdup(strrchr(files[i],'/')+1);
            sprintf(tmp_name, "%s/theme.png", files[i]);
            images[i]->value = strdup(files[i]);
            if (stat(tmp_name, &file_stat)==0) { //If file is there
                images[i]->file = strdup(tmp_name);
            } else {
                images[i]->file = strdup(DMENU_THEME_MISSING);
            }
        }
        free_erase(files[i]);
    }
    free_erase(files);
    images[count] = NULL;
    
    rc = imageviewer_init(title, executable, path, images);
    for (i=0;i<count;i++) 
    {
        free_erase(images[i]->file);
        free_erase(images[i]->title);
        free_erase(images[i]->value);
        free_erase(images[i]);
    }
    //TODO: Figure this out. For some reason, this is 
    // causing a segfault on my machine.  
    //free_erase(images);
    
    return rc;
}

void imageviewer_deinit()
{
    int i = 0;
    if (!iv_global.is_ready) return;
    log_debug("De-initializing");
    
    free_surface(imageviewer_preview);
    free_surface(imageviewer_preview_title);
    free_surface(imageviewer_background);
    free_surface(imageviewer_highlight);
    free_surface(imageviewer_title_bg);
    free_surface(imageviewer_title);
    free_color(imageviewer_font_color);
    free_font(imageviewer_font);
    
    for (i=0;i<iv_global.total_size;i++) {
        free_erase(iv_global.images[i]->file);
        free_erase(iv_global.images[i]->title);
        free_erase(iv_global.images[i]->value);
        free_erase(iv_global.images[i]);
    }
    for (i=0;i<iv_global.set_size;i++) free_surface(iv_global.entries[i]);
    
    free_erase(iv_global.images);
    free_erase(iv_global.entries);
    
    imageviewer_reset_pagination();
    iv_global.is_ready = 0;
}

int imageviewer_draw(SDL_Surface* screen)
{
    iv_global.title_ticks++;    
    
    if (!iv_global.state_changed) return 0;
    
    int i;
    SDL_Rect image_rect;
    SDL_BlitSurface(imageviewer_background, NULL, screen, NULL);
    
    //Draw thumbnails
    init_rect_pos(&image_rect, IMAGE_THUMB_PAD_X,IMAGE_THUMB_TOP+IMAGE_THUMB_PAD_Y);
    for (i = 0; i < iv_global.set_size;i++) 
    {
        if (iv_global.entries[i]) {
            SDL_BlitSurface(iv_global.entries[i],   0, screen, &image_rect);
        }
        image_rect.x += IMAGE_THUMB_WIDTH;
    }

    //Draw highlight for active item
    init_rect_pos(&image_rect, (iv_global.relative_pos)*IMAGE_THUMB_WIDTH, IMAGE_THUMB_TOP);
    SDL_BlitSurface(imageviewer_highlight, 0, screen, &image_rect);

    //Draw imageviewer_preview
    init_rect_pos(&image_rect, screen->w/2-imageviewer_preview->w/2, (screen->h-IMAGE_THUMB_HEIGHT)/2-imageviewer_preview->h/2);
    SDL_BlitSurface(imageviewer_preview, 0, screen, &image_rect);
    
    iv_global.state_changed = 0;
    
    return 1;
}

void imageviewer_animate(SDL_Surface* screen) { }

void imageviewer_osd(SDL_Surface* screen) 
{
    SDL_Rect text_rect;
    
    //Draw top message
    init_rect_pos(&text_rect, 0,0);
    SDL_BlitSurface(imageviewer_title_bg, 0, screen, &text_rect);
    text_rect.x += DOSD_PADDING;
    SDL_BlitSurface(imageviewer_title, 0, screen, &text_rect);
 
    if (imageviewer_preview_title)
    {
        text_rect.x = SCREEN_WIDTH-imageviewer_preview_title->w - DOSD_PADDING*2;
        SDL_BlitSurface(imageviewer_preview_title, 0, screen, &text_rect);
    }
}

void imageviewer_update_list()
{
    int i,j;
    int size = iv_global.set_size;
    int start = iv_global.page * size;
    char tmp[PATH_MAX];
    
    for (i=0;i<size;i++) free_surface(iv_global.entries[i]);
        
    for (i=0,j=start;j<iv_global.total_size&&i<size;i++,j++) 
    {
        get_root_file(tmp, j);
        iv_global.entries[i] = load_resized_image(tmp, 
            SCREEN_WIDTH  * IMAGE_THUMB_RATIO_INNER, 
            SCREEN_HEIGHT * IMAGE_THUMB_RATIO_INNER);
    }
}

void imageviewer_update_preview() 
{
    char tmp[PATH_MAX];
    get_root_file(tmp, iv_global.absolute_pos);
    free_surface(imageviewer_preview);
    imageviewer_preview = load_resized_image(tmp, 
        SCREEN_WIDTH  * IMAGE_PREVIEW_RATIO, 
        SCREEN_HEIGHT * IMAGE_PREVIEW_RATIO);
        
    if (iv_global.images[iv_global.absolute_pos]->title) {
        free_surface(imageviewer_preview_title);
        imageviewer_preview_title = imageviewer_render_text(
            iv_global.images[iv_global.absolute_pos]->title);
    }
}

void imageviewer_move_page(Direction dir)
{
    int delta = (dir == PREV) ? -1 : 1;
    int prev_p = iv_global.page;
    iv_global.page = bound(iv_global.page + delta, 0, iv_global.max_pages);
    
    sound_out( MENU_MOVE );
    int iop = iv_global.set_size;
    if (iv_global.page == iv_global.max_pages) iop = (iv_global.total_size-1)%iv_global.set_size;

    if (prev_p == iv_global.page)
    {
        iv_global.relative_pos = dir == PREV ? 0 :  iop;
    }

    int oldpos = iv_global.absolute_pos;
    iv_global.absolute_pos  = iv_global.page*iv_global.set_size + iv_global.relative_pos;
    if (iv_global.absolute_pos != oldpos) 
    {
        iv_global.state_changed = 1;
        iv_global.relative_pos = bound(iv_global.relative_pos, 0, iop);
        imageviewer_update_list();
        imageviewer_update_preview();
    }
}

void imageviewer_move(Direction dir) 
{
    int delta = (dir == PREV) ? -1 : 1;
    int prev = iv_global.absolute_pos;
    int prev_p = iv_global.page;
    
    iv_global.absolute_pos = bound(iv_global.absolute_pos + delta, 0, iv_global.total_size-1);
    iv_global.page = iv_global.absolute_pos/iv_global.set_size;
 
    sound_out( MENUITEM_MOVE );
    
    if (prev_p == iv_global.page) {
        iv_global.relative_pos = iv_global.absolute_pos % iv_global.set_size;
    } else {
        iv_global.relative_pos = (dir == PREV ? iv_global.set_size-1 : 0);
        imageviewer_update_list();
    }
    
    if (prev !=  iv_global.absolute_pos) {
        iv_global.state_changed = 1;
        imageviewer_update_preview();
    }
}

MenuState imageviewer_select()
{
    sound_out( DECIDE );
    
    char tmp[PATH_MAX];
    if (iv_global.images[iv_global.absolute_pos]->value != NULL) {
        strcpy(tmp, iv_global.images[iv_global.absolute_pos]->value);
    } else {
        get_root_file(tmp, iv_global.absolute_pos);
    }
    
    if (strlen(iv_global.executable) > 0) 
    {
        run_command(iv_global.executable, tmp, iv_global.root);
    }
    
    imageviewer_deinit();
    
    return MAINMENU;
}

MenuState imageviewer_keypress(SDLKey key)
{
    Direction dir = getKeyDir(key);

    switch (key) {
        case DINGOO_BUTTON_LEFT:
        case DINGOO_BUTTON_RIGHT:
            imageviewer_move(dir);
            break;
        case DINGOO_BUTTON_L:
        case DINGOO_BUTTON_R:
            imageviewer_move_page(dir);
            break;
        case DINGOO_BUTTON_B:
            sound_out( CANCEL );
            imageviewer_deinit();
            return MAINMENU;
        case DINGOO_BUTTON_A:
            return imageviewer_select();
        default: break;
    }
    iv_global.title_ticks = 0;
    
    return IMAGEVIEWER;
}