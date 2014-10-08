/* ending.c */

#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

extern unsigned int _binary_graphics_tiles_png_start;
extern unsigned int _binary_graphics_tiles_png_end;
extern unsigned int _binary_graphics_ending_png_start;
extern unsigned int _binary_graphics_ending_png_end;

extern unsigned int _binary_sounds_PrayerofHopeN_ogg_start;
extern unsigned int _binary_sounds_PrayerofHopeN_ogg_end;

void ending (SDL_Window *screen,uint *state) {

	/* Creating renderer */
	SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE); // SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawColor(renderer,0,0,0,255);

    SDL_RWops *rw = SDL_RWFromMem(&_binary_graphics_tiles_png_start,
        (unsigned int)&_binary_graphics_tiles_png_end - (unsigned int)&_binary_graphics_tiles_png_start);
	SDL_Texture *tiles = IMG_LoadTexture_RW(renderer,rw,1);
    rw = SDL_RWFromMem(&_binary_graphics_ending_png_start,
        (unsigned int)&_binary_graphics_ending_png_end - (unsigned int)&_binary_graphics_ending_png_start);
	SDL_Texture *text = IMG_LoadTexture_RW(renderer,rw,1);

	SDL_Rect srcdoor = {600,72,64,48};
	SDL_Rect desdoor = {96,72,64,48};

    rw = SDL_RWFromMem(&_binary_sounds_PrayerofHopeN_ogg_start,
        (unsigned int)&_binary_sounds_PrayerofHopeN_ogg_end - (unsigned int)&_binary_sounds_PrayerofHopeN_ogg_start);
	Mix_Music *bso = Mix_LoadMUS_RW(rw,1);

	int i = 0;
	int x = 0;
	int height = 0;
	int width = 0;
	char message[25];

	Mix_PlayMusic (bso,0);

	for (i=0;i<951;i++) {

		/* Cleaning the renderer */
		SDL_RenderClear(renderer);

		if (i<360)
			x = i/60;
		else
			x = 5;

		if (i > 365)
			SDL_RenderCopy(renderer,text,NULL,NULL);

		srcdoor.x = 600 + (64 * x);
		SDL_RenderCopy(renderer,tiles,&srcdoor,&desdoor);

		/* Flip */
		SDL_RenderPresent(renderer);

	}

	/* Cleaning */
	SDL_DestroyTexture (tiles);
	SDL_DestroyTexture(text);
	Mix_FreeMusic (bso);
	SDL_DestroyRenderer (renderer);

	*state = 0;

}
