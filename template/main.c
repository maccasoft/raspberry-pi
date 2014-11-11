/*
 * Copyright (c) 2014 Marco Maccaferri and Others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CSUD
#include <usbd/usbd.h>
#include <device/hid/keyboard.h>
#endif

#ifdef HAVE_USPI
#include <uspi.h>
#endif

#include "platform.h"
#include "wiring.h"
#include "console.h"

#if BYTES_PER_PIXEL == 2

#define BORDER_COLOR        RGB(26, 5, 10)
#define BACKGROUND_COLOR    RGB(12, 0, 4)

#elif BYTES_PER_PIXEL == 4

#define BORDER_COLOR        RGB(213, 41, 82)
#define BACKGROUND_COLOR    RGB(98, 0, 32)

#endif

#if defined(__cplusplus)
extern "C" {
#endif

__attribute__ ((interrupt ("IRQ"))) void interrupt_irq() {
#ifdef HAVE_USPI
    USPiInterruptHandler ();
#endif
}

#if defined(__cplusplus)
}
#endif

#define KEY_ESC     0x2900

#define KEY_F1      0x3A00
#define KEY_F2      0x3B00
#define KEY_F3      0x3C00
#define KEY_F4      0x3D00
#define KEY_F5      0x3E00
#define KEY_F6      0x3F00
#define KEY_F7      0x4000
#define KEY_F8      0x4100
#define KEY_F9      0x4200
#define KEY_F10     0x4300
#define KEY_F11     0x4400
#define KEY_F12     0x4500

#define KEY_CANC    0x4C00
#define KEY_INS     0x4900
#define KEY_PGDN    0x4E00
#define KEY_PGUP    0x4B00
#define KEY_END     0x4D00
#define KEY_HOME    0x4A00
#define KEY_RIGHT   0x4F00
#define KEY_LEFT    0x5000
#define KEY_DOWN    0x5100
#define KEY_UP      0x5200

#if defined(HAVE_CSUD) || defined(HAVE_USPI)

#define SWAP_BYTES(b)   (((b & 0xFF) << 8) | ((b & 0xFF00) >> 8))

static unsigned char keyNormal_it[] = {
    0x0, 0x0, 0x0, 0x0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
    'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '0', '\r', 0x0, '\b', '\t', ' ', '\'', 0x0, 0x0,
    '+', '<', 0x0, 0x0, 0x0, '\\', ',', '.', '-', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+', '\r', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '0', '.', '<', 0x0, 0x0, '='
};

static unsigned char keyShift_it[] = {
    0x0, 0x0, 0x0, 0x0, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '"',
    0x0, '$', '%', '&', '/', '(', ')', '=', '\r', 0x0, '\b', '\t', ' ', '?', '^', 0x0,
    '*', '>', 0x0, 0x0, 0x0, '|', ';', ':', '_', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+', '\r', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '0', '.', '>', 0x0, 0x0, '='
};

#define MAX_KEYS        6

#endif

#ifdef HAVE_CSUD

int kbd_getchar() {
    static int keydown[MAX_KEYS] = { 0, 0, 0, 0, 0, 0 };

    u32 kbdCount = KeyboardCount();
    if (kbdCount == 0) {
        for (int i = 0; i < MAX_KEYS; i++) {
            keydown[i] = 0;
        }
        return -1;
    }

    u32 kbdAddress = KeyboardGetAddress(0);
    if (kbdAddress == 0) {
        for (int i = 0; i < MAX_KEYS; i++) {
            keydown[i] = 0;
        }
        return -1;
    }

    struct KeyboardModifiers modifiers = KeyboardGetModifiers(kbdAddress);

    u32 count = KeyboardGetKeyDownCount(kbdAddress);
    for (int index = 0; index < count && index < MAX_KEYS; index++) {
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
                    keydown[i] = key;
                    if (modifiers.LeftShift || modifiers.RightShift) {
                        if (key >= sizeof(keyShift_it)) {
                            return key;
                        }
                        return keyShift_it[key] != 0 ? keyShift_it[key] : SWAP_BYTES(key);
                    }
                    else {
                        if (key >= sizeof(keyNormal_it)) {
                            return key;
                        }
                        return keyNormal_it[key] != 0 ? keyNormal_it[key] : SWAP_BYTES(key);
                    }
                    return -1;
                }
            }
        }
    }

    KeyboardPoll(kbdAddress);

    for (int i = 0; i < MAX_KEYS; i++) {
        u16 key = keydown[i];
        if (key != 0) {
            if (!KeyboadGetKeyIsDown(kbdAddress, key)) {
                keydown[i] = 0;
            }
        }
    }

    return -1;
}

#endif // HAVE_CSUD

#ifdef HAVE_USPI

void LogWrite (const char *pSource, unsigned Severity, const char *pMessage, ...)
{
    // Do nothing
}

void uspi_assertion_failed (const char *pExpr, const char *pFile, unsigned nLine)
{
    char temp[256];

    sprintf(temp, "\r\nUSPi assertion failed: %s at %s:%d\r\n\r\n", pExpr, pFile, nLine);
    addstr(temp);

    while(1);
}

void DebugHexdump (const void *pBuffer, unsigned nBufLen, const char *pSource /* = 0 */)
{
    // Do nothing
}

#define MAX_KEYBUFFER  16

static int readIndex;
static int writeIndex;
static int keybufferAvailable;
static unsigned short keyBuffer[MAX_KEYBUFFER];

int kbd_getchar() {
    if (keybufferAvailable <= 0)
        return -1;

    int key = keyBuffer[readIndex++];
    if (readIndex >= MAX_KEYBUFFER)
        readIndex = 0;

    keybufferAvailable--;

    return key;
}

static void put_key(unsigned short key) {
    if (keybufferAvailable >= MAX_KEYBUFFER)
        return;

    keyBuffer[writeIndex++] = key;
    if (writeIndex >= MAX_KEYBUFFER)
        writeIndex = 0;

    keybufferAvailable++;
}

static void USPiKeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char *pRawKeys) {
    static int keydown[MAX_KEYS] = { 0, 0, 0, 0, 0, 0 };
    const unsigned char *ptr;

    ptr = pRawKeys;
    while (*ptr) {
        short key = *ptr++;
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
                    if ((ucModifiers & (LSHIFT | RSHIFT)) != 0) {
                        if (key >= sizeof(keyShift_it)) {
                            put_key(key);
                        }
                        put_key(keyShift_it[key] != 0 ? keyShift_it[key] : SWAP_BYTES(key));
                    }
                    else {
                        if (key >= sizeof(keyNormal_it)) {
                            put_key(key);
                        }
                        put_key(keyNormal_it[key] != 0 ? keyNormal_it[key] : SWAP_BYTES(key));
                    }
                    break;
                }
            }
        }
    }

    for (int i = 0; i < MAX_KEYS; i++) {
        short key = keydown[i];
        if (key != 0) {
            ptr = pRawKeys;
            while (*ptr) {
                if (*ptr == key)
                    break;
                ptr++;
            }
            if (*ptr == '\0') {
                keydown[i] = 0;
            }
        }
    }
}

#endif // HAVE_USPI

void main() {
    int x, y;
    struct timer_wait tw;
    int led_status = LOW;

    // Default screen resolution (set in config.txt or auto-detected)
    //fb_init(0, 0);

    // Sets a specific screen resolution
    fb_init(32 + 320 + 32, 32 + 200 + 32);

    fb_fill_rectangle(0, 0, fb_width - 1, fb_height - 1, BORDER_COLOR);

    initscr(40, 25);
    cur_fore = WHITE;
    cur_back = BACKGROUND_COLOR;
    clear();

    mvaddstr(1, 9, "**** RASPBERRY-PI ****");
    mvaddstr(3, 7, "BARE-METAL SYSTEM TEMPLATE\r\n");

#ifdef HAVE_CSUD
    UsbInitialise();
    if (KeyboardCount() == 0)
        addstr("\r\nNO KEYBOARD DETECTED\r\n");
#endif

#ifdef HAVE_USPI
    USPiInitialize ();
    if (USPiKeyboardAvailable()) {
        USPiKeyboardRegisterKeyStatusHandlerRaw (USPiKeyStatusHandlerRaw);
    }
    else
        addstr("\r\nNO KEYBOARD DETECTED\r\n");
#endif

    pinMode(16, OUTPUT);
    register_timer(&tw, 250000);

    addstr("\r\nREADY\r\n");

    while(1) {
        int c = kbd_getchar();
        if (c != -1) {
            switch(c) {
                case KEY_UP:
                    getyx(y, x);
                    move(y - 1, x);
                    break;
                case KEY_DOWN:
                    getyx(y, x);
                    move(y + 1, x);
                    break;
                case KEY_LEFT:
                    getyx(y, x);
                    x--;
                    if (x < 0) {
                        x = 39;
                        y--;
                    }
                    move(y, x);
                    break;
                case KEY_RIGHT:
                    getyx(y, x);
                    x++;
                    if (x >= 40) {
                        x = 0;
                        y++;
                    }
                    move(y, x);
                    break;
                case KEY_HOME:
                    getyx(y, x);
                    move(y, 0);
                    break;
                case KEY_PGUP:
                    move(0, 0);
                    break;
                default:
                    if (c > 0 && c < 0x7F) {
                        addch(c);
                        if (c == '\r')
                            addch('\n');
                    }
                    break;
            }
        }

        if (compare_timer(&tw)) {
            led_status = led_status == LOW ? HIGH : LOW;
            digitalWrite(16, led_status);
            toggle_cursor();
        }

        fb_flip();
    }
}
