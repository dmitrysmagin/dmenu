#include "sound.h"
#include "conf.h"
#include "common.h"
#include "resource.h"

extern cfg_t *cfg;
Mix_Chunk* sound_effects[6];

void load_sound(char* file, enum MenuSound snd) 
{
    sound_effects[snd] = load_theme_sound(cfg_getstr(cfg, file));
}

void sound_init()
{ 
#if SOUND_ENABLED
    log_debug("Initializing");
    
    if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT,2,1024)<0) {
        log_error( "Can't initialize Sounds." );
    }
 
    load_sound("MenuSE",     MENU_MOVE);
    load_sound("MenuItemSE", MENUITEM_MOVE);
    load_sound("DecideSE",   DECIDE);
    load_sound("CancelSE",   CANCEL);
    load_sound("OutSE",      OUT);
    load_sound("TestSE",     GLOBAL_KEY);
#endif
}

void sound_out(enum MenuSound se)
{
#if SOUND_ENABLED
    Mix_PlayChannel( -1, sound_effects[se], 0 );
#endif
}

void sound_deinit()
{
#if SOUND_ENABLED
    log_debug("De-initializing");
    int i = 0;
    Mix_HaltChannel(-1);
    for(;i<6;i++) free_sound(sound_effects[i]);
    Mix_CloseAudio();
#endif
}
