#include "sound.h"
#include "conf.h"

extern cfg_t *cfg;

Mix_Chunk*	gSE[6];

void SE_Init()
{

	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT,2,512)<0) {
		printf( "Can't initialize Sounds." );
	}

	gSE[0] = Mix_LoadWAV(cfg_getstr(cfg, "MenuSE"));
	gSE[1] = Mix_LoadWAV(cfg_getstr(cfg, "MenuItemSE"));
	gSE[2] = Mix_LoadWAV(cfg_getstr(cfg, "DecideSE"));
	gSE[3] = Mix_LoadWAV(cfg_getstr(cfg, "CancelSE"));
	gSE[4] = Mix_LoadWAV(cfg_getstr(cfg, "OutSE"));
	gSE[5] = Mix_LoadWAV(cfg_getstr(cfg, "TestSE"));

}


void SE_out(int se)
{

	if ((se < 0) || (se > 5)) se = 4;
	Mix_PlayChannel( -1, gSE[se], 0 );

}

void SE_deInit()
{
	int i;
	Mix_HaltChannel( -1 );
	for( i=0; i<6; i++ ) { Mix_FreeChunk( gSE[i] ); }
	Mix_CloseAudio();
}
