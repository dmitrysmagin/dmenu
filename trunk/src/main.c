/*
 *  Copyright (C) 2009 Rookie1 <mr.rookie1@gmail.com>
 *                     Timothy Soehnlin <timothy.soehnlin@gmail.com>
 *
 *  Author: <mr.rookie1@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <SDL.h>
#include <SDL_image.h>
#include "env.h" 
#include "common.h"
#include "resource.h"

#include "main.h"
#include "menu.h"
#include "filelist.h"
#include "colorpicker.h"
#include "imageviewer.h"

#include "conf.h"
#include "persistent.h"
#include "dosd/dosd.h"

extern cfg_t *cfg;

static Uint32 next_time;
SDL_Surface *screen, *screen_cache;
MenuState state = MAINMENU;
int program_done;

Uint32 time_left(void)
{
    Uint32 now = SDL_GetTicks();
    return next_time <= now ? 0 : next_time - now;
}

int init_system() {

    //Move to dmenu root dir
    change_dir(DMENU_PATH);
    
    // load config
    if (conf_load()) 
    {
        return 1;
    }
    
    // Read saved persistent state
    if (!persistent_init())
    {
        log_error("Unable to initialize persistent memory");
    }
        
    // init menu config
    if (menu_init())
    {
        log_error("Unable to load menu");
        return 1;
    }
    
    persistent_restore_menu_position();
       
    return 0;
}

int init_display() {
    
    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        log_error( "Unable to init SDL: %s", SDL_GetError() );
        return 1;
    }
        
    // make sure SDL cleans up before exit
    atexit(SDL_Quit);
    
    // disable mouse pointer
    SDL_ShowCursor(SDL_DISABLE);
        
    // create a new window
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_DEPTH, SDL_SWSURFACE);
    if ( !screen )
    {
        log_error("Unable to set %dx%d video: %s", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
        return 1;
    }
    
    screen_cache = create_surface(SCREEN_WIDTH, SCREEN_HEIGHT, 24, 0xFF,0xFF,0xFF,0);
    
    //Show saved image as soon as possible to help hide startup times
    show_menu_snapshot(screen);
  
    return 0;
}

int init() {
    log_debug("Initializing");
    
    clear_last_command();
    
    #ifdef DINGOO_BUILD
    // Need to ignore SIGHUP if we are started by init.
    // In FB_CloseKeyboard() of SDL_fbevents.c , it does
    // ioctl(tty0_fd, TIOCNOTTY, 0) to detach the process from tty,
    // this will send a SIGHUP to us and dmenu will quit at 
    // SDL_Init() below if it's not ignored.
    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
        log_error("Unable to ignore SIGHUP");
    }
    #endif
    
    int rc =  init_display() || init_system();
    next_time = SDL_GetTicks() + TICK_INTERVAL;
    return rc;
}

void deinit() {
    log_debug("De-initializing");
    /* Call destructors, otherwise open FDs will be leaked to the
    exec()'ed process.
    Yes, this is ugly. If another situation like this arises we should write
    a custom atexit implementation.
    */ 
    
    // de-init everything
    colorpicker_deinit();
    filelist_deinit();
    imageviewer_deinit();
    
    // Save snapshot on menu deinit
    state = MAINMENU;
    update_display();
    save_menu_snapshot(screen, 1);
    
    // Save current menu state
    persistent_store_menu_position();
    
    menu_deinit();
    conf_unload();
}

void reload() {
    deinit();
    conf_load();
    menu_init();
}

void quick_quit() {
    // Exit without calling any atexit() functions
    _exit(1);
}

/**
 * Draw Screen
 */
void update_display() {

    // DRAWING STARTS HERE
    int recache = 0;
    switch (state) {
        case MAINMENU: recache = menu_draw(screen); break;
        case FILELIST: recache = filelist_draw(screen); break;
        case IMAGEVIEWER: recache = imageviewer_draw(screen); break;
        case COLORPICKER: recache = colorpicker_draw(screen); break;
    }
    
    if (!recache) {
        SDL_BlitSurface(screen_cache, NULL, screen, NULL);
    } else {
        SDL_BlitSurface(screen, NULL, screen_cache, NULL);
    }
    
    switch (state) {
        case MAINMENU: menu_animate(screen); break; 
        case FILELIST: filelist_animate(screen); break;
        case IMAGEVIEWER: imageviewer_animate(screen); break;
        case COLORPICKER: colorpicker_animate(screen); break;
    }
    
    switch (state) {
        case MAINMENU: menu_osd(screen); break;
        case FILELIST: filelist_osd(screen); break;
        case IMAGEVIEWER: imageviewer_osd(screen); break;
        case COLORPICKER: colorpicker_osd(screen); break;
    }
    
    // finally, update the screen :)
    SDL_Flip(screen);
    
    // wait for remaining time of the frame
    SDL_Delay(time_left());
    next_time += TICK_INTERVAL;    
}

void quit()
{
    program_done = 1;
}

void listen() {
    
    // program main loop
    program_done = 0;
    SDLKey key;
    SDL_Event event;
    MenuState prevstate;
    
    //Allow for easier menu nav
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
        
    while (!program_done)
    {
        // message processing loop
        while (SDL_PollEvent(&event))
        {            
            // check for messages
            switch (event.type)
            {
                // exit if the window is closed
                case SDL_QUIT:
                    quit();
                    break;
                    
                // check for keypresses
                case SDL_KEYDOWN:
                    key = event.key.keysym.sym;
                    // exit if ESCAPE is pressed
                    if (key == SDLK_ESCAPE) {
                        quit();
                        break;
                    }
                    
                    if (dosd_is_locked()) 
                        break;
                    
                    prevstate = state;
                    switch (state) {
                        case MAINMENU:
                            state = menu_keypress(key);
                            break;
                        case FILELIST:
                            state = filelist_keypress(key);
                            break;
                        case IMAGEVIEWER:
                            state = imageviewer_keypress(key);
                            break;
                        case COLORPICKER:
                            state = colorpicker_keypress(key);
                            break;
                    }
                    if (state == MAINMENU && prevstate != state) menu_state_changed();
                    break;
                    
            } // end switch
        } // end of message processing
        
        update_display(screen, state);
        
    } // end main loop
}

int main ( int argc, char** argv )
{
    int rc = init();
    log_debug("Fully initialized");

    if (rc==0) {
        listen(); //main loop
        deinit();
        // all is well ;)
        log_debug("Exited cleanly");
    }
    
    return rc;
}