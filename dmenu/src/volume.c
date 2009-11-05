#include "volume.h"
#include "resource.h"

extern TTF_Font* status_font;
extern cfg_t *cfg_value, *cfg;

int base_volume;

SDL_Surface* volume_status;
SDL_Surface* volume_text;
SDL_Color status_color = {OSD_COLOR_RGBA};
SDL_Rect dst_icon, dst_text;

int vol_enabled()
{
    return cfg_getbool(cfg,"VolDisp");
}

void vol_init() {
    log_debug("Initializing");
    
    base_volume = (int)cfg_getint(cfg_value, "SndVol");
    volume_status = load_user_image("STATspeaker.png");
    
    //Set volume
    vol_set(0);

    //Initialize OSD positions
    init_rect(&dst_icon, 
        VOLUME_ICON_X, VOLUME_ICON_Y,
        VOLUME_ICON_W, VOLUME_ICON_H);
    init_rect_pos(&dst_text, 
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

	cfg_setint( cfg_value, "SndVol", (long)base_volume );
    vol_set_text(base_volume);
}

void vol_set_text(int volume)
{
    char buff[10];
    sprintf(buff, "%4d%%", volume);
    
    free_surface(volume_text);
    volume_text = render_text(buff, status_font, &status_color, 1);
}

void vol_show(SDL_Surface* surface) 
{    
	SDL_BlitSurface(volume_status, NULL, surface, &dst_icon );    
    SDL_BlitSurface(volume_text,   NULL, surface, &dst_text );
}

void vol_deinit() 
{
    log_debug("De-initializing");
    free_surface(volume_status);
    free_surface(volume_text);
}