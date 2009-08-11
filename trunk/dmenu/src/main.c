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
#include "common.h"
#include "menu.h"
#include "filelist.h"
#include "conf.h"
#include "persistent.h"
#include "dosd/dosd.h"

enum MenuState state;

#define TICK_INTERVAL    100 // this is 1000/100 = 10 fps

static Uint32 next_time;

Uint32 time_left(void)
{
    Uint32 now;

    now = SDL_GetTicks();
    if(next_time <= now)
        return 0;
    else
        return next_time - now;
}

int main ( int argc, char** argv )
{
    int rc=0;

#ifdef DINGOO_BUILD
    // Need to ignore SIGHUP if we are started by init.
    // In FB_CloseKeyboard() of SDL_fbevents.c , it does
    // ioctl(tty0_fd, TIOCNOTTY, 0) to detach the process from tty,
    // this will send a SIGHUP to us and dmenu will quit at 
    // SDL_Init() below if it's not ignored.
    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
        printf("Unable to ignore SIGHUP\n");
        perror(0);
    }

    // cd to dmenu directory - hardcode to /usr/local/dmenu on dingux
    rc = chdir("/usr/local/dmenu");
    if (rc != 0) {
        printf("Unable to change to /usr/local/dmenu\n");
        perror(0);
        return rc;
    }
#endif

    // load config
    rc = conf_load();
    if (rc != CFG_SUCCESS) {
        return rc;
    }

    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    SDL_Surface* screen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE);
    if ( !screen )
    {
        printf("Unable to set 320x240 video: %s\n", SDL_GetError());
        return 1;
    }

    // disable mouse pointer
    SDL_ShowCursor(SDL_DISABLE);

    // Read saved persistent state
    if (!persistent_init())
    {
        printf("Unable to initialize persistent memory\n");
        return 1;
    }
    persistent_read();


    // init menu config
    if (menu_init())
    {
        printf("Unable to load menu\n");
        return 1;
    }

    state = MAINMENU;
    
    // Init OSD
    if (!dosd_init(0xFFFFFF))
    {
        printf("Unable to initialize OSD\n");
        return 1;
    }

    next_time = SDL_GetTicks() + TICK_INTERVAL;

    // program main loop
    int done = 0;
    while (!done)
    {
        // message processing loop
        SDL_Event event;
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
                {
                    // exit if ESCAPE is pressed
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        done = 1;
                    
                    if (dosd_is_locked())
                        continue;

                    if (state == MAINMENU)
                        state = menu_keypress(event.key.keysym.sym);
                    else if (state == FILELIST)
                        state = filelist_keypress(event.key.keysym.sym);
                    //printf("key pressed - %d\n", event.key.keysym.sym);

                    break;
                }
            } // end switch
        } // end of message processing

        // DRAWING STARTS HERE
        if (state == MAINMENU)
            menu_draw(screen);
        else if (state == FILELIST)
            filelist_draw(screen);

        dosd_show(screen);
        
        // DRAWING ENDS HERE

        // finally, update the screen :)
        SDL_Flip(screen);

        // wait for remaining time of the frame
        SDL_Delay(time_left());
        next_time += TICK_INTERVAL;
    } // end main loop

    // de-init everything
    menu_deinit();
    filelist_deinit();
    conf_unload();
    dosd_deinit();
    
    // Write persistent data
    persistent_write();

    // all is well ;)
    //printf("Exited cleanly\n");
    return 0;
}
