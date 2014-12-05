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

/* RaspberryPi Baremetal SDL video driver implementation.
 *
 * Initial work by Marco Maccaferri (macca@maccasoft.com).
 */

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_rpivideo.h"
#include "SDL_rpievents_c.h"
#include "SDL_rpiframebuffer_c.h"

#include "../../core/raspberry/SDL_raspberry.h"

#ifdef HAVE_USPI
#include "uspi.h"

#define MAX_KEYS        6

static unsigned char keydown[MAX_KEYS] = { 0, 0, 0, 0, 0, 0 };
static unsigned char old_modifiers;
#endif // HAVE_USPI

#define RASPBERRYVID_DRIVER_NAME "raspberry"

/* Initialization/Query functions */
static int RASPBERRY_VideoInit(_THIS);
static int RASPBERRY_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
static void RASPBERRY_VideoQuit(_THIS);
static int RASPBERRY_GetDisplayBounds (_THIS, SDL_VideoDisplay * display, SDL_Rect * rect);

static unsigned int phys_width;
static unsigned int phys_height;

static int
RASPBERRY_CreateWindow(_THIS, SDL_Window * window)
{
    /* Adjust the window data to match the screen */
    window->x = 0;
    window->y = 0;
    if (window->w > 1 && window->h > 1)
    {
        SDL_DisplayMode mode;
        SDL_zero(mode);
        mode.format = SDL_PIXELFORMAT_ABGR8888;
        mode.w = phys_width = window->w;
        mode.h = phys_height = window->h;
        mode.refresh_rate = 60;
        mode.driverdata = NULL;
        SDL_AddDisplayMode(&_this->displays[0], &mode);
    }
    window->w = phys_width;
    window->h = phys_height;

    window->flags &= ~SDL_WINDOW_RESIZABLE;     /* window is NEVER resizeable */
    window->flags |= SDL_WINDOW_FULLSCREEN;     /* window is always fullscreen */
    window->flags &= ~SDL_WINDOW_HIDDEN;
    window->flags |= SDL_WINDOW_SHOWN;          /* only one window */
    window->flags |= SDL_WINDOW_INPUT_FOCUS;    /* always has input focus */

    /* One window, it always has focus */
    SDL_SetMouseFocus(window);
    SDL_SetKeyboardFocus(window);
}

/* driver bootstrap functions */

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

#ifdef HAVE_USPI
static void
RASPBERRY_USPiKeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]) {
    int i, index;
    unsigned char key;

    if ((old_modifiers & LSHIFT) != (ucModifiers & LSHIFT)) {
        SDL_SendKeyboardKey((ucModifiers & LSHIFT) != 0 ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_LSHIFT);
    }
    if ((old_modifiers & RSHIFT) != (ucModifiers & RSHIFT)) {
        SDL_SendKeyboardKey((ucModifiers & RSHIFT) != 0 ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_RSHIFT);
    }
    if ((old_modifiers & ALT) != (ucModifiers & ALT)) {
        SDL_SendKeyboardKey((ucModifiers & ALT) != 0 ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_LALT);
    }
    if ((old_modifiers & ALTGR) != (ucModifiers & ALTGR)) {
        SDL_SendKeyboardKey((ucModifiers & ALTGR) != 0 ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_RALT);
    }
    if ((old_modifiers & LCTRL) != (ucModifiers & LCTRL)) {
        SDL_SendKeyboardKey((ucModifiers & LCTRL) != 0 ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_LCTRL);
    }
    if ((old_modifiers & RCTRL) != (ucModifiers & RCTRL)) {
        SDL_SendKeyboardKey((ucModifiers & RCTRL) != 0 ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_RCTRL);
    }
    if ((old_modifiers & LWIN) != (ucModifiers & LWIN)) {
        SDL_SendKeyboardKey((ucModifiers & LWIN) != 0 ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_LGUI);
    }
    if ((old_modifiers & RWIN) != (ucModifiers & RWIN)) {
        SDL_SendKeyboardKey((ucModifiers & RWIN) != 0 ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_RGUI);
    }

    old_modifiers = ucModifiers;

    for (index = 0; index < MAX_KEYS; index++) {
        key = RawKeys[index];
        if (key < 4) { // Invalid key code
            continue;
        }
        for (i = 0; i < MAX_KEYS; i++) {
            if (key == keydown[i]) {
                break;
            }
        }
        if (i >= MAX_KEYS) {
            for (i = 0; i < MAX_KEYS; i++) {
                if (keydown[i] == 0) {
                    SDL_SendKeyboardKey(SDL_PRESSED, key);
                    keydown[i] = key;
                    break;
                }
            }
        }
    }

    for (i = 0; i < MAX_KEYS; i++) {
        key = keydown[i];
        if (key != 0) {
            for (index = 0; index < MAX_KEYS; index++) {
                if (RawKeys[index] == key)
                    break;
            }
            if (index >= MAX_KEYS) {
                SDL_SendKeyboardKey(SDL_RELEASED, key);
                keydown[i] = 0;
            }
        }
    }
}
#endif // HAVE_USPI

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
    device->CreateWindow = RASPBERRY_CreateWindow;
    device->CreateWindowFramebuffer = RASPBERRY_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = RASPBERRY_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = RASPBERRY_DestroyWindowFramebuffer;
    device->GetDisplayBounds = RASPBERRY_GetDisplayBounds;

    device->free = RASPBERRY_DeleteDevice;

#ifdef HAVE_USPI
    USPiInitialize ();
    if (USPiKeyboardAvailable()) {
        USPiKeyboardRegisterKeyStatusHandlerRaw (RASPBERRY_USPiKeyStatusHandlerRaw);
    }
#endif

    return device;
}

VideoBootStrap RASPBERRY_bootstrap = {
    RASPBERRYVID_DRIVER_NAME, "SDL Raspberry Pi video driver",
    RASPBERRY_Available, RASPBERRY_CreateDevice
};

int
RASPBERRY_VideoInit(_THIS)
{
    unsigned int mb_addr = 0x40007000;      // 0x7000 in L2 cache coherent mode
    volatile unsigned int *mailbuffer = (unsigned int *) mb_addr;
    SDL_DisplayMode mode;

    mailbuffer[0] = 8 * 4;             // size of this message
    mailbuffer[1] = 0;                  // this is a request
    mailbuffer[2] = TAG_GET_PHYS_WH;    // get physical width/height tag
    mailbuffer[3] = 8;                  // value buffer size
    mailbuffer[4] = 0;                  // request/response
    mailbuffer[5] = 0;                  // space to return width
    mailbuffer[6] = 0;                  // space to return height
    mailbuffer[7] = 0;
    Raspberry_MailboxWrite(MAIL_TAGS, mb_addr);

    Raspberry_MailboxRead(MAIL_TAGS);
    if (mailbuffer[1] != MAIL_FULL) {
        return SDL_SetError("Can't get physiscal video size");
    }

    phys_width = mailbuffer[5];
    phys_height = mailbuffer[6];

    SDL_zero(mode);
    mode.format = SDL_PIXELFORMAT_ABGR8888;
    mode.w = phys_width;
    mode.h = phys_height;
    mode.refresh_rate = 60;
    mode.driverdata = NULL;

    if (SDL_AddBasicVideoDisplay(&mode) < 0) {
        return -1;
    }

    SDL_AddDisplayMode(&_this->displays[0], &mode);

    return 0;
}

static int
RASPBERRY_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    phys_width = mode->w;
    phys_height = mode->h;
    return 0;
}

void
RASPBERRY_VideoQuit(_THIS)
{
}

static int
RASPBERRY_GetDisplayBounds (_THIS, SDL_VideoDisplay * display, SDL_Rect * rect)
{
    rect->x = 0;
    rect->y = 0;
    rect->w = phys_width;
    rect->h = phys_height;
    return 0;
}

#endif /* SDL_VIDEO_DRIVER_RASPBERRY */

/* vi: set ts=4 sw=4 expandtab: */
