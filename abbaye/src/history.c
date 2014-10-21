/* history.c */

# include <stdio.h>
# include <stdlib.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

extern unsigned int _binary_graphics_tiles_png_start;
extern unsigned int _binary_graphics_tiles_png_end;
extern unsigned int _binary_graphics_history_png_start;
extern unsigned int _binary_graphics_history_png_end;

extern unsigned int _binary_sounds_ManhuntN_ogg_start;
extern unsigned int _binary_sounds_ManhuntN_ogg_end;

extern void blit(SDL_Surface *des, SDL_Surface *src, SDL_Rect *srcrect, SDL_Rect *desrect);

void history(SDL_Window *screen,uint *state,uint *grapset,uint *fullscreen) {

	/* Renderer */
	SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE); // SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_ACCELERATED);
	//SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "0");
	//SDL_RenderSetLogicalSize(renderer, 256, 192);
	SDL_Surface * screen_surface = SDL_GetWindowSurface(screen);

	SDL_Event keyp;

	/* Load audio */
	SDL_RWops *rw = SDL_RWFromMem(&_binary_sounds_ManhuntN_ogg_start,
        (unsigned int)&_binary_sounds_ManhuntN_ogg_end - (unsigned int)&_binary_sounds_ManhuntN_ogg_start);
    Mix_Music *music = Mix_LoadMUS_RW(rw,1);

	/* Loading PNG */
    rw = SDL_RWFromMem(&_binary_graphics_tiles_png_start,
        (unsigned int)&_binary_graphics_tiles_png_end - (unsigned int)&_binary_graphics_tiles_png_start);
	SDL_Surface *tiles = IMG_Load_RW(rw,1);
    rw = SDL_RWFromMem(&_binary_graphics_history_png_start,
        (unsigned int)&_binary_graphics_history_png_end - (unsigned int)&_binary_graphics_history_png_start);
	SDL_Surface *text = IMG_Load_RW(rw,1);

	SDL_Rect srcjean = {384,88,16,24};
	SDL_Rect desjean = {0,100,16,24};
	SDL_Rect srcenem = {96,64,16,24};
	SDL_Rect desenem = {0,100,16,24};

	uint exit = 0;
	float posjean = -16;
	float posenem[4] = {-17,-17,-17,-17};
	uint animation = 0;
	uint i = 0;
	uint musicload = 0;

	while (exit != 1) {

		/* Cleaning the renderer */
		SDL_RenderClear(renderer);

		/* Play music at start */
		if (musicload == 0) {
			musicload = 1;
			Mix_PlayMusic(music, 0);
		}

		/* Show text */
		blit(screen_surface,text,NULL,NULL);

		/* Animation control */
		if (animation < 13)
			animation ++;
		else
			animation = 0;

		/* Jean running */
		if (posjean < 257) {
			posjean += 0.75;
			desjean.x = posjean;
			srcjean.x = 384 + ((animation / 7) * 16); /* Walking animation */
			srcjean.y = 88 + (*grapset * 120); /* 8 or 16 bits sprite */
			blit(screen_surface,tiles,&srcjean,&desjean);
		}

		/* Crusaders running */
		/* When start running */
		for (i=0;i<4;i++) {
			if (posjean > (35 + (30 * i)))
				posenem[i] += 0.65;
		}
		/* Draw */
		for (i=0;i<4;i++) {
			if ((posenem[i] > -17) && (posenem[i] < 257)) {
				desenem.x = posenem[i];
				srcenem.x = 96 + ((animation / 7) * 16);
				srcenem.y = 64 + (*grapset * 120);
				blit(screen_surface,tiles,&srcenem,&desenem);
			}
		}

		/* Check keyboard */
		if ( SDL_PollEvent(&keyp) ) {
			if (keyp.type == SDL_KEYDOWN) { /* Key pressed */
				if (keyp.key.keysym.sym == SDLK_c) { /* Change graphic set */
					if (*grapset == 0)
						*grapset = 1;
					else
						*grapset = 0;
				}
				if (keyp.key.keysym.sym == SDLK_SPACE) { /* Start game */
					*state = 2;
					exit = 1;
				}
			}
            else if (keyp.type == SDL_CONTROLLERBUTTONDOWN) { /* Game controller button pressed */
                if (keyp.cbutton.button == SDL_CONTROLLER_BUTTON_GUIDE) { /* Change graphic set */
                    if (*grapset == 0)
                        *grapset = 1;
                    else
                        *grapset = 0;
                }
                if (keyp.cbutton.button == SDL_CONTROLLER_BUTTON_START) { /* Start game */
                    *state = 2;
                    exit = 1;
                }
            }
		}

		if (posenem[3] > 256) { /* Ending history */
			exit = 1;
			*state = 2;
		}

		/* Flip ! */
		SDL_RenderPresent(renderer);

	}

	/* Cleaning */
	SDL_FreeSurface(tiles);
	SDL_FreeSurface(text);
	SDL_DestroyRenderer(renderer);
	Mix_FreeMusic(music);

}
