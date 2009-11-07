#include <limits.h>

//Currently the hardcoded defaults are intended for Dingoo A320
#ifndef DMENU_PATH
#define DMENU_PATH           "/usr/local/dmenu/"
#endif 

#define DMENU_THEMES         DMENU_PATH "themes/"
#define DMENU_CONF_FILE      DMENU_PATH "dmenu.ini"
#define DMENU_COMMAND_FILE   DMENU_PATH ".next"
#define DMENU_BACKGROUNDS    DMENU_PATH "wallpapers/"
#define GLOBAL_RESOURCE_PATH DMENU_PATH "resources/"
char THEME_PATH[PATH_MAX];

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

#define DOSD_COLOR           0xffffff
#define DOSD_COLOR_RGB       255,255,255
#define DOSD_COLOR_RGBA      255,255,255,255
#define DOSD_UPDATE_INTERVAL 500
#define DOSD_PADDING         4
#define DOSD_BATTERY_WIDTH   19
#define DOSD_LOCK_WIDTH      7
#define DOSD_HEIGHT          9
#define DOSD_START_X         (SCREEN_WIDTH-DOSD_PADDING*2-DOSD_BATTERY_WIDTH-DOSD_LOCK_WIDTH)

//Volume Positioning
#define VOLUME_TEXT_W 33
#define VOLUME_TEXT_H DOSD_VOLUME_HEIGHT
#define VOLUME_TEXT_X (DOSD_START_X - VOLUME_TEXT_W - DOSD_PADDING) 
#define VOLUME_TEXT_Y -1

#define VOLUME_ICON_W 9
#define VOLUME_ICON_H DOSD_HEIGHT
#define VOLUME_ICON_Y DOSD_PADDING
#define VOLUME_ICON_X (VOLUME_TEXT_X - VOLUME_ICON_W - DOSD_PADDING)

//Brightness positioning
#define BRIGHTNESS_ICON_W 9
#define BRIGHTNESS_ICON_H DOSD_HEIGHT
#define BRIGHTNESS_ICON_X (VOLUME_ICON_X - BRIGHTNESS_ICON_W - DOSD_PADDING)
#define BRIGHTNESS_ICON_Y DOSD_PADDING

// the system ARG_MAX might be too large. just use a fixed
// value here.
#define MAX_CMD_LEN   4096
#define TICK_INTERVAL 100 // this is 1000/100 = 10 fps
