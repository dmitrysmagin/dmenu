#include "volume.h"

#define ICON_x 253
#define ICON_y 3
#define ICON_w 9
#define ICON_h 9

#define TEXT_x 258
#define TEXT_y -1

int base_volume;

SDL_Surface* volume_status;
SDL_Surface* tmp_surface;
SDL_Color status_color = {255,255,255,0};

extern TTF_Font* status_font;

SDL_Rect dst_icon, dst_text;

void vol_init() {

	FILE *vol_fd;
	vol_fd = fopen("../../../home/.dmenu/sound_volume.ini", "r");
	if (vol_fd == NULL) {
		printf("Failed to load ../../../home/.dmenu/sound_volume.ini\n");
		exit(EXIT_FAILURE);
	}
	fscanf(vol_fd,"%d", &base_volume);
	fclose(vol_fd);

	tmp_surface = IMG_Load("../../../home/.dmenu/STATspeaker.png");
	if (tmp_surface == NULL) {
		printf("Failed to load ../../../home/.dmenu/STATspeaker.png: %s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}
	volume_status = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_FreeSurface(tmp_surface);

	vol_set(0);

}

void vol_set(int change) {
	char *mixer_device = "/dev/mixer";
	int mixer;
	FILE *vol_fd;
	int fd_no;

	base_volume += change;

	if (base_volume >= 100) base_volume = 100;
	if (base_volume <= 0) base_volume = 0;

	int oss_volume = base_volume | (base_volume << 8); // set volume for both channels
	mixer = open(mixer_device, O_WRONLY);
	if (ioctl(mixer, SOUND_MIXER_WRITE_VOLUME, &oss_volume) == -1) {
		fprintf(stderr, "Failed opening mixer for write - VOLUME\n");
	}
	close(mixer);

	vol_fd = fopen("../../../home/.dmenu/sound_volume.ini", "w");
	if (vol_fd == NULL) {
		printf("Failed to open ../../../home/.dmenu/sound_volume.ini\n");
		exit(EXIT_FAILURE);
	}
	fprintf(vol_fd, "%d", base_volume);
	fd_no = fileno(vol_fd);
	fsync(fd_no);
	fclose(vol_fd);

}

void vol_show(SDL_Surface* surface) {

	char buff[10];
	SDL_Surface* volume_text;

	dst_icon.x = ICON_x;
	dst_icon.y = ICON_y;
	dst_icon.w = ICON_w;
	dst_icon.h = ICON_h;

	dst_text.x = TEXT_x;
	dst_text.y = TEXT_y;

	SDL_BlitSurface( volume_status, NULL, surface, &dst_icon );
	sprintf(buff, "%4d%%", base_volume);
	tmp_surface = TTF_RenderUTF8_Solid( status_font, buff, status_color);
	volume_text = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_BlitSurface( volume_text, NULL, surface, &dst_text );

	SDL_FreeSurface(tmp_surface);
	SDL_FreeSurface(volume_text);

}

void vol_deinit() {

	SDL_FreeSurface(volume_status);

}

