#include "brightness.h"
#include "common.h"

int bright_level;

int bright[5]={10,25,50,75,99};

SDL_Surface* bright_status[5];
SDL_Rect dst_icon;

extern cfg_t *cfg_value;

void load_image(char* file, int pos) 
{
    bright_status[pos] = load_user_image(file);
}

void bright_init() 
{
    bright_level = (int)cfg_getint(cfg_value, "Bright");
    load_image("STATbright0.png", 0);
    load_image("STATbright1.png", 1);
    load_image("STATbright2.png", 2);
    load_image("STATbright3.png", 3);
    load_image("STATbright4.png", 4);
    bright_set(0);
}

void bright_set(int change) {

	bright_level = bound(bright_level + change, 0, 4);

#ifdef DINGOO_BUILD
	int file_no;
    FILE *brt_fd = open_file_or_die(BACKLIGHT_DEVICE, "w");
    fprintf(brt_fd, "%d", bright[bright_level] );
    file_no = fileno(brt_fd);
    fsync(file_no);
    fclose(brt_fd);
#endif

	cfg_setint( cfg_value, "Bright", (long)bright_level );

}

void bright_show(SDL_Surface *surface) {

	dst_icon.x = BRIGHTNESS_ICON_X;
    dst_icon.y = BRIGHTNESS_ICON_Y;
    dst_icon.w = BRIGHTNESS_ICON_W;
    dst_icon.h = BRIGHTNESS_ICON_H;
    
    bright_level = bound(bright_level, 0, 4);

	SDL_BlitSurface( bright_status[bright_level], NULL, surface, &dst_icon );

}

void bright_deinit() {
    int i;
    for (i=0;i<5;i++) SDL_FreeSurface(bright_status[i]);
}
