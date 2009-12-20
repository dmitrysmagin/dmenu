#include "loading.h"
#include "common.h"
#include "resource.h"
#include "env.h"

SDL_Surface* loading_bg;
SDL_Surface* loading_status_bg;
SDL_Surface* loading_status_fg;
SDL_Surface* loading_screen;

void loading_init(SDL_Surface* screen) 
{
    loading_screen = screen;
    int pad = LOADING_BAR_PADDING*2;
    loading_bg = load_global_image("loading.png");
    loading_status_bg = create_surface(
        LOADING_BAR_WIDTH, LOADING_BAR_HEIGHT, 
        32, LOADING_BAR_BG_COLOR, 0xFF);
    loading_status_fg = create_surface(
        LOADING_BAR_WIDTH-pad, LOADING_BAR_HEIGHT-pad, 
        32, LOADING_BAR_FG_COLOR, 0xFF);
    loading_start();
}

void loading_start() 
{
    SDL_BlitSurface(loading_bg, NULL, loading_screen, NULL);
    SDL_Flip(loading_screen);
}

void loading_set_level(int level)
{
    int outerx = (SCREEN_WIDTH-LOADING_BAR_WIDTH)/2,
        outery = (SCREEN_HEIGHT-LOADING_BAR_HEIGHT)/2,
        pad = LOADING_BAR_PADDING;
    float pct = (level/100.0f);
        
    SDL_Rect dest_rect = {outerx,outery,0,0},
             src_rect  = {0,0, (int)(pct*(LOADING_BAR_WIDTH-pad*2)),LOADING_BAR_HEIGHT-pad*2};
    
    SDL_BlitSurface(loading_status_bg, NULL, loading_screen, &dest_rect);
    init_rect_pos(&dest_rect, outerx+pad, outery+pad);
    SDL_BlitSurface(loading_status_fg, &src_rect, loading_screen, &dest_rect);
    SDL_Flip(loading_screen);
}

void loading_deinit()
{
    free_surface(loading_bg);
    free_surface(loading_status_bg);
    free_surface(loading_status_fg);
}