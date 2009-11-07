#include "volume.h"
#include "resource.h"

extern cfg_t *cfg_main;

int base_volume;

SDL_Surface* volume_status;
SDL_Surface* volume_text;
TTF_Font* volume_font;

SDL_Color status_color = {DOSD_COLOR_RGBA};
SDL_Rect vol_dst_icon, vol_dst_text;

int vol_enabled()
{
    return cfg_getbool(cfg_main, "VolDisp");
}

void vol_init() {
    
    log_debug("Initializing");
    
    base_volume = (int)cfg_getint(cfg_main, "SndVol");
    volume_status = load_global_image("STATspeaker.png");
    volume_font = get_osd_font();
    
    //Set volume
    vol_set(0);

    //Initialize OSD positions
    init_rect(&vol_dst_icon, 
        VOLUME_ICON_X, VOLUME_ICON_Y,
        VOLUME_ICON_W, VOLUME_ICON_H);
    init_rect_pos(&vol_dst_text, 
        VOLUME_TEXT_X, VOLUME_TEXT_Y);
}

void vol_set(int change) {
	int mixer;

	base_volume = bound(base_volume + change, 0, 100);

	int oss_volume = base_volume | (base_volume << 8); // set volume for both channels
    
	mixer = open(MIXER_DEVICE, O_WRONLY);
	if (ioctl(mixer, SOUND_MIXER_WRITE_VOLUME, &oss_volume) == -1) {
        log_error("Failed opening mixer for write - VOLUME");
	}
	close(mixer);

    cfg_setint( cfg_main, "SndVol", (long)base_volume );
    vol_set_text(base_volume);
}

void vol_set_text(int volume)
{
    char buff[10];
    sprintf(buff, "%3d%%", volume);
    
    free_surface(volume_text);
    volume_text = render_text(buff, volume_font, &status_color, 1);
}

void vol_show(SDL_Surface* surface) 
{    
    SDL_BlitSurface(volume_status, NULL, surface, &vol_dst_icon );    
    SDL_BlitSurface(volume_text,   NULL, surface, &vol_dst_text );
}

void vol_deinit() 
{
    log_debug("De-initializing");
    free_font(volume_font);
    free_surface(volume_status);
    free_surface(volume_text);
}