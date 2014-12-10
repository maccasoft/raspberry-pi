/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_RASPBERRY

#include "../SDL_sysvideo.h"
#include "SDL_rpivideo.h"
#include "SDL_rpiframebuffer_c.h"

#include "../../core/raspberry/SDL_raspberry.h"

static unsigned int fb_width;
static unsigned int fb_height;
static unsigned int fb_addr;
static unsigned int fb_size;
static unsigned int fb_pitch;

static unsigned int fb_buffer_addr[2];
static unsigned int fb_buffer;

int RASPBERRY_CreateWindowFramebuffer(_THIS, SDL_Window * window, Uint32 * format, void ** pixels, int *pitch) {
    unsigned int mb_addr = 0x40007000;      // 0x7000 in L2 cache coherent mode
    volatile unsigned int *mailbuffer = (unsigned int *) mb_addr;
    SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);

    /* Now set the physical and virtual sizes and bit depth and allocate the framebuffer */
    mailbuffer[0] = 22 * 4;
    mailbuffer[1] = 0;

    mailbuffer[2] = TAG_SET_PHYS_WH;
    mailbuffer[3] = 8;
    mailbuffer[4] = 8;
    mailbuffer[5] = window->w;
    mailbuffer[6] = window->h;

    mailbuffer[7] = TAG_SET_VIRT_WH;
    mailbuffer[8] = 8;
    mailbuffer[9] = 8;
    mailbuffer[10] = window->w;
    mailbuffer[11] = window->h * 2;

    mailbuffer[12] = TAG_SET_DEPTH;
    mailbuffer[13] = 4;
    mailbuffer[14] = 4;
    mailbuffer[15] = SDL_BITSPERPIXEL(display->current_mode.format);

    mailbuffer[16] = TAG_ALLOCATE_BUFFER;
    mailbuffer[17] = 8;  // response size = 8
    mailbuffer[18] = 4;  // request size = 4
    mailbuffer[19] = 16; // requested alignment of buffer, space for returned address
    mailbuffer[20] = 0;  // space for returned size

    mailbuffer[21] = 0;  // terminating tag
    Raspberry_MailboxWrite(MAIL_TAGS, mb_addr);

    Raspberry_MailboxRead(MAIL_TAGS);

    /* Check the allocate_buffer response */
    if (mailbuffer[1] != MAIL_FULL || mailbuffer[18] != (MAIL_FULL | 8)) {
        return SDL_SetError("Can't allocate framebuffer");
    }
    if (mailbuffer[5] != window->w || mailbuffer[6] != window->h) {
        return SDL_SetError("Resolution not supported");
    }

    fb_width = window->w;
    fb_height = window->h;

    fb_addr = mailbuffer[19];
    fb_size = mailbuffer[20];

    /* Get the pitch of the display */
    mailbuffer[0] = 7 * 4;
    mailbuffer[1] = 0;
    mailbuffer[2] = TAG_GET_PITCH;
    mailbuffer[3] = 4;
    mailbuffer[4] = 0;
    mailbuffer[5] = 0;
    mailbuffer[6] = 0;
    Raspberry_MailboxWrite(MAIL_TAGS, mb_addr);

    Raspberry_MailboxRead(MAIL_TAGS);
    if (mailbuffer[1] != MAIL_FULL || mailbuffer[4] != (MAIL_FULL | 4)) {
        return SDL_SetError("Can't get pitch");
    }

    fb_pitch = mailbuffer[5];

    fb_buffer_addr[0] = fb_addr;
    fb_buffer_addr[1] = fb_addr + fb_height * fb_pitch;

    fb_buffer = 0;
    fb_addr = fb_buffer == 0 ? fb_buffer_addr[1] : fb_buffer_addr[0];

    *format = display->current_mode.format;
    *pixels = (void *) fb_addr;
    *pitch = fb_pitch;

    return 0;
}

int RASPBERRY_UpdateWindowFramebuffer(_THIS, SDL_Window * window, const SDL_Rect * rects, int numrects) {
    unsigned int mb_addr = 0x40007000;      // 0x7000 in L2 cache coherent mode
    volatile unsigned int *mailbuffer = (unsigned int *) mb_addr;

    SDL_Surface *surface = (SDL_Surface *) SDL_GetWindowSurface(window);
    if (!surface) {
        return SDL_SetError("Couldn't find surface for window");
    }

    flush_cache();

    if (fb_addr != fb_buffer_addr[fb_buffer]) {
        fb_buffer = fb_buffer == 0 ? 1 : 0;
        fb_addr = fb_buffer == 0 ? fb_buffer_addr[1] : fb_buffer_addr[0];

        mailbuffer[0] = 12 * 4;
        mailbuffer[1] = 0;

        mailbuffer[2] = TAG_SET_VSYNC;
        mailbuffer[3] = 4;
        mailbuffer[4] = 4;
        mailbuffer[5] = 0;

        mailbuffer[6] = TAG_SET_VIRT_OFFSET;
        mailbuffer[7] = 8;
        mailbuffer[8] = 8;
        mailbuffer[9] = 0;
        mailbuffer[10] = fb_buffer * fb_height;

        mailbuffer[11] = 0;
        Raspberry_MailboxWrite(MAIL_TAGS, mb_addr);

        Raspberry_MailboxRead(MAIL_TAGS);

        surface->pixels = (void *) fb_addr;
    }

    return 0;
}

void RASPBERRY_DestroyWindowFramebuffer(_THIS, SDL_Window * window) {

}

#endif /* SDL_VIDEO_DRIVER_RASPBERRY */

/* vi: set ts=4 sw=4 expandtab: */
