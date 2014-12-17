/* Abbaye des Morts */
/* Version 2.0 */

/* (c) 2010 - Locomalito & Gryzor87 */
/* 2013 - David "Nevat" Lara */

/* GPL v3 license */

#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

#include "kernel.h"

extern void startscreen(SDL_Window *screen,uint *state,uint *grapset,uint *fullscreen);
extern void history(SDL_Window *screen,uint *state,uint *grapset,uint *fullscreen);
extern void game(SDL_Window *screen,uint *state,uint *grapset,uint *fullscreen);
extern void gameover (SDL_Window *screen,uint *state);
extern void ending (SDL_Window *screen,uint *state);

#if defined(__cplusplus)
extern "C" {
#endif

__attribute__ ((interrupt ("IRQ"))) void interrupt_irq() {
    SDL_Interrupt_Handler();
}

#if defined(__cplusplus)
}
#endif

void main () {

	uint state = 0; /* 0-intro,1-history,2-game */
	uint grapset = 1; /* 0-8bits, 1-16bits */
	uint fullscreen = 0; /* 0-Windowed,1-Fullscreen */

	mount("sd:");

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);

	/* Creating window */
	SDL_Window *screen = SDL_CreateWindow("Abbaye des Morts v2.0",0,0,256,192,SDL_WINDOW_FULLSCREEN);

	/* Init audio */
	Mix_OpenAudio (22050,MIX_DEFAULT_FORMAT,2,4096);
	Mix_AllocateChannels(5);

	/* Init game controllers */
    SDL_GameControllerAddMappingsFromFile("control.txt");
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            SDL_GameControllerOpen(i);
        }
    }

	while (1) {
		switch (state) {
			case 0: startscreen(screen,&state,&grapset,&fullscreen);
							break;
			case 1: history(screen,&state,&grapset,&fullscreen);
							break;
			case 2: game(screen,&state,&grapset,&fullscreen);
							break;
			case 3: gameover(screen,&state);
							break;
			case 4: ending(screen,&state);
							break;
		}
	}

}
