/* startscreen.c */

#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

extern unsigned int _binary_graphics_intro_png_start;
extern unsigned int _binary_graphics_intro_png_end;
extern unsigned int _binary_graphics_intromd_png_start;
extern unsigned int _binary_graphics_intromd_png_end;

extern unsigned int _binary_sounds_MainTitleN_ogg_start;
extern unsigned int _binary_sounds_MainTitleN_ogg_end;

void startscreen(SDL_Window *screen,uint *state,uint *grapset,uint *fullscreen) {

	/* Renderer (with VSync, nice !) */
	SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE); // SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_ACCELERATED);
	SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "0");
	SDL_RenderSetLogicalSize(renderer, 256, 192);

	uint exit = 0;
	uint musicplay = 0;

	SDL_Rect srcintro = {0,0,256,192};
	SDL_Rect desintro = {0,0,256,192};

	SDL_Event keyp;

	/* Loading PNG */
	SDL_RWops *rw = SDL_RWFromMem(&_binary_graphics_intro_png_start,
	    (unsigned int)&_binary_graphics_intro_png_end - (unsigned int)&_binary_graphics_intro_png_start);
	SDL_Texture *intro = IMG_LoadTexture_RW(renderer,rw,1);
    rw = SDL_RWFromMem(&_binary_graphics_intromd_png_start,
        (unsigned int)&_binary_graphics_intromd_png_end - (unsigned int)&_binary_graphics_intromd_png_start);
	SDL_Texture *intromd = IMG_LoadTexture_RW(renderer,rw,1);

	/* Load audio */
    rw = SDL_RWFromMem(&_binary_sounds_MainTitleN_ogg_start,
        (unsigned int)&_binary_sounds_MainTitleN_ogg_end - (unsigned int)&_binary_sounds_MainTitleN_ogg_start);
	Mix_Music *music = Mix_LoadMUS_RW(rw,1);

	while (exit != 1) {

		/* Cleaning the renderer */
		SDL_RenderClear(renderer);

		/* Put image on renderer */
		if (*grapset == 0)
			SDL_RenderCopy(renderer, intro, &srcintro, &desintro);
		else
			SDL_RenderCopy(renderer, intromd, &srcintro, &desintro);

		/* Flip ! */
		SDL_RenderPresent(renderer);

		/* Play music if required */
		if (musicplay == 0) {
			musicplay = 1;
			Mix_PlayMusic(music, 0);
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
				if (keyp.key.keysym.sym == SDLK_i) { /* Show instructions */
					if (srcintro.y == 0)
						srcintro.y = 192;
					else {
						srcintro.y = 0;
						musicplay = 0;
					}
				}
				if (keyp.key.keysym.sym == SDLK_SPACE) { /* Start game */
					*state = 1;
					exit = 1;
				}
			}
            if (keyp.type == SDL_CONTROLLERBUTTONDOWN) { /* Game controller button pressed */
                if (keyp.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) { /* Change graphic set */
                    if (*grapset == 0)
                        *grapset = 1;
                    else
                        *grapset = 0;
                }
                if (keyp.cbutton.button == SDL_CONTROLLER_BUTTON_A || keyp.cbutton.button == SDL_CONTROLLER_BUTTON_B) { /* Show instructions */
                    if (srcintro.y == 0)
                        srcintro.y = 192;
                    else {
                        srcintro.y = 0;
                        musicplay = 0;
                    }
                }
                if (keyp.cbutton.button == SDL_CONTROLLER_BUTTON_START) { /* Start game */
                    *state = 1;
                    exit = 1;
                }
            }
		}

	}

	/* Cleaning */
	SDL_DestroyTexture(intro);
	SDL_DestroyTexture(intromd);
	SDL_DestroyRenderer(renderer);

}
