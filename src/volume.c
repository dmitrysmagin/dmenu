#include "volume.h"
#include "resource.h"

extern cfg_t *cfg_main;

int volume_level;
SDL_Surface* volume_status;
SDL_Rect volume_src_rect, volume_dst_rect;

int volume_enabled()
{
    return cfg_getbool(cfg_main, "VolDisp");
}

void volume_init() 
{    
    log_debug("Initializing");
    
    volume_level  = (int)cfg_getint(cfg_main, "SndVol");
    volume_status = load_osd_image("volume.png");
    
    //Initialize OSD positions
    init_rect(&volume_dst_rect, 
        VOLUME_ICON_X, VOLUME_ICON_Y,
        VOLUME_ICON_MAX_W, VOLUME_ICON_H);
    
    init_rect(&volume_src_rect, 0, 0,
        VOLUME_ICON_MIN_W, VOLUME_ICON_H);
              
    //Set volume
    volume_set(volume_level);
}

void volume_change(Direction dir) 
{
    volume_set(volume_level + (dir==PREV?-5:5));
}

void volume_set(int level) 
{
    int min = VOLUME_ICON_MIN_W, max = VOLUME_ICON_MAX_W;
    volume_level = bound(level, 0, 100);

    //Calculate clipping of volume rect
    volume_src_rect.w = min+(int)((max-min)*(volume_level/100.0f));

    int oss_volume = volume_level | (volume_level << 8); // set volume for both channels

	int mixer = open(MIXER_DEVICE, O_WRONLY);
	if (ioctl(mixer, SOUND_MIXER_WRITE_VOLUME, &oss_volume) == -1) {
        log_error("Failed opening mixer for write - VOLUME");
	}
	close(mixer);

    cfg_setint( cfg_main, "SndVol", (long)volume_level );
}

void volume_show(SDL_Surface* surface) 
{
    SDL_BlitSurface(volume_status, &volume_src_rect, surface, &volume_dst_rect );    
}

void volume_deinit() 
{
    log_debug("De-initializing");
    free_surface(volume_status);
}