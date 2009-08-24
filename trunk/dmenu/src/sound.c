#include <SDL_mixer.h>
#include "sound.h"
#include "conf.h"

extern cfg_t *cfg;

Mix_Music*	gSE[6];

void SE_Init()
{

	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT,2,1024)<0) {
		printf( "Can't initialize Sounds." );
	}

	gSE[0] = Mix_LoadMUS(cfg_getstr(cfg, "MenuSE"));
	gSE[1] = Mix_LoadMUS(cfg_getstr(cfg, "MenuItemSE"));
	gSE[2] = Mix_LoadMUS(cfg_getstr(cfg, "DecideSE"));
	gSE[3] = Mix_LoadMUS(cfg_getstr(cfg, "CancelSE"));
	gSE[4] = Mix_LoadMUS(cfg_getstr(cfg, "OutSE"));
	gSE[5] = Mix_LoadMUS(cfg_getstr(cfg, "TestSE"));

}


void SE_out(int se)
{

	if ((se < 0) || (se > 5)) se = 4;
	Mix_PlayMusic( gSE[se], 1 );

}

void SE_deInit()
{
	Mix_HaltMusic();
	Mix_FreeMusic( gSE[0] );
	Mix_FreeMusic( gSE[1] );
	Mix_FreeMusic( gSE[2] );
	Mix_FreeMusic( gSE[3] );
	Mix_FreeMusic( gSE[4] );
	Mix_FreeMusic( gSE[5] );

	Mix_CloseAudio();
}
