#include "volume.h"
#include "resource.h"

extern cfg_t *cfg_main;

int base_volume;

SDL_Surface* volume_status;
SDL_Surface* volume_text;
TTF_Font* volume_font;

SDL_Color status_color = {DOSD_COLOR_RGBA};
SDL_Rect vol_src_rect, vol_dst_rect;

int vol_enabled()
{
    return cfg_getbool(cfg_main, "VolDisp");
}

void vol_init() {
    
    log_debug("Initializing");
    
    base_volume = (int)cfg_getint(cfg_main, "SndVol");
    volume_status = load_global_image("STATspeaker.png");
    
    //Initialize OSD positions
    init_rect(&vol_dst_rect, 
        VOLUME_ICON_X, VOLUME_ICON_Y,
        VOLUME_ICON_MAX_W, VOLUME_ICON_H);
    
    init_rect(&vol_src_rect, 0, 0,
        VOLUME_ICON_MIN_W, VOLUME_ICON_H);
              
    //Set volume
    vol_set(0);
}

void vol_set(int change) {
	int mixer;

	base_volume = bound(base_volume + change, 0, 100);

    vol_src_rect.w = VOLUME_ICON_MIN_W+(int)((VOLUME_ICON_MAX_W-VOLUME_ICON_MIN_W)*(base_volume/100.0f));
              
	int oss_volume = base_volume | (base_volume << 8); // set volume for both channels
    
	mixer = open(MIXER_DEVICE, O_WRONLY);
	if (ioctl(mixer, SOUND_MIXER_WRITE_VOLUME, &oss_volume) == -1) {
        log_error("Failed opening mixer for write - VOLUME");
	}
	close(mixer);

    cfg_setint( cfg_main, "SndVol", (long)base_volume );
}

void vol_show(SDL_Surface* surface) 
{    
    SDL_BlitSurface(volume_status, &vol_src_rect, surface, &vol_dst_rect );    
}

void vol_deinit() 
{
    log_debug("De-initializing");
    free_surface(volume_status);
}