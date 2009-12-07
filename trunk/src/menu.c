/*
 *  Copyright (C) 2009 Rookie1 <mr.rookie1@gmail.com>
 *                     sca <scahoo <at> gmail <dot> com>
                       timothy.soehnlin <timothy.soehnlin@gmail.com>
 *
 *  Author: <mr.rookie1@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <unistd.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "menu.h"

#include "env.h"
#include "common.h"
#include "conf.h"
#include "resource.h"
#include "colorpicker.h"
#include "filelist.h"
#include "imageviewer.h"
#include "persistent.h"
#include "sound.h"
#include "dingoo.h"
#include "brightness.h"
#include "volume.h"

extern cfg_t *cfg;
cfg_t *m;
cfg_t *mi;
cfg_t *smi;

int menu_needs_redraw = 1; //First draw

SDL_Surface* background;

SDL_Surface** menu_icons;
SDL_Surface** menu_text;
int number_of_menu;

SDL_Surface*** menuitem_icons;
SDL_Surface*** menuitem_text;
int* number_of_menuitem;

SDL_Surface** submenuitem_icons;
SDL_Surface** submenuitem_text;
int number_of_submenuitem;

SDL_Surface* tmp_surface;

int current_menu_index;
int current_menuitem_index;
int current_submenuitem_index;

TTF_Font* menu_font;
TTF_Font* menuitem_font;

void menu_reload_background() 
{
    free_surface(background);
    background = get_theme_background();
}

void menu_state_changed()
{
    menu_needs_redraw = 1;
}

int menu_init()
{
    log_debug("Initializing");
    
    int i, j;
    SDL_Color* color = get_theme_font_color();
    menu_reload_background();
    
    /*fill_fb(background->pixels);*/

    // load font
    TTF_Init();
    menu_font     = get_theme_font(18);
    menuitem_font = get_theme_font(14);

    // load menu
    number_of_menu     = cfg_size(cfg, "Menu");
    number_of_menuitem = new_array(int, number_of_menu);
    menu_icons         = new_array(SDL_Surface*,  number_of_menu);
    menu_text          = new_array(SDL_Surface*,  number_of_menu);
    menuitem_icons     = new_array(SDL_Surface**, number_of_menu);
    menuitem_text      = new_array(SDL_Surface**, number_of_menu);

    for (i=0;i<number_of_menu;i++) {
        m = cfg_getnsec(cfg, "Menu", i);
        menu_icons[i] = load_theme_image(cfg_getstr(m, "Icon"));
        menu_text[i]  = draw_text(cfg_getstr(m, "Name"), menu_font, color);

        number_of_menuitem[i] = cfg_size(m, "MenuItem");
        menuitem_icons[i] = new_array(SDL_Surface*, number_of_menuitem[i]);
        menuitem_text[i]  = new_array(SDL_Surface*, number_of_menuitem[i]);
        
        for (j=0;j<number_of_menuitem[i];j++) {
            mi = cfg_getnsec(m, "MenuItem", j);
            menuitem_icons[i][j] = load_theme_image(cfg_getstr(mi, "Icon")); 
            menuitem_text[i][j]  = draw_text(cfg_getstr(mi, "Name"), menuitem_font, color);
        }
    }
    
    free(color);
    
    // Restore menu position
    current_menu_index     = g_persistent->current_menu;
    current_menuitem_index = g_persistent->current_menuitem;

    if (!in_bounds(current_menu_index, 0, number_of_menu)) {
        current_menu_index     = 0;
        current_menuitem_index = 0;
    } else if (!in_bounds(current_menuitem_index, 0, number_of_menuitem[current_menu_index])) {
        current_menuitem_index = 0;
    }

    // Init sound
    SE_Init();

    //load volume and brightness
    bright_init();
    vol_init();

    return 0;
}

void menu_deinit()
{
    log_debug("De-initializing");
    
    int i, j;

    bright_deinit();
    vol_deinit();

    // De-init sound
    SE_deInit();
    
    // Save current menu state
    g_persistent->current_menu     = current_menu_index;
    g_persistent->current_menuitem = current_menuitem_index;

    free_surface(background);
    
    for (i=0;i<number_of_menu;i++) {
        SDL_FreeSurface(menu_icons[i]);
        SDL_FreeSurface(menu_text[i]);
        for (j=0;j<number_of_menuitem[i];j++) {
            SDL_FreeSurface(menuitem_icons[i][j]);
            SDL_FreeSurface(menuitem_text[i][j]);
        }
        free(menuitem_icons[i]);
        free(menuitem_text[i]);
    }

    free_erase(menuitem_icons);
    free_erase(menuitem_text);
    free_erase(number_of_menuitem);
    free_erase(menu_icons);
    free_erase(menu_text);

    free_font(menu_font);
    free_font(menuitem_font);
    
    TTF_Quit();

    if (number_of_submenuitem > 0) submenu_close();
}

//Draw single menu item
void menu_draw_single_item(SDL_Surface* screen,
                           SDL_Surface* icon, SDL_Rect* icon_rect, 
                           SDL_Surface* text, SDL_Rect* text_rect,
                           int alpha, int position) 
{
    if (position >= 0) {
        if (position == 0) {
            text_rect->x = icon_rect->x + icon->w;
            text_rect->y = icon_rect->y + icon->h/2 - text->h/2;
        } else {
            text_rect->x = icon_rect->x + icon->w/2 - text->w/2;
            text_rect->y = icon_rect->y + icon->h; 
        }

        blitSurfaceAlpha(text, NULL, screen, text_rect, alpha);
    }

    blitSurfaceAlpha(icon, NULL, screen, icon_rect, alpha);
}


//Draw vertical menu items
void menu_draw_vitems(SDL_Surface* screen, SDL_Rect* offset, 
                      SDL_Surface** icons, SDL_Rect* icon_rect, 
                      SDL_Surface** text,  SDL_Rect* text_rect, 
                      int current_item,    int number_of_items, 
                      int child_showing,   int is_parent, int lower_offset) {
  
    int draw_item = 0, alpha = 0, ypos = 0;
    
    icon_rect->x = offset->x;

    if (number_of_items > 0 ) {

        // draw upper items 
        draw_item = current_item - 1;
        
        if (current_item > 0 && !child_showing) {

            while (1) {
                
                alpha = item_alpha(current_item, draw_item); 
                icon_rect->y = offset->y - (current_item - draw_item)*icons[draw_item]->h;
                ypos = icon_rect->y; //icon_rect's position is overwritten by blitting 
                menu_draw_single_item(screen, icons[draw_item], icon_rect,   
                                      text[draw_item], text_rect,
                                      alpha, child_showing ? -1 : 0);
                --draw_item; 
                if (ypos < 0 || draw_item < 0) break;
            }
        }

        // draw lower items 
        draw_item = current_item;
        icon_rect->y = offset->y + lower_offset;

        while (1) { 
            alpha = item_alpha(current_item, draw_item); 
            menu_draw_single_item(screen, 
                                  icons[draw_item], icon_rect, 
                                  text[draw_item], text_rect, 
                                  alpha, child_showing ? -1 : 0); 
            icon_rect->y += icons[draw_item]->h;
            draw_item++;
            alpha--;

            if (child_showing || icon_rect->y > screen->h || draw_item >= number_of_items) {
                break; //Only draw first item if child showing
            }
        }
    }
}

//Draw horizontal menu items
void menu_draw_hitems(SDL_Surface* screen, SDL_Rect* child_offset, 
                      SDL_Surface** icons, SDL_Rect* icon_rect, 
                      SDL_Surface** text,  SDL_Rect* text_rect, 
                      int current_item, int number_of_items, int subchild_showing)
{
    int draw_item = current_item - 1;

    //While there are empty slots to the left of the menu
    //   create a gap that is a single icon wide
    while (draw_item<0) {
        icon_rect->x += icons[0]->w; 
        draw_item++;
    }
    
    while (1) {
        int alpha = item_alpha(current_item, draw_item); 
        menu_draw_single_item(screen, 
                              icons[draw_item], icon_rect, 
                              text[draw_item],  text_rect, 
                              alpha, draw_item != current_item ? -1 : 1);
   
        //Store calculated position for current menu items
        if (draw_item == current_item) { 
            init_rect_pos(child_offset, icon_rect->x, icon_rect->y);
        }

        icon_rect->x += icons[draw_item]->w;

        draw_item++;
        alpha --;
        
        if ((icon_rect->x >= screen->w) || 
           (draw_item >= number_of_items) ||
           (subchild_showing && draw_item > current_menu_index)) 
        {
            break;
        }
    }
}

void menu_draw(SDL_Surface* screen)
{
    if (!menu_needs_redraw) return;
    menu_needs_redraw = 0;
    
    SDL_Rect icon_rect, text_rect, rect, offset;

    init_rect_pos(&icon_rect, 0,0);
    init_rect_pos(&text_rect, 0,0);
    init_rect_pos(&rect, 0, 0);
    
    // clear screen
    SDL_BlitSurface(background, 0, screen, &rect);
    
    //Set menu offset
    icon_rect.y = (screen->h - menu_icons[0]->h - 20) / 3; // assuming the font height for menu name is 20

    //Draw main menu items
    menu_draw_hitems(screen, &offset, menu_icons, &icon_rect, menu_text, &text_rect, 
                     current_menu_index,  number_of_menu, number_of_submenuitem > 0); 

    //Draw menu items, starting at (x,y)
    menu_draw_vitems(screen, &offset, 
                     menuitem_icons[current_menu_index], &icon_rect, 
                     menuitem_text[current_menu_index], &text_rect, 
                     current_menuitem_index, number_of_menuitem[current_menu_index],
                     number_of_submenuitem > 0, 1, menu_icons[current_menu_index]->h + 20); 

    //Draw submenu (will only show if it is active)
    // at this point, text_rect.x and text_rect.y should be at the position where we want to draw the current

    if (number_of_submenuitem > 0) {
        icon_rect.x += menu_text[current_menu_index]->w;
        text_rect.x += menu_text[current_menu_index]->w;
    }
    
    //-1 is a hack to push the text above the OSD info
    init_rect_pos(&offset, text_rect.x, text_rect.y-1); 
        
    menu_draw_vitems(screen, &offset, 
                     submenuitem_icons, &icon_rect, 
                     submenuitem_text, &text_rect, 
                     current_submenuitem_index, 
                     number_of_submenuitem, 0, 0, 0); 
}

void menu_move(enum Direction dir) 
{
    int delta =  dir==PREV ? -1 : 1;
    
    if (number_of_submenuitem > 0) {
        if (dir == PREV) {
            submenu_close();
        } else {
            SE_out( OUT );
        }
        return;
    }
    
    menu_state_changed();
    
    current_menu_index = wrap(current_menu_index+ delta, 0, number_of_menu-1);
    SE_out( MENU_MOVE );
    current_menuitem_index = 0;
}

void menuitem_move(enum Direction dir ) 
{
    menu_state_changed();
    
    int delta = dir == PREV ? -1 : 1;
    int nosi = number_of_submenuitem;
    int nomi = number_of_menuitem[current_menu_index];
    int next_mi = current_menuitem_index + delta;
    int next_si = current_submenuitem_index + delta;
    
    if (!nosi) {
        current_menuitem_index = wrap(next_mi, 0, nomi-1);
    }
    else {
        current_submenuitem_index = wrap(next_si, 0, nosi-1);
    }
    SE_out( MENUITEM_MOVE );    
}

void submenu_open()
{
    menu_state_changed();
    
    int i;
    SDL_Color* color = get_theme_font_color();
    SE_out( DECIDE );

    number_of_submenuitem = cfg_size(mi, "SubMenuItem");
    submenuitem_icons = new_array(SDL_Surface*, number_of_submenuitem);
    submenuitem_text  = new_array(SDL_Surface*, number_of_submenuitem);
    
    for (i=0;i<number_of_submenuitem;i++) {
        smi = cfg_getnsec(mi, "SubMenuItem", i);
        submenuitem_icons[i] = load_theme_image(cfg_getstr(smi, "Icon"));
        submenuitem_text[i]  = draw_text(cfg_getstr(smi, "Name"), menuitem_font, color);
    }    
    
    free(color);
    current_submenuitem_index = 0;
}

void submenu_close()
{
    menu_state_changed();
    
    int i;

    SE_out( CANCEL );
    for (i=0;i<number_of_submenuitem;i++) {
        SDL_FreeSurface(submenuitem_icons[i]);
        SDL_FreeSurface(submenuitem_text[i]);
    }

    free(submenuitem_icons);
    free(submenuitem_text);
    
    number_of_submenuitem = 0;
}

enum MenuState menuitem_runinternal()
{
    char* executable = cfg_getstr(mi, "Executable");
    char* name       = cfg_getstr(mi, "Name");
    char* tmp;
    char **files;
    ImageEntry** images;
    enum MenuState state = MAINMENU;
    int rc=0, i=0, count=0;
    
    switch (get_command(executable))
    {
        case THEMESELECT:
            //A lot of string manipulation happens to convert the file names
            // Of the theme directories to the proper paths for the thumbnails
            // and the theme titles.  This is why we handle so much malloc
            // and free in this case.  The goal is to ensure no memory
            // is leaked.
            files = get_theme_previews();
            while (files[count] != NULL) count++;
            images = new_array(ImageEntry*, count+1);
            
            for (i=0;i<count;i++) {
                images[i] = new_item(ImageEntry); {
                    images[i]->file = strdup(files[i]);
                    tmp = strndup(files[i], strrpos(files[i], '/'));
                    images[i]->title = strdup(strrchr(tmp,'/')+1);
                    free(tmp);
                }
                free_erase(files[i]);
            }
            free_erase(files);
            images[count] = NULL;
            
            rc = imageviewer_init(name, executable, DMENU_THEMES, images);
            if (!rc) state = IMAGEVIEWER;
            
            for (i=0;i<count;i++) {
                free(images[i]->file);
                free(images[i]->title);
                free(images[i]);
            }
            free(images);
            
            break;
        case BACKGROUNDSELECT:
            rc = imageviewer_init(name, executable, DMENU_BACKGROUNDS, NULL);
            if (!rc) state = IMAGEVIEWER;
            break;
        case COLORSELECT:
            tmp = theme_file(get_user_attr("Background"));
            rc = colorpicker_init(name, executable, NULL, get_theme_font_color_string(), tmp);
            if (!rc) state = COLORPICKER;
            free(tmp);
            break;
    }
    
    return state;
}

enum MenuState menuitem_run()
{
    char* executable = cfg_getstr(mi, "Executable");
    
    if (executable) {
        if (internal_command(executable)) {
            return menuitem_runinternal();
        } 
        else {
            run_command(executable, NULL, cfg_getstr(mi, "WorkDir"));
        }
    } else {
        submenu_open();
    }
    
    return MAINMENU;
}

enum MenuState menu_keypress(SDLKey keysym)
{
    enum MenuState state = MAINMENU;
    
    switch (keysym) {
        case DINGOO_BUTTON_L:
        case DINGOO_BUTTON_R:
            vol_set(keysym==DINGOO_BUTTON_L?-5:5);
            menu_state_changed();
            SE_out( TEST );
            break;
        case DINGOO_BUTTON_X:
        case DINGOO_BUTTON_Y:
            bright_set(keysym==DINGOO_BUTTON_X?-1:1);
            menu_state_changed();
            SE_out( TEST );
            break;
        case DINGOO_BUTTON_LEFT:
        case DINGOO_BUTTON_RIGHT:
            menu_move(keysym==DINGOO_BUTTON_LEFT?PREV:NEXT);
            break;
        case DINGOO_BUTTON_UP:
        case DINGOO_BUTTON_DOWN:
            menuitem_move(keysym==DINGOO_BUTTON_UP?PREV:NEXT);
            break;
        case DINGOO_BUTTON_B:
            if (number_of_submenuitem > 0) submenu_close();
            break;
        case DINGOO_BUTTON_A:
            m  = cfg_getnsec(cfg, "Menu", current_menu_index);
            mi = cfg_getnsec(m, "MenuItem", current_menuitem_index);
            
            if (number_of_submenuitem > 0) 
            {
                mi = cfg_getnsec(mi, "SubMenuItem", current_submenuitem_index);
            }
            
            if (cfg_getbool(mi, "Selector")) 
            {
                char* listdir = cfg_getstr(mi, "SelectorDir");
                if (!listdir ) listdir = cfg_getstr(mi, "WorkDir");
                if (!filelist_init(cfg_getstr(mi, "Name"),
                        cfg_getstr(mi, "Executable"),
                        cfg_getstr(mi, "WorkDir"),
                        listdir, 1))
                {
                    state = FILELIST;
                } // else we are not able to initialise the filelist display
            } 
            else {
                state = menuitem_run();
            }
            break;
        default: break;
    }
    return state;
}