/*
 *  Copyright (C) 2009 sca <scahoo <at> gmail <dot> com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdarg.h>
#include "dosd/dosd.h"
#include "dosd/images.h"

#define GPIO_LOCK_MASK    (0x400000)
#define GPIO_POWER_MASK (0x40000000)

typedef enum {
    BAT_EMPTY = 0,
    BAT_LV1,
    BAT_LV2,
    BAT_FULL,
    BAT_MAX
} battery_state_e;

// ___ Globals ___________________________________________
// Global state
struct {
    FILE* proc_battery;
    FILE* proc_gpio3;
    FILE* proc_gpio1;
    bool is_locked;
    bool is_charging;
    battery_state_e battery;
    int battery_anim;
    unsigned int update_counter;
    // Measured in SDL_GetTick()s
    uint32_t next_update;
} g_state;

// Processed images
image_t g_images[IMG_MAX];

// ___ Functions ___________________________________________________
void _blit(SDL_Surface *surface, image_e which_image, ...);
void _update();

// Color should be passed in RRGGBB format, e.g. 0xFFFFFF for white
int dosd_init(uint32_t color)
{
    int i;

    memset(&g_state, 0, sizeof(g_state));
    memset(&g_images, 0, sizeof(g_images));
    
#ifdef DINGOO_BUILD
    g_state.proc_battery = fopen("/proc/jz/battery", "rb");
    if (!g_state.proc_battery) goto init_error;
    
    g_state.proc_gpio1 = fopen("/proc/jz/gpio1_pxpin", "rb");
    if (!g_state.proc_gpio1) goto init_error;
    
    g_state.proc_gpio3 = fopen("/proc/jz/gpio3_pxpin", "rb");
    if (!g_state.proc_gpio3) goto init_error;
#endif
    
    // Make room for alpha "channel"
    color <<= 8; // RRGGBBAA
    
    for (i = 0; i < IMG_MAX; i++)
    {
        const image_data_t* image_data;
        image_t* image;
        uint32_t* buf;
        SDL_Surface* surface;
        int j;
        
        image_data =  c_images[i];
        image      = &g_images[i];
        
        image->w  = image_data->w;
        image->h  = image_data->h;
        
        buf = malloc(image->w * image->h * 4);
        if (!buf) goto init_error;
        
        for (j = 0; j < image->w * image->h; j++)
        {
            // We OR in the alpha information into the color
            // passed as an argument.
            // The image data is in effect an "alpha map":
            // 0x00 is transparent, 0xFF is fully opaque
            buf[j] = color | image_data->data[j];
        }
        
        surface = SDL_CreateRGBSurfaceFrom(
            buf, image->w, image->h, 32, image->w * 4,
            0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
        );
        
        if (!surface) goto init_error;
        
        // Convert surface to target surface format
        image->sfc = SDL_DisplayFormatAlpha(surface);
        if (!image->sfc) 
        {
            SDL_FreeSurface(surface);
            goto init_error;
        }
        
        SDL_FreeSurface(surface);
        free(buf);
    }
    
    // Init g_state
    _update();
    
    return 1;
init_error:
    dosd_deinit();
    return 0;
}

void dosd_deinit()
{
    int i;
    
#ifdef DINGOO_BUILD
    if (g_state.proc_battery)
        fclose(g_state.proc_battery);
    
    if (g_state.proc_gpio1)
        fclose(g_state.proc_gpio1);
    
    if (g_state.proc_gpio3)
        fclose(g_state.proc_gpio3);
#endif
    
    for (i = 0; i < IMG_MAX; i++)
    {
        if (g_images[i].sfc)
            SDL_FreeSurface(g_images[i].sfc);
    }
}

void dosd_show(SDL_Surface* surface)
{
    image_e battery_status;
    
    if (SDL_GetTicks() >= g_state.next_update)
        _update();
    
    // Battery
    _blit(surface, IMG_BATTERY, -1);
    
    // This is a bit of a hack, as it relies on battery_state_e
    // To have the same index as image_e
    battery_status = g_state.is_charging ? g_state.battery_anim : g_state.battery;
    if (battery_status != BAT_EMPTY)
        _blit(surface, battery_status, -1);
    
    // Lock
    if (g_state.is_locked)
        _blit(surface, IMG_LOCK, IMG_BATTERY, -1);
}

inline bool dosd_is_locked()
{
    return g_state.is_locked;
}

// ___ Helpers ___________________________________________________
void _blit(SDL_Surface *surface, image_e which_image, ...)
{
    /*
        image_e are expected in the variable arguments.
        The image will be offset by the combined width + padding
        of the images in the varargs.
        ALWAYS pass -1 as the last argument!
    */
    SDL_Rect dst;
    va_list ap;
    image_e  padding;
    image_t* image;
    
    if (which_image < 0 || which_image >= IMG_MAX)
        return;
    
    image = &g_images[which_image];
    
    dst.y = DOSD_PADDING;
    dst.x = surface->w - image->w - DOSD_PADDING; 
    
    va_start(ap, which_image);
    while ((padding = va_arg(ap, image_e)) != -1)
    {
        dst.x -= DOSD_PADDING + g_images[padding].w;
    }
    
    va_end(ap);
    
    SDL_BlitSurface(image->sfc, NULL, surface, &dst);
}

void _update()
{
#ifdef DINGOO_BUILD
    char buf[128];
    int mvolts;
    uint32_t gpio;
    
    rewind(g_state.proc_battery);
    fgets(buf, 127, g_state.proc_battery);
    sscanf(buf, "%d", &mvolts);
    
#if 0
    // Charge status
    rewind(g_state.proc_gpio1);
    fgets(buf, 127, g_state.proc_gpio1);
    sscanf(buf, "%x", &gpio);
    
    // FIXME: the GPIO bit indicating charge status seems
    // to randomly jump when a USB cable is attached.
    g_state.is_charging = ((gpio & GPIO_POWER_MASK) == 0);
#endif
    
    // Lock status
    rewind(g_state.proc_gpio3);
    fgets(buf, 127, g_state.proc_gpio3);
    sscanf(buf, "%x", &gpio);
    
    g_state.is_locked = ((gpio & GPIO_LOCK_MASK) == 0);
    
    // Battery charge level
    if      (mvolts >= 3739) g_state.battery = BAT_FULL;
    else if (mvolts >= 3675) g_state.battery = BAT_LV2;
    else if (mvolts >= 3611) g_state.battery = BAT_LV1;
    else                     g_state.battery = BAT_EMPTY;
#else
    g_state.is_locked   = false;
    g_state.is_charging = true;
    g_state.battery     = BAT_EMPTY;
#endif
    
    // Next update
    g_state.next_update = SDL_GetTicks() + DOSD_UPDATE_INTERVAL;
    g_state.update_counter++;
    
    if (g_state.is_charging && g_state.update_counter % 2 == 0)
    {
        // This happens every 2 * DOSD_UPDATE_INTERVAL ticks
        g_state.battery_anim++;
        if (g_state.battery_anim >= BAT_MAX)
            g_state.battery_anim = BAT_EMPTY;
    }
}
