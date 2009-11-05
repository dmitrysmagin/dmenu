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
#include <stdlib.h>
#include <signal.h>
#include <SDL.h>
#include <SDL_image.h>
#include "env.h" 
#include "common.h"

#include "menu.h"
#include "filelist.h"
#include "imageviewer.h"

#include "conf.h"
#include "persistent.h"
#include "dosd/dosd.h"
#include "volume.h"
#include "brightness.h"

extern cfg_t *cfg;

static Uint32 next_time;
SDL_Surface* screen;
enum MenuState state = MAINMENU;

Uint32 time_left(void)
{
    Uint32 now;

    now = SDL_GetTicks();
    if(next_time <= now)
        return 0;
    else
        return next_time - now;
}

int init_env() {
    int rc=0;
    
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

    //Move to dmenu root dir
    change_dir(DMENU_PATH);
    
    // load config
    rc = conf_load();
    if (rc) return rc;
    
    // Read saved persistent state
    if (!persistent_init())
    {
        log_error("Unable to initialize persistent memory");
    }
    
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
    
    // create a new window
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_DEPTH, SDL_SWSURFACE);
    if ( !screen )
    {
        log_error("Unable to set %dx%d video: %s", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
        return 1;
    }
    
    // disable mouse pointer
    SDL_ShowCursor(SDL_DISABLE);
   
    // init menu config
    if (menu_init())
    {
        log_error("Unable to load menu");
        return 1;
    }
    
    // Init OSD
    if (!dosd_init(0xFFFFFF))
    {
        log_error("Unable to initialize OSD");
        return 1;
    }
    
    next_time = SDL_GetTicks() + TICK_INTERVAL;
    
    return 0;
}

int init() {
    return init_env() || init_display();
}

void deinit() {
    /* Call destructors, otherwise open FDs will be leaked to the
    exec()'ed process.
    Yes, this is ugly. If another situation like this arises we should write
    a custom atexit implementation.
    */ 
    
    // de-init everything
    filelist_deinit();
    imageviewer_deinit();
    menu_deinit();
    conf_unload();
    dosd_deinit();
}

void reload() {
    deinit();
    conf_load();
    menu_init();
    dosd_init(0xFFFFFF);
}

void quick_exit() {
    // Exit without calling any atexit() functions
    _exit(1);
}

/**
 * Draw Screen
 */
void update_display() {

    // DRAWING STARTS HERE
    switch (state) {
        case MAINMENU:
            menu_draw(screen);
            break;
        case FILELIST:
            filelist_draw(screen);
            break;
        case IMAGEVIEWER:
            imageviewer_draw(screen);
            break;
    }
    
    dosd_show(screen);
    
    // check VolDisp & BrightDisp
    if (cfg_getbool(cfg,"VolDisp"))    vol_show(screen);
    if (cfg_getbool(cfg,"BrightDisp")) bright_show(screen);
    
    // finally, update the screen :)
    SDL_Flip(screen);
    
    // wait for remaining time of the frame
    SDL_Delay(time_left());
    next_time += TICK_INTERVAL;    
}

void listen() {
    
    // program main loop
    int done = 0;
    SDLKey key;
    SDL_Event event;
    
    while (!done)
    {
        // message processing loop
        while (SDL_PollEvent(&event))
        {            
            // check for messages
            switch (event.type)
            {
                // exit if the window is closed
                case SDL_QUIT:
                    done = 1;
                    break;
                    
                // check for keypresses
                case SDL_KEYDOWN:
                    key = event.key.keysym.sym;
                    
                    // exit if ESCAPE is pressed
                    if (key == SDLK_ESCAPE) {
                        done = 1;
                        break;
                    }
                    
                    if (dosd_is_locked())
                        break;
                    
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
                    }
                    break;
            } // end switch
        } // end of message processing
        
        update_display(screen, state);
        
    } // end main loop
}

int main ( int argc, char** argv )
{
    int rc = init();

    if (rc==0) {
        listen(); //main loop
        deinit();
    }
    
    // all is well ;)
    //printf("Exited cleanly\n");
    return rc;
}
