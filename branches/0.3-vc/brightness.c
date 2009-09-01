#include "brightness.h"

int bright_level;

int bright[5]={0,25,50,75,99};

void bright_init() {

	FILE *brt_fd;
	brt_fd = fopen("/usr/local/home/.dmenu/brightness_level.ini", "r");
	fscanf(brt_fd,"%d", &bright_level);
	fclose(brt_fd);

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
