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

#if defined(SDL_JOYSTICK_RASPBERRY)

#include "SDL_joystick.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"

#include "SDL_events.h"

#ifdef HAVE_NES
#include "../../core/raspberry/SDL_raspberry.h"

#define JOY_CLK         24
#define JOY_LCH         23
#define JOY_DATAOUT0    25
#define JOY_DATAOUT1    22

#define NES0_RIGHT      0x0001
#define NES0_LEFT       0x0002
#define NES0_DOWN       0x0004
#define NES0_UP         0x0008
#define NES0_START      0x0010
#define NES0_SELECT     0x0020
#define NES0_B          0x0040
#define NES0_A          0x0080

#define NES1_RIGHT      0x0100
#define NES1_LEFT       0x0200
#define NES1_DOWN       0x0400
#define NES1_UP         0x0800
#define NES1_START      0x1000
#define NES1_SELECT     0x2000
#define NES1_B          0x4000
#define NES1_A          0x8000

uint32_t old_nes_bits;

#endif // HAVE_NES

/* Function to scan the system for joysticks.
 * It should return 0, or -1 on an unrecoverable fatal error.
 */
int
SDL_SYS_JoystickInit(void)
{
    return (0);
}

int SDL_SYS_NumJoysticks()
{
    int num = 0;

#ifdef HAVE_NES
    num += 1;
#endif // HAVE_NES

    return num;
}

void SDL_SYS_JoystickDetect()
{
}

SDL_bool SDL_SYS_JoystickNeedsPolling()
{
    return SDL_TRUE;
}

/* Function to get the device-dependent name of a joystick */
const char *
SDL_SYS_JoystickNameForDeviceIndex(int device_index)
{
    int num = 0;

#ifdef HAVE_NES
    if (device_index == num) {
        return "NES Gamepad";
    }
    num++;
#endif // HAVE_NES

    SDL_SetError("Logic error: No joysticks available");
    return (NULL);
}

/* Function to perform the mapping from device index to the instance id for this index */
SDL_JoystickID SDL_SYS_GetInstanceIdOfDeviceIndex(int device_index)
{
    return device_index;
}

/* Function to open a joystick for use.
   The joystick to open is specified by the index field of the joystick.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
int
SDL_SYS_JoystickOpen(SDL_Joystick * joystick, int device_index)
{
    int num = 0;

#ifdef HAVE_NES
    if (device_index == num) {
        /* set I/Os pins */
        Raspberry_pinMode(JOY_CLK, 0);
        Raspberry_pinMode(JOY_LCH, 0);
        Raspberry_pinMode(JOY_DATAOUT0, 1);
        Raspberry_pinMode(JOY_DATAOUT1, 1);

        joystick->nbuttons = 8;
        joystick->naxes = 0;
        joystick->nhats = 0;

        old_nes_bits = 0xFF;

        return 0;
    }
    num++;
#endif // HAVE_NES

    return SDL_SetError("Logic error: No joysticks available");
}

/* Function to determine is this joystick is attached to the system right now */
SDL_bool SDL_SYS_JoystickAttached(SDL_Joystick *joystick)
{
    return SDL_TRUE;
}

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
void
SDL_SYS_JoystickUpdate(SDL_Joystick * joystick)
{
#ifdef HAVE_NES
    uint32_t nes_bits;

#ifdef HAVE_NES_SN74165 /* Experimental controller with SN74165 shift register */

    /* set clock and latch to 1 */
    Raspberry_digitalWrite(JOY_LCH, 1);
    Raspberry_digitalWrite(JOY_CLK, 1);
    usleep(100);

    /* set latch to 0 */
    Raspberry_digitalWrite(JOY_LCH, 0);
    usleep(100);

    /* set latch to 1 and clock to 0 */
    Raspberry_digitalWrite(JOY_LCH, 1);
    Raspberry_digitalWrite(JOY_CLK, 0);
    usleep(100);

#else

    /* set clock and latch to 0 */
    Raspberry_digitalWrite(JOY_LCH, 0);
    Raspberry_digitalWrite(JOY_CLK, 0);
    usleep(100);

    /* set latch to 1 */
    Raspberry_digitalWrite(JOY_LCH, 1);
    usleep(100);

    /* set latch to 0 */
    Raspberry_digitalWrite(JOY_LCH, 0);
    usleep(100);

#endif // HAVE_NES_SN74165

    /* data is now ready to shift out, clear storage */
    nes_bits = 0;

    /* read 8 bits, 1st bits are already latched and ready, simply save and clock remaining bits */
    for (int i = 0; i < 8; i++) {
        nes_bits <<= 1;
        if (Raspberry_digitalRead(JOY_DATAOUT0))
            nes_bits |= 0x00000001;
        if (Raspberry_digitalRead(JOY_DATAOUT1))
            nes_bits |= 0x00000100;

        Raspberry_digitalWrite(JOY_CLK, 1);
        usleep(100);
        Raspberry_digitalWrite(JOY_CLK, 0);
        usleep(100);
    }

    uint32_t mask = 1;
    for (int i = 0; i < 8; i++, mask <<= 1) {
        if ((nes_bits & mask) != (old_nes_bits & mask)) {
            SDL_PrivateJoystickButton(joystick, i, !(nes_bits & mask) ? SDL_PRESSED : SDL_RELEASED);
        }
    }

    old_nes_bits = nes_bits;

#endif // HAVE_NES
    return;
}

/* Function to close a joystick after use */
void
SDL_SYS_JoystickClose(SDL_Joystick * joystick)
{
    return;
}

/* Function to perform any system-specific joystick related cleanup */
void
SDL_SYS_JoystickQuit(void)
{
    return;
}

SDL_JoystickGUID SDL_SYS_JoystickGetDeviceGUID( int device_index )
{
    SDL_JoystickGUID guid;
    /* the GUID is just the first 16 chars of the name for now */
    const char *name = SDL_SYS_JoystickNameForDeviceIndex( device_index );
    SDL_zero( guid );
    SDL_memcpy( &guid, name, SDL_min( sizeof(guid), SDL_strlen( name ) ) );
    return guid;
}


SDL_JoystickGUID SDL_SYS_JoystickGetGUID(SDL_Joystick * joystick)
{
    SDL_JoystickGUID guid;
    /* the GUID is just the first 16 chars of the name for now */
    const char *name = joystick->name;
    SDL_zero( guid );
    SDL_memcpy( &guid, name, SDL_min( sizeof(guid), SDL_strlen( name ) ) );
    return guid;
}

#endif /* SDL_JOYSTICK_RASPBERRY */

/* vi: set ts=4 sw=4 expandtab: */
