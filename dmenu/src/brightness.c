#include "brightness.h"

#define ICON_x 236
#define ICON_y 3
#define ICON_w 9
#define ICON_h 9

int bright_level;

int bright[5]={0,25,50,75,99};

SDL_Surface* bright_status[5];
SDL_Surface* tmp_surface;

SDL_Rect dst_icon;

void bright_init() {

	FILE *brt_fd;
	brt_fd = fopen("/usr/local/home/.dmenu/brightness_level.ini", "r");
	fscanf(brt_fd,"%d", &bright_level);
	fclose(brt_fd);

	tmp_surface = IMG_Load("/usr/local/home/.dmenu/STATbright0.png");
	if (!tmp_surface) {
		printf("Failed to load .dmenu_ini/STATbright0.png: %s\n", IMG_GetError());
	}
	bright_status[0] = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_FreeSurface(tmp_surface);

	tmp_surface = IMG_Load("/usr/local/home/.dmenu/STATbright1.png");
	if (!tmp_surface) {
		printf("Failed to load .dmenu_ini/STATbright1.png: %s\n", IMG_GetError());
	}
	bright_status[1] = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_FreeSurface(tmp_surface);

	tmp_surface = IMG_Load("/usr/local/home/.dmenu/STATbright2.png");
	if (!tmp_surface) {
		printf("Failed to load .dmenu_ini/STATbright2.png: %s\n", IMG_GetError());
	}
	bright_status[2] = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_FreeSurface(tmp_surface);

	tmp_surface = IMG_Load("/usr/local/home/.dmenu/STATbright3.png");
	if (!tmp_surface) {
		printf("Failed to load .dmenu_ini/STATbright3.png: %s\n", IMG_GetError());
	}
	bright_status[3] = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_FreeSurface(tmp_surface);

	tmp_surface = IMG_Load("/usr/local/home/.dmenu/STATbright4.png");
	if (!tmp_surface) {
		printf("Failed to load .dmenu_ini/STATbright4.png: %s\n", IMG_GetError());
	}
	bright_status[4] = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_FreeSurface(tmp_surface);

	bright_set(0);

}

void bright_set(int change) {
	char *backlight = "/proc/jz/lcd_backlight";
	FILE *brt_fd;

	bright_level += change;

	if (bright_level >= 4) bright_level = 4;
	if (bright_level <= 0) bright_level = 0;

	brt_fd = fopen(backlight, "w");
	fprintf(brt_fd, "%d", bright[bright_level] );

// printf("Bright-Level:%d / %d\n",bright_level,bright[bright_level]);

	fclose(brt_fd);

	brt_fd = fopen("/usr/local/home/.dmenu/brightness_level.ini", "w");
	fprintf(brt_fd, "%d", bright_level);
	fclose(brt_fd);
}

void bright_show(SDL_Surface *surface) {

	dst_icon.x = ICON_x;
	dst_icon.y = ICON_y;
	dst_icon.w = ICON_w;
	dst_icon.h = ICON_h;

	if (bright_level <=0 ) bright_level = 0;
	if (bright_level >=4 ) bright_level = 4;

	SDL_BlitSurface( bright_status[bright_level], NULL, surface, &dst_icon );

}

void bright_deinit() {

	SDL_FreeSurface(bright_status[0]);
	SDL_FreeSurface(bright_status[1]);
	SDL_FreeSurface(bright_status[2]);
	SDL_FreeSurface(bright_status[3]);
	SDL_FreeSurface(bright_status[4]);

}
