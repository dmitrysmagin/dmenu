#include "brightness.h"

#define ICON_x 238
#define ICON_y 3
#define ICON_w 9
#define ICON_h 9

int bright_level;

int bright[5]={10,25,50,75,99};

SDL_Surface* bright_status[5];
SDL_Surface* tmp_surface;

SDL_Rect dst_icon;

extern cfg_t *cfg_value;

void bright_init() {

	bright_level = (int)cfg_getint(cfg_value, "Bright");

	tmp_surface = IMG_Load("../../../home/.dmenu/STATbright0.png");
	if (tmp_surface == NULL) {
		printf("Failed to load ../../../home/.dmenu/STATbright0.png: %s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}
	bright_status[0] = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_FreeSurface(tmp_surface);

	tmp_surface = IMG_Load("../../../home/.dmenu/STATbright1.png");
	if (tmp_surface == NULL) {
		printf("Failed to load ../../../home/.dmenu/STATbright1.png: %s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}
	bright_status[1] = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_FreeSurface(tmp_surface);

	tmp_surface = IMG_Load("../../../home/.dmenu/STATbright2.png");
	if (tmp_surface == NULL) {
		printf("Failed to load ../../../home/.dmenu/STATbright2.png: %s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}
	bright_status[2] = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_FreeSurface(tmp_surface);

	tmp_surface = IMG_Load("../../../home/.dmenu/STATbright3.png");
	if (tmp_surface == NULL) {
		printf("Failed to load ../../../home/.dmenu/STATbright3.png: %s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}
	bright_status[3] = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_FreeSurface(tmp_surface);

	tmp_surface = IMG_Load("../../../home/.dmenu/STATbright4.png");
	if (tmp_surface == NULL) {
		printf("Failed to load ../../../home/.dmenu/STATbright4.png: %s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}
	bright_status[4] = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_FreeSurface(tmp_surface);

	bright_set(0);

}

void bright_set(int change) {

	bright_level += change;

	if (bright_level >= 4) bright_level = 4;
	if (bright_level <= 0) bright_level = 0;

#ifdef DINGOO_BUILD
	FILE *brt_fd;
	int file_no;

	char *backlight = "/proc/jz/lcd_backlight";
	brt_fd = fopen(backlight, "w");
	if (brt_fd == NULL) {
		printf("Failed to open /proc/jz/lcd_backlight\n");
		exit(EXIT_FAILURE);
	}
	fprintf(brt_fd, "%d", bright[bright_level] );
	file_no = fileno(brt_fd);
	fsync(file_no);
	fclose(brt_fd);
#endif

	cfg_setint( cfg_value, "Bright", (long)bright_level );

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
