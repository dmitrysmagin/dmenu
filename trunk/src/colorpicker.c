#include "conf.h"
#include "env.h"
#include "common.h"
#include "colorpicker.h"
#include "resource.h"
#include "menu.h"
#include "sound.h"
#include "dingoo.h"

SDL_Surface* cp_title;
SDL_Surface* cp_title_bg;
SDL_Surface* cp_background;
SDL_Surface* cp_gradient[3];
SDL_Surface* cp_highlight_inactive;
SDL_Surface* cp_highlight_active;
SDL_Surface* cp_preview;
SDL_Surface* cp_preview_bg;
SDL_Surface* cp_demo_text;
TTF_Font*    cp_font;
SDL_Color*   cp_fontcolor;

char* colorpicker_demo_text = "Lorem ipsum dolor sit amet, consectetur adipisicing elit";

typedef struct ColorPickerGLobal {
    char executable[PATH_MAX];
    char root[PATH_MAX];
    char background[PATH_MAX];
    char title[PATH_MAX];
    char color_string[20];
    int active;
    int is_ready, state_changed, color_changed;
    int colors[4];
    
} ColorPickerGlobal;

ColorPickerGlobal cp_global;

SDL_Surface* render_colorpicker_text(char* text) {
    return draw_text(text, cp_font, cp_fontcolor);
}

SDL_Surface* colorpicker_load_gradient(int color) {
    SDL_Surface *out, *tmp = load_global_image("gradient.png");
    out = tint_surface(tmp, color, 0);
    free_surface(tmp);
    return out;
}

int  colorpicker_init(char* title, char* executable,  char* path, char* color, char* background) {
    log_debug("Initializing");

    if (title != NULL)      strcpy(cp_global.title, title);
    if (executable != NULL) strcpy(cp_global.executable, executable);
    if (color != NULL)      strcpy(cp_global.color_string, color);
    if (path != NULL)       strcpy(cp_global.root, path);
    if (background != NULL) strcpy(cp_global.background, background);

    //Read color
    char* fmt = "%3d,%3d,%3d";
    if (color[0] == '#') fmt = "#%2x%2x%2x";
    sscanf(color, fmt, 
        &cp_global.colors[0], 
        &cp_global.colors[1],
        &cp_global.colors[2]);
        
    // load font
    cp_font      = get_theme_font(14);
    cp_fontcolor = get_theme_font_color();
        
    if (strlen(cp_global.title) > 0) 
    {
        cp_title     = render_colorpicker_text(title);
        cp_title_bg = create_surface(
            SCREEN_WIDTH, SELECT_TITLE_HEIGHT, 32, 
            SELECT_TITLE_COLOR, SELECT_TITLE_ALPHA);
    }
    
    cp_global.active = 0;
    
    cp_background = create_surface(
        SCREEN_WIDTH, SCREEN_HEIGHT, 24, SELECT_BG_COLOR, 0);

    unsigned int mask = 0xFF<<24, i =0;
    for (;i<3;i++) {
        mask >>= 8;
        cp_gradient[i]  = colorpicker_load_gradient(mask);
    }

    cp_highlight_active = create_surface(3, cp_gradient[0]->h+4, 
        32, COLORPICKER_SELECT_ACTIVE);

    cp_highlight_inactive = create_surface(3, cp_gradient[0]->h+4, 
        32, COLORPICKER_SELECT_INACTIVE);
        
    cp_preview = create_surface(
        COLORPICKER_PREVIEW_W, COLORPICKER_PREVIEW_H,
        24, -1,-1,-1, 0);
        
    if (strlen(cp_global.background) > 0) {
        cp_preview_bg = load_resized_image(
            cp_global.background, COLORPICKER_PREVIEW_W, COLORPICKER_PREVIEW_H);
    } 
    
    colorpicker_update_color();

    cp_global.color_changed = 1;
    cp_global.state_changed = 1;
    
    return 0;
}

void colorpicker_deinit() {
    if (!cp_global.is_ready) return;
    log_debug("De-initializing");
    
    free_surface(cp_highlight_active);
    free_surface(cp_highlight_inactive);    
    
    int i = 0;
    for (;i<3;i++) {
        free_surface(cp_gradient[i]);
    }
    
    free_surface(cp_demo_text);
    free_surface(cp_preview_bg);
    free_surface(cp_preview);
    free_surface(cp_background);
    free_surface(cp_title);
    free_surface(cp_title_bg);
    free_color(cp_fontcolor);
    free_font(cp_font);
    
    cp_global.is_ready = 0;
    cp_global.state_changed = 0;
    cp_global.color_changed = 0;
}

int colorpicker_draw(SDL_Surface* screen) {
    if (!cp_global.state_changed) return 0;
    
    SDL_Rect dstrect, txtrect, dhtrect;
  
    // clear screen
    SDL_BlitSurface(cp_background, NULL, screen, NULL);
    
    int i = 0;
    SDL_Surface *high;
    
    init_rect_pos(&dstrect, COLORPICKER_COLOR_X, COLORPICKER_COLOR_Y);
    init_rect_pos(&dhtrect, 0, COLORPICKER_COLOR_Y - COLORPICKER_PAD/4);
    
    for (;i<3;i++) 
    {
        high = cp_highlight_inactive;
        if ( i==cp_global.active ) high = cp_highlight_active;
        dhtrect.x = COLORPICKER_COLOR_X + cp_global.colors[i];
        SDL_BlitSurface(cp_gradient[i], NULL, screen, &dstrect);
        SDL_BlitSurface(high, NULL, screen, &dhtrect);
        dstrect.y += COLORPICKER_COLOR_FULL;
        dhtrect.y += COLORPICKER_COLOR_FULL;
    }
    
    colorpicker_update_preview();
    
    init_rect(&dstrect, 
        COLORPICKER_PREVIEW_X, COLORPICKER_PREVIEW_Y, 
        COLORPICKER_PREVIEW_W, COLORPICKER_PREVIEW_H);
    SDL_BlitSurface(cp_preview, 0, screen, &dstrect);
        
    //Draw top message
    init_rect_pos(&txtrect, 0,0);
    SDL_BlitSurface(cp_title_bg, 0, screen, &txtrect);
    txtrect.x += DOSD_PADDING;
    SDL_BlitSurface(cp_title, 0, screen, &txtrect);
    
    cp_global.state_changed = 0;
    
    return 1;
}

void colorpicker_animate(SDL_Surface* screen) { }
void colorpicker_osd(SDL_Surface* screen)     { }

enum MenuState colorpicker_select()
{
    SE_out( DECIDE );
 
    sprintf(cp_global.color_string, "%3d,%3d,%3d", 
        cp_global.colors[0], cp_global.colors[1], cp_global.colors[2]);
    
    if (strlen(cp_global.executable) > 0) 
    {
        run_command(cp_global.executable, cp_global.color_string, cp_global.root);
    }
    
    colorpicker_deinit();
    
    return MAINMENU;
}

void colorpicker_change_color_set(enum Direction dir) 
{
    int delta = (dir == UP) ? -1 : 1;
    SE_out ( MENUITEM_MOVE );
    cp_global.active = wrap(cp_global.active+delta,0,2);
    cp_global.state_changed = 1;
}

void colorpicker_update_color()
{
    int* cols = cp_global.colors;

    cp_fontcolor->r = cols[0];
    cp_fontcolor->g = cols[1];
    cp_fontcolor->b = cols[2];
    cp_global.color_changed = 1;
}

void colorpicker_update_preview() 
{
    int* cols = cp_global.colors;
    
    if (!cp_global.color_changed) return;
    cp_global.color_changed = 0;
    
    if (strlen(cp_global.background) == 0) {
        SDL_FillRect(cp_preview, NULL, SDL_MapRGB(cp_preview->format,cols[0],cols[1],cols[2]));
    } else {
        SDL_BlitSurface(cp_preview_bg, NULL, cp_preview, NULL);
        free_surface(cp_demo_text);
        cp_demo_text = render_colorpicker_text(colorpicker_demo_text);
        SDL_Rect pos = {4,cp_preview->h/2-cp_demo_text->h/2,
            COLORPICKER_PREVIEW_W,COLORPICKER_PREVIEW_H};
        SDL_BlitSurface(cp_demo_text,  NULL, cp_preview, &pos);
    }
}

void colorpicker_change_color_value(enum Direction dir, int amt)
{
    int delta = (dir == PREV) ? -1 : 1, i = cp_global.active;
    cp_global.colors[i] = bound(cp_global.colors[i]+delta*amt,0,0xFF);
    colorpicker_update_color();
    cp_global.state_changed = 1;
}

enum MenuState colorpicker_keypress(SDLKey keysym) {
    switch (keysym) {
        case DINGOO_BUTTON_B:
            SE_out( CANCEL );
            colorpicker_deinit();
            return MAINMENU;
        case DINGOO_BUTTON_A:
            return colorpicker_select();
        case DINGOO_BUTTON_L:
        case DINGOO_BUTTON_R:
            colorpicker_change_color_value(DINGOO_BUTTON_L==keysym?PREV:NEXT,1);
            break;
        case DINGOO_BUTTON_RIGHT:
        case DINGOO_BUTTON_LEFT:
            colorpicker_change_color_value(DINGOO_BUTTON_LEFT==keysym?PREV:NEXT,4);
            break;
        case DINGOO_BUTTON_UP:
        case DINGOO_BUTTON_DOWN:
            colorpicker_change_color_set(DINGOO_BUTTON_UP==keysym?UP:DOWN);
            break;            
        default: break;
    }
    
    return COLORPICKER;
}