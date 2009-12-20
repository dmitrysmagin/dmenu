#include "brightness.h"
#include "common.h"
#include "resource.h"

int brightness_level;
int brightness_dimmed;

SDL_Surface* brightness_icon;
SDL_Rect brightness_icon_rect;

extern cfg_t *cfg_main;

int brightness_enabled()
{
    return cfg_getbool(cfg_main,"BrightDisp");
}

void brightness_init() 
{
    log_debug("Initializing");

    brightness_icon = load_osd_image("brightness.png");
    
    //Icon position
    init_rect(&brightness_icon_rect,
        BRIGHTNESS_ICON_X, BRIGHTNESS_ICON_Y,
        BRIGHTNESS_ICON_W, BRIGHTNESS_ICON_H);
    
    brightness_set((int)cfg_getint(cfg_main, "Bright"));
}

void brightness_write(int level)
{
    #ifdef DINGOO_BUILD
        FILE *brt_fd = load_file_or_die(BACKLIGHT_DEVICE, "w");
        fprintf(brt_fd, "%d", level );
        fclose(brt_fd);
    #endif
}

void brightness_change(Direction dir)
{
    brightness_set(brightness_level + (dir==PREV?-1:1)*10);
}

void brightness_dim(int on) 
{
    if (brightness_dimmed != on)
    {
        log_debug("Setting Dim to %d", on);
        brightness_dimmed = on;
        brightness_write(on?0:brightness_level);
    }
}

void brightness_set(int level) 
{
    brightness_dimmed = 0;
    brightness_level = bound(level, 9, 99);
    brightness_write(brightness_level);
    cfg_setint( cfg_main, "Bright", (long)brightness_level );
}

void brightness_show(SDL_Surface *surface) 
{
    int alpha = (int)(255.0*(brightness_level/100.0));
    blitSurfaceAlpha(brightness_icon, NULL, surface, &brightness_icon_rect, alpha);
}

void brightness_deinit() 
{
    log_debug("De-initializing");
    free_surface(brightness_icon);
}