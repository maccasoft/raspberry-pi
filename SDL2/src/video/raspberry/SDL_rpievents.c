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

/* Being a null driver, there's no event stream. We just define stubs for
   most of the API. */

#include "../../events/SDL_events_c.h"

#include "SDL_rpivideo.h"
#include "SDL_rpievents_c.h"

#ifdef HAVE_CSUD
#include "usbd/usbd.h"
#include "device/hid/keyboard.h"

#define MAX_KEYS        6

static int keydown[MAX_KEYS] = { 0, 0, 0, 0, 0, 0 };
static struct KeyboardModifiers modifiers;
static struct KeyboardModifiers old_modifiers;
#endif // HAVE_CSUD

void
RASPBERRY_PumpEvents(_THIS)
{
#ifdef HAVE_CSUD
    u32 kbdCount = KeyboardCount();
    if (kbdCount == 0) {
        for (int i = 0; i < MAX_KEYS; i++) {
            keydown[i] = 0;
        }
        return;
    }

    u32 kbdAddress = KeyboardGetAddress(0);
    if (kbdAddress == 0) {
        for (int i = 0; i < MAX_KEYS; i++) {
            keydown[i] = 0;
        }
        return;
    }

    KeyboardPoll(kbdAddress);

    modifiers = KeyboardGetModifiers(kbdAddress);
    if (modifiers.LeftShift != old_modifiers.LeftShift) {
        SDL_SendKeyboardKey(modifiers.LeftShift ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_LSHIFT);
    }
    if (modifiers.RightShift != old_modifiers.RightShift) {
        SDL_SendKeyboardKey(modifiers.RightShift ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_RSHIFT);
    }
    if (modifiers.LeftAlt != old_modifiers.LeftAlt) {
        SDL_SendKeyboardKey(modifiers.LeftAlt ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_LALT);
    }
    if (modifiers.RightAlt != old_modifiers.RightAlt) {
        SDL_SendKeyboardKey(modifiers.RightAlt ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_RALT);
    }
    if (modifiers.LeftControl != old_modifiers.LeftControl) {
        SDL_SendKeyboardKey(modifiers.LeftControl ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_LCTRL);
    }
    if (modifiers.RightControl != old_modifiers.RightControl) {
        SDL_SendKeyboardKey(modifiers.RightControl ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_RCTRL);
    }
    if (modifiers.LeftGui != old_modifiers.LeftGui) {
        SDL_SendKeyboardKey(modifiers.LeftGui ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_LGUI);
    }
    if (modifiers.RightGui != old_modifiers.RightGui) {
        SDL_SendKeyboardKey(modifiers.RightGui ? SDL_PRESSED : SDL_RELEASED, SDL_SCANCODE_RGUI);
    }
    old_modifiers = modifiers;

    int keydownCount = KeyboardGetKeyDownCount(kbdAddress);
    for (int index = 0; index < keydownCount; index++) {
        u16 key = KeyboardGetKeyDown(kbdAddress, index);
        for (int i = 0; i < MAX_KEYS; i++) {
            if (key == keydown[i]) {
                key = 0;
                break;
            }
        }
        if (key != 0) {
            for (int i = 0; i < MAX_KEYS; i++) {
                if (keydown[i] == 0) {
                    SDL_SendKeyboardKey(SDL_PRESSED, key);
                    keydown[i] = key;
                    break;
                }
            }
        }
    }

    for (int index = 0; index < MAX_KEYS; index++) {
        u16 key = keydown[index];
        if (key != 0) {
            if (!KeyboadGetKeyIsDown(kbdAddress, key)) {
                SDL_SendKeyboardKey(SDL_RELEASED, key);
                keydown[index] = 0;
            }
        }
    }
#endif // HAVE_CSUD
}

#endif /* SDL_VIDEO_DRIVER_RASPBERRY */

/* vi: set ts=4 sw=4 expandtab: */
