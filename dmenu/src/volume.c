#include "volume.h"

extern TTF_Font* status_font;
extern cfg_t *cfg_value;

int base_volume;

SDL_Surface* volume_status;
SDL_Color status_color = {255,255,255,0};
SDL_Rect dst_icon, dst_text;

void vol_init() {
        
    base_volume = (int)cfg_getint(cfg_value, "SndVol");
    volume_status = load_user_image("STATspeaker.png");
    vol_set(0);

}

void vol_set(int change) {
	int mixer;

	base_volume = bound(base_volume + change, 0, 100);

	int oss_volume = base_volume | (base_volume << 8); // set volume for both channels
    
	mixer = open(MIXER_DEVICE, O_WRONLY);
	if (ioctl(mixer, SOUND_MIXER_WRITE_VOLUME, &oss_volume) == -1) {
		fprintf(stderr, "Failed opening mixer for write - VOLUME\n");
	}
	close(mixer);

	cfg_setint( cfg_value, "SndVol", (long)base_volume );

}

void vol_show(SDL_Surface* surface) {

    SDL_Surface* volume_text;
	char buff[10];
    sprintf(buff, "%4d%%", base_volume);
    
	init_rect(&dst_icon, 
        VOLUME_ICON_X, VOLUME_ICON_Y,
        VOLUME_ICON_W, VOLUME_ICON_H);
        
    init_rect_pos(&dst_text, 
        VOLUME_TEXT_X, VOLUME_TEXT_Y);
    
	SDL_BlitSurface( volume_status, NULL, surface, &dst_icon );
    
    volume_text = render_text(buff, status_font, &status_color, 1);
	SDL_BlitSurface(volume_text, NULL, surface, &dst_text );
	SDL_FreeSurface(volume_text);

}

void vol_deinit() {

	SDL_FreeSurface(volume_status);

}