/*
 *  Copyright (C) 2009 Rookie1 <mr.rookie1@gmail.com>
 *                     sca <scahoo <at> gmail <dot> com>
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

SDL_Surface* background;
SDL_Surface* cursor;

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
TTF_Font* status_font;

void submenu_open();
void submenu_close();

void menu_reload_background() 
{
    free_surface(background);
    background = load_theme_background(get_user_attr("Background"));
}

int menu_init()
{
    log_debug("Initializing");
    
    int i, j;
    SDL_Color* color = get_theme_font_color();
    menu_reload_background();
    
    /*fill_fb(background->pixels);*/
    cursor = load_theme_image(cfg_getstr(cfg, "Cursor"));

    // load font
    TTF_Init();
    menu_font     = get_theme_font(18);
    menuitem_font = get_theme_font(14);
    status_font   = get_theme_font(13);

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
    free_surface(cursor);
    
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
    free_font(status_font);
    
    TTF_Quit();

    if (number_of_submenuitem > 0) submenu_close();
}

//Draw single menu item
void menu_draw_single_item(SDL_Surface* screen, SDL_Rect* dstrect, SDL_Rect* txtrect,
                           int alpha,  SDL_Surface* icon, SDL_Surface* text, 
                           int hide_text, int text_below) 
{
    if (!text_below) {
        txtrect->x = dstrect->x + icon->w;
    } else {
        txtrect->x = dstrect->x + icon->w/2 - text->w/2;
    }
    
    if (!hide_text) {
        if (!text_below) {
            txtrect->y = dstrect->y + icon->h/2 - text->h/2;
        } else {
            txtrect->y = dstrect->y + icon->h; 
        }
        SDL_SetAlpha(text, SDL_SRCALPHA, alpha);
        SDL_BlitSurface(text, 0, screen, txtrect);
    }
    SDL_SetAlpha(icon, SDL_SRCALPHA, alpha);
    SDL_BlitSurface(icon, 0, screen, dstrect);
}


//Draw vertical menu items
void menu_draw_vitems(SDL_Surface* screen, SDL_Rect* offset, SDL_Rect* dstrect, SDL_Rect* txtrect, 
                     int current_item, int number_of_items, SDL_Surface** icons, SDL_Surface** text,
                     int is_parent, int child_showing, int lower_offset) {
  
    int draw_item = 0;
    dstrect->x = offset->x;

    if (number_of_items > 0 ) {

        // draw upper items 
        draw_item = current_item - 1;
        
        if (current_item > 0 && !child_showing) {

            while (1) {
                dstrect->y = offset->y - (current_item - draw_item)*icons[draw_item]->h;
     
                menu_draw_single_item(screen, dstrect, txtrect, 128,  
                                      icons[draw_item], text[draw_item], 
                                      child_showing, 0);
                --draw_item; 
                if (dstrect->y < 0 || draw_item < 0) break;
            }
        }

        // draw lower items 
        draw_item = current_item;
        dstrect->y = offset->y + lower_offset;

        while (1) { 

            if (is_parent && draw_item == current_item) {
                SDL_BlitSurface(cursor, 0, screen, dstrect);
            }

            int alpha = (draw_item==current_item)?SDL_ALPHA_OPAQUE:SDL_ALPHA_TRANSPARENT; 
            menu_draw_single_item(screen, dstrect, txtrect, alpha, 
                                  icons[draw_item], text[draw_item],
                                  child_showing, 0); 
            dstrect->y += icons[draw_item]->h;
            draw_item++;

            if (child_showing || dstrect->y > screen->h || draw_item >= number_of_items) {
                break; //Only draw first item if child showing
            }
        }
    }
}

//Draw horizontal menu items
void menu_draw_hitems(SDL_Surface* screen, SDL_Rect* child_offset, 
                      SDL_Rect* dstrect, SDL_Rect* txtrect, 
                      int current_item, int number_of_items,
                      SDL_Surface** icons, SDL_Surface** text,
                      int showing_submenu)
{
    int draw_item = current_item - 1;

    //While there are empty slots to the left of the menu
    //   create a gap that is a single icon wide
    while (draw_item<0) {
        dstrect->x += icons[0]->w; 
        draw_item++;
    }

    while (1) {
        menu_draw_single_item(screen, dstrect, txtrect, SDL_ALPHA_OPAQUE, 
                              icons[draw_item], text[draw_item], 
                              draw_item != current_item, 1);
   
        //Store calculated position for current menu items
        if (draw_item == current_item) { 
            init_rect_pos(child_offset, dstrect->x, dstrect->y);
        }

        dstrect->x += icons[draw_item]->w;

        draw_item++;
        if ((dstrect->x >= screen->w) || 
           (draw_item >= number_of_items) ||
           (showing_submenu && draw_item > current_menu_index)) 
        {
            break;
        }
    }
}

void menu_draw(SDL_Surface* screen)
{
    SDL_Rect dstrect, txtrect, offset;

    init_rect_pos(&dstrect, 0,0);
    init_rect_pos(&txtrect, 0,0);
    
    // clear screen
    //SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_BlitSurface(background, 0, screen, &dstrect);
    
    //Set menu offset
    dstrect.y = (screen->h - menu_icons[0]->h - 20) / 3; // assuming the font height for menu name is 20

    //Draw main menu items
    menu_draw_hitems(screen, &offset, &dstrect, &txtrect, current_menu_index, 
                     number_of_menu, menu_icons, menu_text, number_of_submenuitem > 0); 

    //Draw menu items, starting at (x,y)
    menu_draw_vitems(screen, &offset, &dstrect, &txtrect, 
                     current_menuitem_index, number_of_menuitem[current_menu_index], 
                     menuitem_icons[current_menu_index], menuitem_text[current_menu_index], 
                     1, number_of_submenuitem > 0, menu_icons[current_menu_index]->h + 20); 

    //Draw submenu (will only show if it is active)
    // at this point, txtrect.x and txtrect.y should be at the position where we want to draw the current

    //-1 is a hack to push the text above the OSD info
    init_rect_pos(&offset, txtrect.x, txtrect.y-1); 
        
    menu_draw_vitems(screen, &offset, &dstrect, &txtrect, 
                     current_submenuitem_index, number_of_submenuitem, 
                     submenuitem_icons, submenuitem_text, 0, 0, 0); 
}

enum MenuState menu_keypress(SDLKey keysym)
{
    enum MenuState state = MAINMENU;

    switch (keysym) {
        case DINGOO_BUTTON_L:
            vol_set(-5);
            SE_out( TEST );
            break;
        case DINGOO_BUTTON_R:
            vol_set(+5);
            SE_out( TEST );
            break;
        case DINGOO_BUTTON_X:
            bright_set(+1);
            SE_out( TEST );
            break;
        case DINGOO_BUTTON_Y:
            bright_set(-1);
            SE_out( TEST );
            break;
        case DINGOO_BUTTON_LEFT:
            menu_move( PREV );
            break;
        case DINGOO_BUTTON_RIGHT:
            menu_move( NEXT );
            break;
        case DINGOO_BUTTON_UP:
            menuitem_move( PREV );
            break;
        case DINGOO_BUTTON_DOWN:
            menuitem_move( NEXT ); 
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
        case DINGOO_BUTTON_B:
            if (number_of_submenuitem > 0) submenu_close();
            break;

        default: break;
    }
    return state;
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
    current_menu_index = wrap(current_menu_index+ delta, 0, number_of_menu-1);
    SE_out( MENU_MOVE );
    current_menuitem_index = 0;
}

void menuitem_move(enum Direction dir ) 
{
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

enum MenuState menuitem_runinternal()
{
    char* executable = cfg_getstr(mi, "Executable");
    char* name       = cfg_getstr(mi, "Name");

    switch (get_command(executable))
    {
        case THEMESELECT:
            if (!filelist_init(name, executable, DMENU_THEMES, DMENU_THEMES, 0)) 
            {
                return FILELIST;
            } // else we are not able to initialise the filelist display
            break;
            
        case BACKGROUNDSELECT:
            if (!imageviewer_init(name, executable, USER_BACKGROUNDS))
            {
                return IMAGEVIEWER;
            }
            break;
        default: break;
    }
    
    return MAINMENU;
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

void submenu_open()
{
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
