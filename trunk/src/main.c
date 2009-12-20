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
#include "dingoo.h"
#include "common.h"
#include "resource.h"

#include "main.h"
#include "menu.h"
#include "filelist.h"
#include "colorpicker.h"
#include "imageviewer.h"

#include "loading.h"
#include "conf.h"
#include "brightness.h"
#include "volume.h"
#include "sound.h"
#include "persistent.h"
#include "dosd/dosd.h"

extern cfg_t *cfg_main;

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
    if (conf_load(NULL)) return 1;
    
    loading_set_level(22);
    
    if (can_write_fs()) clear_last_command();
    
    // Read saved persistent state
    if (!persistent_init())
    {
        log_error("Unable to initialize persistent memory");
    }
    
    loading_set_level(33);
        
    //Init OSD 
    {
        brightness_init();
        loading_set_level(44);
        
        volume_init();
        loading_set_level(55);
        
        if (!dosd_init())
        {
            log_error("Unable to initialize OSD");
            return 1;
        }
        loading_set_level(66);
    }
    
    // Init sound
    sound_init();
    loading_set_level(77);
    
    
    // init menu
    if (menu_init())
    {
        log_error("Unable to load menu");
        return 1;
    }
    loading_set_level(88);
    
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
    } else {
        loading_init(screen);
        loading_set_level(0);
    }
    
    screen_cache = create_surface(SCREEN_WIDTH, SCREEN_HEIGHT, 24, 0xFFFFFF,0);
    
    
    //Ready fonts
    TTF_Init();
    
    //Load Screen
    loading_set_level(11);
    
    return 0;
}

int init() {
    log_debug("Initializing");
    
    set_write_fs(filesystem_writeable()); 
    log_message("Filesystem Writeable: %d", filesystem_writeable());
    
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
    state = MAINMENU;
    return rc;
}

void deinit(DeinitLevel level) {
    log_debug("De-initializing, level %d", level);
    
    int shutdown = level == SHUTDOWN;
    
    if (shutdown) {
        loading_start();
        loading_set_level(99);
    }
    
    // de-init everything
    colorpicker_deinit();
    filelist_deinit();
    imageviewer_deinit();
    state = MAINMENU;

    if (shutdown) 
    {   
        sound_deinit();
        loading_set_level(88);
        brightness_deinit();
        loading_set_level(77);
        volume_deinit();
        loading_set_level(66);
        dosd_deinit();
        loading_set_level(55);
    }
    
    menu_deinit();
    
    if (level == SHUTDOWN) 
    {
        loading_set_level(44);
        //Close down fonts
        TTF_Quit();
        loading_set_level(33);
        conf_unload();
        loading_set_level(22);
        loading_deinit();
    }
}

void reload(DeinitLevel level) {
    deinit(level);
    if (level == RELOAD_THEME) 
    {
        conf_reload_theme(THEME_NAME);
    }
    menu_init();
}

void draw_osd(SDL_Surface* screen) {
    dosd_show(screen);    
    if (volume_enabled())     volume_show(screen);
    if (brightness_enabled()) brightness_show(screen);
}

/**
 * Draw Screen
 */
void update_display() {

    // DRAWING STARTS HERE
    int recache = 0;
    switch (state) {
        case MAINMENU:    recache = menu_draw(screen); break;
        case FILELIST:    recache = filelist_draw(screen); break;
        case IMAGEVIEWER: recache = imageviewer_draw(screen); break;
        case COLORPICKER: recache = colorpicker_draw(screen); break;
    }
    
    if (!recache) {
        SDL_BlitSurface(screen_cache, NULL, screen, NULL);
    } else {
        SDL_BlitSurface(screen, NULL, screen_cache, NULL);
    }
    
    switch (state) {
        case MAINMENU:    menu_animate(screen); break;
        case FILELIST:    filelist_animate(screen); break;
        case IMAGEVIEWER: imageviewer_animate(screen); break;
        case COLORPICKER: colorpicker_animate(screen); break;
    }
    
    switch (state) {
        case MAINMENU:    menu_osd(screen); draw_osd(screen);break;
        case FILELIST:    filelist_osd(screen); break;
        case IMAGEVIEWER: imageviewer_osd(screen); break;
        case COLORPICKER: colorpicker_osd(screen); break;
    }
    
    // finally, update the screen :)
    SDL_Flip(screen);
    
    // wait for remaining time of the frame
    SDL_Delay(time_left());
    next_time += TICK_INTERVAL;    
}

void handle_global_key(SDLKey key) {
    //Handle OSD activity
    Direction dir  = getKeyDir(key);
    if (state == MAINMENU) {
        switch (key) {
            case DINGOO_BUTTON_L:
            case DINGOO_BUTTON_R:
                volume_change(dir);
                sound_out( GLOBAL_KEY );
                break;
            case DINGOO_BUTTON_X:
            case DINGOO_BUTTON_Y:
                brightness_change(dir);
                sound_out( GLOBAL_KEY );
                break;
            default: break;
        }
    }
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
    
    int last_key_time = SDL_GetTicks();
    int inactive_delay = cfg_getint(cfg_main, "DimmerDelay");
    
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
                case SDL_QUIT: quit(); break;

                // check for keypresses
                case SDL_KEYDOWN:
                    key = event.key.keysym.sym;
                    // exit if ESCAPE is pressed
                    if (key == SDLK_ESCAPE) {
                        quit();
                        break;
                    }

                    if (dosd_is_locked()) break;

                    prevstate = state;
                    switch (state) {
                        case MAINMENU:    state = menu_keypress(key); break;
                        case FILELIST:    state = filelist_keypress(key); break;
                        case IMAGEVIEWER: state = imageviewer_keypress(key); break;
                        case COLORPICKER: state = colorpicker_keypress(key); break;
                    }
                    if (state == MAINMENU && prevstate != state) {
                        menu_force_redraw(screen_cache);
                    }
                    
                    handle_global_key(key);
                    
                    brightness_dim(0);
                    last_key_time = SDL_GetTicks();
                    break;
            } // end switch
        } // end of message processing
        
        if (inactive_delay > 0 && (SDL_GetTicks()-last_key_time) > (inactive_delay*1000)) 
        {
            brightness_dim(1);
        }

        update_display(screen, state);
    } // end main loop
}

int main ( int argc, char** argv )
{
    int rc = init();
    log_debug("Fully initialized");

    if (rc==0) {
        listen(); //main loop
        deinit(SHUTDOWN);
        
        // all is well ;)
        log_debug("Exited cleanly");
    }
    
    return rc;
}