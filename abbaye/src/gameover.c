/* gameover.c */

#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

extern unsigned int _binary_graphics_gameover_png_start;
extern unsigned int _binary_graphics_gameover_png_end;

extern unsigned int _binary_sounds_GameOverV2N_ogg_start;
extern unsigned int _binary_sounds_GameOverV2N_ogg_end;

void gameover (SDL_Window *screen,uint *state) {

	SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE); // SDL_RENDERER_PRESENTVSYNC);
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	//SDL_RenderSetLogicalSize(renderer, 256, 192);
	SDL_SetRenderDrawColor(renderer,0,0,0,255);

	SDL_RWops *rw = SDL_RWFromMem(&_binary_graphics_gameover_png_start,
        (unsigned int)&_binary_graphics_gameover_png_end - (unsigned int)&_binary_graphics_gameover_png_start);
	SDL_Texture *gameover = IMG_LoadTexture_RW(renderer,rw,1);

    rw = SDL_RWFromMem(&_binary_sounds_GameOverV2N_ogg_start,
        (unsigned int)&_binary_sounds_GameOverV2N_ogg_end - (unsigned int)&_binary_sounds_GameOverV2N_ogg_start);
	Mix_Music *bso = Mix_LoadMUS_RW(rw,1);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer,gameover,NULL,NULL);

	/* Flip */
	SDL_RenderPresent(renderer);
	Mix_PlayMusic(bso, 0);

	/* Wait */
	sleep(12);

	/* Cleaning */
	Mix_FreeMusic (bso);
	SDL_DestroyTexture(gameover);
	SDL_DestroyRenderer(renderer);

	*state = 0;

}
