/* loading.c */

#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_mixer.h"

extern unsigned int _binary_data_map_txt_start;
extern unsigned int _binary_data_map_txt_end;
extern unsigned int _binary_data_enemies_txt_start;
extern unsigned int _binary_data_enemies_txt_end;

static char * SDL_fgets_RW(char * buf, int n, SDL_RWops *rw) {
    char *s = buf, b;
    int c = 0;

    while (--n > 0) {
        if ((*rw->read)(rw, &b, 1, 1) == 0) {
            c = EOF;
            break;
        }
        *s++ = b;
        if (b == '\n')
            break;
    }

    if (c == EOF && s == buf)
        return NULL;

    *s = '\0';

    return buf;
}

void loaddata(uint stagedata[][22][32],int enemydata[][7][15]) {

    SDL_RWops *datafile;
	int i = 0;
	int j = 0;
	int k = 0;
	char line[129],temp[4],line2[61];
	temp[3] = 0;

	/* Loading file */
	datafile = SDL_RWFromMem(&_binary_data_map_txt_start,
        (unsigned int)&_binary_data_map_txt_end - (unsigned int)&_binary_data_map_txt_start);
	SDL_fgets_RW (line, 129, datafile);
	SDL_fgets_RW (line, 129, datafile);

	/* Cargamos los datos del fichero en el array */
	for (i=0; i<=24; i++) {
		for (j=0; j<=21; j++) {
			for (k=0; k<=31; k++) {
				temp[0] = line[k*4];
				temp[1] = line[(k*4) + 1];
				temp[2] = line[(k*4) + 2];
				sscanf (temp, "%d", &stagedata[i][j][k]);
			}
			SDL_fgets_RW (line, 129, datafile);
		}
		SDL_fgets_RW (line, 129, datafile);
	}

	/* Cerramos fichero */
	//fclose (datafile);

    datafile = SDL_RWFromMem(&_binary_data_enemies_txt_start,
        (unsigned int)&_binary_data_enemies_txt_end - (unsigned int)&_binary_data_enemies_txt_start);
    SDL_fgets_RW (line2, 61, datafile);
    SDL_fgets_RW (line2, 61, datafile);

	/* Cargamos los datos del fichero en el array */
	for (i=0; i<=24; i++) {
		for (j=0; j<7; j++) {
			for (k=0; k<15; k++) {
				temp[0] = line2[k*4];
				temp[1] = line2[(k*4) + 1];
				temp[2] = line2[(k*4) + 2];
				sscanf (temp, "%d", &enemydata[i][j][k]);
			}
			SDL_fgets_RW (line2, 61, datafile);
		}
		SDL_fgets_RW (line2, 61, datafile);
	}

	//fclose (datafile);

}

extern unsigned int _binary_sounds_PrayerofHopeN_ogg_start;
extern unsigned int _binary_sounds_PrayerofHopeN_ogg_end;
extern unsigned int _binary_sounds_AreaIChurchN_ogg_start;
extern unsigned int _binary_sounds_AreaIChurchN_ogg_end;
extern unsigned int _binary_sounds_GameOverV2N_ogg_start;
extern unsigned int _binary_sounds_GameOverV2N_ogg_end;
extern unsigned int _binary_sounds_HangmansTree_ogg_start;
extern unsigned int _binary_sounds_HangmansTree_ogg_end;
extern unsigned int _binary_sounds_AreaIICavesV2N_ogg_start;
extern unsigned int _binary_sounds_AreaIICavesV2N_ogg_end;
extern unsigned int _binary_sounds_EvilFightN_ogg_start;
extern unsigned int _binary_sounds_EvilFightN_ogg_end;
extern unsigned int _binary_sounds_AreaIIIHellN_ogg_start;
extern unsigned int _binary_sounds_AreaIIIHellN_ogg_end;
extern unsigned int _binary_sounds_ManhuntwoodN_ogg_start;
extern unsigned int _binary_sounds_ManhuntwoodN_ogg_end;

extern unsigned int _binary_sounds_shoot_ogg_start;
extern unsigned int _binary_sounds_shoot_ogg_end;
extern unsigned int _binary_sounds_doorfx_ogg_start;
extern unsigned int _binary_sounds_doorfx_ogg_end;
extern unsigned int _binary_sounds_Item_ogg_start;
extern unsigned int _binary_sounds_Item_ogg_end;
extern unsigned int _binary_sounds_jump_ogg_start;
extern unsigned int _binary_sounds_jump_ogg_end;
extern unsigned int _binary_sounds_slash_ogg_start;
extern unsigned int _binary_sounds_slash_ogg_end;
extern unsigned int _binary_sounds_mechanismn_ogg_start;
extern unsigned int _binary_sounds_mechanismn_ogg_end;
extern unsigned int _binary_sounds_onedeathn_ogg_start;
extern unsigned int _binary_sounds_onedeathn_ogg_end;

void loadingmusic(Mix_Music *bso[],Mix_Chunk *fx[]) {

	/* Musics */
    SDL_RWops *rw = SDL_RWFromMem(&_binary_sounds_PrayerofHopeN_ogg_start,
        (unsigned int)&_binary_sounds_PrayerofHopeN_ogg_end - (unsigned int)&_binary_sounds_PrayerofHopeN_ogg_start);
	bso[0] = Mix_LoadMUS_RW(rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_AreaIChurchN_ogg_start,
        (unsigned int)&_binary_sounds_AreaIChurchN_ogg_end - (unsigned int)&_binary_sounds_AreaIChurchN_ogg_start);
	bso[1] = Mix_LoadMUS_RW(rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_GameOverV2N_ogg_start,
        (unsigned int)&_binary_sounds_GameOverV2N_ogg_end - (unsigned int)&_binary_sounds_GameOverV2N_ogg_start);
	bso[2] = Mix_LoadMUS_RW(rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_HangmansTree_ogg_start,
        (unsigned int)&_binary_sounds_HangmansTree_ogg_end - (unsigned int)&_binary_sounds_HangmansTree_ogg_start);
	bso[3] = Mix_LoadMUS_RW(rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_AreaIICavesV2N_ogg_start,
        (unsigned int)&_binary_sounds_AreaIICavesV2N_ogg_end - (unsigned int)&_binary_sounds_AreaIICavesV2N_ogg_start);
	bso[4] = Mix_LoadMUS_RW(rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_EvilFightN_ogg_start,
        (unsigned int)&_binary_sounds_EvilFightN_ogg_end - (unsigned int)&_binary_sounds_EvilFightN_ogg_start);
	bso[5] = Mix_LoadMUS_RW(rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_AreaIIIHellN_ogg_start,
        (unsigned int)&_binary_sounds_AreaIIIHellN_ogg_end - (unsigned int)&_binary_sounds_AreaIIIHellN_ogg_start);
	bso[6] = Mix_LoadMUS_RW(rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_ManhuntwoodN_ogg_start,
        (unsigned int)&_binary_sounds_ManhuntwoodN_ogg_end - (unsigned int)&_binary_sounds_ManhuntwoodN_ogg_start);
	bso[7] = Mix_LoadMUS_RW(rw,1);

	/* Fxs */
    rw = SDL_RWFromMem(&_binary_sounds_shoot_ogg_start,
        (unsigned int)&_binary_sounds_shoot_ogg_end - (unsigned int)&_binary_sounds_shoot_ogg_start);
	fx[0] = Mix_LoadWAV_RW (rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_doorfx_ogg_start,
        (unsigned int)&_binary_sounds_doorfx_ogg_end - (unsigned int)&_binary_sounds_doorfx_ogg_start);
	fx[1] = Mix_LoadWAV_RW (rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_Item_ogg_start,
        (unsigned int)&_binary_sounds_Item_ogg_end - (unsigned int)&_binary_sounds_Item_ogg_start);
	fx[2] = Mix_LoadWAV_RW (rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_jump_ogg_start,
        (unsigned int)&_binary_sounds_jump_ogg_end - (unsigned int)&_binary_sounds_jump_ogg_start);
	fx[3] = Mix_LoadWAV_RW (rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_slash_ogg_start,
        (unsigned int)&_binary_sounds_slash_ogg_end - (unsigned int)&_binary_sounds_slash_ogg_start);
	fx[4] = Mix_LoadWAV_RW (rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_mechanismn_ogg_start,
        (unsigned int)&_binary_sounds_mechanismn_ogg_end - (unsigned int)&_binary_sounds_mechanismn_ogg_start);
	fx[5] = Mix_LoadWAV_RW (rw,1);
    rw = SDL_RWFromMem(&_binary_sounds_onedeathn_ogg_start,
        (unsigned int)&_binary_sounds_onedeathn_ogg_end - (unsigned int)&_binary_sounds_onedeathn_ogg_start);
	fx[6] = Mix_LoadWAV_RW (rw,1);

}
