#include "brightness.h"
#include "common.h"
#include "resource.h"

int bright_level;

int bright[5]={10,25,50,75,99};

SDL_Surface* bright_status[5];
SDL_Rect bright_icon;

extern cfg_t *cfg_value, *cfg;

void load_image(char* file, int pos) 
{
    bright_status[pos] = load_global_image(file);
}

int bright_enabled()
{
    return cfg_getbool(cfg,"BrightDisp");
}

void bright_init() 
{
    log_debug("Initializing");
    
    char file[30];
    int i;
    for (i=0;i<5;i++) {
        sprintf(file, "STATbright%d.png", i);
        load_image(file, i);
    }
    
    bright_level = bound((int)cfg_getint(cfg_value, "Bright"), 0, 4);
    
    //Icon position
    init_rect(&bright_icon,
        BRIGHTNESS_ICON_X, BRIGHTNESS_ICON_Y,
        BRIGHTNESS_ICON_W, BRIGHTNESS_ICON_H);
    
    bright_set(0);
}

void bright_set(int change) {

    bright_level = bound(bright_level + change, 0, 4);

    #ifdef DINGOO_BUILD
        int file_no;
        FILE *brt_fd = load_file_or_die(BACKLIGHT_DEVICE, "w");
        fprintf(brt_fd, "%d", bright[bright_level] );
        file_no = fileno(brt_fd);
        fsync(file_no);
        fclose(brt_fd);
    #endif

    cfg_setint( cfg_value, "Bright", (long)bright_level );
}

void bright_show(SDL_Surface *surface) {
    SDL_BlitSurface( bright_status[bright_level], NULL, surface, &bright_icon );
}

void bright_deinit() {
    log_debug("De-initializing");
    int i;
    for (i=0;i<5;i++) free_surface(bright_status[i]);
}
