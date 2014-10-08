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

#include <usbd/usbd.h>
#include <device/hid/keyboard.h>

#include "platform.h"
#include "fb.h"

extern void sdl_audio_dma_irq();

extern void startscreen(SDL_Window *screen,uint *state,uint *grapset,uint *fullscreen);
extern void history(SDL_Window *screen,uint *state,uint *grapset,uint *fullscreen);
extern void game(SDL_Window *screen,uint *state,uint *grapset,uint *fullscreen);
extern void gameover (SDL_Window *screen,uint *state);
extern void ending (SDL_Window *screen,uint *state);

#if defined(__cplusplus)
extern "C" {
#endif

__attribute__ ((interrupt ("IRQ"))) void interrupt_irq() {
    if ((IRQ->irqBasicPending & INTERRUPT_ARM_TIMER) != 0)
        ;
    if ((IRQ->irq1Pending & INTERRUPT_DMA0) != 0)
        sdl_audio_dma_irq();
}

#if defined(__cplusplus)
}
#endif

void main () {

	uint state = 0; /* 0-intro,1-history,2-game */
	uint grapset = 1; /* 0-8bits, 1-16bits */
	uint fullscreen = 0; /* 0-Windowed,1-Fullscreen */

	/* Initialize framebuffer */
    fb_init(16+256+16, 16+192+16);
    fb_begin_doublebuffer();

    UsbInitialise();

	/* Creating window */
	SDL_Window *screen = SDL_CreateWindow("Abbaye des Morts v2.0",16,16,256,192,SDL_WINDOWEVENT_SHOWN);

	/* Init audio */
	Mix_OpenAudio (22050,MIX_DEFAULT_FORMAT,2,4096);
	Mix_AllocateChannels(5);

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

static unsigned char keyNormal_it[] = {
    0x0, 0x0, 0x0, 0x0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
    'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '0', '\r', 0x0, '\b', '\t', ' ', '\'', 0x0, 0x0,
    '+', '<', 0x0, 0x0, 0x0, '\\', ',', '.', '-', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+', '\n', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '0', '.', '<', 0x0, 0x0, '='
};

static unsigned char keyShift_it[] = {
    0x0, 0x0, 0x0, 0x0, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '"',
    0x0, '$', '%', '&', '/', '(', ')', '=', '\r', 0x0, '\b', '\t', ' ', '?', '^', 0x0,
    '*', '>', 0x0, 0x0, 0x0, '|', ';', ':', '_', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+', '\n', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '0', '.', '>', 0x0, 0x0, '='
};

#define MAX_KEYS        6

int poll_event(SDL_Event *keyp) {
    static int keydown[MAX_KEYS] = { 0, 0, 0, 0, 0, 0 };
    static int keydownCount = 0;
    static int keydownIndex = 0;
    static int keyupIndex = MAX_KEYS;
    static struct KeyboardModifiers modifiers;

    //SDL_zerop(keyp);

    u32 kbdCount = KeyboardCount();
    if (kbdCount == 0) {
        for (int i = 0; i < MAX_KEYS; i++) {
            keydown[i] = 0;
        }
        keydownCount = 0;
        keydownIndex = 0;
        keyupIndex = MAX_KEYS;
        return 0;
    }

    u32 kbdAddress = KeyboardGetAddress(0);
    if (kbdAddress == 0) {
        for (int i = 0; i < MAX_KEYS; i++) {
            keydown[i] = 0;
        }
        keydownCount = 0;
        keydownIndex = 0;
        keyupIndex = MAX_KEYS;
        return 0;
    }

    if (keydownIndex >= keydownCount && keyupIndex >= MAX_KEYS) {
        KeyboardPoll(kbdAddress);
        keydownCount = KeyboardGetKeyDownCount(kbdAddress);
        KeyboardGetModifiers(kbdAddress);
        keydownIndex = 0;
        keyupIndex = 0;
    }

    while (keydownIndex < keydownCount) {
        u16 key = KeyboardGetKeyDown(kbdAddress, keydownIndex++);
        for (int i = 0; i < MAX_KEYS; i++) {
            if (key == keydown[i]) {
                key = 0;
                break;
            }
        }
        if (key != 0) {
            for (int i = 0; i < MAX_KEYS; i++) {
                if (keydown[i] == 0) {
                    keydown[i] = key;
                    keyp->type = SDL_KEYDOWN;
                    keyp->key.state = SDL_PRESSED;
                    if (modifiers.LeftShift)
                        keyp->key.keysym.mod |= KMOD_LSHIFT;
                    if (modifiers.RightShift)
                        keyp->key.keysym.mod |= KMOD_RSHIFT;
                    keyp->key.keysym.scancode = key;
                    keyp->key.keysym.sym = SDL_SCANCODE_TO_KEYCODE(key);
                    if (modifiers.LeftShift || modifiers.RightShift) {
                        if (key < sizeof(keyShift_it) && keyShift_it[key] != 0) {
                            keyp->key.keysym.sym = keyShift_it[key];
                        }
                    }
                    else {
                        if (key < sizeof(keyNormal_it) && keyNormal_it[key] != 0) {
                            keyp->key.keysym.sym = keyNormal_it[key];
                        }
                    }
                    return 1;
                }
            }
        }
    }

    while (keyupIndex < MAX_KEYS) {
        u16 key = keydown[keyupIndex];
        if (key != 0) {
            if (!KeyboadGetKeyIsDown(kbdAddress, key)) {
                keyp->type = SDL_KEYUP;
                keyp->key.state = SDL_RELEASED;
                if (modifiers.LeftShift)
                    keyp->key.keysym.mod |= KMOD_LSHIFT;
                if (modifiers.RightShift)
                    keyp->key.keysym.mod |= KMOD_RSHIFT;
                keyp->key.keysym.scancode = key;
                keyp->key.keysym.sym = SDL_SCANCODE_TO_KEYCODE(key);
                if (modifiers.LeftShift || modifiers.RightShift) {
                    if (key < sizeof(keyShift_it) && keyShift_it[key] != 0) {
                        keyp->key.keysym.sym = keyShift_it[key];
                    }
                }
                else {
                    if (key < sizeof(keyNormal_it) && keyNormal_it[key] != 0) {
                        keyp->key.keysym.sym = keyNormal_it[key];
                    }
                }
                keydown[keyupIndex++] = 0;
                return 1;
            }
        }
        keyupIndex++;
    }

    return 0;
}
