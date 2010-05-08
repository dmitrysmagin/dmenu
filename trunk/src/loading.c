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
    loading_bg = load_global_image("loading.png");
    loading_start();
}

void loading_start() 
{
    SDL_BlitSurface(loading_bg, NULL, loading_screen, NULL);
    SDL_Flip(loading_screen);
}

void loading_deinit()
{
    free_surface(loading_bg);
    free_surface(loading_status_bg);
    free_surface(loading_status_fg);
}
