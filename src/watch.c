#include "watch.h"
#include "resource.h"

extern cfg_t *cfg_main;

SDL_Rect watch_dst_rect1, watch_dst_rect2;
TTF_Font* watch_font;
SDL_Color* watch_font_color;

time_t watch;
struct tm *t_st;

int watch_enabled()
{
    return cfg_getbool(cfg_main, "WatchDisp");
}

void watch_init() 
{    
    log_debug("Initializing");
    
    //Initialize OSD positions
    init_rect_pos( &watch_dst_rect1, WATCH_DISP_X, WATCH_DISP_Y);
    init_rect_pos( &watch_dst_rect2, WATCH_DISP_X+1, WATCH_DISP_Y);
	watch_font = get_theme_font(WATCH_FONT_SIZE);
	watch_font_color =get_theme_font_color();
}

void watch_color_reset() {
	watch_font_color = get_theme_font_color();
}

void watch_show(SDL_Surface* surface) 
{
	char watch_text[20];
	SDL_Surface* watch_display;

	time(&watch);
	t_st = localtime(&watch);
	if (t_st->tm_year < 1900 ) t_st->tm_year += 1900;

	sprintf( watch_text, "%04d/%02d/%02d %02d:%02d:%02d",
			t_st->tm_year,
			t_st->tm_mon +1,
			t_st->tm_mday,
			t_st->tm_hour,
			t_st->tm_min,
			t_st->tm_sec);

	watch_display = draw_text(watch_text, watch_font, watch_font_color);

    SDL_BlitSurface(watch_display, 0, surface, &watch_dst_rect1 );
    SDL_BlitSurface(watch_display, 0, surface, &watch_dst_rect2 );

    free_surface(watch_display);
}

void watch_deinit() 
{
    log_debug("De-initializing");

    free_font(watch_font);
    free_color(watch_font_color);
}
