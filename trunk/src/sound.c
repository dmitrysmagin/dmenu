#include "sound.h"
#include "conf.h"
#include "common.h"
#include "resource.h"

extern cfg_t *cfg;
Mix_Music* gSE[6];

void load_sound(char* file, enum MenuSound snd) 
{
    gSE[snd] = load_theme_sound(cfg_getstr(cfg, file));
}

void SE_Init()
{
    log_debug("Initializing");
    
    if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT,2,1024)<0) {
        log_error( "Can't initialize Sounds." );
    }
 
    load_sound("MenuSE",     MENU_MOVE);
    load_sound("MenuItemSE", MENUITEM_MOVE);
    load_sound("DecideSE",   DECIDE);
    load_sound("CancelSE",   CANCEL);
    load_sound("OutSE",      OUT);
    load_sound("TestSE",     TEST);
}

void SE_out(enum MenuSound se)
{
    Mix_PlayMusic( gSE[se], 1 );
}

void SE_deInit()
{
    log_debug("De-initializing");
    int i = 0;
    Mix_HaltMusic();
    for(;i<6;i++){
         Mix_FreeMusic(gSE[i]);
         gSE[i] = NULL;
    }
    Mix_CloseAudio();
}
