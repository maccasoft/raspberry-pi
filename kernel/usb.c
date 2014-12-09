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

#include <ctype.h>

#ifdef HAVE_USPI
#include "uspi.h"
#endif

#define SWAP_BYTES(b)   (((b & 0xFF) << 8) | ((b & 0xFF00) >> 8))

#define MAX_KEYS        6
#define MAX_KEYBUFFER  16

#define KEY_MAX_CODE    105

#define KEY_NORMAL      0
#define KEY_SHIFT       1
#define KEY_ALTGR       2
#define KEY_SHIFT_ALTGR 3

static int readIndex;
static int writeIndex;
static int keybufferAvailable;
static unsigned short keyBuffer[MAX_KEYBUFFER];

unsigned char kbd_status[KEY_MAX_CODE + 1];

static unsigned char keyMap[KEY_MAX_CODE + 1][KEY_SHIFT_ALTGR + 1] = {
    { 0x00, 0x00, 0x00, 0x00 }, // 00 (0)
    { 0x00, 0x00, 0x00, 0x00 }, // 01 (1)
    { 0x00, 0x00, 0x00, 0x00 }, // 02 (2)
    { 0x00, 0x00, 0x00, 0x00 }, // 03 (3)
    {  'a',  'A', 0x00, 0x00 }, // 04 (4)
    {  'b',  'B', 0x00, 0x00 }, // 05 (5)
    {  'c',  'C', 0x00, 0x00 }, // 06 (6)
    {  'd',  'D', 0x00, 0x00 }, // 07 (7)
    {  'e',  'E', 0x00, 0x00 }, // 08 (8)
    {  'f',  'F', 0x00, 0x00 }, // 09 (9)
    {  'g',  'G', 0x00, 0x00 }, // 0a (10)
    {  'h',  'H', 0x00, 0x00 }, // 0b (11)
    {  'i',  'I', 0x00, 0x00 }, // 0c (12)
    {  'j',  'J', 0x00, 0x00 }, // 0d (13)
    {  'k',  'K', 0x00, 0x00 }, // 0e (14)
    {  'l',  'L', 0x00, 0x00 }, // 0f (15)
    {  'm',  'M', 0x00, 0x00 }, // 10 (16)
    {  'n',  'N', 0x00, 0x00 }, // 11 (17)
    {  'o',  'O', 0x00, 0x00 }, // 12 (18)
    {  'p',  'P', 0x00, 0x00 }, // 13 (19)
    {  'q',  'Q', 0x00, 0x00 }, // 14 (20)
    {  'r',  'R', 0x00, 0x00 }, // 15 (21)
    {  's',  'S', 0x00, 0x00 }, // 16 (22)
    {  't',  'T', 0x00, 0x00 }, // 17 (23)
    {  'u',  'U', 0x00, 0x00 }, // 18 (24)
    {  'v',  'V', 0x00, 0x00 }, // 19 (25)
    {  'w',  'W', 0x00, 0x00 }, // 1a (26)
    {  'x',  'X', 0x00, 0x00 }, // 1b (27)
    {  'y',  'Y', 0x00, 0x00 }, // 1c (28)
    {  'z',  'Z', 0x00, 0x00 }, // 1d (29)
    {  '1',  '!', 0x00, 0x00 }, // 1e (30)
    {  '2',  '"', 0x00, 0x00 }, // 1f (31)
    {  '3', 0x00, 0x00, 0x00 }, // 20 (32)
    {  '4',  '$', 0x00, 0x00 }, // 21 (33)
    {  '5',  '%', 0x00, 0x00 }, // 22 (34)
    {  '6',  '&', 0x00, 0x00 }, // 23 (35)
    {  '7',  '/', 0x00, 0x00 }, // 24 (36)
    {  '8',  '(', 0x00, 0x00 }, // 25 (37)
    {  '9',  ')', 0x00, 0x00 }, // 26 (38)
    {  '0',  '=', 0x00, 0x00 }, // 27 (39)
    { 0x0d, 0x0d, 0x00, 0x00 }, // 28 (40)
    { 0x1a, 0x1a, 0x1a, 0x00 }, // 29 (41)
    { 0x08, 0x08, 0x00, 0x00 }, // 2a (42)
    { 0x09, 0x09, 0x00, 0x00 }, // 2b (43)
    {  ' ',  ' ', 0x00, 0x00 }, // 2c (44)
    {  '\'',  '?',  '`', 0x00 }, // 2d (45)
    { 0x00,  '^',  '~', 0x00 }, // 2e (46)
    { 0x00, 0x00,  '[',  '{' }, // 2f (47)
    {  '+',  '*',  ']',  '}' }, // 30 (48)
    {  '<',  '>', 0x00, 0x00 }, // 31 (49)
    { 0x00, 0x00, 0x00, 0x00 }, // 32 (50)
    { 0x00, 0x00,  '@', 0x00 }, // 33 (51)
    { 0x00, 0x00,  '#', 0x00 }, // 34 (52)
    {  '\\',  '|', 0x00, 0x00 }, // 35 (53)
    {  ',',  ';', 0x00, 0x00 }, // 36 (54)
    {  '.',  ':', 0x00, 0x00 }, // 37 (55)
    {  '-',  '_', 0x00, 0x00 }, // 38 (56)
    { 0x00, 0x00, 0x00, 0x00 }, // 39 (57)
    { 0x00, 0x00, 0x00, 0x00 }, // 3a (58)
    { 0x00, 0x00, 0x00, 0x00 }, // 3b (59)
    { 0x00, 0x00, 0x00, 0x00 }, // 3c (60)
    { 0x00, 0x00, 0x00, 0x00 }, // 3d (61)
    { 0x00, 0x00, 0x00, 0x00 }, // 3e (62)
    { 0x00, 0x00, 0x00, 0x00 }, // 3f (63)
    { 0x00, 0x00, 0x00, 0x00 }, // 40 (64)
    { 0x00, 0x00, 0x00, 0x00 }, // 41 (65)
    { 0x00, 0x00, 0x00, 0x00 }, // 42 (66)
    { 0x00, 0x00, 0x00, 0x00 }, // 43 (67)
    { 0x00, 0x00, 0x00, 0x00 }, // 44 (68)
    { 0x00, 0x00, 0x00, 0x00 }, // 45 (69)
    { 0x00, 0x00, 0x00, 0x00 }, // 46 (70)
    { 0x00, 0x00, 0x00, 0x00 }, // 47 (71)
    { 0x00, 0x00, 0x00, 0x00 }, // 48 (72)
    { 0x00, 0x00, 0x00, 0x00 }, // 49 (73)
    { 0x00, 0x00, 0x00, 0x00 }, // 4a (74)
    { 0x00, 0x00, 0x00, 0x00 }, // 4b (75)
    { 0x00, 0x00, 0x00, 0x00 }, // 4c (76)
    { 0x00, 0x00, 0x00, 0x00 }, // 4d (77)
    { 0x00, 0x00, 0x00, 0x00 }, // 4e (78)
    { 0x00, 0x00, 0x00, 0x00 }, // 4f (79)
    { 0x00, 0x00, 0x00, 0x00 }, // 50 (80)
    { 0x00, 0x00, 0x00, 0x00 }, // 51 (81)
    { 0x00, 0x00, 0x00, 0x00 }, // 52 (82)
    { 0x00, 0x00, 0x00, 0x00 }, // 53 (83)
    {  '/',  '/', 0x00, 0x00 }, // 54 (84)
    {  '*',  '*', 0x00, 0x00 }, // 55 (85)
    {  '-',  '-', 0x00, 0x00 }, // 56 (86)
    {  '+',  '+', 0x00, 0x00 }, // 57 (87)
    { 0x0d, 0x0d, 0x00, 0x00 }, // 58 (88)
    {  '1',  '1', 0x00, 0x00 }, // 59 (89)
    {  '2',  '2', 0x00, 0x00 }, // 5a (90)
    {  '3',  '3', 0x00, 0x00 }, // 5b (91)
    {  '4',  '4', 0x00, 0x00 }, // 5c (92)
    {  '5',  '5', 0x00, 0x00 }, // 5d (93)
    {  '6',  '6', 0x00, 0x00 }, // 5e (94)
    {  '7',  '7', 0x00, 0x00 }, // 5f (95)
    {  '8',  '8', 0x00, 0x00 }, // 60 (96)
    {  '9',  '9', 0x00, 0x00 }, // 61 (97)
    {  '0',  '0', 0x00, 0x00 }, // 62 (98)
    {  '.',  '.', 0x00, 0x00 }, // 63 (99)
    {  '<',  '>', 0x00, 0x00 }, // 64 (100)
    { 0x00, 0x00, 0x00, 0x00 }, // 65 (101)
    { 0x00, 0x00, 0x00, 0x00 }, // 66 (102)
    {  '=',  '=', 0x00, 0x00 }, // 67 (103)
    { 0x00, 0x00, 0x00, 0x00 }, // 68 (104)
};

int getch() {
    if (keybufferAvailable <= 0)
        return -1;

    int key = keyBuffer[readIndex++];
    if (readIndex >= MAX_KEYBUFFER)
        readIndex = 0;

    keybufferAvailable--;

    return key;
}

#ifdef HAVE_USPI

static void put_key(unsigned short key) {
    if (keybufferAvailable >= MAX_KEYBUFFER)
        return;

    keyBuffer[writeIndex++] = key;
    if (writeIndex >= MAX_KEYBUFFER)
        writeIndex = 0;

    keybufferAvailable++;
}

static void USPiKeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]) {
    static int keydown[MAX_KEYS] = { 0, 0, 0, 0, 0, 0 };
    static int capsLock = 1;
    int i, index, c;
    short key;

    for (index = 0; index < MAX_KEYS; index++) {
        key = RawKeys[index];
        if (key < 4 || key > KEY_MAX_CODE) { // Invalid key code
            continue;
        }
        for (i = 0; i < MAX_KEYS; i++) {
            if (key == keydown[i]) {
                break;
            }
        }
        if (i >= MAX_KEYS) {
            for (int i = 0; i < MAX_KEYS; i++) {
                if (keydown[i] == 0) {
                    if (key == 57) {
                        capsLock = capsLock ? 0 : 1;
                    }
                    else if ((ucModifiers & (LSHIFT | RSHIFT)) != 0 && (ucModifiers & ALTGR) != 0) {
                        c = keyMap[key][KEY_SHIFT_ALTGR];
                        put_key(c != 0 ? c : SWAP_BYTES(key));
                    }
                    else if ((ucModifiers & ALTGR) != 0) {
                        c = keyMap[key][KEY_ALTGR];
                        put_key(c != 0 ? c : SWAP_BYTES(key));
                    }
                    else if ((ucModifiers & (LSHIFT | RSHIFT)) != 0) {
                        c = keyMap[key][KEY_SHIFT];
                        if (capsLock) {
                            c = tolower(c);
                        }
                        put_key(c != 0 ? c : SWAP_BYTES(key));
                    }
                    else {
                        c = keyMap[key][KEY_NORMAL];
                        if (c >= 'a' && c <= 'z' && (ucModifiers & (LCTRL | RCTRL)) != 0) {
                            c = (c - 'a') + 1;
                        }
                        else if (capsLock) {
                            c = toupper(c);
                        }
                        put_key(c != 0 ? c : SWAP_BYTES(key));
                    }
                    keydown[i] = key;
                    kbd_status[key] = 1;
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
                keydown[i] = 0;
                kbd_status[key] = 0;
            }
        }
    }
}

#endif // HAVE_USPI

int usb_init() {
#ifdef HAVE_USPI
    if (USPiInitialize()) {
        return 0;
    }
    return -1;
#else
    return 0;
#endif
}

int keyboard_init() {
#ifdef HAVE_USPI
    if (USPiKeyboardAvailable()) {
        USPiKeyboardRegisterKeyStatusHandlerRaw (USPiKeyStatusHandlerRaw);
        return 0;
    }
#endif
    return -1;
}
