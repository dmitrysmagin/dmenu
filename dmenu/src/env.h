#include <limits.h>

//Currently the hardcoded defaults are intended for Dingoo A320
#ifndef DMENU_PATH
#define DMENU_PATH           "/usr/local/dmenu/"
#endif 
#define DMENU_THEMES         DMENU_PATH "themes/"
#define DMENU_CONF_FILE      DMENU_PATH "main.cfg"
#define DMENU_COMMAND_FILE   DMENU_PATH ".next"
char THEME_PATH[PATH_MAX];

/*
#ifndef HOME_PATH
#define HOME_PATH            "/usr/local/home/"
#endif
*/

#define USER_PATH            DMENU_PATH "ini/"
#define USER_CONF_FILE       USER_PATH "dmenu.ini"
#define USER_BACKGROUNDS     DMENU_PATH "wallpapers/"

//Device Info
#define BACKLIGHT_DEVICE     "/proc/jz/lcd_backlight"
#define MIXER_DEVICE         "/dev/mixer"
#define BATTERY_DEVICE       "/proc/jz/battery"
#define CHARGE_STATUS_DEVICE "/proc/jz/gpio1_pxpin"
#define LOCK_STATUS_DEVICE   "/proc/jz/gpio3_pxpin"

#define SCREEN_WIDTH         320
#define SCREEN_HEIGHT        240
#define SCREEN_COLOR_DEPTH   16
#define SCREEN_BPP           (SCREEN_COLOR_DEPTH>>3)

//Filelist options
#define FILE_LIST_OFFSET     24
#define FILE_ENTRY_HEIGHT    27
#define FILES_PER_PAGE       ((SCREEN_HEIGHT - FILE_LIST_OFFSET)/FILE_ENTRY_HEIGHT)

//Imageview options
#define IMAGE_THUMB_RATIO_OUTER .25
#define IMAGE_THUMB_RATIO_INNER .225
#define IMAGE_PREVIEW_RATIO     .7
#define IMAGE_THUMBS_PER_PAGE   (1/IMAGE_THUMB_RATIO_OUTER)
#define IMAGE_THUMB_HEIGHT      (IMAGE_THUMB_RATIO_OUTER*SCREEN_HEIGHT)
#define IMAGE_THUMB_WIDTH       (IMAGE_THUMB_RATIO_OUTER*SCREEN_WIDTH)
#define IMAGE_THUMB_PAD_X       ((SCREEN_WIDTH*IMAGE_THUMB_RATIO_OUTER-SCREEN_WIDTH*IMAGE_THUMB_RATIO_INNER)/2.0)
#define IMAGE_THUMB_PAD_Y       ((SCREEN_HEIGHT*IMAGE_THUMB_RATIO_OUTER-SCREEN_HEIGHT*IMAGE_THUMB_RATIO_INNER)/2.0)
#define IMAGE_THUMB_TOP         (SCREEN_HEIGHT-IMAGE_THUMB_HEIGHT-2)

#define OSD_COLOR 0xffffff
#define OSD_COLOR_RGB 255,255,255
#define OSD_COLOR_RGBA 255,255,255,255


//Volume Positioning
#define VOLUME_ICON_X 253
#define VOLUME_ICON_Y 3
#define VOLUME_ICON_W 9
#define VOLUME_ICON_H 9

#define VOLUME_TEXT_X 258
#define VOLUME_TEXT_Y -1

//Brightness positioning
#define BRIGHTNESS_ICON_X 238
#define BRIGHTNESS_ICON_Y 3
#define BRIGHTNESS_ICON_W 9
#define BRIGHTNESS_ICON_H 9

// the system ARG_MAX might be too large. just use a fixed
// value here.
#define MAX_CMD_LEN   4096
#define TICK_INTERVAL 100 // this is 1000/100 = 10 fps
