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
#include <errno.h>

#include "kernel.h"
#include "wiring.h"

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

void main() {
    int x, y, c;
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

    if (mount("sd:") != 0) {
        addstrf("\r\nSD CARD NOT MOUNTED (%s)\r\n", strerror(errno));
    }

    usb_init();
    if (keyboard_init() != 0) {
        addstr("\r\nNO KEYBOARD DETECTED\r\n");
    }

    pinMode(16, OUTPUT);
    register_timer(&tw, 250000);

    addstr("\r\nREADY\r\n");

    while(1) {
        if ((c = getch()) != -1) {
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
                case '\r':
                    addch(c);
                    addch('\n');
                    break;
                default:
                    if (c < 0x7F) {
                        addch(c);
                    }
                    break;
            }
        }

        if (compare_timer(&tw)) {
            led_status = led_status == LOW ? HIGH : LOW;
            digitalWrite(16, led_status);
            toggle_cursor();
        }

        refresh();
    }
}
