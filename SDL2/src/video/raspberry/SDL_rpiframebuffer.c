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
#include "SDL_rpiframebuffer_c.h"

#include "../../../../kernel/fb.h"

#define DUMMY_SURFACE   "_SDL_DummySurface"

int SDL_RASPBERRY_CreateWindowFramebuffer(_THIS, SDL_Window * window, Uint32 * format, void ** pixels, int *pitch) {
    SDL_Surface *surface;
#if BYTES_PER_PIXEL == 2
    const Uint32 surface_format = SDL_PIXELFORMAT_RGB565;
#else
    const Uint32 surface_format = SDL_PIXELFORMAT_ABGR8888;
#endif
    int w, h;
    int bpp;
    Uint32 Rmask, Gmask, Bmask, Amask;

    /* Free the old framebuffer surface */
    surface = (SDL_Surface *) SDL_GetWindowData(window, DUMMY_SURFACE);
    SDL_FreeSurface(surface);

    /* Create a new one */
    SDL_PixelFormatEnumToMasks(surface_format, &bpp, &Rmask, &Gmask, &Bmask, &Amask);
    SDL_GetWindowSize(window, &w, &h);
    surface = SDL_CreateRGBSurface(0, w, h, bpp, Rmask, Gmask, Bmask, Amask);
    if (!surface) {
        return -1;
    }

    /* Save the info and return! */
    SDL_SetWindowData(window, DUMMY_SURFACE, surface);
    *format = surface_format;
    *pixels = surface->pixels;
    *pitch = surface->pitch;
    return 0;
}

int SDL_RASPBERRY_UpdateWindowFramebuffer(_THIS, SDL_Window * window, const SDL_Rect * rects, int numrects) {
    SDL_Surface *surface;
    unsigned char *dst, *src;

    surface = (SDL_Surface *) SDL_GetWindowData(window, DUMMY_SURFACE);
    if (!surface) {
        return SDL_SetError("Couldn't find dummy surface for window");
    }

    /* Send the data to the display */
    dst = (unsigned char *) fb_get_pixel_address(window->x, window->y);
    src = (unsigned char *) surface->pixels;
    for (int y = 0; y < window->h; y++) {
        SDL_memcpy4(dst, src, window->w);
        dst += fb_pitch;
        src += surface->pitch;
    }

    fb_flip();

    return 0;
}

void SDL_RASPBERRY_DestroyWindowFramebuffer(_THIS, SDL_Window * window) {
    SDL_Surface *surface;

    surface = (SDL_Surface *) SDL_SetWindowData(window, DUMMY_SURFACE, NULL);
    SDL_FreeSurface(surface);
}

#endif /* SDL_VIDEO_DRIVER_RASPBERRY */

/* vi: set ts=4 sw=4 expandtab: */
