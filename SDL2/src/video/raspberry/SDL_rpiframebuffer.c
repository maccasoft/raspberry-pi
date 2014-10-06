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

int SDL_RASPBERRY_CreateWindowFramebuffer(_THIS, SDL_Window * window, Uint32 * format, void ** pixels, int *pitch) {

    *format = SDL_PIXELFORMAT_ABGR8888;
    *pixels = fb_get_pixel_address(window->x, window->y);;
    *pitch = fb_pitch;

    return 0;
}

int SDL_RASPBERRY_UpdateWindowFramebuffer(_THIS, SDL_Window * window, const SDL_Rect * rects, int numrects) {
    SDL_Surface *surface;

    surface = (SDL_Surface *) SDL_GetWindowSurface(window);
    if (!surface) {
        return SDL_SetError("Couldn't find dummy surface for window");
    }

    fb_flip();

    surface->pixels = fb_get_pixel_address(window->x, window->y);

    return 0;
}

void SDL_RASPBERRY_DestroyWindowFramebuffer(_THIS, SDL_Window * window) {

}

#endif /* SDL_VIDEO_DRIVER_RASPBERRY */

/* vi: set ts=4 sw=4 expandtab: */
