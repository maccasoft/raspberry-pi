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

/* Dummy SDL video driver implementation; this is just enough to make an
 *  SDL-based application THINK it's got a working video driver, for
 *  applications that call SDL_Init(SDL_INIT_VIDEO) when they don't need it,
 *  and also for use as a collection of stubs when porting SDL to a new
 *  platform for which you haven't yet written a valid video driver.
 *
 * This is also a great way to determine bottlenecks: if you think that SDL
 *  is a performance problem for a given platform, enable this driver, and
 *  then see if your application runs faster without video overhead.
 *
 * Initial work by Ryan C. Gordon (icculus@icculus.org). A good portion
 *  of this was cut-and-pasted from Stephane Peter's work in the AAlib
 *  SDL video driver.  Renamed to "DUMMY" by Sam Lantinga.
 */

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_rpivideo.h"
#include "SDL_rpievents_c.h"
#include "SDL_rpiframebuffer_c.h"

#include "../../../../kernel/fb.h"

#define RASPBERRYVID_DRIVER_NAME "raspberry"

extern uint32_t fb_width;
extern uint32_t fb_height;

/* Initialization/Query functions */
static int RASPBERRY_VideoInit(_THIS);
static int RASPBERRY_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
static void RASPBERRY_VideoQuit(_THIS);
static int SDL_RASPBERRY_GetDisplayBounds (_THIS, SDL_VideoDisplay * display, SDL_Rect * rect);

/* DUMMY driver bootstrap functions */

static int
RASPBERRY_Available(void)
{
    return (1);
}

static void
RASPBERRY_DeleteDevice(SDL_VideoDevice * device)
{
    SDL_free(device);
}

static SDL_VideoDevice *
RASPBERRY_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        SDL_OutOfMemory();
        SDL_free(device);
        return (0);
    }

    /* Set the function pointers */
    device->VideoInit = RASPBERRY_VideoInit;
    device->VideoQuit = RASPBERRY_VideoQuit;
    device->SetDisplayMode = RASPBERRY_SetDisplayMode;
    device->PumpEvents = RASPBERRY_PumpEvents;
    device->CreateWindowFramebuffer = SDL_RASPBERRY_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = SDL_RASPBERRY_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = SDL_RASPBERRY_DestroyWindowFramebuffer;
    device->GetDisplayBounds = SDL_RASPBERRY_GetDisplayBounds;

    device->free = RASPBERRY_DeleteDevice;

    return device;
}

VideoBootStrap RASPBERRY_bootstrap = {
    RASPBERRYVID_DRIVER_NAME, "SDL Raspberry Pi video driver",
    RASPBERRY_Available, RASPBERRY_CreateDevice
};


int
RASPBERRY_VideoInit(_THIS)
{
    SDL_DisplayMode mode;

#if BYTES_PER_PIXEL == 2
    mode.format = SDL_PIXELFORMAT_RGB565;
#else
    mode.format = SDL_PIXELFORMAT_ABGR8888;
#endif
    mode.w = fb_width;
    mode.h = fb_height;
    mode.refresh_rate = 0;
    mode.driverdata = NULL;
    if (SDL_AddBasicVideoDisplay(&mode) < 0) {
        return -1;
    }

    SDL_zero(mode);
    SDL_AddDisplayMode(&_this->displays[0], &mode);

    /* We're done! */
    return 0;
}

static int
RASPBERRY_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    return 0;
}

void
RASPBERRY_VideoQuit(_THIS)
{
}

static int
SDL_RASPBERRY_GetDisplayBounds (_THIS, SDL_VideoDisplay * display, SDL_Rect * rect)
{
    rect->x = 0;
    rect->y = 0;
    rect->w = fb_width;
    rect->h = fb_height;
    return 0;
}

#endif /* SDL_VIDEO_DRIVER_RASPBERRY */

/* vi: set ts=4 sw=4 expandtab: */
