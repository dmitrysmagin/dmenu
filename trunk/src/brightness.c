#include "brightness.h"
#include "common.h"
#include "resource.h"

int brightness_levels[5]={10,25,50,75,99};
int brightness_level;
int brightness_dimmed;

SDL_Surface* brightness_icons[5];
SDL_Rect brightness_icon_rect;

extern cfg_t *cfg_main;

int brightness_enabled()
{
    return cfg_getbool(cfg_main,"BrightDisp");
}

void brightness_init() 
{
    log_debug("Initializing");
    
    char file[30];
    int i;
    for (i=0;i<5;i++) {
        sprintf(file, "STATbright%d.png", i);
        brightness_icons[i] = load_osd_image(file);
    }
    
    brightness_level = bound((int)cfg_getint(cfg_main, "Bright"), 0, 4);
    
    //Icon position
    init_rect(&brightness_icon_rect,
        BRIGHTNESS_ICON_X, BRIGHTNESS_ICON_Y,
        BRIGHTNESS_ICON_W, BRIGHTNESS_ICON_H);
    
    brightness_set(brightness_level);
}

void brightness_write(int level)
{
    #ifdef DINGOO_BUILD
    int file_no;
    FILE *brt_fd = load_file_or_die(BACKLIGHT_DEVICE, "w");
    fprintf(brt_fd, "%d", brightness_levels[level] );
    file_no = fileno(brt_fd);
    fsync(file_no);
    fclose(brt_fd);
    #endif
    
}

void brightness_change(Direction dir)
{
    brightness_set(brightness_level + (dir==PREV?-1:1));
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
    brightness_level = bound(level, 0, 4);
    brightness_write(brightness_level);
    cfg_setint( cfg_main, "Bright", (long)brightness_level );
}

void brightness_show(SDL_Surface *surface) 
{
    SDL_BlitSurface( brightness_icons[brightness_level], NULL, surface, &brightness_icon_rect );
}

void brightness_deinit() 
{
    log_debug("De-initializing");
    int i;
    for (i=0;i<5;i++) free_surface(brightness_icons[i]);
}