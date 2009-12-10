#include "brightness.h"
#include "common.h"
#include "resource.h"

int bright_levels[5]={10,25,50,75,99};
int bright_level;
int bright_dimmed;

SDL_Surface* bright_icons[5];
SDL_Rect bright_icon_rect;

extern cfg_t *cfg_main;

int bright_enabled()
{
    return cfg_getbool(cfg_main,"BrightDisp");
}

void bright_init() 
{
    log_debug("Initializing");
    
    char file[30];
    int i;
    for (i=0;i<5;i++) {
        sprintf(file, "STATbright%d.png", i);
        bright_icons[i] = load_osd_image(file);
    }
    
    bright_level = bound((int)cfg_getint(cfg_main, "Bright"), 0, 4);
    
    //Icon position
    init_rect(&bright_icon_rect,
        BRIGHTNESS_ICON_X, BRIGHTNESS_ICON_Y,
        BRIGHTNESS_ICON_W, BRIGHTNESS_ICON_H);
    
    bright_set(bright_level);
}

void bright_write(int level)
{
    #ifdef DINGOO_BUILD
    int file_no;
    FILE *brt_fd = load_file_or_die(BACKLIGHT_DEVICE, "w");
    fprintf(brt_fd, "%d", bright_levels[level] );
    file_no = fileno(brt_fd);
    fsync(file_no);
    fclose(brt_fd);
    #endif
    
}

void bright_change(Direction dir)
{
    bright_set(bright_level + (dir==PREV?-1:1));
}

void bright_dim(int on) 
{
    if (bright_dimmed != on)
    {
        log_debug("Setting Dim to %d", on);
        bright_dimmed = on;
        bright_write(on?0:bright_level);
    }
}

void bright_set(int level) 
{
    bright_dimmed = 0;
    bright_level = bound(level, 0, 4);
    bright_write(bright_level);
    cfg_setint( cfg_main, "Bright", (long)bright_level );
}

void bright_show(SDL_Surface *surface) 
{
    SDL_BlitSurface( bright_icons[bright_level], NULL, surface, &bright_icon_rect );
}

void bright_deinit() 
{
    log_debug("De-initializing");
    int i;
    for (i=0;i<5;i++) free_surface(bright_icons[i]);
}